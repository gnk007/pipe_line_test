#ifndef TCP_CLIENT_H_INCLUDED
#define TCP_CLIENT_H_INCLUDED

#include "stream.h"

#include <string>
#include <cstddef>

#include <netinet/in.h>

namespace bb_kvstore {

class TCPClient : public Stream {
private:
	std::string host;
	int port;
	int socketfd;
	struct sockaddr_in server;

	ssize_t send(const void* data, size_t data_size);
	ssize_t recv(void* data, size_t data_size);

public:
	TCPClient(std::string host, int port);
	TCPClient& operator=(const TCPClient&) = delete;
    TCPClient(const TCPClient&) = delete;
	virtual ~TCPClient();

	void sendData(const void* data, size_t data_size);
	virtual void write(const std::string& data);
	virtual std::string read();

};

} // namespace bb_kvstore

#endif /* TCP_CLIENT_H_INCLUDED */
