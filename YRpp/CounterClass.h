#pragma once

#include <DynamicVectorClass.h>
#include <numeric>

//========================================================================
//=== CounterClass =======================================================
//========================================================================

class CounterClass : public VectorClass<int>
{
public:
	// Type aliases for STL compatibility
	using typename VectorClass<int>::value_type;
	using typename VectorClass<int>::size_type;
	using typename VectorClass<int>::reference;
	using typename VectorClass<int>::const_reference;
	using typename VectorClass<int>::pointer;
	using typename VectorClass<int>::const_pointer;
	using typename VectorClass<int>::iterator;
	using typename VectorClass<int>::const_iterator;
	using typename VectorClass<int>::reverse_iterator;
	using typename VectorClass<int>::const_reverse_iterator;

	static constexpr ArrayType Type = ArrayType::Counter;

	constexpr CounterClass() noexcept = default;

	explicit CounterClass(int capacity)
		: VectorClass<int>(capacity)
	{
		if (capacity > 0)
		{
			std::fill(this->Items, this->Items + capacity, 0);
		}
	}

	CounterClass(const CounterClass& other)
		: VectorClass<int>(other), Total(other.Total)
	{ }

	CounterClass(CounterClass&& other) noexcept
		: VectorClass<int>(std::move(other)), Total(other.Total)
	{ }

	// Initializer list constructor
	CounterClass(std::initializer_list<int> init)
		: VectorClass<int>(static_cast<int>(init.size()))
	{
		std::copy(init.begin(), init.end(), this->Items);
		Total = std::accumulate(init.begin(), init.end(), 0);
	}

	CounterClass& operator=(const CounterClass& other)
	{
		if (this != &other)
		{
			CounterClass(other).swap(*this);
		}
		return *this;
	}

	CounterClass& operator=(CounterClass&& other) noexcept
	{
		if (this != &other)
		{
			CounterClass(std::move(other)).swap(*this);
		}
		return *this;
	}

	CounterClass& operator=(std::initializer_list<int> init)
	{
		CounterClass(init).swap(*this);
		return *this;
	}

	int operator[](int index) const
	{
		return get_count(index);
	}

	// Non-const version for modification
	reference at(int index)
	{
		if (!ensure_item(index))
		{
			throw std::out_of_range("CounterClass::at: failed to ensure capacity");
		}
		return this->Items[index];
	}

	const_reference at(int index) const
	{
		if (index < 0 || index >= this->Capacity)
		{
			throw std::out_of_range("CounterClass::at: index out of range");
		}
		return this->Items[index];
	}

	// STL-style interface
	FORCEDINLINE void clear()
	{
		if (this->Items && this->Capacity > 0)
		{
			std::fill(this->Items, this->Items + this->Capacity, 0);
		}
		this->Total = 0;
	}

	[[nodiscard]] constexpr int total() const noexcept
	{
		return this->Total;
	}

	[[nodiscard]] int get_count(int index) const noexcept
	{
		return (static_cast<size_t>(index) < static_cast<size_t>(this->Capacity))
			? this->Items[index] : 0;
	}

	int increment(int index)
	{
		if (ensure_item(index))
		{
			++this->Total;
			return ++this->Items[index];
		}
		return 0;
	}

	int decrement(int index)
	{
		if (ensure_item(index))
		{
			--this->Total;
			return --this->Items[index];
		}
		return 0;
	}

	// Modern increment/decrement with amount
	int add(int index, int amount)
	{
		if (ensure_item(index) && amount != 0)
		{
			this->Total += amount;
			this->Items[index] += amount;
			return this->Items[index];
		}
		return 0;
	}

	int subtract(int index, int amount)
	{
		return add(index, -amount);
	}

	// Set a specific count
	bool set_count(int index, int value)
	{
		if (!ensure_item(index))
		{
			return false;
		}

		int diff = value - this->Items[index];
		this->Total += diff;
		this->Items[index] = value;
		return true;
	}

	// Reset all counts
	FORCEDINLINE void reset()
	{
		if (this->Items && this->Capacity > 0)
		{
			std::fill(this->Items, this->Items + this->Capacity, 0);
		}
		this->Total = 0;
	}

	// Get the index with maximum count
	[[nodiscard]] int max_index() const noexcept
	{
		if (this->Capacity <= 0)
		{
			return -1;
		}
		return static_cast<int>(std::max_element(this->Items, this->Items + this->Capacity) - this->Items);
	}

	// Get the index with minimum count
	[[nodiscard]] int min_index() const noexcept
	{
		if (this->Capacity <= 0)
		{
			return -1;
		}
		return static_cast<int>(std::min_element(this->Items, this->Items + this->Capacity) - this->Items);
	}

	// Count non-zero entries
	[[nodiscard]] int count_non_zero() const noexcept
	{
		if (this->Capacity <= 0)
		{
			return 0;
		}
		return static_cast<int>(std::count_if(this->Items, this->Items + this->Capacity,
			[](int val) { return val != 0; }));
	}

	void swap(CounterClass& other) noexcept
	{
		VectorClass<int>::swap(other);
		using std::swap;
		swap(this->Total, other.Total);
	}

	/*
	// Legacy interface (deprecated - kept for reference during migration)
	// Use modern STL-style methods instead:
	//
	// Clear()              → clear()
	// GetTotal()           → total()
	// EnsureItem(idx)      → ensure_item(idx) [private]
	// GetItemCount(idx)    → get_count(idx)
	// Increment(idx)       → increment(idx)
	// Decrement(idx)       → decrement(idx)
	// Add(idx, amt)        → add(idx, amt)
	// Subtract(idx, amt)   → subtract(idx, amt)
	// SetCount(idx, val)   → set_count(idx, val)
	// Reset()              → reset()
	// GetMaxIndex()        → max_index()
	// GetMinIndex()        → min_index()
	// CountNonZero()       → count_non_zero()
	// Swap(other)          → swap(other)
	*/

	int Total { 0 };

private:
	virtual void clear_impl() override
	{
		if (this->Items && this->Capacity > 0)
		{
			std::fill(this->Items, this->Items + this->Capacity, 0);
		}
		this->Total = 0;
	}

	bool ensure_item(int index)
	{
		if (index < 0)
		{
			return false;
		}

		if (index < this->Capacity)
		{
			return true;
		}

		int oldCapacity = this->Capacity;
		if (this->set_capacity(index + 10, nullptr))
		{
			// Zero-initialize new elements
			std::fill(this->Items + oldCapacity, this->Items + this->Capacity, 0);
			return true;
		}

		return false;
	}
};

// Swap specialization for ADL
inline void swap(CounterClass& lhs, CounterClass& rhs) noexcept
{
	lhs.swap(rhs);
}