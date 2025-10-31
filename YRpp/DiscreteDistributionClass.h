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
	constexpr DistributionObject() = default;
	constexpr explicit DistributionObject(T value, unsigned int weight = 1u)
		: Value(std::move(value)), Weight(weight) { }

	constexpr bool operator==(const DistributionObject<T>&) const noexcept = default;
	constexpr bool operator!=(const DistributionObject<T>&) const noexcept = default;

	// For std::set support
	constexpr bool operator<(const DistributionObject<T>& other) const
	{
		if (Weight != other.Weight)
		{
			return Weight < other.Weight;
		}
		return Value < other.Value;
	}

	~DistributionObject() = default;

	T Value {};
	unsigned int Weight { 0u };
};

template <typename T, typename Container = std::vector<DistributionObject<T>>>
class DiscreteDistributionClass
{
public:
	DiscreteDistributionClass() = default;
	~DiscreteDistributionClass() = default;

	DiscreteDistributionClass(const DiscreteDistributionClass&) = default;
	DiscreteDistributionClass(DiscreteDistributionClass&&) noexcept = default;
	DiscreteDistributionClass& operator=(const DiscreteDistributionClass&) = default;
	DiscreteDistributionClass& operator=(DiscreteDistributionClass&&) noexcept = default;

	[[nodiscard]] const Container& GetItems() const noexcept
	{
		return this->items_;
	}

	void Add(DistributionObject<T> item)
	{
		if (item.Weight == 0u)
		{
			return;
		}
		this->totalWeight_ += item.Weight;
		if constexpr (std::is_same_v<Container, std::set<DistributionObject<T>>>)
		{
			this->items_.insert(std::move(item));
		}
		else
		{
			this->items_.push_back(std::move(item));
		}
	}

	void Add(T value, unsigned int weight = 1u)
	{
		if (weight == 0u)
		{
			return;
		}
		this->Add(DistributionObject<T>(std::move(value), weight));
	}

	void Clear()
	{
		this->totalWeight_ = 0u;
		this->items_.clear();
	}

	[[nodiscard]] constexpr unsigned int GetTotalWeight() const noexcept
	{
		return this->totalWeight_;
	}

	// CRITICAL FIX: Return int like original, with proper overflow check
	[[nodiscard]] int GetCount() const noexcept
	{
		const auto size = this->items_.size();
		assert(size <= static_cast<size_t>(std::numeric_limits<int>::max()));
		return static_cast<int>(size);
	}

	[[nodiscard]] bool IsValid() const noexcept
	{
		return this->totalWeight_ > 0u && !this->items_.empty();
	}

	// CRITICAL FIX: Use != instead of < for iterator comparison
	[[nodiscard]] bool Select(unsigned int value, T* pOut) const
	{
		if (!this->IsValid() || value == 0u || value > this->totalWeight_)
		{
			return false;
		}

		unsigned int acc = 0u;
		// FIX: Use != instead of < to work with all iterator types
		for (auto it = this->items_.begin(); it != this->items_.end(); ++it)
		{
			acc += it->Weight;
			if (acc >= value)
			{
				if (pOut)
				{
					*pOut = it->Value;
				}
				return true;
			}
		}
		return false;
	}

	bool Select(Random2Class& random, T* pOut) const
	{
		if (this->totalWeight_ == 0u)
		{
			return false;
		}
		const int value = random.RandomRanged(1, static_cast<int>(this->totalWeight_));
		return this->Select(static_cast<unsigned int>(value), pOut);
	}

	[[nodiscard]] T Select(unsigned int index, T nDefault = T()) const
	{
		this->Select(index, &nDefault);
		return nDefault;
	}

	[[nodiscard]] T Select(Random2Class& random, T nDefault = T()) const
	{
		this->Select(random, &nDefault);
		return nDefault;
	}

	[[nodiscard]] T SelectOrDefault(T defaultval = T()) const
	{
		this->Select(ScenarioClass::Instance->Random, &defaultval);
		return defaultval;
	}

	// Modern alternative: return std::optional
	[[nodiscard]] std::optional<T> TrySelect(unsigned int value) const
	{
		T result {};
		if (this->Select(value, &result))
		{
			return result;
		}
		return std::nullopt;
	}

	[[nodiscard]] std::optional<T> TrySelect(Random2Class& random) const
	{
		T result {};
		if (this->Select(random, &result))
		{
			return result;
		}
		return std::nullopt;
	}

private:
	Container items_ {};
	unsigned int totalWeight_ { 0u };
};

template<typename T>
using DiscreteDistributionVector = DiscreteDistributionClass<T, std::vector<DistributionObject<T>>>;

template<typename T>
using DiscreteDistributionList = DiscreteDistributionClass<T, std::list<DistributionObject<T>>>;