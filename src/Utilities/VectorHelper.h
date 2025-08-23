#pragma once

#include <vector>
#include <algorithm>
#include <Helpers/Concepts.h>

#include <Lib/fast_remove_if.h>

#include <Utilities/Savegame.h>

template<typename T, typename A = std::allocator<T>>
struct HelperedVector : public std::vector<T , A>
{

	bool FORCEDINLINE remove_at(size_t index) {
		if (this->valid_index(index)) {
			this->erase(this->begin() + index);
			return true;
		}

		return false;
	}

	bool FORCEDINLINE remove(const T& item) {
		const auto iter = this->find(item);

		if (iter != this->end()) {
			this->erase(iter);
			return true;
		}

		return false;
	}

	template <typename Func>
	void FORCEDINLINE remove_all_duplicates(Func&& act) {
		std::sort(this->begin(), this->end(), std::forward<Func>(act));
		this->erase(std::unique(this->begin(), this->end()), this->end());
	}

	void FORCEDINLINE remove_all_duplicates_noshort() {
		this->erase(std::unique(this->begin(), this->end()), this->end());
	}

	template <typename Func>
	bool FORCEDINLINE remove_all_if(Func&& act)
	{
		return fast_remove_if(this, std::forward<Func>(act));
	}

	template <typename Func>
	auto FORCEDINLINE find_if(Func&& act) const {
		return std::find_if(this->begin(), this->end(), std::forward<Func>(act));
	}

	template <typename Func>
	auto FORCEDINLINE find_if(Func&& act) {
		return std::find_if(this->begin(), this->end(), std::forward<Func>(act));
	}

	auto FORCEDINLINE find(const T& item) const {
		if COMPILETIMEEVAL (direct_comparable<T>) {
			return std::find_if(this->begin(), this->end(), [item](auto& i) { return i == item; });
		} else {
			return std::find(this->begin(), this->end(), item);
		}
	}

	auto FORCEDINLINE find(const T& item) {
		if COMPILETIMEEVAL (direct_comparable<T>) {
			return std::find_if(this->begin(), this->end(), [item](auto& i) { return i == item; });
		} else {
			return std::find(this->begin(), this->end(), item);
		}
	}

	bool FORCEDINLINE contains(const T& other) const {
		return this->find(other) != this->end();
	}

	bool FORCEDINLINE push_back_unique(const T& other) {
		if (!this->contains(other)){
			this->push_back(other);
			return true;
		}

		return false;
	}

	int FORCEDINLINE index_of(const T& other) const {
		auto iter = this->find(other);
		return iter != this->end() ? std::distance(this->begin(), iter) : -1;
	}

	bool FORCEDINLINE valid_index(int index) const {
		return static_cast<size_t>(index) < this->size();
	}

	template <typename Func>
	void FORCEDINLINE for_each(Func&& act) const {
		for (auto i = this->begin(); i != this->end(); ++i) {
        	act(*i);
    	}
	}

	template <typename Func>
	void FORCEDINLINE for_each(Func&& act) {
		for (auto i = this->begin(); i != this->end(); ++i) {
        	act(*i);
    	}
	}

	template<typename func>
	bool FORCEDINLINE none_of(func&& fn) const {
		for (auto i = this->begin(); i != this->end(); ++i) {
       	 	if (fn(*i)) {
           	 	return false;
        	}
    	}

    	return true;
	}

	template<typename func>
	bool FORCEDINLINE none_of(func&& fn) {
		for (auto i = this->begin(); i != this->end(); ++i) {
       	 	if (fn(*i)) {
           	 	return false;
        	}
    	}

    	return true;
	}

	template<typename func>
	bool FORCEDINLINE any_of(func&& fn) const {
		for (auto i = this->begin(); i != this->end(); ++i) {
       		if (fn(*i)) {
            	return true;
			}
        }

		return false;
	}

	template<typename func>
	bool FORCEDINLINE any_of(func&& fn) {
		for (auto i = this->begin(); i != this->end(); ++i) {
       		if (fn(*i)) {
            	return true;
			}
        }

		return false;
	}

	template<typename func>
	bool FORCEDINLINE all_of(func&& fn) const {
		for (auto i = this->begin(); i != this->end(); ++i) {
			if (!fn(*i)) {
				return false;
			}
		}

		return true;
	}

	template<typename func>
	bool FORCEDINLINE all_of(func&& fn) {
		for (auto i = this->begin(); i != this->end(); ++i) {
			if (!fn(*i)) {
				return false;
			}
		}

		return true;
	}

	bool load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		std::vector<T>* me = this;
		return Stm.Process(*me, RegisterForChange);
	}

	bool save(PhobosStreamWriter& Stm) const
	{
		const std::vector<T>* me = this;
		return Stm.Process(*me);
	}
};