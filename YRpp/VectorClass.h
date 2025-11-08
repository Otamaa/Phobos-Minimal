#pragma once

#include <Memory.h>
#include <Helpers/Concepts.h>
#include <YRPPCore.h>

#include <algorithm>
#include <stdexcept>

//========================================================================
//=== VectorClass ========================================================
//========================================================================

enum class ArrayType : int
{
	Vector, DynamicVector, TypeList, Counter
};

template <typename T>
class VectorClass
{
public:
	// Type aliases for STL compatibility
	using value_type = T;
	using size_type = int;
	using difference_type = std::ptrdiff_t;
	using reference = T&;
	using const_reference = const T&;
	using pointer = T*;
	using const_pointer = const T*;

	// Iterator adapter class
	template<typename ValueType>
	class iterator_base
	{
	public:
		using iterator_category = std::random_access_iterator_tag;
		using value_type = ValueType;
		using difference_type = std::ptrdiff_t;
		using pointer = ValueType*;
		using reference = ValueType&;

		constexpr iterator_base() noexcept : ptr(nullptr) { }
		constexpr explicit iterator_base(pointer p) noexcept : ptr(p) { }

		reference operator*() const noexcept { return *ptr; }
		pointer operator->() const noexcept { return ptr; }
		reference operator[](difference_type n) const noexcept { return ptr[n]; }

		iterator_base& operator++() noexcept { ++ptr; return *this; }
		iterator_base operator++(int) noexcept { iterator_base tmp = *this; ++ptr; return tmp; }
		iterator_base& operator--() noexcept { --ptr; return *this; }
		iterator_base operator--(int) noexcept { iterator_base tmp = *this; --ptr; return tmp; }

		iterator_base& operator+=(difference_type n) noexcept { ptr += n; return *this; }
		iterator_base& operator-=(difference_type n) noexcept { ptr -= n; return *this; }

		friend iterator_base operator+(iterator_base it, difference_type n) noexcept { return iterator_base(it.ptr + n); }
		friend iterator_base operator+(difference_type n, iterator_base it) noexcept { return iterator_base(it.ptr + n); }
		friend iterator_base operator-(iterator_base it, difference_type n) noexcept { return iterator_base(it.ptr - n); }
		friend difference_type operator-(const iterator_base& lhs, const iterator_base& rhs) noexcept { return lhs.ptr - rhs.ptr; }

		friend bool operator==(const iterator_base& lhs, const iterator_base& rhs) noexcept { return lhs.ptr == rhs.ptr; }
		friend bool operator!=(const iterator_base& lhs, const iterator_base& rhs) noexcept { return lhs.ptr != rhs.ptr; }
		friend bool operator<(const iterator_base& lhs, const iterator_base& rhs) noexcept { return lhs.ptr < rhs.ptr; }
		friend bool operator<=(const iterator_base& lhs, const iterator_base& rhs) noexcept { return lhs.ptr <= rhs.ptr; }
		friend bool operator>(const iterator_base& lhs, const iterator_base& rhs) noexcept { return lhs.ptr > rhs.ptr; }
		friend bool operator>=(const iterator_base& lhs, const iterator_base& rhs) noexcept { return lhs.ptr >= rhs.ptr; }

	private:
		pointer ptr;
	};

	using iterator = iterator_base<T>;
	using const_iterator = iterator_base<const T>;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	static_assert(!needs_vector_delete<T>::value || (__alignof(T) <= 4),
		"Alignment of T needs to be less than or equal to 4.");
	static constexpr ArrayType Type = ArrayType::Vector;

	constexpr VectorClass() noexcept = default;

	explicit VectorClass(int capacity, T* pMem = nullptr)
	{
		if (capacity != 0)
		{
			this->Capacity = capacity;

			if (pMem)
			{
				this->Items = pMem;
			}
			else
			{
				this->Items = GameCreateArray<T>(static_cast<size_t>(capacity));
				this->IsAllocated = true;
			}
		}
	}

	VectorClass(const VectorClass<T>& other)
	{
		if (other.Capacity > 0)
		{
			this->Items = GameCreateArray<T>(static_cast<size_t>(other.Capacity));
			this->IsAllocated = true;
			this->Capacity = other.Capacity;
			std::copy(other.Items, other.Items + other.Capacity, this->Items);
		}
	}

	VectorClass(VectorClass<T>&& other) noexcept :
		Items(other.Items),
		Capacity(other.Capacity),
		IsInitialized(other.IsInitialized),
		IsAllocated(std::exchange(other.IsAllocated, false))
	{ }

	virtual ~VectorClass() noexcept
	{
		if (this->Items && this->IsAllocated)
		{
			GameDeleteArray(this->Items, static_cast<size_t>(this->Capacity));
		}
	}

	VectorClass<T>& operator=(const VectorClass<T>& other)
	{
		if (this != &other)
		{
			VectorClass<T>(other).swap(*this);
		}
		return *this;
	}

	VectorClass<T>& operator=(VectorClass<T>&& other) noexcept
	{
		if (this != &other)
		{
			VectorClass<T>(std::move(other)).swap(*this);
		}
		return *this;
	}

	virtual bool operator==(const VectorClass<T>& other) const
	{
		if (this->Capacity != other.Capacity)
		{
			return false;
		}

		return std::equal(this->Items, this->Items + this->Capacity, other.Items);
	}

	bool operator!=(const VectorClass<T>& other) const
	{
		return !(*this == other);
	}

	reference operator[](int i) noexcept
	{
		return this->Items[i];
	}

	const_reference operator[](int i) const noexcept
	{
		return this->Items[i];
	}

	reference at(int i)
	{
		if (i < 0 || i >= this->Capacity)
		{
			throw std::out_of_range("VectorClass::at: index out of range");
		}
		return this->Items[i];
	}

	const_reference at(int i) const
	{
		if (i < 0 || i >= this->Capacity)
		{
			throw std::out_of_range("VectorClass::at: index out of range");
		}
		return this->Items[i];
	}

	// STL-style interface
	bool reserve(int capacity)
	{
		if (!this->IsInitialized)
		{
			return false;
		}

		if (this->Capacity >= capacity)
		{
			return true;
		}

		return set_capacity_impl(capacity, nullptr);
	}

	FORCEDINLINE void clear()
	{
		VectorClass<T>(std::move(*this));
		this->Items = nullptr;
		this->Capacity = 0;
	}

	[[nodiscard]] int find(const T& item) const
	{
		if (this->IsInitialized)
		{
			auto it = std::find(this->Items, this->Items + this->Capacity, item);
			if (it != this->Items + this->Capacity)
			{
				return static_cast<int>(it - this->Items);
			}
		}
		return -1;
	}

	[[nodiscard]] int index_of(const T* pItem) const
	{
		if (!this->IsInitialized) {
			return -1;
		}
		return static_cast<int>(pItem - this->Items);
	}

	void swap(VectorClass<T>& other) noexcept
	{
		using std::swap;
		swap(this->Items, other.Items);
		swap(this->Capacity, other.Capacity);
		swap(this->IsInitialized, other.IsInitialized);
		swap(this->IsAllocated, other.IsAllocated);
	}

	// Iterator methods
	iterator begin() noexcept { return iterator(this->Items); }
	const_iterator begin() const noexcept { return const_iterator(this->Items); }
	const_iterator cbegin() const noexcept { return const_iterator(this->Items); }

	iterator end() noexcept { return iterator(this->Items + this->Capacity); }
	const_iterator end() const noexcept { return const_iterator(this->Items + this->Capacity); }
	const_iterator cend() const noexcept { return const_iterator(this->Items + this->Capacity); }

	reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
	const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
	const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(end()); }

	reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
	const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }
	const_reverse_iterator crend() const noexcept { return const_reverse_iterator(begin()); }

	pointer data() noexcept { return this->Items; }
	const_pointer data() const noexcept { return this->Items; }

	size_type capacity() const noexcept { return this->Capacity; }
	[[nodiscard]] bool empty() const noexcept { return this->Capacity == 0; }

	// Legacy interface (deprecated - kept for reference during migration)
	// bool SetCapacity(int capacity, T* pMem = nullptr) { return set_capacity_impl(capacity, pMem); }
	// void Clear() { clear_impl(); }
	// int FindItemIndex(const T& item) const { return find_item_index_impl(item); }
	// int GetItemIndex(const T* pItem) const { return get_item_index_impl(pItem); }
	// T GetItem(int i) const { return this->Items[i]; }
	// bool Reserve(int capacity) { return reserve(capacity); }
	// void Swap(VectorClass<T>& other) noexcept { swap(other); }

	T* Items { nullptr };
	int Capacity { 0 };
	bool IsInitialized { true };
	bool IsAllocated { false };

	FORCEDINLINE bool set_capacity(int capacity, T* pMem) {
		if (capacity > 0)
		{
			this->IsInitialized = false;

			bool bMustAllocate = (pMem == nullptr);
			if (!pMem)
			{
				pMem = GameCreateArray<T>(static_cast<size_t>(capacity));
			}

			this->IsInitialized = true;

			if (!pMem)
			{
				return false;
			}

			if (this->Items)
			{
				auto n = std::min(capacity, this->Capacity);
				std::move(this->Items, this->Items + n, pMem);

				if (this->IsAllocated)
				{
					GameDeleteArray(this->Items, static_cast<size_t>(this->Capacity));
					this->Items = nullptr;
				}
			}

			this->IsAllocated = bMustAllocate;
			this->Items = pMem;
			this->Capacity = capacity;
		}
		else
		{
			clear();
		}
		return true;
	}

private: // function below is not used direcly , we dont want to do vtable calling unless nessessarry

	virtual bool set_capacity_impl(int capacity, T* pMem = nullptr)
	{
		if (capacity > 0)
		{
			this->IsInitialized = false;

			bool bMustAllocate = (pMem == nullptr);
			if (!pMem)
			{
				pMem = GameCreateArray<T>(static_cast<size_t>(capacity));
			}

			this->IsInitialized = true;

			if (!pMem)
			{
				return false;
			}

			if (this->Items)
			{
				auto n = std::min(capacity, this->Capacity);
				std::move(this->Items, this->Items + n, pMem);

				if (this->IsAllocated)
				{
					GameDeleteArray(this->Items, static_cast<size_t>(this->Capacity));
					this->Items = nullptr;
				}
			}

			this->IsAllocated = bMustAllocate;
			this->Items = pMem;
			this->Capacity = capacity;
		}
		else
		{
			clear_impl();
		}
		return true;
	}

	virtual void clear_impl()
	{
		VectorClass<T>(std::move(*this));
		this->Items = nullptr;
		this->Capacity = 0;
	}

	virtual int find_item_index_impl(const T& item) const
	{
		if (this->IsInitialized)
		{
			auto it = std::find(this->Items, this->Items + this->Capacity, item);
			if (it != this->Items + this->Capacity)
			{
				return static_cast<int>(it - this->Items);
			}
		}
		return -1;
	}

	virtual int get_item_index_impl(const T* pItem) const final
	{
		if (!this->IsInitialized)
		{
			return -1;
		}
		return static_cast<int>(pItem - this->Items);
	}

	virtual T get_item_impl(int i) const final {
		return this->Items[i];
	}
};

// Swap specialization for ADL
template <typename T>
void swap(VectorClass<T>& lhs, VectorClass<T>& rhs) noexcept
{
	lhs.swap(rhs);
}