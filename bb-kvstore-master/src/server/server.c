#include "server.h"
#include "client-state.h"

#include <config.h>

#if defined(HAVE_EPOLL_CREATE) && defined(HAVE_SYS_EPOLL_H)
#include "events_epoll.h"
#elif defined(HAVE_KQUEUE) && defined(HAVE_SYS_EVENT_H)
#include "events_kqueue.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

struct _Server {
	int server_socketfd;
	request_handler_fn make_response;
	void* p_data;
	Logger* logger;

	int eventfd;
	void* events;
	size_t max_events;
};

static int create_server_socket_and_bind(Server* self, const char *port);
static int make_socket_non_blocking(Server* self, int socketfd);
static int handle_read_write_errors(Server* self, ssize_t read_return_value, int* done);

static void setup_server_socket(Server* self);
static void process_event(Server* self, size_t event_index);
static void handle_server_errors(const Server* self, size_t event_index);
static int handle_client_close(const Server* self, ClientState* client, size_t event_index);
static int is_server_event(const Server* self, size_t event_index);
static void accept_new_connections(Server* self);
static void handle_client(Server* self, ClientState* client);
static void handle_read(Server* self, ClientState* client);
static void handle_write(Server* self, ClientState* client);
static void set_read_state(Server* self, ClientState* client);
static void set_write_state(Server* self, ClientState* client);


Server*
server_new(const char* port, request_handler_fn request_handler, void* p_data, Logger* logger)
{
	Server* self = (Server *) calloc(1, sizeof(Server));

	self->make_response = request_handler;
	self->p_data = p_data;
	self->logger = logger;

	self->eventfd = events_create_context();
	if (self->eventfd == -1) {
		logger_write(self->logger, LOGLEVEL_ERROR, "events_create_context failed");
		abort();
	}

	self->server_socketfd = create_server_socket_and_bind(self, port);
	setup_server_socket(self);

	self->max_events = 64;
	self->events = events_create_events_array(self->max_events);

	if (listen(self->server_socketfd, SOMAXCONN) == -1) {
		logger_write(self->logger, LOGLEVEL_ERROR, "listen failed");
		abort();
	}

	events_register_socket(self->eventfd, self->server_socketfd);
	events_set_read_state(self->eventfd, self->server_socketfd, &self->server_socketfd);

	return self;
}

void
server_free(Server* self)
{
	events_destroy_events_array(self->events);
	close(self->server_socketfd);
	close(self->eventfd);

	free(self);
}

void
server_set_request_handler_fn(Server* self, request_handler_fn request_handler)
{
	self->make_response = request_handler;
}

void
server_loop_run(Server* self)
{
	logger_write(self->logger, LOGLEVEL_INFO, "server started");

	while (1) {
		size_t events_size = events_wait(self->eventfd, self->events, self->max_events, -1);

		// for all events
		size_t event_index;
		for (event_index = 0; event_index < events_size; ++event_index) {
			process_event(self, event_index);
		}
	}
}


static int
create_server_socket_and_bind(Server* self, const char *port)
{
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int server_socketfd;

	memset (&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;     /* IPv4 and IPv6 */
	hints.ai_socktype = SOCK_STREAM; /* TCP socket */
	hints.ai_flags = AI_PASSIVE;     /* All interfaces */

	int err;
	if ((err = getaddrinfo(0, port, &hints, &result)) != 0) {
		logger_write(self->logger, LOGLEVEL_ERROR, "getaddrinfo failed");
		abort();
	}

	for (rp = result; rp != 0; rp = rp->ai_next) {
		server_socketfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (server_socketfd == -1) continue;

		int reuse = 1;
		if (setsockopt(server_socketfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
			logger_write(self->logger, LOGLEVEL_WARNING, "setsockopt SO_REUSEADDR failed");

		if (bind(server_socketfd, rp->ai_addr, rp->ai_addrlen) == 0) break; // ok

		close (server_socketfd);
	}

	if (rp == 0) {
		logger_write(self->logger, LOGLEVEL_ERROR, "server could not bind");
		abort();
	}

	freeaddrinfo(result);

	return server_socketfd;
}

static int
make_socket_non_blocking(Server* self, int socketfd)
{
	int flags;

	flags = fcntl(socketfd, F_GETFL, 0);
	if (flags == -1) {
		logger_write(self->logger, LOGLEVEL_ERROR, "fcntl F_GETFL failed");
		return -1;
	}

	flags |= O_NONBLOCK;

	if (-1 == fcntl(socketfd, F_SETFL, flags) ) {
		logger_write(self->logger, LOGLEVEL_ERROR, "fcntl set O_NONBLOCK failed");
		return -1;
	}

	return 0;
}

static int
handle_read_write_errors(Server* self, ssize_t read_return_value, int* done)
{
	if (read_return_value == 0) {
		*done = 1;
		return 1;
	}

	if (read_return_value == -1) {
		if ((errno != EAGAIN) && (errno != EWOULDBLOCK)) {
			logger_write(self->logger, LOGLEVEL_ERROR, "read error");
			*done = 1;
		}

		return 1;
	}

	return 0;
}

static void
setup_server_socket(Server* self)
{
	if (make_socket_non_blocking(self, self->server_socketfd) == -1) abort();
}

static void
process_event(Server* self, size_t event_index)
{
	if (is_server_event(self, event_index)) {
		handle_server_errors(self, event_index);
		accept_new_connections(self);
	}
	else { // event from client
		ClientState* client = (ClientState *) events_get_user_data(self->events, event_index);

		if (!handle_client_close(self, client, event_index)) {
			handle_client(self, client);
		}
	}
}

static void
handle_server_errors(const Server* self, size_t event_index)
{
	if(events_is_error_event(self->events, event_index)) {
		logger_write(self->logger, LOGLEVEL_ERROR, "server socket error");
		close(self->server_socketfd);
		close(self->eventfd);
		exit(1);
	}
}

static int
handle_client_close(const Server* self, ClientState* client, size_t event_index)
{
	if(events_is_error_event(self->events, event_index)) {
		logger_write(self->logger, LOGLEVEL_INFO, "client disconnected");
		close(client_state_get_socketfd(client));
		client_state_free(client);
		return 1;
	}

	return 0;
}

static int
is_server_event(const Server* self, size_t event_index)
{
	return (&self->server_socketfd == events_get_user_data(self->events, event_index));
}

static void
accept_new_connections(Server* self)
{
	while (1) {
		int client_socketfd;

		client_socketfd = accept(self->server_socketfd, 0, 0);
		if (client_socketfd == -1) {
			if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
				break; //done
			}
			else {
				logger_write(self->logger, LOGLEVEL_ERROR, "accepting client connection failed");
				break;
			}
		}

		if (make_socket_non_blocking(self, client_socketfd) == -1) abort();

		ClientState* client = client_state_new(client_socketfd);
		events_register_socket(self->eventfd, client_socketfd);

		logger_write(self->logger, LOGLEVEL_INFO, "client connected");

		set_read_state(self, client);
	}
}

static void
handle_client(Server* self, ClientState* client)
{
	int client_state = client_state_get_state(client);

	switch (client_state) {
	case CS_RECVREQUEST:
		handle_read(self, client);
		break;

	case CS_SENDRESPONSE:
		handle_write(self, client);
		break;

	default: break;
	}
}

static void
handle_read(Server* self, ClientState* client)
{
	int done = 0;
	int client_socket = client_state_get_socketfd(client);

	while (1) {
		ssize_t count;
		char buf[128];

		count = read(client_socket, buf, sizeof(buf));

		if (handle_read_write_errors(self, count, &done)) break;

		client_state_append_request_data(client, buf, count);
	}

	if (done) {
		logger_write(self->logger, LOGLEVEL_INFO, "client disconnected");
		close(client_socket);
		client_state_free(client);
		return;
	}

	if (client_state_is_request_complete(client)) {

		char* request = client_state_get_full_request_data(client);
		char* response = self->make_response(request, self->p_data);

		client_state_set_response_data(client, response, strlen(response) + 1);
		free(response);
		client_state_clear_request_data(client);

		logger_write(self->logger, LOGLEVEL_INFO, "client request received");

		set_write_state(self, client);
	}
}

static void
handle_write(Server* self, ClientState* client)
{
	int done = 0;
	int client_socket = client_state_get_socketfd(client);

	while (1) {
		ssize_t count;

		if (!client_state_is_response_available(client)) break;

		const char* response = client_state_get_next_response_chunk(client);
		size_t response_size = client_state_get_remaining_response_data_size(client);

		count = write(client_socket, response, response_size);
		if (handle_read_write_errors(self, count, &done)) break;

		client_state_add_response_data_offset(client, count);
	}

	if (!client_state_is_response_available(client)) {
		logger_write(self->logger, LOGLEVEL_INFO, "client response sent");
		set_read_state(self, client);
	}
}

static void
set_read_state(Server* self, ClientState* client)
{
	events_set_read_state(self->eventfd, client_state_get_socketfd(client), client);
	client_state_set_state(client, CS_RECVREQUEST);
}

static void
set_write_state(Server* self, ClientState* client)
{
	events_set_write_state(self->eventfd, client_state_get_socketfd(client), client);
	client_state_set_state(client, CS_SENDRESPONSE);
}
