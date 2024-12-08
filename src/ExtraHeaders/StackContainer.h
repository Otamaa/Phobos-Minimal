#pragma once

#include "StackAllocator.h"

// A wrapper around STL containers that maintains a stack-sized buffer that the
// initial capacity of the vector is based on. Growing the container beyond the
// stack capacity will transparently overflow onto the heap. The container must
// support reserve().
//
// WATCH OUT: the ContainerType MUST use the proper StackAllocator for this
// type. This object is really intended to be used only internally. You'll want
// to use the wrappers below for different types.
template<typename TContainerType, int stack_capacity>
class StackContainer
{
public:
	typedef TContainerType ContainerType;
	typedef typename ContainerType::value_type ContainedType;
	typedef StackAllocator<ContainedType, stack_capacity> Allocator;
	// Allocator must be constructed before the container!
	constexpr StackContainer() : allocator_(&stack_data_), container_(allocator_)
	{
		// Make the container use the stack allocation by reserving our buffer size
		// before doing anything else.
		container_.reserve(stack_capacity);
	}
	// Getters for the actual container.
	//
	// Danger: any copies of this made using the copy constructor must have
	// shorter lifetimes than the source. The copy will share the same allocator
	// and therefore the same stack buffer as the original. Use std::copy to
	// copy into a "real" container for longer-lived objects.
	constexpr ContainerType& container() { return container_; }
	constexpr const ContainerType& container() const { return container_; }

	// Support operator-> to get to the container. This allows nicer syntax like:
	//   StackContainer<...> foo;
	//   std::sort(foo->begin(), foo->end());
	constexpr ContainerType* operator->() { return &container_; }
	constexpr const ContainerType* operator->() const { return &container_; }

protected:
	typename Allocator::Source stack_data_;
	Allocator allocator_;
	ContainerType container_;
	DISALLOW_COPY_AND_ASSIGN(StackContainer);
};
