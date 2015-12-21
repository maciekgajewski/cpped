#pragma once

#include <cstring>
#include <unistd.h>

namespace cpp { namespace backend {

// Implements the 'writer', with writes buffered in stack-allocated buffer.
template<typename UnderlyingWriter, unsigned BUFFER_SIZE = 64*1024>
class buffered_writer
{
public:

	~buffered_writer()
	{
		flush();
	}

	buffered_writer(UnderlyingWriter& w) : underlying_(w) {}
	void write(const void* data, std::size_t sz);

private:

	void flush();

	UnderlyingWriter& underlying_;
	char buffer_[BUFFER_SIZE];
	std::size_t buffer_usage_ = 0;
	int fd_;
};

template<typename UnderlyingWriter, unsigned BUFFER_SIZE>
void buffered_writer<UnderlyingWriter, BUFFER_SIZE>::write(const void* data, std::size_t sz)
{
	if (sz > BUFFER_SIZE)
	{
		flush();
		underlying_.write(data, sz);
	}
	else
	{
		if (sz + buffer_usage_ > BUFFER_SIZE)
		{
			flush();
		}
		std::memcpy(buffer_ + buffer_usage_, data, sz);
		buffer_usage_ += sz;
	}
}

template<typename UnderlyingWriter, unsigned BUFFER_SIZE>
void buffered_writer<UnderlyingWriter, BUFFER_SIZE>::flush()
{
	if (buffer_usage_ > 0)
	{
		underlying_.write(buffer_, buffer_usage_);
		buffer_usage_ = 0;
	}
}



}}
