#include "tcp-client.h"
#include "stream.h"
#include "cli.h"

#include <memory>
#include <string>
#include <iostream>
#include <stdexcept>

void usage()
{
	std::cout << "Usage: bb-kvstore_cli hostname [port]" << std::endl << std::endl;
}

int main(int argc, char** argv)
{
	if (argc < 2 || argc > 3) {
		usage();
		return 1;
	}

	std::string hostname{argv[1]};
	int port = 6378;

	try {
		port = (argc == 3) ? std::stoi(argv[2]) : port;
	}
	catch (const std::logic_error& err) {
		std::cerr << "ERROR: invalid port number" << std::endl;
		return 1;
	}


	try {
		bb_kvstore::TCPClient client{hostname, port};
		bb_kvstore::CLI cli(client);

		cli.printHelp();
		while (!cli.isQuit()) {
			cli.printPrompt();
			cli.readCommand();
			cli.sendRequest();
			cli.printResponse();
		}
	}
	catch (const bb_kvstore::StreamError& err) {
		std::cerr << "FATAL ERROR: " << err.message << std::endl;
		return 1;
	}

	return 0;
}
