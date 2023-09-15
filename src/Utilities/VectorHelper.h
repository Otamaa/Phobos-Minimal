#pragma once

#include <vector>
#include <Helpers/Concepts.h>

template<typename T>
struct HelperedVector : public std::vector<T>
{

	bool remove_at(int index) {
		if (!this->valid_index(index)) {
			this->erase(this->begin() + index);
			return true;
		}

		return false;
	}

	bool remove(const T& item) {
		const auto iter = this->find(item);

		if (iter != this->end()) {
			this->erase(iter);
			return true;
		}

		return false;
	}

	auto find(const T& item) const {
		if constexpr (direct_comparable<T>) {
			return std::find_if(this->begin(), this->end(), [item](const auto item_here) { return item_here == item; });
		} else {
			return std::find(this->begin(), this->end(), item);
		}
	}

	auto find(const T& item) {
		if constexpr (direct_comparable<T>) {
			return std::find_if(this->begin(), this->end(), [item](const auto item_here) { return item_here == item; });
		} else {
			return std::find(this->begin(), this->end(), item);
		}
	}

	bool contains(const T& other) const {
		return this->find(other) != this->end();
	}

	void push_back_unique(const T& other) {
		if (!this->contains(other))
			this->push_back(other);
	}

	int index_of(const T& other) const {
		auto iter = this->find(other);
		return iter != this->end() ? std::distance(this->begin(), iter) : -1;
	}

	bool valid_index(int index) const {
		return static_cast<size_t>(index) < this->size();
	}

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
	bool any_of(func&& fn) const {
		return std::any_of(this->begin(), this->end(), std::forward<func>(fn));
	}

	template<typename func>
	bool any_of(func&& fn) {
		return std::any_of(this->begin(), this->end(), std::forward<func>(fn));
	}

	template<typename func>
	bool all_of(func&& fn) const {
		return std::all_of(this->begin(), this->end(), std::forward<func>(fn));
	}

	template<typename func>
	bool all_of(func&& fn) {
		return std::all_of(this->begin(), this->end(), std::forward<func>(fn));
	}
};