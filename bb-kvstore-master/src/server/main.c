#include "keyvalue-server.h"
#include "logger.h"
#include "utils.h"

#include <stdio.h>

#define DEFAULT_PORT "6378"

void
usage(void)
{
	printf("Usage: bb-kvstore_server [options]\n\n"
		"Options:\n"
		"  -F, --foreground                 Do not daemonize, run in the foreground\n"
		"  -p port, --port=<port>           Server port (default: " DEFAULT_PORT ")\n"
	);
}

int
main(int argc, char *argv[])
{
	int do_daemon = 1;
	char port[6] = DEFAULT_PORT;

	parse_args(argc, argv, &do_daemon, port, usage);

	if (do_daemon) {
		daemonize();
	}

	Logger* logger = logger_new("kvstore_server.log");

	KeyValueServer* kvserver = key_value_server_new(port, logger);
	key_value_server_start(kvserver);
	key_value_server_free(kvserver);

	logger_free(logger);

	return 0;
}
