#pragma once

// Very fast, non-portable serialization for inter-process communication

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>

#include <cstddef>
#include <type_traits>
#include <string>


namespace cpped { namespace ipc {

// Utility that blocks until it reads specific number of bytes
template<typename Reader>
void read(Reader& reader, void* buf, std::size_t sz)
{
	std::size_t completed = 0;
	while(completed < sz)
	{
		auto result = reader.read(static_cast<char*>(buf)+completed, sz-completed);
		if (result <= 0)
		{
			throw std::runtime_error("Error reading from socket");
		}
		completed += result;
	}
}

// For POD types
template<typename Writer, typename T, typename tag = std::enable_if_t<std::is_pod<T>::value>>
void serialize(Writer& writer, const T& value)
{
	writer.write(&value, sizeof(T));
}

template<typename Reader, typename T, typename tag = std::enable_if_t<std::is_pod<T>::value>>
void deserialize(Reader& reader, T& value)
{
	read(reader, &value, sizeof(T));
}

// std::string
template<typename Writer>
void serialize(Writer& writer, const std::string& s)
{
	serialize(writer, s.size());
	writer.write(s.data(), s.size());
}

template<typename Reader>
void deserialize(Reader& reader, std::string& s)
{
	std::string::size_type size;
	deserialize(reader, size);

	s.clear();
	s.resize(size);
	read(reader, &s[0], size);
}

// boost::filesystem::path
template<typename Writer>
void serialize(Writer& writer, const boost::filesystem::path& p)
{
	serialize(writer, p.string());
}

template<typename Reader>
void deserialize(Reader& reader, boost::filesystem::path& p)
{
	std::string s;
	deserialize(reader, s);
	p = s;
}

// vector
template<typename Writer, typename T>
void serialize(Writer& writer, const std::vector<T>& vec)
{
	serialize(writer, vec.size());
	for(const T& v : vec)
	{
		serialize(writer, v);
	}
}

template<typename Reader, typename T>
void deserialize(Reader& reader, std::vector<T>& vec)
{
	std::size_t sz = 0;
	deserialize(reader, sz);
	vec.resize(sz);
	for(T& v : vec)
	{
		deserialize(reader, v);
	}
}

// optional
template<typename Writer, typename T>
void serialize(Writer& writer, const boost::optional<T>& opt)
{
	if (opt)
	{
		serialize(writer, true);
		serialize(writer, *opt);
	}
	else
	{
		serialize(writer, false);
	}

}

template<typename Reader, typename T>
void deserialize(Reader& reader, boost::optional<T>& opt)
{
	bool initialized;
	deserialize(reader, initialized);
	if (initialized)
	{
		opt.emplace();
		deserialize(reader, *opt);
	}
	else
	{
		opt.reset();
	}
}

}}
