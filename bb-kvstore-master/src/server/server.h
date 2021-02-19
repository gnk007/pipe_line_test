#ifndef SERVER_H_INCLUDED
#define SERVER_H_INCLUDED

#include "logger.h"

typedef struct _Server Server;
typedef char* (*request_handler_fn)(char*, void*);

Server* server_new(const char* port, request_handler_fn request_handler, void* p_data, Logger* logger);
void server_free(Server* self);

void server_set_request_handler_fn(Server* self, request_handler_fn request_handler);
void server_loop_run(Server* self);

#endif /* SERVER_H_INCLUDED */
