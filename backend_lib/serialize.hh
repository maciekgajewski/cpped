#pragma once

// Very fast, non-portable serialization for inter-process communication

#include <type_traits>


namespace cpp { namespace backend {

// Utility that blocks until it reads specific number of bytes
template<typename Reader>
void read(Reader& reader, void* buf, std::size_t sz)
{
	std::size_t completed = 0;
	while(completed < sz)
	{
		auto result = reader.read(buf+completed, sz-completed);
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

}}
