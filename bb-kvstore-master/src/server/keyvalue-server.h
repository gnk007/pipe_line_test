#ifndef KEYVALUE_SERVER_H_INCLUDED
#define KEYVALUE_SERVER_H_INCLUDED

#include "logger.h"

typedef struct _KeyValueServer KeyValueServer;

KeyValueServer* key_value_server_new(const char* port, Logger* logger);
void key_value_server_free(KeyValueServer* self);

void key_value_server_start(KeyValueServer* self);

#endif /* KEYVALUE_SERVER_H_INCLUDED */
