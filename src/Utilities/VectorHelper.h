#pragma once

#include <vector>

template<typename T>
struct HelperedVector : public std::vector<T>
{
	template <typename Func>
	void for_each(Func&& act) const {
		std::for_each(this->begin(), this->end(), std::forward<Func>(act));
	}

	template <typename Func>
	void for_each(Func&& act) {
		std::for_each(this->begin(), this->end(), std::forward<Func>(act));
	}

	template<typename func>
	bool none_of(func&& fn) const {
		return std::none_of(this->begin(), this->end(), std::forward<func>(fn));
	}

	template<typename func>
	bool none_of(func&& fn) {
		return std::none_of(this->begin(), this->end(), std::forward<func>(fn));
	}

	template<typename func>
	bool any_of(func&& fn) const
	{
		return std::any_of(this->begin(), this->end(), std::forward<func>(fn));
	}

	template<typename func>
	bool any_of(func&& fn) {
		return std::any_of(this->begin(), this->end(), std::forward<func>(fn));
	}
};