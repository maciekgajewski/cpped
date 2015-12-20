#pragma once

// Very fast, non-portable serialization for inter-process communication

#include <type_traits>


namespace cpp { namespace backend {

// For POD types
template<typename Writer, typename T, typename tag = std::enable_if_t<std::is_pod<T>::value>>
void serialize(Writer& w, const T& value)
{
	w.write(&value, sizeof(T));
}

}}
