#pragma once

#include "../Base.h"
#include <AnimClass.h>
#include "AnimationType.h"

class Animation : public Effect<AnimationType>
{
public:

	Animation() : Effect<AnimationType> { MyType::Anim }
		, Anim { nullptr }
		, OnwerIsDead { false }
	{ }

	Animation(const AnimationType& nType) : Effect<AnimationType> { MyType::Anim }
		, Anim { nullptr }
		, OnwerIsDead { false }
	{ SetTypeData(nType); }

	virtual void OnEnable(ObjectClass* pObject, HouseClass* pHouse, TechnoClass* pAttacker) override
	{
		if (!TypeData)
			return;

		if (auto pAnimType = TypeData->ActiveAnim.Get())
		{
			auto pCreated = GameCreate<AnimClass>(pAnimType, pObject->GetCoords());
			{
				pCreated->SetOwnerObject(pObject);
				if (!pCreated->GetOwningHouse())
					pCreated->SetHouse(pObject->GetOwningHouse());

				Anim = pCreated;
			}
		}

		CreateAnim(pObject);
	}

	virtual void Disable(CoordStruct location) override
	{
		KillAnim();

		if (!TypeData)
			return;

		if (auto pAnimType = TypeData->DoneAnim.Get())
			GameCreate<AnimClass>(pAnimType, location);
	}

	void OnPut(ObjectClass* pObject, CoordStruct* pCoord, DirStruct faceDir) override
	{
		CreateAnim(pObject);
	}

	void OnUpdate(ObjectClass* pOwner, bool isDead)override
	{
		OnwerIsDead = isDead;
	}

	void OnRemove(ObjectClass* pObject) override
	{
		KillAnim();
	}

	void OnReceiveDamage(ObjectClass* pObject, int* pDamage, int DistanceFromEpicenter, WarheadTypeClass* pWH, ObjectClass* pAttacker, bool IgnoreDefenses, bool PreventPassengerEscape, HouseClass* pAttackingHouse) override
	{
		if (!TypeData)
			return;

		if (auto  pAnimType = TypeData->HitAnim.Get())
		{
			auto pCreated = GameCreate<AnimClass>(pAnimType, pObject->GetCoords());
			{
				pCreated->SetOwnerObject(pObject);
				if (!pCreated->GetOwningHouse())
					pCreated->SetHouse(pObject->GetOwningHouse());
			}
		}
	}

	void OnDestroy(ObjectClass* pOwner) override
	{
		OnwerIsDead = true;
	}

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm)
	{ return Serialize(Stm); }

private:
	template <typename T>
	bool Serialize(T& Stm)
	{
		//Debug::Log("Processing Element From Animation ! \n");
		return Stm
			.Process(Type)
			.Process(TypeData)
			.Process(Anim)
			.Process(OnwerIsDead)
			.Success()
			;
	}

	void CreateAnim(ObjectClass* pObject)
	{
		if (Anim)
			KillAnim();

		if (pObject && !Anim)
		{
			if (auto pAnimType = TypeData->IdleAnim.Get())
			{
				auto pCreated = GameCreate<AnimClass>(pAnimType, pObject->GetCoords());
				{
					pCreated->SetOwnerObject(pObject);
					if (!pCreated->GetOwningHouse())
						pCreated->SetHouse(pObject->GetOwningHouse());

					pCreated->RemainingIterations = 0xFF;

					Anim = pCreated;
				}
			}
		}
	}

	void KillAnim()
	{
		if (Anim)
		{
			if (!OnwerIsDead)
			{
				Anim->TimeToDie = true;
				Anim->UnInit();
			}

			Anim = nullptr;
		}
	}

private:
	AnimClass* Anim;
	bool OnwerIsDead;

};