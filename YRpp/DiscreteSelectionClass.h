/*
	A helper to randomly select from many equally good items with the best
	rating.

	Add one or more items you want to select from, giving each a rating value.
	The instance is able to select values if there is at least one item with
	a rating that compares equal to or better to the initial value. Items with
	a worse rating than the best are discarded.

	You can select multiple items using the same instance of this class, the
	probabilities of items to be selected will not change. Valid values for
	selection are 0..GetCount()-1. The select function will return true if and
	only if the instance is set up correctly and an item could be selected. The
	out parameter is written to if and only if an item could be selected.
*/

#pragma once

#include <ArrayClasses.h>
#include <Randomizer.h>

#include <utility>

template <typename T, typename Container = DynamicVectorClass<T>>
class DiscreteSelectionClass
{
public:
	DiscreteSelectionClass() = default;
	explicit DiscreteSelectionClass(int initial) : Rating(initial) { }

	// Add item with rating
	void add(T value, int rating)
	{
		if (this->Rating > rating)
		{
			return;
		}

		if (rating > this->Rating)
		{
			this->clear();
			this->Rating = rating;
		}

		add_item_impl(std::move(value));
	}

	// Clear all items
	void clear()
	{
		clear_impl();
		// Don't reset Rating - it represents the best rating seen
	}

	// Reset both items and rating
	void reset(int initial_rating = 0)
	{
		clear_impl();
		this->Rating = initial_rating;
	}

	[[nodiscard]] int rating() const noexcept
	{
		return this->Rating;
	}

	[[nodiscard]] std::size_t size() const noexcept
	{
		return size_impl();
	}

	[[nodiscard]] bool empty() const noexcept
	{
		return size() == 0;
	}

	[[nodiscard]] bool is_valid() const noexcept
	{
		return !empty();
	}

	// Select by index with output parameter
	bool select(std::size_t index, T* pOut) const
	{
		if (index < size())
		{
			if (pOut)
			{
				*pOut = get_item_impl(index);
			}
			return true;
		}
		return false;
	}

	// Select by index returning optional
	[[nodiscard]] std::optional<T> select(std::size_t index) const
	{
		if (index < size())
		{
			return get_item_impl(index);
		}
		return std::nullopt;
	}

	// Random selection with output parameter
	bool select(Random2Class& random, T* pOut) const
	{
		if (!is_valid())
		{
			return false;
		}

		auto value = random.RandomRanged(0, static_cast<int>(size()) - 1);
		return select(static_cast<std::size_t>(value), pOut);
	}

	// Random selection returning optional
	[[nodiscard]] std::optional<T> select(Random2Class& random) const
	{
		if (!is_valid())
		{
			return std::nullopt;
		}

		auto value = random.RandomRanged(0, static_cast<int>(size()) - 1);
		return select(static_cast<std::size_t>(value));
	}

	// Select with default value (legacy compatibility)
	// BUG FIX: Original code modified the default parameter!
	[[nodiscard]] T select_or(std::size_t index, T default_value) const
	{
		auto result = select(index);
		return result ? std::move(*result) : std::move(default_value);
	}

	// Random select with default value (legacy compatibility)
	// BUG FIX: Original code modified the default parameter!
	[[nodiscard]] T select_or(Random2Class& random, T default_value) const
	{
		auto result = select(random);
		return result ? std::move(*result) : std::move(default_value);
	}

	// Iterator support for range-based for loops
	auto begin() const { return Items.begin(); }
	auto end() const { return Items.end(); }
	auto begin() { return Items.begin(); }
	auto end() { return Items.end(); }

	// Legacy interface (commented for migration reference)
	/*
	void Add(T value, int rating) { add(std::move(value), rating); }
	void Clear() { clear(); }
	int GetRating() const { return rating(); }
	int GetCount() const { return static_cast<int>(size()); }
	bool IsValid() const { return is_valid(); }
	bool Select(int index, T* pOut) const { return select(static_cast<std::size_t>(index), pOut); }
	bool Select(Randomizer& random, T* pOut) const { return select(random, pOut); }
	T Select(int index, T default_value = T()) const { return select_or(static_cast<std::size_t>(index), std::move(default_value)); }
	T Select(Randomizer& random, T default_value = T()) const { return select_or(random, std::move(default_value)); }
	*/

private:
	void add_item_impl(T value)
	{
		Items.push_back(std::move(value));
	}

	void clear_impl()
	{
		Items.clear();
	}

	[[nodiscard]] std::size_t size_impl() const noexcept
	{
		return Items.size();
	}

	[[nodiscard]] const T& get_item_impl(std::size_t index) const
	{
		return Items[static_cast<int>(index)];
	}

	Container Items {};
	int Rating { 0 };
};

// Type aliases for common uses
template<typename T>
using DiscreteSelection = DiscreteSelectionClass<T, DynamicVectorClass<T>>;

template<typename T>
using StdDiscreteSelection = DiscreteSelectionClass<T, std::vector<T>>;