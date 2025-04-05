#pragma once

#include "AlignedMemory.h"

// This allocator can be used with STL containers to provide a stack buffer
// from which to allocate memory and overflows onto the heap. This stack buffer
// would be allocated on the stack and allows us to avoid heap operations in
// some situations.
//
// STL likes to make copies of allocators, so the allocator itself can't hold
// the data. Instead, we make the creator responsible for creating a
// StackAllocator::Source which contains the data. Copying the allocator
// merely copies the pointer to this shared source, so all allocators created
// based on our allocator will share the same stack buffer.
//
// This stack buffer implementation is very simple. The first allocation that
// fits in the stack buffer will use the stack buffer. Any subsequent
// allocations will not use the stack buffer, even if there is unused room.
// This makes it appropriate for array-like containers, but the caller should
// be sure to reserve() in the container up to the stack buffer size. Otherwise
// the container will allocate a small array which will "use up" the stack
// buffer.
template<typename T, size_t stack_capacity>
class StackAllocator
{
public:

	// Backing store for the allocator. The container owner is responsible for
	// maintaining this for as long as any containers using this allocator are
	// live.
	struct Source
	{
		COMPILETIMEEVAL Source() : used_stack_buffer_(false)
		{ }
		// Casts the buffer in its right type.
		COMPILETIMEEVAL T* stack_buffer() { return stack_buffer_.data_as<T>(); }
		COMPILETIMEEVAL const T* stack_buffer() const
		{
			return stack_buffer_.template data_as<T>();
		}
		// The buffer itself. It is not of type T because we don't want the
		// constructors and destructors to be automatically called. Define a POD
		// buffer of the right size instead.
		AlignedMemory<sizeof(T[stack_capacity]), ALIGNOF(T)> stack_buffer_;

		// Set when the stack buffer is used for an allocation. We do not track
		// how much of the buffer is used, only that somebody is using it.
		bool used_stack_buffer_;
	};

	// Used by containers when they want to refer to an allocator of type U.
	template<typename U>
	struct rebind
	{
		typedef StackAllocator<U, stack_capacity> other;
	};

	explicit StackAllocator(Source* src)
		: source_(src) { }

	// Primary allocate function.
// n is the number of T objects requested.
	void* allocate(size_t n, int flags = 0)
	{
		if (source_ && !source_->used_stack_buffer_ && n <= stack_capacity)
		{
			source_->used_stack_buffer_ = true;
			return static_cast<void*>(source_->stack_buffer());
		}
		// Fallback: allocate from the default allocator.
		return eastl::GetDefaultAllocator()->allocate(n * sizeof(T), flags);
	}

	// Overloaded allocate to handle alignment and offset.
	void* allocate(size_t n, size_t alignment, size_t offset, int flags = 0)
	{
		// For our stack allocation, we require that the requested alignment is no stricter than the natural alignment of T.
		if (source_ && !source_->used_stack_buffer_ && n <= stack_capacity && alignment <= ALIGNOF(T))
		{
			source_->used_stack_buffer_ = true;
			return static_cast<void*>(source_->stack_buffer());
		}
		// Fallback: forward to the default allocator with alignment parameters.
		return eastl::GetDefaultAllocator()->allocate(n * sizeof(T), alignment, offset, flags);
	}

	// Deallocate: if the pointer matches our stack buffer, mark it free; otherwise, forward to the default allocator.
	void deallocate(void* p, size_t n)
	{
		if (source_ && p == static_cast<void*>(source_->stack_buffer()))
		{
			source_->used_stack_buffer_ = false;
		}
		else
		{
			eastl::GetDefaultAllocator()->deallocate(p, n * sizeof(T));
		}
	}

	COMPILETIMEEVAL StackAllocator(const StackAllocator<T, stack_capacity>& rhs) :
		source_(rhs.source_)
	{ }

	template<typename U, size_t other_capacity>
	COMPILETIMEEVAL StackAllocator(const StackAllocator<U, other_capacity>& other)
		: source_(NULL)
	{ }
private:
	Source* source_;
};
