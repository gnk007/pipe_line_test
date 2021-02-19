#include "cli.h"
#include "stream.h"

#include <iostream>
#include <string>

using bb_kvstore::CLI;
using bb_kvstore::StreamError;

CLI::CLI(Stream& stream) : stream(stream), quit{false} {}

void CLI::readCommand()
{
	std::string line;
	std::getline(std::cin, line);

	if (line == "quit") {
		setQuit(true);
		return;
	}

	request = parseCommand(line);
}

void CLI::sendRequest()
{
	if (isQuit()) return;

	try {
		stream.write(request);
	}
	catch (const StreamError& err) {
		std::cerr << "FATAL ERROR: " << err.message << std::endl;
		setQuit(true);
	}
}

void CLI::printResponse()
{
	if (isQuit()) return;

	try {
		response = stream.read();
	}
	catch (const StreamError& err) {
		std::cerr << "FATAL ERROR: " << err.message << std::endl;
		setQuit(true);
		return;
	}

	switch (response[0]) {
	case 'O':
		std::cout << response.substr(2) << std::endl << std::endl;
		break;
	case 'E':
		std::cerr << "ERROR: " << response.substr(2) << std::endl << std::endl;
	}
}

std::string CLI::parseCommand(const std::string& command)
{
	// TODO: proper command parsing (catch errors on the client side, etc.)
	return command;
}

void CLI::printPrompt()
{
	std::cout << "$ bb-kvstore_cli > ";
}

void CLI::printHelp()
{
	// TODO: thin client - server should send the available commands
	std::cout << "available commands:" << std::endl
		<< "  put key=value" << std::endl
		<< "  get key" << std::endl
		<< "  del key" << std::endl
		<< "  list" << std::endl
		<< "  count" << std::endl
		<< "  quit" << std::endl << std::endl;
}
