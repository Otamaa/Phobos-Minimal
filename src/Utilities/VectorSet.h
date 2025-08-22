#pragma once
#include <vector>
#include <algorithm>
#include <utility>
#include <initializer_list>

#include <Utilities/Savegame.h>

template <typename T, typename Compare = std::less<T>>
class VectorSet
{
public:
	using value_type = T;
	using container_type = std::vector<T>;
	using size_type = typename container_type::size_type;
	using difference_type = typename container_type::difference_type;
	using reference = typename container_type::reference;
	using const_reference = typename container_type::const_reference;
	using iterator = typename container_type::iterator;
	using const_iterator = typename container_type::const_iterator;

	VectorSet() = default;

	VectorSet(std::initializer_list<T> init) {
		for (auto& v : init) insert(v);
	}

	template <typename InputIt>
	VectorSet(InputIt first, InputIt last) {
		for (; first != last; ++first) insert(*first);
	}

	// basic capacity
	[[nodiscard]] bool empty() const noexcept { return data_.empty(); }
	[[nodiscard]] size_type size() const noexcept { return data_.size(); }
	void clear() noexcept { data_.clear(); }

	// iterators
	iterator begin() noexcept { return data_.begin(); }
	const_iterator begin() const noexcept { return data_.begin(); }
	const_iterator cbegin() const noexcept { return data_.cbegin(); }
	iterator end() noexcept { return data_.end(); }
	const_iterator end() const noexcept { return data_.end(); }
	const_iterator cend() const noexcept { return data_.cend(); }

	// modifiers
	std::pair<iterator, bool> insert(const T& value) {
		auto it = std::lower_bound(data_.begin(), data_.end(), value, comp_);
		if (it != data_.end() && !comp_(value, *it))
			return { it, false }; // already exists
		it = data_.insert(it, value);
		return { it, true };
	}

	std::pair<iterator, bool> insert(T&& value) {
		auto it = std::lower_bound(data_.begin(), data_.end(), value, comp_);
		if (it != data_.end() && !comp_(value, *it))
			return { it, false }; // already exists
		it = data_.insert(it, std::move(value));
		return { it, true };
	}

	std::pair<iterator, bool> insert(iterator hint, const T& value)
	{
		// ignore hint for correctness, find proper position
		auto it = std::lower_bound(data_.begin(), data_.end(), value, comp_);
		if (it != data_.end() && !comp_(value, *it))
			return { it, false }; // already exists
		it = data_.insert(it, value);
		return { it, true };
	}

	std::pair<iterator, bool> insert(iterator hint, T&& value)
	{
		auto it = std::lower_bound(data_.begin(), data_.end(), value, comp_);
		if (it != data_.end() && !comp_(value, *it))
			return { it, false };
		it = data_.insert(it, std::move(value));
		return { it, true };
	}

	template <typename InputIt>
	void insert(InputIt first, InputIt last)
	{
		for (; first != last; ++first)
		{
			insert(*first);
		}
	}

	template <typename... Args>
	std::pair<iterator, bool> emplace_hint(iterator /*hint*/, Args&&... args) {
		T value(std::forward<Args>(args)...);
		auto it = std::lower_bound(data_.begin(), data_.end(), value, comp_);
		if (it != data_.end() && !comp_(value, *it)) { return { it, false }; }
		it = data_.insert(it, std::move(value));
		return { it, true };
	}

	template <typename... Args>
	std::pair<iterator, bool> emplace(Args&&... args) {
		T value(std::forward<Args>(args)...);
		auto it = std::lower_bound(data_.begin(), data_.end(), value, comp_);
		if (it != data_.end() && !comp_(value, *it)) { return { it, false }; }
		it = data_.insert(it, std::move(value));
		return { it, true };
	}

	size_type erase(const T& value) {
		auto it = find(value);
		if (it == data_.end())
			return 0;
		data_.erase(it);
		return 1;
	}

	void erase(iterator pos) {
		data_.erase(pos);
	}

	// lookup
	size_type count(const T& value) const {
		return contains(value) ? 1 : 0;
	}

	iterator find(const T& value) {
		auto it = std::lower_bound(data_.begin(), data_.end(), value, comp_);
		if (it != data_.end() && !comp_(value, *it))
			return it;
		return data_.end();
	}

	const_iterator find(const T& value) const{
		auto it = std::lower_bound(data_.begin(), data_.end(), value, comp_);
		if (it != data_.end() && !comp_(value, *it))
			return it;
		return data_.end();
	}

	template <typename Func>
	bool remove_all_if(Func&& act) {
		auto new_end = std::remove_if(data_.begin(), data_.end(), act);

		// Count how many items are removed
		const size_type removed_count = std::distance(new_end, data_.end());

		// Actually erase them from the vector
		data_.erase(new_end, data_.end());

		return removed_count;
	}

	bool contains(const T& value) const {
		return find(value) != data_.end();
	}

	iterator lower_bound(const T& value) {
		return std::lower_bound(data_.begin(), data_.end(), value, comp_);
	}

	const_iterator lower_bound(const T& value) const {
		return std::lower_bound(data_.begin(), data_.end(), value, comp_);
	}

	iterator upper_bound(const T& value) {
		return std::upper_bound(data_.begin(), data_.end(), value, comp_);
	}

	const_iterator upper_bound(const T& value) const {
		return std::upper_bound(data_.begin(), data_.end(), value, comp_);
	}

	std::pair<iterator, iterator> equal_range(const T& value) {
		return std::equal_range(data_.begin(), data_.end(), value, comp_);
	}

	std::pair<const_iterator, const_iterator> equal_range(const T& value) const {
		return std::equal_range(data_.begin(), data_.end(), value, comp_);
	}

	void InvalidatePointer(void* ptr, bool removed) {
		if (std::is_pointer<T>::value) {
			if (removed) {
				this->erase((T)ptr);
			}
		}
	}

	bool load(PhobosStreamReader& Stm, bool RegisterForChange) {
		this->clear();

		size_t size = 0;
		auto ret = Stm.Load(size);

		if (ret && size) {
			//Debug::LogInfo("Loading VectorSet of {} , before loading the changes !" , size);

			data_.resize(size); // no default-init

			for (size_t i = 0; i < size; ++i) {
				//Debug::LogInfo("Loading VectorSet items at {} - value before {} !" , i , (unsigned)data_[i]);

				if (!Savegame::ReadPhobosStream(Stm, data_[i], RegisterForChange)) {
					return false;
				}

				//Debug::LogInfo("Loading VectorSet items at {} - value after {} !", i, (unsigned)data_[i]);
			}
		}

		return ret;
	}

	bool save(PhobosStreamWriter& Stm) const {
		Stm.Save(this->data_.size());
		//Debug::LogInfo("Saving VectorSet of {} !", this->data_.size());

		for (size_t i = 0; i < this->data_.size(); ++i) {
			//Debug::LogInfo("Saving VectorSet items at {} - value before {} !", i, (unsigned)data_[i]);
			if (!Savegame::WritePhobosStream(Stm, this->data_[i])) {
				return false;
			}
		}

		return true;
	}

private:
	container_type data_;
	Compare comp_;
};
