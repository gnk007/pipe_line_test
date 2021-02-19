#ifndef CLIENT_STATE_H_INCLUDED
#define CLIENT_STATE_H_INCLUDED

#include <stddef.h>

enum State { CS_DONE, CS_RECVREQUEST, CS_SENDRESPONSE };

struct ClientRequest {
	char* data;
	size_t capacity;
	size_t size;
};

struct ClientResponse {
	char* data;
	size_t size;
	size_t offset;
};

typedef struct ClientState {
	int socketfd;
	enum State state;
	struct ClientRequest request;
	struct ClientResponse response;
} ClientState;


ClientState* client_state_new(int socketfd);
void client_state_free(ClientState* self);

int client_state_get_socketfd(const ClientState* self);
enum State client_state_get_state(const ClientState* self);
char* client_state_get_full_request_data(const ClientState* self);
const char* client_state_get_next_response_chunk(const ClientState* self);
size_t client_state_get_remaining_response_data_size(const ClientState* self);
void client_state_add_response_data_offset(ClientState* self, size_t offset);
int client_state_is_request_complete(const ClientState* self);
int client_state_is_response_available(const ClientState* self);
void client_state_append_request_data(ClientState* self, const void* chunk, size_t chunk_size);
void client_state_set_response_data(ClientState* self, const void* data, size_t size);
void client_state_clear_request_data(ClientState* self);
void client_state_set_state(ClientState* self, enum State state);


#endif /*CLIENT_STATE_H_INCLUDED*/
