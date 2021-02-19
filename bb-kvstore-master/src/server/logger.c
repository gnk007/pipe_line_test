#include "logger.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

Logger*
logger_new(const char* log_filename)
{
	Logger* self = (Logger *) calloc(1, sizeof(Logger));
	self->log_filename = malloc((strlen(log_filename) + 1) * sizeof(char));
	strcpy(self->log_filename, log_filename);

	return self;
}

void
logger_free(Logger* self)
{
	free(self->log_filename);
	free(self);
}

void
logger_write(Logger* self, enum LogLevel level, const char* message)
{
	const int time_buffer_size = 128;
	char time_buffer[time_buffer_size];
	time_t now = time(0);

	FILE* logfile = fopen(self->log_filename, "a");
	if (!logfile) return;

	const char* level_label = "[]";
	switch (level) {
	case LOGLEVEL_INFO:
		level_label = "[INFO]";
		break;
	case LOGLEVEL_WARNING:
		level_label = "[WARNING]";
		break;
	case LOGLEVEL_ERROR:
		level_label = "[ERROR]";
		break;
	}

	strftime (time_buffer, time_buffer_size, "%Y-%m-%d %H:%M:%S", localtime(&now));
	fprintf(logfile, "%s %s %s\n", time_buffer, level_label, message);

	fclose(logfile);
}
