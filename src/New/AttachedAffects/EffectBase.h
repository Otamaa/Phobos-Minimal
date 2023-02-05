#pragma once

#include <guiddef.h>
#include <Utilities/SavegameDef.h>

class ObjectClass;
class TemporalClass;
class WarheadTypeClass;
class HouseClass;
struct AttachEffect;
class EffectBaseType;
class EffectBase
{
public :

	std::string m_UniqueID {};
	EffectBaseType* m_Type {};
	AttachEffect* m_ParentAE {};
	ObjectClass* m_OwnerObject {};
	bool m_IsActive {};


	EffectBase() = default;
	virtual ~EffectBase() = default;

	EffectBase(EffectBaseType* pType) : m_Type { pType }
	{ }

	// 返回AE是否还存活
	virtual bool IsAlive() = 0;
	virtual void Enable(AttachEffect* AE) = 0;
	virtual void Disable(CoordStruct location) = 0;
	virtual void ResetDuration() = 0;
	virtual void OnGScreenRender(CoordStruct location) = 0;
	virtual void OnUpdate(CoordStruct location, bool isDead) = 0;
	virtual void OnWarpUpdate(CoordStruct location, bool isDead) = 0;
	virtual void OnTemporalUpdate(TemporalClass* pTemporal) = 0;
	virtual void OnTemporalEliminate(TemporalClass* pTemporal) = 0;
	virtual void OnRocketExplosion() = 0;
	virtual void OnPut(CoordStruct* pCoord, DirType dirType) = 0;
	virtual void OnRemove() = 0;
	virtual void OnReceiveDamage(int* pDamage, int DistanceFromEpicenter, WarheadTypeClass* pWH, ObjectClass* pAttacker, bool IgnoreDefenses, bool PreventPassengerEscape, HouseClass* pAttackingHouse) = 0;
	virtual void OnReceiveDamage2(int* pRealDamage, WarheadTypeClass* pWH, DamageState damageState, ObjectClass* pAttacker, HouseClass* pAttackingHouse) = 0;
	virtual void OnReceiveDamageDestroy() = 0;
	virtual void OnGuardCommand() = 0;
	virtual void OnStopCommand() = 0;


	virtual bool Load(PhobosStreamReader& stm, bool RegisterForChange) = 0;
	virtual bool Save(PhobosStreamWriter& stm) = 0;

	bool LoadFromStream(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		return Stm
			.Process(this->m_UniqueID)
			.Process(this->m_Type)
			.Process(this->m_ParentAE)
			.Process(this->m_OwnerObject)
			.Process(this->m_IsActive)
			.Success()
			&& Stm.RegisterChange(this); // announce this type
	}

	bool SaveToStream(PhobosStreamWriter& Stm)
	{
		return Stm
			.Process(this->m_UniqueID)
			.Process(this->m_Type)
			.Process(this->m_ParentAE)
			.Process(this->m_OwnerObject)
			.Process(this->m_IsActive)
			.Success()
			&& Stm.RegisterChange(this); // announce this type
	}
};