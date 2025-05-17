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
struct MemoryPoolUniquePointer final : public std::unique_ptr<T, PoolDeleter>
{

	COMPILETIMEEVAL MemoryPoolUniquePointer<T>() noexcept : std::unique_ptr<T, PoolDeleter>()
	{ }

	COMPILETIMEEVAL MemoryPoolUniquePointer<T>(T* pObj) noexcept : std::unique_ptr<T, PoolDeleter>()
	{
		this->reset(pObj);
	}

	MemoryPoolUniquePointer(std::unique_ptr<T>) = delete;
	template<typename U = T, typename D = PoolDeleter>
	static std::unique_ptr<U, D> Make(U* ptr)
	{
		static_assert(std::is_same_v<D, PoolDeleter>,
			"Only PoolDeleter is allowed. Use MemoryPoolUniquePointer instead of std::unique_ptr.");

		return std::unique_ptr<U, D>(ptr);
	}
};