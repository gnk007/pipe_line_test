#ifndef LOGGER_H_INCLUDED
#define LOGGER_H_INCLUDED

#include <stdio.h>

enum LogLevel { LOGLEVEL_INFO, LOGLEVEL_WARNING, LOGLEVEL_ERROR };

typedef struct Logger {
	char* log_filename;
} Logger;


Logger* logger_new(const char* log_filename);
void logger_free(Logger* self);

void logger_write(Logger* self, enum LogLevel level, const char* message);


#endif /*LOGGER_H_INCLUDED*/
