#pragma once

#include <Phobos.h>
#include "SavegameDef.h"

// Lightweight entity manager - stores entity handle per extension
// This is just a pointer-sized field (8 bytes on x64, 4 bytes on x86)
class EntityHandle
{
private:
	entt::entity entity;

public:
	EntityHandle() : entity(entt::null) { }

	// Check if entity exists (inline, super fast)
	FORCEDINLINE bool HasEntity() const
	{
		return entity != entt::null && Phobos::gEntt->valid(entity);
	}

	// Get entity (inline)
	FORCEDINLINE entt::entity Get() const { return entity; }

	// Create entity ONLY when first component is added
	FORCEDINLINE entt::entity EnsureEntity()
	{
		if (entity == entt::null || !Phobos::gEntt->valid(entity))
		{
			entity = Phobos::gEntt->create();
		}
		return entity;
	}

	// Destroy entity
	FORCEDINLINE void Destroy()
	{
		if (entity != entt::null && Phobos::gEntt->valid(entity))
		{
			Phobos::gEntt->destroy(entity);
		}
		entity = entt::null;
	}

	// TryGet - ULTRA LIGHTWEIGHT (just 2 pointer checks + sparse set lookup)
	template<typename T>
	FORCEDINLINE T* TryGet()
	{
		if (entity == entt::null) return nullptr;
		return Phobos::gEntt->try_get<T>(entity);
	}

	template<typename T>
	FORCEDINLINE const T* TryGet() const
	{
		if (entity == entt::null) return nullptr;
		return Phobos::gEntt->try_get<T>(entity);
	}

	// Add component (creates entity if needed)
	template<typename T, typename... Args>
	T& Add(Args&&... args)
	{
		return Phobos::gEntt->emplace<T>(EnsureEntity(), std::forward<Args>(args)...);
	}

	// Has component check
	template<typename T>
	FORCEDINLINE bool Has() const
	{
		return entity != entt::null && Phobos::gEntt->all_of<T>(entity);
	}

	// Remove component
	template<typename T>
	void Remove()
	{
		if (entity != entt::null)
		{
			Phobos::gEntt->remove<T>(entity);
		}
	}
};

// Minimal extension to AbstractExtended - just adds one pointer-sized field
class EntityExtension
{
private:
	EntityHandle handle; // Only 4-8 bytes!

public:
	EntityExtension() = default;
	~EntityExtension() { handle.Destroy(); }

	// Forward all operations to handle
	FORCEDINLINE EntityHandle& GetHandle() { return handle; }
	FORCEDINLINE const EntityHandle& GetHandle() const { return handle; }

	// Convenience methods
	template<typename T, typename... Args>
	FORCEDINLINE T& AddComponent(Args&&... args)
	{
		return handle.Add<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	FORCEDINLINE T* TryGetComponent()
	{
		return handle.TryGet<T>();
	}

	template<typename T>
	FORCEDINLINE const T* TryGetComponent() const
	{
		return handle.TryGet<T>();
	}

	template<typename T>
	FORCEDINLINE bool HasComponent() const
	{
		return handle.Has<T>();
	}

	template<typename T>
	void RemoveComponent()
	{
		handle.Remove<T>();
	}

	FORCEDINLINE bool HasEntity() const { return handle.HasEntity(); }
	FORCEDINLINE entt::entity GetEntity() const { return handle.Get(); }

	// Save/Load - only saves if entity exists
	void SaveToStream(PhobosStreamWriter& Stm) const
	{
		bool hasEntity = handle.HasEntity();
		Stm.Save(hasEntity);
		// Don't save component data - components should be recreated on load if needed
	}

	void LoadFromStream(PhobosStreamReader& Stm)
	{
		bool hadEntity;
		Stm.Load(hadEntity);
		// Entity will be recreated when components are added
	}
};