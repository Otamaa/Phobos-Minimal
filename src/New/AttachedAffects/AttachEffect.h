#pragma once

#include <Utilities/SavegameDef.h>
#include "EffectBase.h"

class AttachEffectType;
class ObjectClass;
class TemporalClass;
class WarheadTypeClass;
class HouseClass;
struct AttachEffect
{
	AttachEffectType* m_Type {};
	ObjectClass* m_AttachedTo {};
	ObjectClass* m_Source {};
	HouseClass* m_SourceHouse {};

	CoordStruct m_DetonateLoc {};
	bool m_FromWarhead {};

	int m_AEMode;
	bool m_FromPassenger;

	bool m_NonInheritable {};

	AttachEffect() = default;
	~AttachEffect() = default;

	AttachEffect(AttachEffectType* pType);

private:

	 bool m_active {};
	 int m_Duration {};
	 bool m_Isimmortal {};
	 CDTimerClass m_LifeTimer {};
	 CDTimerClass m_InitialDelayTimer {};
	 bool m_IsdelayToEnable {};

	 std::vector<std::unique_ptr<EffectBase>> m_Effects {};
public:

	void Init();
	//bool IsAlive();
	//void Disable(CoordStruct location);
	//void ResetDuration();
	//void OnGScreenRender(CoordStruct location);
	//void OnUpdate(CoordStruct location, bool isDead);
	//void OnWarpUpdate(CoordStruct location, bool isDead);
	//void OnTemporalUpdate(TemporalClass* pTemporal);
	//void OnTemporalEliminate(TemporalClass* pTemporal);
	//void OnRocketExplosion();
	//void OnPut(CoordStruct* pCoord, DirType dirType);
	//void OnRemove();
	//void OnReceiveDamage(int* pDamage, int DistanceFromEpicenter, WarheadTypeClass* pWH, ObjectClass* pAttacker, bool IgnoreDefenses, bool PreventPassengerEscape, HouseClass* pAttackingHouse);
	//void OnReceiveDamage2(int* pRealDamage, WarheadTypeClass* pWH, DamageState damageState, ObjectClass* pAttacker, HouseClass* pAttackingHouse);
	//void OnReceiveDamageDestroy();
	//void OnGuardCommand();
	//void OnStopCommand();

	void SetupLifeTimer();
	void Enable(TechnoClass* pSource, HouseClass* pSourceHouse, const CoordStruct& warheadLocation, int aeMode, bool fromPassenger);

private :

	void EnableEffects();

	template <typename T>
	bool Serialize(T& Stm)
	{
		return true;
	}

public:

	void InvalidatePointer(AbstractClass* ptr, bool bRemoved)
	{
		AnnounceInvalidPointer(m_Source, ptr);
		AnnounceInvalidPointer(m_SourceHouse, ptr);
	}

	static std::vector<std::unique_ptr<AttachEffect>> Array;

	static void PointerGotInvalid(AbstractClass* ptr, bool bRemoved)
	{
		for (size_t i = 0; i < Array.size(); ++i)
		{
			auto& pThis = Array.at(i);

			if (pThis->m_AttachedTo == ptr)
				Array.erase(Array.begin() + i);
			else
				pThis->InvalidatePointer(ptr, bRemoved);
		}
	}

	static void Clear()
	{
		Array.clear();
	}

	static bool LoadGlobals(PhobosStreamReader& Stm)
	{
		Clear();

		size_t Count = 0;
		if (!Stm.Load(Count))
			return false;


		for (size_t i = 0; i < Count; ++i)
		{
			AttachEffect* oldPtr = nullptr;
			if (!Stm.Load(oldPtr))
				return false;

			auto newPtr = std::make_unique<AttachEffect>();
			PhobosSwizzle::Instance.RegisterChange(oldPtr, newPtr.get());

			if (!newPtr->Serialize(Stm))
				break;
		}

		return true;
	}

	static bool SaveGlobals(PhobosStreamWriter& Stm)
	{
		Stm.Save(Array.size());

		for (const auto& item : Array)
		{
			Stm.Save(item.get());
			if (!item->Serialize(Stm))
				break;
		}

		return true;
	}
};