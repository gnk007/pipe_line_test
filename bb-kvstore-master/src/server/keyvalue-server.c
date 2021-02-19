#include "keyvalue-server.h"
#include "server.h"
#include "string-hashtable.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define KV_PUT_STR "put "
#define KV_GET_STR "get "
#define KV_DEL_STR "del "
#define KV_LIST_STR "list"
#define KV_COUNT_STR "count"
#define KV_SEPARATOR_CHAR '='
#define KV_STATUS_OK 'O'
#define KV_STATUS_ERROR 'E'

#define KV_OP_COUNT 5
enum KVOperation { KV_PUT, KV_GET, KV_DEL, KV_LIST, KV_NUMOFKEYS, KV_ERROR};

struct KVRequest {
	enum KVOperation operation;
	char* param1;
	char* param2;
};

typedef char* (*operation_fn)(KeyValueServer*, struct KVRequest*);

struct _KeyValueServer {
	Server* server;
	HashTable* hash_table;
	operation_fn operations[KV_OP_COUNT];
};

static char* request_handler(char* request, void* p_data);
static struct KVRequest parse_request(char* request);
static char* operation_put(KeyValueServer* self, struct KVRequest* request);
static char* operation_get(KeyValueServer* self, struct KVRequest* request);
static char* operation_del(KeyValueServer* self, struct KVRequest* request);
static char* operation_list(KeyValueServer* self, struct KVRequest* request);
static char* operation_numofkeys(KeyValueServer* self, struct KVRequest* request);
static char* make_response(char status, const char* res);
static int is_request_starts_with(const char* prefix, const char* request);

KeyValueServer*
key_value_server_new(const char* port, Logger* logger)
{
	KeyValueServer* self = (KeyValueServer *) calloc(1, sizeof(KeyValueServer));
	self->server = server_new(port, request_handler, self, logger);
	self->hash_table = string_hash_table_new(16);

	self->operations[KV_PUT] = operation_put;
	self->operations[KV_GET] = operation_get;
	self->operations[KV_DEL] = operation_del;
	self->operations[KV_LIST] = operation_list;
	self->operations[KV_NUMOFKEYS] = operation_numofkeys;

	return self;
}

void
key_value_server_free(KeyValueServer* self)
{
	server_free(self->server);
	hash_table_free(self->hash_table);
	free(self);
}

void
key_value_server_start(KeyValueServer* self)
{
	server_loop_run(self->server);
}

static char*
request_handler(char* request, void* p_data)
{
	KeyValueServer* self = (KeyValueServer *) p_data;

	struct KVRequest req = parse_request(request);

	if (req.operation == KV_ERROR)
		return make_response(KV_STATUS_ERROR, "command not found");

	return self->operations[req.operation](self, &req);
}

static struct KVRequest
parse_request(char* request)
{
	struct KVRequest req = { KV_ERROR, 0, 0 };

	if (is_request_starts_with(KV_PUT_STR, request)) {
		request += sizeof(KV_PUT_STR) - 1;
		req.operation = KV_PUT;

		const char* separator = strchr(request, KV_SEPARATOR_CHAR);
		if (!separator) {
			req.operation = KV_ERROR;
			return req;
		}

		size_t param1_length = (separator - request) + 1;

		req.param1 = (char *) malloc(param1_length * sizeof(char));
		memcpy(req.param1, request, param1_length);
		req.param1[param1_length - 1] = 0;

		size_t param2_length = strlen(separator + 1) + 1;
		req.param2 = (char *) malloc(param2_length * sizeof(char));
		memcpy(req.param2, separator + 1, param2_length);
	}
	else if (is_request_starts_with(KV_GET_STR, request)) {
		request += sizeof(KV_GET_STR) - 1;
		req.operation = KV_GET;
		req.param1 = request;
	}
	else if (is_request_starts_with(KV_DEL_STR, request)) {
		request += sizeof(KV_DEL_STR) - 1;
		req.operation = KV_DEL;
		req.param1 = request;
	}
	else if (strcmp(KV_LIST_STR, request) == 0) {
		req.operation = KV_LIST;
	}
	else if (strcmp(KV_COUNT_STR, request) == 0) {
		req.operation = KV_NUMOFKEYS;
	}
	else {
		req.operation = KV_ERROR;
	}

	return req;
}

static char*
operation_put(KeyValueServer* self, struct KVRequest* request)
{
	if (!hash_table_contains(self->hash_table, request->param1)) {
		hash_table_put(self->hash_table, request->param1, request->param2);
		return make_response(KV_STATUS_OK, "OK");
	}
	else
		return make_response(KV_STATUS_ERROR, "key already exists");
}

static char*
operation_get(KeyValueServer* self, struct KVRequest* request)
{
	char* response = hash_table_get(self->hash_table, request->param1);
	if (response) return make_response(KV_STATUS_OK, response);
	else return make_response(KV_STATUS_ERROR, "key not found");
}

static char*
operation_del(KeyValueServer* self, struct KVRequest* request)
{
	hash_table_delete(self->hash_table, request->param1);
	return make_response(KV_STATUS_OK, "OK");
}

static char*
operation_list(KeyValueServer* self, struct KVRequest* request)
{
	size_t response_size = 2;
	char* response = (char *) malloc(response_size * sizeof(char));
	response[0] = KV_STATUS_OK;
	response[1] = ' ';

	unsigned int i;
	for (i = 0; i < hash_table_capacity(self->hash_table); ++i) {
		if (self->hash_table->keyValues.keys[i] != 0) {
			size_t key_length = strlen(self->hash_table->keyValues.keys[i]);
			size_t new_response_size = response_size + key_length + 1;

			response = (char *) realloc(response, new_response_size * sizeof(char));
			memcpy(response + response_size, self->hash_table->keyValues.keys[i], key_length);
			response[new_response_size - 1] = '\n';

			response_size = new_response_size;
		}
	}

	response[response_size - 1] = 0;

	return response;
}

static char*
operation_numofkeys(KeyValueServer* self, struct KVRequest* request)
{
	char size_as_string[32];
	unsigned int size = hash_table_size(self->hash_table);
	snprintf(size_as_string, 32, "%d", size);
	return make_response(KV_STATUS_OK, size_as_string);
}

static char*
make_response(char status, const char* res)
{
	size_t response_length = strlen(res) + 3;
	char* response = (char *) malloc(response_length * sizeof(char));

	snprintf(response, response_length, "%c %s", status, res);

	return response;
}

static int
is_request_starts_with(const char* prefix, const char* request)
{
	size_t prefix_length = strlen(prefix);
	size_t request_length = strlen(request);

    return (request_length < prefix_length) ? 0 : (strncmp(prefix, request, prefix_length) == 0);
}
