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
	 TimerStruct m_LifeTimer {};
	 TimerStruct m_InitialDelayTimer {};
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

public:

	inline bool Load(PhobosStreamReader& stm, bool RegisterForChange)
	{
		return true
			;
	}

	inline bool Save(PhobosStreamWriter& stm)
	{
		return true
			;
	}
};