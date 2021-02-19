#include "tcp-client.h"
#include "stream.h"

#include <string>
#include <cstring>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

using bb_kvstore::TCPClient;
using bb_kvstore::StreamError;

TCPClient::TCPClient(std::string host, int port) : host{host}, port{port}
{
	socketfd = socket(AF_INET, SOCK_STREAM, 0);
	if (socketfd == -1)
		throw StreamError("could not create socket");

	if (inet_addr(host.c_str()) == -1) {
		struct hostent* he;

		if ((he = gethostbyname(host.c_str())) == nullptr)
			throw StreamError("could not resolve hostname");

		memcpy(&server.sin_addr, he->h_addr, he->h_length);
	}
	else {
		server.sin_addr.s_addr = inet_addr(host.c_str());
	}

	server.sin_family = AF_INET;
	server.sin_port = htons(port);

	if (connect(socketfd, (struct sockaddr *) &server, sizeof(server)) < 0)
		throw StreamError("could not connect to host");
}

TCPClient::~TCPClient()
{
	close(socketfd);
}

ssize_t TCPClient::send(const void* data, size_t data_size)
{
	return ::write(socketfd, data, data_size);
}

ssize_t TCPClient::recv(void* data, size_t data_size)
{
	return ::read(socketfd, data, data_size);
}

void TCPClient::sendData(const void* data, size_t data_size)
{
	size_t sent = 0;
	while (sent < data_size) {
		ssize_t count = this->send(((char *)data) + sent, data_size - sent);

		if (count <= 0)
			throw StreamError("send error");

		sent += count;
	}
}

void TCPClient::write(const std::string& data)
{
	this->sendData(data.c_str(), data.size() + 1);
}

std::string TCPClient::read()
{
	const int buffer_size = 32;
	std::string data;
	data.reserve(buffer_size);

	char buf[buffer_size];
	while (1) {
		ssize_t count = this->recv(buf, buffer_size);

		if (count <= 0) throw StreamError("send error");

		data.append(buf, count);

		if (buf[count - 1] == 0) break;
	}

	return data;
}
