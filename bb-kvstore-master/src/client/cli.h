#ifndef CLI_H_INCLUDED
#define CLI_H_INCLUDED

#include "stream.h"

namespace bb_kvstore {

class CLI {
private:
	Stream& stream;
	bool quit;
	std::string request;
	std::string response;

	static std::string parseCommand(const std::string& command);

public:
	CLI(Stream& stream);

	bool isQuit() const { return quit; }
	void setQuit(bool q) { quit = q; }
	void readCommand();
	void sendRequest();
	void printResponse();

	static void printPrompt();
	static void printHelp();
};

} // namespace bb_kvstore

#endif /* CLI_H_INCLUDED */
