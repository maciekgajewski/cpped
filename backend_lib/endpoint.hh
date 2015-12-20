#pragma once

#include <cstdlib>


namespace cpp { namespace backend {

// Communication endpoint used to talk to the other process
class endpoint
{
public:

	endpoint(int fd);
	endpoint(const endpoint&) = delete;
	endpoint(endpoint&& e);
	~endpoint();

	void set_fd(int fd);

private:

	void close();
	void send(const char* data, std::size_t sz);

	int fd_ = -1;
};

}}
