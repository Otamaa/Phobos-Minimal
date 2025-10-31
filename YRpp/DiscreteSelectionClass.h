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

namespace detail
{
	template<typename C>
	using has_random_access = std::is_same<
		typename std::iterator_traits<typename C::iterator>::iterator_category,
		std::random_access_iterator_tag
	>;

	template<typename C>
	constexpr bool is_random_access_v = has_random_access<C>::value;
}

template <typename T, typename Container = std::vector<T>>
class DiscreteSelectionClass
{
public:
	DiscreteSelectionClass() = default;
	explicit DiscreteSelectionClass(int initial) : rating_(initial) { }
	~DiscreteSelectionClass() = default;

	DiscreteSelectionClass(const DiscreteSelectionClass&) = default;
	DiscreteSelectionClass(DiscreteSelectionClass&&) noexcept = default;
	DiscreteSelectionClass& operator=(const DiscreteSelectionClass&) = default;
	DiscreteSelectionClass& operator=(DiscreteSelectionClass&&) noexcept = default;

	void Add(T value, int rating)
	{
		if (rating < this->rating_)
		{
			return;
		}
		if (rating > this->rating_)
		{
			this->Clear();
			this->rating_ = rating;
		}
		if constexpr (std::is_same_v<Container, std::set<T>>)
		{
			this->items_.insert(std::move(value));
		}
		else
		{
			this->items_.push_back(std::move(value));
		}
	}

	void Clear()
	{
		this->items_.clear();
	}

	[[nodiscard]] constexpr int GetRating() const noexcept
	{
		return this->rating_;
	}

	// CRITICAL FIX: Return int like original, with proper overflow check
	[[nodiscard]] int GetCount() const noexcept
	{
		const auto size = this->items_.size();
		// Ensure we don't overflow int (though unlikely in practice)
		assert(size <= static_cast<size_t>(std::numeric_limits<int>::max()));
		return static_cast<int>(size);
	}

	[[nodiscard]] bool IsValid() const noexcept
	{
		return !this->items_.empty();
	}

	// CRITICAL FIX: Proper signed comparison to match DynamicVectorClass behavior
	bool Select(int index, T* pOut) const
	{
		// Check bounds properly: index must be non-negative AND less than count
		// This matches DynamicVectorClass::ValidIndex() behavior
		const int count = this->GetCount();
		if (index < 0 || index >= count)
		{
			return false;
		}

		// Now safe to cast to size_t for indexing
		const auto idx = static_cast<size_t>(index);

		// Random access (vector, deque)
		if constexpr (detail::is_random_access_v<Container>)
		{
			if (pOut)
			{
				*pOut = this->items_[idx];
			}
			return true;
		}
		// Sequential access (list, set)
		else
		{
			auto it = this->items_.begin();
			std::advance(it, index);
			if (it != this->items_.end())
			{
				if (pOut)
				{
					*pOut = *it;
				}
				return true;
			}
			return false;
		}
	}

	bool Select(Random2Class& random, T* pOut) const
	{
		if (!this->IsValid())
		{
			return false;
		}
		const int value = random.RandomRanged(0, this->GetCount() - 1);
		return this->Select(value, pOut);
	}

	[[nodiscard]] T Select(int index, T nDefault = T()) const
	{
		this->Select(index, &nDefault);
		return nDefault;
	}

	[[nodiscard]] T Select(Random2Class& random, T nDefault = T()) const
	{
		this->Select(random, &nDefault);
		return nDefault;
	}

	// Modern alternative: return std::optional instead of pointer out-param
	[[nodiscard]] std::optional<T> TrySelect(int index) const
	{
		const int count = this->GetCount();
		if (index < 0 || index >= count)
		{
			return std::nullopt;
		}

		const auto idx = static_cast<size_t>(index);

		if constexpr (detail::is_random_access_v<Container>)
		{
			return this->items_[idx];
		}
		else
		{
			auto it = this->items_.begin();
			std::advance(it, index);
			if (it != this->items_.end())
			{
				return *it;
			}
			return std::nullopt;
		}
	}

	[[nodiscard]] std::optional<T> TrySelect(Random2Class& random) const
	{
		if (!this->IsValid())
		{
			return std::nullopt;
		}
		const int value = random.RandomRanged(0, this->GetCount() - 1);
		return this->TrySelect(value);
	}

	// Access to underlying container
	[[nodiscard]] const Container& GetItems() const noexcept
	{
		return this->items_;
	}

private:
	Container items_ {};
	int rating_ { 0 };
};

template<typename T>
using DiscreteSelectionVector = DiscreteSelectionClass<T, std::vector<T>>;

template<typename T>
using DiscreteSelectionList = DiscreteSelectionClass<T, std::list<T>>;

template<typename T>
using DiscreteSelectionSet = DiscreteSelectionClass<T, std::set<T>>;
