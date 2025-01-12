#pragma once

#include <vector>
#include <algorithm>
#include <Helpers/Concepts.h>

template<typename T, typename A = std::allocator<T>>
struct HelperedVector : public std::vector<T , A>
{

	//bool FORCEINLINE remove_at(size_t index) {
	//	if (this->valid_index(index)) {
	//		this->erase(this->begin() + index);
	//		return true;
	//	}

	//	return false;
	//}

	bool FORCEINLINE remove(const T& item) {
		const auto iter = std::remove(this->begin() , this->end() , item);

		if (iter != this->end()) {
			this->erase(iter);
			return true;
		}

		return false;
	}

	template <typename Func>
	bool FORCEINLINE remove_if(Func&& act) {
		const auto iter = std::remove_if(this->begin(), this->end(), std::forward<Func>(act));

		if (iter != this->end()) {
			this->erase(iter);
			return true;
		}

		return false;
	}

	template <typename Func>
	void FORCEINLINE remove_all_duplicates(Func&& act) {
		std::sort(this->begin(), this->end(), std::forward<Func>(act));
		this->erase(std::unique(this->begin(), this->end()), this->end());
	}

	void FORCEINLINE remove_all_duplicates_noshort() {
		this->erase(std::unique(this->begin(), this->end()), this->end());
	}

	template <typename Func>
	bool FORCEINLINE remove_all_if(Func&& act)
	{
		const auto iter = std::remove_if(this->begin(), this->end(), std::forward<Func>(act));

		if (iter != this->end()) {
			this->erase(iter, this->end());
			return true;
		}

		return false;
	}

	template <typename Func>
	auto FORCEINLINE find_if(Func&& act) {
		return std::find_if(this->begin(), this->end(), std::forward<Func>(act));
	}

	auto FORCEINLINE find(const T& item) const {
		if constexpr (direct_comparable<T>) {
			auto i = this->begin();

			for (; i != this->end(); ++i) {
				if (*i == item) {
					break;
				}
			}

			return i;
		} else {
			return std::find(this->begin(), this->end(), item);
		}
	}

	auto FORCEINLINE find(const T& item) {
		if constexpr (direct_comparable<T>) {
			auto i = this->begin();

			for (; i != this->end(); ++i) {
				if (*i == item) {
					break;
				}
			}

			return i;
		} else {
			return std::find(this->begin(), this->end(), item);
		}
	}

	bool FORCEINLINE contains(const T& other) const {
		return this->find(other) != this->end();
	}

	bool FORCEINLINE push_back_unique(const T& other) {
		if (!this->contains(other)){
			this->push_back(other);
			return true;
		}

		return false;
	}

	int FORCEINLINE index_of(const T& other) const {
		auto iter = this->find(other);
		return iter != this->end() ? std::distance(this->begin(), iter) : -1;
	}

	bool FORCEINLINE valid_index(int index) const {
		return static_cast<size_t>(index) < this->size();
	}

	template <typename Func>
	void FORCEINLINE for_each(Func&& act) const {
		for (auto i = this->begin(); i != this->end(); ++i) {
        	act(*i);
    	}
	}

	template <typename Func>
	void FORCEINLINE for_each(Func&& act) {
		for (auto i = this->begin(); i != this->end(); ++i) {
        	act(*i);
    	}
	}

	template<typename func>
	bool FORCEINLINE none_of(func&& fn) const {
		for (auto i = this->begin(); i != this->end(); ++i) {
       	 	if (fn(*i)) {
           	 	return false;
        	}
    	}

    	return true;
	}

	template<typename func>
	bool FORCEINLINE none_of(func&& fn) {
		for (auto i = this->begin(); i != this->end(); ++i) {
       	 	if (fn(*i)) {
           	 	return false;
        	}
    	}

    	return true;
	}

	template<typename func>
	bool FORCEINLINE any_of(func&& fn) const {
		for (auto i = this->begin(); i != this->end(); ++i) {
       		if (fn(*i)) {
            	return true;
			}
        }

		return false;
	}

	template<typename func>
	bool FORCEINLINE any_of(func&& fn) {
		for (auto i = this->begin(); i != this->end(); ++i) {
       		if (fn(*i)) {
            	return true;
			}
        }

		return false;
	}

	template<typename func>
	bool FORCEINLINE all_of(func&& fn) const {
		for (auto i = this->begin(); i != this->end(); ++i) {
			if (!fn(*i)) {
				return false;
			}
		}

		return true;
	}

	template<typename func>
	bool FORCEINLINE all_of(func&& fn) {
		for (auto i = this->begin(); i != this->end(); ++i) {
			if (!fn(*i)) {
				return false;
			}
		}

		return true;
	}
};