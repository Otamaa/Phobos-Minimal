#pragma once
#include <ExtraHeaders/MemoryPool.h>

class PoolDeleter
{
public:
	template<typename T>
	void operator()(T* ptr) const {
		if (ptr) {
			ptr->deleteInstance();
		}
	}
};

template<typename T>
struct MemoryPoolUniquePointer : public std::unique_ptr<T, PoolDeleter>
{

	COMPILETIMEEVAL MemoryPoolUniquePointer<T>() noexcept : std::unique_ptr<T, PoolDeleter>()
	{ }

	COMPILETIMEEVAL MemoryPoolUniquePointer<T>(T* pObj) noexcept : std::unique_ptr<T, PoolDeleter>()
	{
		this->reset(pObj);
	}
};