#pragma once

#include <stdint.h>
#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <stdexcept>
#include <functional>

namespace cpped { namespace backend {

using type_id = std::uint64_t;

template<typename T>
type_id get_type_id()
{
	return (type_id)(typeid(T).name());
}

template<typename ... Args>
class type_dispatcher
{
public:

	using function_type = std::function<void(Args...)>;

	template<typename T>
	void add_type(const function_type& f);

	void dispatch(type_id type, Args&&... args) const;

private:

	std::unordered_map<type_id, function_type> handlers_;

};

template<typename ... Args>
template<typename T>
void type_dispatcher<Args...>::add_type(const function_type& f)
{
	type_id type = get_type_id<T>();
	handlers_[type] = f;
}


template<typename ... Args>
void type_dispatcher<Args...>::dispatch(type_id type, Args&&... args) const
{
	auto it = handlers_.find(type);
	if (it == handlers_.end())
	{
		throw std::runtime_error("Unknown type");
	}

	const function_type& f = it->second;
	f(std::forward<Args>(args)...);
}


}}
