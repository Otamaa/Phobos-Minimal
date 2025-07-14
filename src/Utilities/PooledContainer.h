#pragma once

#include "Container.h"

template<typename T, bool CallDestructor = false>
class ObjectPool
{
private:
	struct Node
	{
		Node* next;
		alignas(alignof(T)) T data;
	};

	Node* pool = nullptr;
	Node* freeList = nullptr;
	size_t poolSize = 0;

public:
	ObjectPool() = default;

	size_t getPoolSize() const { return poolSize; }

	// (Re)initialize pool with new size
	bool init(size_t count)
	{
		destroy(); // clean up existing pool if any

		pool = static_cast<Node*>(::operator new[](sizeof(Node)* count));
		poolSize = count;

		reset(); // reset free list
		return true;
	}

	void destroy()
	{
		if (pool) {
			if constexpr (CallDestructor) {
				if(!Phobos::Otamaa::ExeTerminated){
					for (size_t i = 0; i < poolSize; ++i)
						pool[i].data.~T();
				}
			}
			::operator delete[](pool);
			pool = nullptr;
			freeList = nullptr;
			poolSize = 0;
		}
	}

	T* allocate()
	{
		if (!freeList)
			return nullptr;

		Node* node = freeList;
		freeList = node->next;
		return new (&node->data) T();
	}

	void deallocate(T* ptr)
	{
		if (!ptr) return;

		if constexpr (CallDestructor){
			if (!Phobos::Otamaa::ExeTerminated)
				ptr->~T();
		}

		Node* node = reinterpret_cast<Node*>(
			reinterpret_cast<uint8_t*>(ptr) - offsetof(Node, data)
		);
		node->next = freeList;
		freeList = node;
	}

	template<typename CleanupFunc>
	void deallocate(T* ptr, CleanupFunc&& cleanup)
	{
		if (!ptr) return;

		cleanup(ptr);

		if constexpr (CallDestructor) {
			if (!Phobos::Otamaa::ExeTerminated)
				ptr->~T();
		}

		Node* node = reinterpret_cast<Node*>(
			reinterpret_cast<uint8_t*>(ptr) - offsetof(Node, data)
		);
		node->next = freeList;
		freeList = node;
	}

	void clear()
	{
		if constexpr (CallDestructor)
		{
			if (!Phobos::Otamaa::ExeTerminated){
				for (size_t i = 0; i < poolSize; ++i)
					pool[i].data.~T();
			}
		}
		reset();
	}

	template<typename CleanupFunc>
	void clear(CleanupFunc&& cleanup)
	{
		for (size_t i = 0; i < poolSize; ++i)
		{
			cleanup(&pool[i].data);
			if constexpr (CallDestructor) {
				if (!Phobos::Otamaa::ExeTerminated)
				pool[i].data.~T();
			}
		}
		reset();
	}

	void reset()
	{
		freeList = nullptr;
		for (size_t i = 0; i < poolSize; ++i)
		{
			pool[i].next = freeList;
			freeList = &pool[i];
		}
	}

	~ObjectPool()
	{
		destroy();
	}

	ObjectPool(const ObjectPool&) = delete;
	ObjectPool& operator=(const ObjectPool&) = delete;
};