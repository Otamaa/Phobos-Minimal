#pragma once

#include <array>

template<typename Key , typename Value , std::size_t Size>
struct CompileTimeMap
{
	std::array<std::pair<Key, Value>, Size> data;

	[[nodiscard]] 
	COMPILETIMEEVAL Value at(const Key& key) const
	{
		const auto itr = std::find_if(std::begin(data), std::end(data), [&key](const auto& item) { return item.first == key; });

		if (itr == std::end(data))
			throw std::range_error("Not Found");

		return itr->second;
	}

	[nodiscard]
	COMPILETIMEEVAL bool contains(const Key& key)
	{
		return std::find_if(std::begin(data), std::end(data),
			[&key](const auto& item) {
				return item.first == key;
			}) != std::end(data);
	}
};