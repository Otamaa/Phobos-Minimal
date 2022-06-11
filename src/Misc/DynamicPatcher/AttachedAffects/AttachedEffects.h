#pragma once

#include <GeneralStructures.h>
#include <ScenarioClass.h>
#include <vector>
#include "Effects/Base.h"
#include "Effects/Animation/Animation.h"
#include "Effects/Stand/Stand.h"
#include "Effects/AttachStatus/AttachStatus.h"

#include "AttachEffectType.h"

class AttachEffectType;
class TechnoClass;
class HouseClass;
struct AttachedAffects
{
private:
#pragma region vtable_caller
	void EnableAction(ObjectClass* pOwner, HouseClass* pHouse, TechnoClass* pAttacker)
	{
		for (auto const& eff : effects)
		{
			if (!eff  || eff->WhatAmI() == MyType::None)
				continue;

			eff->Enable(pOwner, pHouse, pAttacker);
		}
	}

	void DisableAction(CoordStruct location)
	{
		for (auto const& eff : effects)
		{
			if (!eff  || eff->WhatAmI() == MyType::None)
				continue;

			eff->Disable(location);
		}
	}

	void ResetDurationAction()
	{
		for (auto const& eff : effects)
		{
			if (!eff  || eff->WhatAmI() == MyType::None)
				continue;

			eff->ResetDuration();
		}
	}

	void OnUpdateAction(ObjectClass* pOwner, bool isDead)
	{
		for (auto const& eff : effects)
		{
			if (!eff  || eff->WhatAmI() == MyType::None)
				continue;

			eff->OnUpdate(pOwner, isDead);
		}
	}

	void OnTemporalUpdateAction(TechnoClass* ext, TemporalClass* pTemporal)
	{
		for (auto const& eff : effects)
		{
			if (!eff  || eff->WhatAmI() == MyType::None)
				continue;

			eff->OnTemporalUpdate(ext, pTemporal);
		}
	}

	void OnPutAction(ObjectClass* pOwner, CoordStruct* pCoord, DirStruct faceDir)
	{
		for (auto const& eff : effects)
		{
			if (!eff  || eff->WhatAmI() == MyType::None)
				continue;

			eff->OnPut(pOwner, pCoord, faceDir);
		}
	}

	void OnRemoveAction(ObjectClass* pOwner)
	{
		for (auto const& eff : effects)
		{
			if (!eff  || eff->WhatAmI() == MyType::None)
				continue;

			eff->OnRemove(pOwner);
		}
	}

	void OnReceiveDamageAction(ObjectClass* pOwner, int* pDamage, int DistanceFromEpicenter, WarheadTypeClass* pWH, ObjectClass* pAttacker, bool IgnoreDefenses, bool PreventPassengerEscape, HouseClass* pAttackingHouse)
	{
		for (auto const& eff : effects)
		{
			if (!eff  || eff->WhatAmI() == MyType::None)
				continue;

			eff->OnReceiveDamage(pOwner, pDamage, DistanceFromEpicenter, pWH, pAttacker, IgnoreDefenses, PreventPassengerEscape, pAttackingHouse);
		}
	}

	void OnDestroyAction(ObjectClass* pOwner)
	{
		for (auto const& eff : effects)
		{
			if (!eff  || eff->WhatAmI() == MyType::None)
				continue;

			eff->OnDestroy(pOwner);
		}
	}

	void OnGuardCommandAction()
	{
		for (auto const& eff : effects)
		{
			if (!eff  || eff->WhatAmI() == MyType::None)
				continue;

			eff->OnGuardCommand();
		}
	}

	void OnStopCommandAction()
	{
		for (auto const& eff : effects)
		{
			if (!eff  || eff->WhatAmI() == MyType::None)
				continue;

			eff->OnStopCommand();
		}
	}
#pragma endregion

public:

	void Enable(ObjectClass* pObject, HouseClass* pHouse, TechnoClass* pAttacker)
	{
		Active = true;
		House = pHouse;
		Attacker = pAttacker;
		if (!delayToEnable || initialDelayTimer.Expired())
		{
			EnableEffects(pObject, pHouse, pAttacker);
		}
	}

	void SetupLifeTimer()
	{
		if (!immortal)
		{
			lifeTimer.Start(duration);
		}
	}

	void Disable(CoordStruct location)
	{
		Active = false;
		if (delayToEnable)
		{
			return;
		}

		DisableAction(location);
	}

	bool IsActive()
	{
		//if (Active)
		//{
			// Logger.Log("AE Type {0} {1} and {2}", Type.Name, IsDeath() ? "is death" : "not dead", IsAlive() ? "is alive" : "not alive");
		//	Active = delayToEnable  //|| (!IsDeath()&& IsAlive())
		//		;
		//}
		return Active;

	}

	bool IsAlive()
	{
		for (auto const& eff : effects)
		{
			if (!eff  || eff->WhatAmI() == MyType::None)
				continue;

			if (!eff->IsAlive())
			{
				return false;
			}
		}

		return true;
	}

	bool IsDeath()
	{
		return duration <= 0 || (!immortal && lifeTimer.Expired());
	}

	bool IsSameGroup(const AttachEffectType* otherType)
	{
		return Type->Group > -1 && otherType->Group > -1 && Type->Group == otherType->Group;
	}

	void MergeDuation(int otherDuration)
	{
		if (delayToEnable || otherDuration == 0)
		{
			return;
		}

		if (otherDuration < 0)
		{
			int timeLeft = immortal ? duration : lifeTimer.GetTimeLeft();
			duration += otherDuration;
			if (duration <= 0 || timeLeft <= 0)
			{
				Active = false;
			}
			else
			{
				timeLeft += otherDuration;

				if (timeLeft <= 0)
				{
					Active = false;
				}
				else
				{
					ForceStartLifeTimer(timeLeft);
				}
			}
		}
		else
		{
			duration += otherDuration;
			if (!immortal)
			{
				int timeLeft = lifeTimer.GetTimeLeft();
				timeLeft += otherDuration;
				ForceStartLifeTimer(timeLeft);
			}
		}
	}

	void ResetDuration()
	{
		SetupLifeTimer();
		ResetDurationAction();
	}

	void OnUpdate(ObjectClass* pObject, bool isDead)
	{
		if (delayToEnable)
		{
			if (initialDelayTimer.InProgress())
			{
				return;
			}
			EnableEffects(pObject, House, Attacker);
		}
		OnUpdateAction(pObject, isDead);
	}

	void OnTemporalUpdate(TechnoClass* ext, TemporalClass* pTemporal)
	{
		if (delayToEnable)
		{
			return;
		}

		OnTemporalUpdateAction(ext, pTemporal);
	}

	void OnPut(ObjectClass* pObject, CoordStruct* pCoord, DirStruct faceDir)
	{
		if (delayToEnable)
		{
			return;
		}
		OnPutAction(pObject, pCoord, faceDir);
	}

	void OnRemove(ObjectClass* pObject)
	{
		if (delayToEnable)
		{
			return;
		}
		OnRemoveAction(pObject);
	}

	void OnReceiveDamage(ObjectClass* pObject, int* pDamage, int distanceFromEpicenter, WarheadTypeClass* pWH,
			ObjectClass* pAttacker, bool ignoreDefenses, bool preventPassengerEscape, HouseClass* pAttackingHouse)
	{
		if (delayToEnable)
		{
			return;
		}
		OnReceiveDamageAction(pObject, pDamage, distanceFromEpicenter, pWH, pAttacker, ignoreDefenses, preventPassengerEscape, pAttackingHouse);
	}

	void OnDestroy(ObjectClass* pObject)
	{
		if (delayToEnable)
		{
			return;
		}
		OnDestroyAction(pObject);
	}

	void OnGuardCommand()
	{
		if (delayToEnable)
		{
			return;
		}
		OnGuardCommandAction();
	}

	void OnStopCommand()
	{
		if (delayToEnable)
		{
			return;
		}
		OnStopCommandAction();
	}

	AttachedAffects() :
		Name { NONE_STR }
		, Type { nullptr }
		, House { nullptr }
		, Attacker { nullptr }
		, Active { false }
		, duration { -1 }
		, immortal { false }
		, lifeTimer { }
		, initialDelayTimer { }
		, delayToEnable { false }
		, effects { }
	{ }

	AttachedAffects(AttachEffectType* pType) :
		Name { pType->Name.data() }
		, Type { pType }
		, House { nullptr }
		, Attacker { nullptr }
		, Active { false }
		, duration { pType->Duration.Get() }
		, immortal { pType->HoldDuration.Get() }
		, lifeTimer { }
		, initialDelayTimer { }
		, delayToEnable { false }
		, effects { 12 }
	{
		RegisterAllEffects();

		int initDelay = 0;
		if (pType->InitialRandomDelay.Get())
		{
			initDelay = ScenarioClass::Instance->Random.RandomRanged(
				pType->InitialMinDelay.Get(),
				pType->InitialMaxDelay.Get()
			);
		}

		if (initDelay > 0)
		{
			initialDelayTimer.Start(initDelay);
			delayToEnable = true;
		}
	}

	/*
	~AttachedAffects() {
		for (auto const& eff : effects)
		{
			if (!eff)
				continue;

			GameDelete(eff);
		}

		effects.clear();

		Type = nullptr;
		House = nullptr;
		Attacker = nullptr;
	}*/
	size_t GetEffectCout() const
	{
		return effects.size();
	}

	Stand* GetStand() {
		return (Stand*)(effects[11]);
	}

	AttachStatus* GetStatus() {
		return (AttachStatus*)(effects[2]);
	}

public:
	PhobosFixedString<0x100> Name;
	AttachEffectType* Type;
	HouseClass* House;
	TechnoClass* Attacker;
	bool Active;
private:
	int duration; // 寿命
	bool immortal; // 永生
	TimerStruct lifeTimer;
	TimerStruct initialDelayTimer;
	bool delayToEnable; // 延迟激活中

	//std::array<UniqueGamePtr<EffectsBase>, 11>effects;
	std::vector<std::uniquePtr<EffectsBase>> effects;
	void RegisterAllEffects()
	{
		if (!Type)
			return;

		/*  enum class MyType : int
		None = 0,
		Anim = 1,
		Stats 2,
		AutoWeapon 3,
		BlackHole 4,
		DestroySelf 5,
		DisableWapon 6,
		FireSuper 7,
		GiftBox 8,
		OverrideWeapon 9 ,
		PaintBall 10,
		Stand 11,
		Transform 12
		*/

		if (Type->AnimationTypeData.Enable) {
			effects[1]=(GameCreate<Animation>(Type->AnimationTypeData));
		}

		effects[2]=(GameCreate<AttachStatus>(Type->AttachStatusTypeData));

		if (GeneralUtils::IsValidString(Type->StandTypeData.Type.data())) {
			effects[11]=(GameCreate<Stand>(Type->StandTypeData));
		}

		//toDo : other base
	}

	void EnableEffects(ObjectClass* pObject, HouseClass* pHouse, TechnoClass* pAttacker)
	{
		delayToEnable = false;
		SetupLifeTimer();
		EnableAction(pObject, pHouse, pAttacker);
	}

	void ForceStartLifeTimer(int timeLeft)
	{
		immortal = false;
		lifeTimer.Start(timeLeft);
	}

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm)
	{ return Serialize(Stm); }

private:
	template <typename T>
	bool Serialize(T& Stm)
	{
		Debug::Log("Processing Element From AttachedAffects ! \n");
		return Stm
			.Process(Name)
			.Process(Type)
			.Process(House)
			.Process(Attacker)
			.Process(Active)
			.Process(duration)
			.Process(immortal)
			.Process(lifeTimer)
			.Process(initialDelayTimer)
			.Process(delayToEnable)
			.Success()
			;
	}
};