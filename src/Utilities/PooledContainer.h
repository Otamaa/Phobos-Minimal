#pragma once

#include "Container.h"
#include <bitset>

template<typename T>
class ObjectPool
{
public:
	ObjectPool() = default;
	~ObjectPool() = default;

	void init(std::size_t count)
	{
		destroy(); // reset pool before reuse

		storage.reserve(count);
		freeList.reserve(count);

		for (std::size_t i = 0; i < count; ++i)
		{
			// allocate raw memory aligned for T
			storage.emplace_back(make_storage());
			freeList.push_back(getPointer(i));
		}

		poolSize = count;
	}

	void clear() noexcept
	{
		freeList.clear();
		for (std::size_t i = 0; i < storage.size(); ++i)
		{
			freeList.push_back(getPointer(i));
		}
	}

	void destroy() noexcept
	{
		storage.clear();
		freeList.clear();
		poolSize = 0;
	}

	template <typename... Args>
		requires std::constructible_from<T, Args...>
	[[nodiscard]] T* allocate(Args&&... args)
	{
		if (freeList.empty())
			return nullptr;

		T* ptr = freeList.back();
		freeList.pop_back();
		return std::construct_at(ptr, std::forward<Args>(args)...);
	}

	void deallocate(T* ptr) noexcept
	{
		if (!ptr) return;
		std::destroy_at(ptr);
		freeList.push_back(ptr);
	}

	[[nodiscard]] std::size_t getPoolSize() const noexcept
	{
		return poolSize;
	}

private:
	std::vector<std::unique_ptr<std::byte[]>> storage;
	std::vector<T*> freeList;
	std::size_t poolSize = 0;

	[[nodiscard]] T* getPointer(std::size_t index) const noexcept
	{
		return std::launder(reinterpret_cast<T*>(storage[index].get()));
	}

	// Clean helper to wrap std::make_unique
	static std::unique_ptr<std::byte[]> make_storage()
	{
		return std::make_unique<std::byte[]>(sizeof(T));
	}
};

template <typename T, std::size_t N>
	requires (N > 0)
class StaticObjectPool
{
public:

	StaticObjectPool()
	{
		// Allocate fixed-size memory once
		for (auto& slot : buffer)
		{
			slot = std::make_unique<std::byte[]>(sizeof(T));
		}
	}

	~StaticObjectPool()
	{
		clear(); // Make sure destructors are called
	}

	// Prevent copying
	StaticObjectPool(const StaticObjectPool&) = delete;
	StaticObjectPool& operator=(const StaticObjectPool&) = delete;

	// Reinitialize entire pool (calls clear, keeps buffer, resets state)
	void reInit() noexcept
	{
		clear();
		used.reset();
	}

	// Clear used objects but preserve memory
	void clear() noexcept
	{

		for (std::size_t i = 0; i < N; ++i)
		{
			if (used[i])
			{
				used[i] = false;
			}
		}
	}

	template <typename... Args>
		requires std::constructible_from<T, Args...>
	[[nodiscard]] T* allocate(Args&&... args) noexcept
	{
		for (std::size_t i = 0; i < N; ++i)
		{
			if (!used[i])
			{
				used[i] = true;
				return std::construct_at(ptr(i), std::forward<Args>(args)...);
			}
		}

		return nullptr; // Pool exhausted
	}

	void deallocate(T* obj) noexcept
	{
		for (std::size_t i = 0; i < N; ++i)
		{
			if (ptr(i) == obj && used[i])
			{
				std::destroy_at(obj);
				used[i] = false;
				return;
			}
		}
	}

	[[nodiscard]] std::size_t getPoolSize() const noexcept { return N; }
	[[nodiscard]] std::size_t in_use() const noexcept { return used.count(); }
	[[nodiscard]] std::size_t available() const noexcept { return N - used.count(); }

private:
	std::array<std::unique_ptr<std::byte[]>, N> buffer;
	std::bitset<N> used;

	[[nodiscard]] T* ptr(std::size_t index) noexcept
	{
		return std::launder(reinterpret_cast<T*>(buffer[index].get()));
	}
};