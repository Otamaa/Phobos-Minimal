#pragma once

#include "AttachEffectBehaviour.h"
#include "IEffectType.h"
#include <type_traits>

template<typename T>
concept IsEffectType = std::is_base_of<EffectType, T>::value;

class AttachEffectType;
struct AttachEffectManager;

template<IsEffectType T>
class Effect : public AttachEffectBehaviour
{
	T* Type;
	AttachEffectType* AEType;
	AttachEffectManager* OwnerAEM;

	protected string token;

	Effect() :
		Type { nullptr }
		, AEType { nullptr }
		, OwnerAEM { nullptr }
	{ }

	Effect(T* eType, AttachEffectType* aeType) : 
		Type { eType }
		, AEType { aeType }
		, OwnerAEM { nullptr }
	{ }

	void Enable(ObjectClass* pOwner, HouseClass* pHouse, TechnoClass* pAttacker) override
	{
		switch (pOwner->WhatAmI())
		{
		case AbstractType::Unit:
		case AbstractType::Infantry:
		case AbstractType::Building:
		case AbstractType::Aircraft:
			OwnerAEM = 
			break;
		default:
			break;
		}
		if (pOwner.CastToTechno(out Pointer<TechnoClass> pTechno))
		{
			OwnerAEM = TechnoExt.ExtMap.Find(pTechno).AttachEffectManager;
		}
		else if (pOwner.CastToBullet(out Pointer<BulletClass> pBullet))
		{
			OwnerAEM = BulletExt.ExtMap.Find(pBullet).AttachEffectManager;
		}
		OnEnable(pOwner, pHouse, pAttacker);
	}

	public abstract void OnEnable(Pointer<ObjectClass> pOwner, Pointer<HouseClass> pHouse, Pointer<TechnoClass> pAttacker);

};

