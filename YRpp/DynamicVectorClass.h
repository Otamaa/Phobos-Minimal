#pragma once

#include <VectorClass.h>

//========================================================================
//=== DynamicVectorClass =================================================
//========================================================================

template <typename T>
class DynamicVectorClass : public VectorClass<T>
{
public:
	// Type aliases inherited/redefined for clarity
	using typename VectorClass<T>::value_type;
	using typename VectorClass<T>::size_type;
	using typename VectorClass<T>::reference;
	using typename VectorClass<T>::const_reference;
	using typename VectorClass<T>::pointer;
	using typename VectorClass<T>::const_pointer;
	using typename VectorClass<T>::iterator;
	using typename VectorClass<T>::const_iterator;
	using typename VectorClass<T>::reverse_iterator;
	using typename VectorClass<T>::const_reverse_iterator;

	static constexpr ArrayType Type = ArrayType::DynamicVector;

	constexpr DynamicVectorClass() noexcept = default;

	explicit DynamicVectorClass(int capacity, T* pMem = nullptr)
		: VectorClass<T>(capacity, pMem)
	{ }

	DynamicVectorClass(const DynamicVectorClass<T>& other)
	{
		if (other.Capacity > 0)
		{
			this->Items = GameCreateArray<T>(static_cast<size_t>(other.Capacity));
			this->IsAllocated = true;
			this->Capacity = other.Capacity;
			this->Count = other.Count;
			this->CapacityIncrement = other.CapacityIncrement;
			std::copy(other.Items, other.Items + other.Count, this->Items);
		}
	}

	DynamicVectorClass(DynamicVectorClass<T>&& other) noexcept
		: VectorClass<T>(std::move(other)),
		Count(other.Count),
		CapacityIncrement(other.CapacityIncrement)
	{ }

	// Initializer list constructor
	DynamicVectorClass(std::initializer_list<T> init)
		: VectorClass<T>(static_cast<int>(init.size()))
	{
		this->Count = static_cast<int>(init.size());
		this->CapacityIncrement = 10;
		std::copy(init.begin(), init.end(), this->Items);
	}

	DynamicVectorClass<T>& operator=(const DynamicVectorClass<T>& other)
	{
		if (this != &other)
		{
			DynamicVectorClass<T>(other).swap(*this);
		}
		return *this;
	}

	DynamicVectorClass<T>& operator=(DynamicVectorClass<T>&& other) noexcept
	{
		if (this != &other)
		{
			DynamicVectorClass<T>(std::move(other)).swap(*this);
		}
		return *this;
	}

	DynamicVectorClass<T>& operator=(std::initializer_list<T> init)
	{
		DynamicVectorClass<T>(init).swap(*this);
		return *this;
	}

	// STL-style interface
	FORCEDINLINE void clear()
	{
		VectorClass<T>::clear();
		this->Count = 0;
	}

	[[nodiscard]] int find(const T& item) const
	{
		if (!this->IsInitialized)
		{
			return -1;
		}

		auto it = std::find(this->Items, this->Items + this->Count, item);
		return (it != this->Items + this->Count) ? static_cast<int>(it - this->Items) : -1;
	}

	[[nodiscard]] constexpr bool valid_index(int index) const noexcept
	{
		return static_cast<size_t>(index) < static_cast<size_t>(this->Count);
	}

	T get_or_default(int i) const
	{
		return get_or_default(i, T());
	}

	T get_or_default(int i, T def) const
	{
		return valid_index(i) ? this->Items[i] : def;
	}

	// Override iterator methods to use Count instead of Capacity
	iterator begin() noexcept { return iterator(this->Items); }
	const_iterator begin() const noexcept { return const_iterator(this->Items); }
	const_iterator cbegin() const noexcept { return const_iterator(this->Items); }

	iterator end() noexcept { return iterator(this->Items + this->Count); }
	const_iterator end() const noexcept { return const_iterator(this->Items + this->Count); }
	const_iterator cend() const noexcept { return const_iterator(this->Items + this->Count); }

	reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
	const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
	const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(end()); }

	reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
	const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }
	const_reverse_iterator crend() const noexcept { return const_reverse_iterator(begin()); }

	reference front()
	{
		if (this->Count <= 0) throw std::out_of_range("front() on empty vector");
		return this->Items[0];
	}

	const_reference front() const
	{
		if (this->Count <= 0) throw std::out_of_range("front() on empty vector");
		return this->Items[0];
	}

	reference back()
	{
		if (this->Count <= 0) throw std::out_of_range("back() on empty vector");
		return this->Items[this->Count - 1];
	}

	const_reference back() const
	{
		if (this->Count <= 0) throw std::out_of_range("back() on empty vector");
		return this->Items[this->Count - 1];
	}

	[[nodiscard]] constexpr size_t size() const noexcept
	{
		return static_cast<size_t>(Count);
	}

	[[nodiscard]] constexpr bool empty() const noexcept
	{
		return Count <= 0;
	}

	// Reset without destroying memory
	constexpr void reset(int resetCount = 0) noexcept
	{
		this->Count = (resetCount < this->Capacity && resetCount > 0) ? resetCount : 0;
	}

	[[nodiscard]] constexpr bool contains(const T& item) const
	{
		if (this->Count <= 0)
		{
			return false;
		}
		return std::find(begin(), end(), item) != end();
	}

	bool push_back(T item)
	{
		if (this->Count >= this->Capacity)
		{
			if (!this->IsAllocated && this->Capacity != 0)
			{
				return false;
			}

			if (this->CapacityIncrement <= 0)
			{
				return false;
			}

			if (!this->set_capacity(this->Capacity + this->CapacityIncrement, nullptr))
			{
				return false;
			}
		}

		this->Items[Count++] = std::move_if_noexcept(item);
		return true;
	}

	template <class... Args>
	reference emplace_back(Args&&... args)
	{
		if (!ensure_valid_array())
		{
			throw std::runtime_error("Failed to allocate space for emplace_back");
		}
		// Placement new just triggers constructor on already allocated memory
		::new (static_cast<void*>(&this->Items[Count])) T(std::forward<Args>(args)...);
		++Count;
		return this->Items[Count - 1];
	}

	bool insert_unique(const T& item)
	{
		int idx = find(item);
		return idx < 0 && push_back(item);
	}

	// Insert item using lower_bound (maintains sorted order)
	bool insert_sorted(T item)
	{
		if (!ensure_valid_array())
		{
			return false;
		}

		auto pos = std::lower_bound(begin(), end(), item);
		int index = static_cast<int>(pos - begin());

		// Shift elements to make room
		std::move_backward(this->Items + index, this->Items + this->Count, this->Items + this->Count + 1);
		this->Items[index] = std::move_if_noexcept(item);
		++this->Count;
		return true;
	}

	// Insert item using lower_bound with custom comparator
	template<typename Compare>
	bool insert_sorted(T item, Compare comp)
	{
		if (!ensure_valid_array())
		{
			return false;
		}

		auto pos = std::lower_bound(begin(), end(), item, comp);
		int index = static_cast<int>(pos - begin());

		std::move_backward(this->Items + index, this->Items + this->Count, this->Items + this->Count + 1);
		this->Items[index] = std::move_if_noexcept(item);
		++this->Count;
		return true;
	}

	// Insert item using upper_bound (allows duplicates after existing equal elements)
	bool insert_upper(T item)
	{
		if (!ensure_valid_array())
		{
			return false;
		}

		auto pos = std::upper_bound(begin(), end(), item);
		int index = static_cast<int>(pos - begin());

		std::move_backward(this->Items + index, this->Items + this->Count, this->Items + this->Count + 1);
		this->Items[index] = std::move_if_noexcept(item);
		++this->Count;
		return true;
	}

	// Insert item using upper_bound with custom comparator
	template<typename Compare>
	bool insert_upper(T item, Compare comp)
	{
		if (!ensure_valid_array())
		{
			return false;
		}

		auto pos = std::upper_bound(begin(), end(), item, comp);
		int index = static_cast<int>(pos - begin());

		std::move_backward(this->Items + index, this->Items + this->Count, this->Items + this->Count + 1);
		this->Items[index] = std::move_if_noexcept(item);
		++this->Count;
		return true;
	}

	// Insert unique item using lower_bound (only if not already present in sorted range)
	bool insert_sorted_unique(T item)
	{
		if (!ensure_valid_array())
		{
			return false;
		}

		auto pos = std::lower_bound(begin(), end(), item);

		// Check if item already exists
		if (pos != end() && *pos == item)
		{
			return false; // Item already exists
		}

		int index = static_cast<int>(pos - begin());
		std::move_backward(this->Items + index, this->Items + this->Count, this->Items + this->Count + 1);
		this->Items[index] = std::move_if_noexcept(item);
		++this->Count;
		return true;
	}

	// Insert unique item using lower_bound with custom comparator
	template<typename Compare>
	bool insert_sorted_unique(T item, Compare comp)
	{
		if (!ensure_valid_array())
		{
			return false;
		}

		auto pos = std::lower_bound(begin(), end(), item, comp);

		// Check if item already exists
		if (pos != end() && !comp(item, *pos) && !comp(*pos, item))
		{
			return false; // Item already exists (equivalent elements)
		}

		int index = static_cast<int>(pos - begin());
		std::move_backward(this->Items + index, this->Items + this->Count, this->Items + this->Count + 1);
		this->Items[index] = std::move_if_noexcept(item);
		++this->Count;
		return true;
	}

	bool erase_at(int index)
	{
		if (!valid_index(index))
		{
			return false;
		}

		--this->Count;
		std::move(this->Items + index + 1, this->Items + this->Count + 1, this->Items + index);
		return true;
	}

	// Pop back
	void pop_back()
	{
		if (this->Count > 0)
		{
			--this->Count;
		}
	}

	template<typename Func>
	bool erase_if(Func&& act)
	{
		if (!this->IsAllocated)
			return false;

		auto newEnd = std::remove_if(begin(), end(), std::forward<Func>(act));
		this->Count = static_cast<int>(std::distance(begin(), newEnd));
		return true;
	}

	bool erase(const T& item)
	{
		int idx = find(item);
		return idx >= 0 && erase_at(idx);
	}

	// Erase by iterator
	iterator erase(iterator pos)
	{
		if (pos >= begin() && pos < end())
		{
			int index = static_cast<int>(pos - begin());
			erase_at(index);
			return iterator(this->Items + index);
		}
		return end();
	}

	iterator erase(iterator first, iterator last)
	{
		if (first >= begin() && last <= end() && first < last)
		{
			int startIdx = static_cast<int>(first - begin());
			int endIdx = static_cast<int>(last - begin());
			int count = endIdx - startIdx;

			std::move(this->Items + endIdx, this->Items + this->Count, this->Items + startIdx);
			this->Count -= count;
			return iterator(this->Items + startIdx);
		}
		return end();
	}

	void swap(DynamicVectorClass& other) noexcept
	{
		VectorClass<T>::swap(other);
		using std::swap;
		swap(this->Count, other.Count);
		swap(this->CapacityIncrement, other.CapacityIncrement);
	}

	pointer find_ptr(const T& item) const
	{
		if (this->Count <= 0)
		{
			return nullptr;
		}
		auto it = std::find(begin(), end(), item);
		return const_cast<const pointer>((it != end()) ? &(*it) : nullptr);
	}

	// STL-style algorithms
	template <typename Func>
	auto find_if(Func&& act) const
	{
		return this->Count > 0 ? std::find_if(begin(), end(), std::forward<Func>(act)) : end();
	}

	template <typename Func>
	auto find_if(Func&& act)
	{
		return this->Count > 0 ? std::find_if(begin(), end(), std::forward<Func>(act)) : end();
	}

	template <typename Func>
	void for_each(Func&& act) const
	{
		if (this->Count > 0)
		{
			std::for_each(begin(), end(), std::forward<Func>(act));
		}
	}

	template <typename Func>
	void for_each(Func&& act)
	{
		if (this->Count > 0)
		{
			std::for_each(begin(), end(), std::forward<Func>(act));
		}
	}

	template<typename Func>
	[[nodiscard]] bool none_of(Func&& fn) const
	{
		return this->Count <= 0 || std::none_of(begin(), end(), std::forward<Func>(fn));
	}

	template<typename Func>
	[[nodiscard]] bool none_of(Func&& fn)
	{
		return this->Count <= 0 || std::none_of(begin(), end(), std::forward<Func>(fn));
	}

	template<typename Func>
	[[nodiscard]] bool any_of(Func&& fn) const
	{
		return this->Count > 0 && std::any_of(begin(), end(), std::forward<Func>(fn));
	}

	template<typename Func>
	[[nodiscard]] bool any_of(Func&& fn)
	{
		return this->Count > 0 && std::any_of(begin(), end(), std::forward<Func>(fn));
	}

	template<typename Func>
	[[nodiscard]] bool all_of(Func&& fn) const
	{
		return this->Count <= 0 || std::all_of(begin(), end(), std::forward<Func>(fn));
	}

	template<typename Func>
	[[nodiscard]] bool all_of(Func&& fn)
	{
		return this->Count <= 0 || std::all_of(begin(), end(), std::forward<Func>(fn));
	}

	[[nodiscard]] bool Empty() const { return empty(); }
	bool IsValidArray() { return ensure_valid_array(); }

	FORCEDINLINE bool set_capacity(int capacity, T* pMem = nullptr)
	{
		bool bRet = VectorClass<T>::set_capacity(capacity, pMem);

		if (this->Capacity < this->Count)
		{
			this->Count = this->Capacity;
		}

		return bRet;
	}

	/*
	// Legacy interface (deprecated - kept for reference during migration)
	// Use modern STL-style methods instead:
	//
	// SetCapacity(cap, ptr)       → reserve(cap) or set_capacity_impl(cap, ptr)
	// Clear()                     → clear()
	// FindItemIndex(item)         → find(item)
	// ValidIndex(idx)             → valid_index(idx)
	// GetItemOrDefault(i)         → get_or_default(i)
	// GetItemOrDefault(i, def)    → get_or_default(i, def)
	// Reset(count)                → reset(count)
	// Contains(item)              → contains(item)
	// AddItem(item)               → push_back(item)
	// AddUnique(item)             → insert_unique(item)
	// RemoveAt(idx)               → erase_at(idx)
	// Remove(item)                → erase(item)
	// remove_if(fn)               → erase_if(fn)
	// Swap(other)                 → swap(other)
	// Find(item)                  → find_ptr(item)
	// Empty()                     → empty()
	// IsValidArray()              → ensure_valid_array()
	// InsertSorted(item)          → insert_sorted(item)
	// InsertUpper(item)           → insert_upper(item)
	// InsertSortedUnique(item)    → insert_sorted_unique(item)
	*/

	int Count { 0 };
	int CapacityIncrement { 10 };

private:
	virtual bool set_capacity_impl(int capacity, T* pMem = nullptr) override
	{
		bool bRet = VectorClass<T>::set_capacity(capacity, pMem);

		if (this->Capacity < this->Count)
		{
			this->Count = this->Capacity;
		}

		return bRet;
	}

	virtual void clear_impl() override
	{
		VectorClass<T>::clear();
		this->Count = 0;
	}

	virtual int find_item_index_impl(const T& item) const override final
	{
		if (!this->IsInitialized)
		{
			return -1;
		}

		auto it = std::find(this->Items, this->Items + this->Count, item);
		return (it != this->Items + this->Count) ? static_cast<int>(it - this->Items) : -1;
	}

	bool ensure_valid_array()
	{
		// ensure_valid_array already checks IsInitialized implicitly
		// because if not initialized, Items would be nullptr and Count >= Capacity
		if (this->Count >= this->Capacity)
		{
			if ((this->IsAllocated || !this->Capacity) && this->CapacityIncrement > 0)
			{
				return this->set_capacity(this->Capacity + this->CapacityIncrement, nullptr);
			}
			else
			{
				return false;
			}
		}
		return true;
	}
};

// Swap specialization for ADL
template <typename T>
void swap(DynamicVectorClass<T>& lhs, DynamicVectorClass<T>& rhs) noexcept
{
	lhs.swap(rhs);
}