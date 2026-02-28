#pragma once

#include <stack>
#include <mutex>
#include <unordered_map>
#include <map>
#include <set>
#include <typeindex>

#include <Misc/Kratos/Common/EventSystems/EventSystem.h>

#include <Utilities/Debug.h>

// 前置声明
class Component;

// 组件对象池管理器 - 支持转移
class ComponentPool
{
public:
	static ComponentPool& GetInstance();

	// 通过名称创建
	Component* Create(const std::string& name);

	void ClearAll();

	static void Clear(EventSystem* sender, Event e, void* args);

	// 从对象池获取组件
	template<typename T>
	T* Acquire()
	{
		static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");

		void* memory = AllocateRaw(typeid(T), sizeof(T));
		if (!memory)
		{
			Debug::Log("ERROR: ComponentPool failed to allocate memory for %s\n",
				typeid(T).name());
			throw std::bad_alloc();
		}

		// 使用placement new构造
		T* obj = nullptr;
		try
		{
			// 分离构造和Clean的日志
			obj = new (memory) T();

			// 重置组件以供复用，清理实例ID
			obj->ResetForReuse();
		}
		catch (const std::exception& e)
		{
			Debug::Log("ERROR: Exception during construction/Clean of %s: %s\n",
				typeid(T).name(), e.what());
			throw;
		}
		catch (...)
		{
			Debug::Log("ERROR: Unknown exception during construction/Clean of %s\n",
				typeid(T).name());
			throw;
		}

		return obj;
	}

	// 释放组件到对象池
	template<typename T>
	void Release(T* component)
	{
		if (!component) return;

		component->Clean();  // 重置状态
		component->~T();
		DeallocateRaw(typeid(T), component);
	}

	// 转移操作
	template<typename T>
	std::shared_ptr<T> TransferOwnership(T* component)
	{
		if (!component) return nullptr;

		// 创建新实例并拷贝数据
		T* newComponent = Acquire<T>();
		newComponent->CopyFrom(*component);

		// 转移extData
		if (component->extData)
		{
			newComponent->SetExtData(component->extData);
		}

		// 释放原组件到对象池
		Release(component);

		// 返回智能指针
		return std::shared_ptr<T>(newComponent, [this](T* ptr) {
			Release(ptr);
			});
	}


	template<typename T>
	void TransferToPool(T* component)

	{
		if (!component) return;

		component->Clean();  // 重置状态
		DeallocateRaw(typeid(T), component);
	}

	// 预分配
	template<typename T>
	void Preallocate(size_t count)
	{
		std::lock_guard<std::mutex> lock(m_poolMutex);

		auto& pool = m_pools[std::type_index(typeid(T))];
		pool.objectSize = sizeof(T);

		for (size_t i = 0; i < count; ++i)
		{
			void* memory = std::malloc(sizeof(T));
			if (memory)
			{
				pool.objects.push(memory);
				pool.totalCreated++;
			}
		}
	}

private:
	ComponentPool();
	~ComponentPool();

	struct Pool
	{
		std::stack<void*> objects;
		size_t objectSize = 0;
		size_t totalCreated = 0;
		size_t totalReused = 0;
	};

	using PoolMap = std::unordered_map<std::type_index, Pool>;
	PoolMap m_pools;
	mutable std::mutex m_poolMutex;

	void* AllocateRaw(const std::type_info& type, size_t size);
	void DeallocateRaw(const std::type_info& type, void* ptr);
};
