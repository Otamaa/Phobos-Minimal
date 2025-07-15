#pragma once

#include "Container.h"

template<typename T>
class ObjectPool
{
public:
	ObjectPool() = default;
	~ObjectPool() = default;

	void init(size_t count)
	{
		destroy(); // clear everything if already initialized

		storage.reserve(count);
		freeList.reserve(count);

		for (size_t i = 0; i < count; ++i)
		{
			// Allocate raw memory, no constructor
			storage.emplace_back(std::make_unique<Storage>());
			freeList.push_back(getPointer(i));
		}

		poolSize = count;
	}

	void clear()
	{
		freeList.clear();
		for (size_t i = 0; i < storage.size(); ++i)
		{
			freeList.push_back(getPointer(i));
		}
	}

	void destroy()
	{
		storage.clear();   // automatically frees memory
		freeList.clear();
		poolSize = 0;
	}

	template <typename... Args>
	T* allocate(Args&&... args)
	{
		if (freeList.empty())
			return nullptr;

		T* ptr = freeList.back();
		freeList.pop_back();

		// Call constructor only now
		return new (ptr) T(std::forward<Args>(args)...);
	}

	void deallocate(T* ptr) {
		if (!ptr) return;
		ptr->~T();
		freeList.push_back(ptr); // you must call destructor manually before
	}

	size_t getPoolSize() const { return poolSize; }

private:
	using Storage = std::aligned_storage_t<sizeof(T), alignof(T)>;

	std::vector<std::unique_ptr<Storage>> storage; // owns raw memory
	std::vector<T*> freeList;
	size_t poolSize = 0;

	T* getPointer(size_t index) {
		return reinterpret_cast<T*>(storage[index].get());
	}
};