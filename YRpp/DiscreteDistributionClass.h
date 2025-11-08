/*
	A helper to randomly select from many weighted items.

	Add one or more items with a weight of 0 or higher you want to select from.
	The instance is able to select values if there is at least one item with a
	with a weight of 1 or higher.

	You can select multiple items using the same instance of this class, the
	probabilities of items to be selected will not change. Valid values for
	selection are 1..GetTotalWeight(). The select function will return true if and
	only if the instance is set up correctly and an item could be selected. The
	out parameter is written to if and only if an item could be selected.
*/

#pragma once

#include <ArrayClasses.h>
#include <Randomizer.h>

#include <utility>
template <typename T>
struct DistributionObject
{
	DistributionObject() = default;
	explicit DistributionObject(T value, unsigned int weight = 1u)
		: Value(std::move(value)), Weight(weight) { }

	// BUG FIX: These should actually compare values, not always return false/true
	bool operator==(const DistributionObject<T>& rhs) const
	{
		return Weight == rhs.Weight && Value == rhs.Value;
	}

	bool operator!=(const DistributionObject<T>& rhs) const
	{
		return !(*this == rhs);
	}

	T Value {};
	unsigned int Weight { 0u };
};

template <typename T, typename Container>
class DiscreteDistributionClass
{
public:
	using distribution_object = DistributionObject<T>;

	DiscreteDistributionClass() = default;

	// Constructor for DynamicVectorClass compatibility
	explicit DiscreteDistributionClass(int capacity, distribution_object* pMem = nullptr)
		: Items(capacity, pMem) { }

	// Add distribution object directly
	void add(distribution_object item)
	{
		this->TotalWeight += item.Weight;
		add_item_impl(std::move(item));
	}

	// Add value with weight
	void add(T value, unsigned int weight = 1u)
	{
		distribution_object item(std::move(value), weight);
		add(std::move(item));
	}

	// Clear all items
	void clear()
	{
		this->TotalWeight = 0u;
		clear_impl();
	}

	[[nodiscard]] unsigned int total_weight() const noexcept
	{
		return this->TotalWeight;
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
		return this->TotalWeight > 0u && !empty();
	}

	// Select by weight value with output parameter
	bool select(unsigned int value, T* pOut) const
	{
		// BUG FIX: Original checked 'value && value <= TotalWeight'
		// but value of 0 is invalid anyway, and checking empty() is clearer
		if (!is_valid() || value == 0u || value > this->TotalWeight)
		{
			return false;
		}

		unsigned int acc = 0u;
		for (const auto& item : Items)
		{
			acc += item.Weight;

			if (acc >= value)
			{
				if (pOut)
				{
					*pOut = item.Value;
				}
				return true;
			}
		}

		// Should never reach here if TotalWeight is correct
		return false;
	}

	// Select by weight value returning optional
	[[nodiscard]] std::optional<T> select(unsigned int value) const
	{
		if (!is_valid() || value == 0u || value > this->TotalWeight)
		{
			return std::nullopt;
		}

		unsigned int acc = 0u;
		for (const auto& item : Items)
		{
			acc += item.Weight;

			if (acc >= value)
			{
				return item.Value;
			}
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

		// BUG FIX: RandomRanged uses inclusive range, so [1, TotalWeight] is correct
		auto value = random.RandomRanged(1, static_cast<int>(this->TotalWeight));
		return select(static_cast<unsigned int>(value), pOut);
	}

	// Random selection returning optional
	[[nodiscard]] std::optional<T> select(Random2Class& random) const
	{
		if (!is_valid())
		{
			return std::nullopt;
		}

		auto value = random.RandomRanged(1, static_cast<int>(this->TotalWeight));
		return select(static_cast<unsigned int>(value));
	}

	// Select with default value (legacy compatibility)
	// BUG FIX: Original code modified the default parameter!
	[[nodiscard]] T select_or(unsigned int value, T default_value) const
	{
		auto result = select(value);
		return result ? std::move(*result) : std::move(default_value);
	}

	// Random select with default value (legacy compatibility)
	// BUG FIX: Original code modified the default parameter!
	[[nodiscard]] T select_or(Random2Class& random, T default_value) const
	{
		auto result = select(random);
		return result ? std::move(*result) : std::move(default_value);
	}

	// Get item by index (for inspection)
	[[nodiscard]] std::optional<distribution_object> get_item(std::size_t index) const
	{
		if (index < size())
		{
			return Items[static_cast<int>(index)];
		}
		return std::nullopt;
	}

	// Iterator support for range-based for loops
	auto begin() const { return Items.begin(); }
	auto end() const { return Items.end(); }
	auto begin() { return Items.begin(); }
	auto end() { return Items.end(); }

	// Legacy interface (commented for migration reference)
	/*
	void Add(distribution_object item) { add(std::move(item)); }
	void Add(T value, unsigned int weight = 1u) { add(std::move(value), weight); }
	void Clear() { clear(); }
	unsigned int GetTotalWeight() const { return total_weight(); }
	int GetCount() const { return static_cast<int>(size()); }
	bool IsValid() const { return is_valid(); }
	bool Select(unsigned int value, T* pOut) const { return select(value, pOut); }
	bool Select(Randomizer& random, T* pOut) const { return select(random, pOut); }
	T Select(unsigned int index, T default_value = T()) const { return select_or(index, std::move(default_value)); }
	T Select(Randomizer& random, T default_value = T()) const { return select_or(random, std::move(default_value)); }
	*/

private:
	void add_item_impl(distribution_object item)
	{
		Items.push_back(std::move(item));
	}

	void clear_impl()
	{
		Items.clear();
	}

	[[nodiscard]] std::size_t size_impl() const noexcept
	{
		return Items.size();
	}

	Container Items {};
	unsigned int TotalWeight { 0u };
};

// Type aliases for common uses
template<typename T>
using DiscreteDistribution = DiscreteDistributionClass<T, DynamicVectorClass<DistributionObject<T>>>;

template<typename T>
using StdDiscreteDistribution = DiscreteDistributionClass<T, std::vector<DistributionObject<T>>>;