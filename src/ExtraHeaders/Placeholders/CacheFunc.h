#pragma once

#include <tuple>
#include <type_traits>
#include <map>

template<typename Func, typename ... Params>
auto cache(Func func, Params &&... params)
{
	using param_set = std::tuple <std::remove_cvref_t<Params>...>;

	paran_set key { params... };

	using result_type = std::remove_cvref_t <std::invoke_result_t<Func, decltype(params)...>;

	// not thread safe
	static std::map<param_set, result_type> cached_values;
	using value_type = decltype(cached_values)::value_type;

	auto itr = cached_values.find(key);

	if (itr != cached_values.end())
		return itr->second;

	return cached_values.insert(value_type {std::move(key),
		func(std::forward<Params>(params)...)}).first->second;
}