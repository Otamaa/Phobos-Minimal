#pragma once

#pragma warning( push )
#pragma warning (disable : 4324)

#include <Lib/entt/entt.hpp>

#pragma warning( pop )

#include <Base/Always.h>

class AbsractClass;
class PhobosEntity
{
public:

	static entt::registry Registry;

public:

	static FORCEDINLINE entt::entity Create()
	{
		return Registry.create();
	}

	static FORCEDINLINE bool IsValid(entt::entity entity)
	{
		return entity != entt::null && Registry.valid(entity);
	}

	template<bool check = true>
	static void DestroyEntity(entt::entity& entity)
	{
		if COMPILETIMEEVAL (check) {
			if (IsValid(entity)) {
				Registry.destroy(entity);
			}
		} else {
			Registry.destroy(entity);
		}

		entity = entt::null;
	}

	template <typename TComp>
	static FORCEDINLINE TComp* TryGet(entt::entity entity)
	{
		if (entity == entt::null)
			return nullptr;

		return Registry.try_get<TComp>(entity);
	}

	template <typename TComp>
	static FORCEDINLINE TComp* Get(entt::entity entity)
	{
		return Registry.try_get<TComp>(entity);
	}
	
	template <typename TComp>
	static FORCEDINLINE bool Has(entt::entity entity)
	{
		if (entity == entt::null)
			return false;

		return Registry.any_of<TComp>(entity);
	}

	template <typename TComp, typename... Args>
	static FORCEDINLINE TComp& Emplace(entt::entity entity, Args&&... args)
	{
		return Registry.emplace<TComp>(entity, std::forward<Args>(args)...);
	}

	template <typename TComp, typename... Args>
	static FORCEDINLINE TComp& EmplaceOrReplace(entt::entity entity, Args&&... args)
	{
		return Registry.emplace_or_replace<TComp>(entity, std::forward<Args>(args)...);
	}

	template <typename TComp>
	static FORCEDINLINE void Remove(entt::entity entity)
	{
		if (entity != entt::null)
			Registry.remove<TComp>(entity);
	}

public:

	static void OnStartup();
	static void OnExit();
	static void OnClear();

};