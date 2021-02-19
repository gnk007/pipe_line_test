#include "utils.h"

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>


void
daemonize(void)
{
	int fd;
	pid_t pid;

	switch (fork()) {
	case -1:
		printf("fork() failed\n");
		exit(2);
	case 0:
		break;
	default:
		exit(0);
	}

	pid = getpid();

	if (setsid() == -1) {
		printf("setsid() failed\n");
		exit(2);
	}

	umask(0);
	chdir("/tmp");

	fd = open("/dev/null", O_RDWR);
	if (fd == -1) {
		printf("open(\"/dev/null\") failed\n");
		exit(2);
	}

	if (dup2(fd, STDIN_FILENO) == -1) {
		printf("dup2(STDIN) failed\n");
		exit(2);
	}

	if (dup2(fd, STDOUT_FILENO) == -1) {
		printf("dup2(STDOUT) failed\n");
		exit(2);
	}

	if (dup2(fd, STDERR_FILENO) == -1) {
		printf("dup2(STDERR) failed\n");
		exit(2);
	}
}

void
parse_args(int argc, char** argv, int* do_daemon, char* port, void (*usage)(void))
{
	struct option options[] =
	{
		{ "port", required_argument, NULL, 'p' },
		{ "foreground", no_argument, NULL, 'F' },
		{ "help", no_argument, NULL, 'h' },
		{ NULL, 0, NULL, 0 }
	};

	int opt;

	while ((opt = getopt_long(argc, argv, "p:Fh", options, NULL)) != -1) {

		switch (opt) {
		case 'p':
			snprintf(port, 6, "%s", optarg);
			break;
		case 'F':
			*do_daemon = 0;
			break;
		case 'h':
		default:
			usage();
			exit(0);
			break;
		}
	}
}
