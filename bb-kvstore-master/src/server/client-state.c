#include "client-state.h"

#include <stdlib.h>
#include <string.h>

ClientState*
client_state_new(int socketfd)
{
	ClientState* self = (ClientState *) calloc(1, sizeof(ClientState));

	self->socketfd = socketfd;
	self->state = CS_DONE;

	return self;
}

void
client_state_free(ClientState* self)
{
	free(self->request.data);
	free(self->response.data);
	free(self);
}

int
client_state_get_socketfd(const ClientState* self)
{
	return self->socketfd;
}

enum State
client_state_get_state(const ClientState* self)
{
	return self->state;
}

char*
client_state_get_full_request_data(const ClientState* self)
{
	return self->request.data;
}

const char*
client_state_get_next_response_chunk(const ClientState* self)
{
	return self->response.data + self->response.offset;
}

size_t
client_state_get_remaining_response_data_size(const ClientState* self)
{
	return self->response.size - self->response.offset;
}

void
client_state_add_response_data_offset(ClientState* self, size_t offset)
{
	self->response.offset += offset;
}

int
client_state_is_request_complete(const ClientState* self)
{
	return self->request.data && self->request.data[self->request.size - 1] == 0;
}

int
client_state_is_response_available(const ClientState* self)
{
	return (self->response.size > 0) && (self->response.offset < self->response.size);
}

void
client_state_append_request_data(ClientState* self, const void* chunk, size_t chunk_size)
{
	char* new_data;
	size_t free_space, new_size;

	if (!self->request.data) {
		self->request.data = (char *) malloc(chunk_size + 1);
		memcpy(self->request.data, chunk, chunk_size);
		self->request.data[chunk_size] = 0;
		self->request.capacity = chunk_size;
		self->request.size = chunk_size;
	}
	else {
		free_space = self->request.capacity - self->request.size;
		if (free_space < (chunk_size + 1)) {
			new_size = self->request.capacity + (chunk_size - free_space + 1);
			new_data = (char *) realloc(self->request.data, new_size);

			if (!new_data) exit(1);

			self->request.data = new_data;
			self->request.capacity = new_size;
		}

		memcpy(self->request.data + self->request.size, chunk, chunk_size);
		self->request.size += chunk_size;
		self->request.data[self->request.size] = 0;
	}
}

void
client_state_set_response_data(ClientState* self, const void* data, size_t size)
{
	free(self->response.data);
	self->response.data = (char *) malloc(size);

	memcpy(self->response.data, data, size);
	self->response.size = size;
	self->response.offset = 0;
}

void
client_state_clear_request_data(ClientState* self)
{
	free(self->request.data);
	self->request.data = 0;
	self->request.size = 0;
	self->request.capacity = 0;
}

void
client_state_set_state(ClientState* self, enum State state)
{
	self->state = state;
}
