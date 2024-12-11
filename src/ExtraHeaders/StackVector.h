#pragma once

#include "StackContainer.h"

// StackVector -----------------------------------------------------------------

// Example:
//   StackVector<int, 16> foo;
//   foo->push_back(22);  // we have overloaded operator->
//   foo[0] = 10;         // as well as operator[]
template<typename T, size_t stack_capacity>
class StackVector : public StackContainer<
	std::vector<T, StackAllocator<T, stack_capacity> >,
	stack_capacity>
{
public:
	constexpr StackVector() : StackContainer<
		std::vector<T, StackAllocator<T, stack_capacity> >,
		stack_capacity>()
	{ }

	// We need to put this in STL containers sometimes, which requires a copy
	// constructor. We can't call the regular copy constructor because that will
	// take the stack buffer from the original. Here, we create an empty object
	// and make a stack buffer of its own.
	constexpr StackVector(const StackVector<T, stack_capacity>& other)
		: StackContainer<
		std::vector<T, StackAllocator<T, stack_capacity> >,
		stack_capacity>()
	{
		this->container().assign(other->begin(), other->end());
	}

	constexpr StackVector<T, stack_capacity>& operator=(
		const StackVector<T, stack_capacity>& other)
	{
		this->container().assign(other->begin(), other->end());
		return *this;
	}

	// Vectors are commonly indexed, which isn't very convenient even with
	// operator-> (using "->at()" does exception stuff we don't want).
	constexpr T& operator[](size_t i) { return this->container().operator[](i); }
	constexpr const T& operator[](size_t i) const
	{
		return this->container().operator[](i);
	}

	template <typename Func>
	constexpr void FORCEINLINE remove_all_duplicates(Func&& act) {
		std::sort(this->begin(), this->end(), std::forward<Func>(act));
		this->erase(std::unique(this->begin(), this->end()), this->end());
	}
};
