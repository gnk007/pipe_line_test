#ifndef STREAM_H_INCLUDED
#define STREAM_H_INCLUDED

#include <string>

namespace bb_kvstore {

struct StreamError {
	std::string message;

	StreamError() = default;
	explicit StreamError(std::string message) : message{message} {}
};

class Stream {
public:
	virtual ~Stream() {}

	virtual void write(const std::string& data) = 0;
	virtual std::string read() = 0;
};

} // namespace bb_kvstore

#endif /* STREAM_H_INCLUDED */
