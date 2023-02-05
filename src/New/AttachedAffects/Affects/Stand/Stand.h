#pragma once

#include "../../EffectBase.h"

class ObjectClass;
class TemporalClass;
class WarheadTypeClass;
class HouseClass;
struct AttachEffect;
class EffectBaseType;
class Stand : public EffectBase
{
public:

	std::string m_UniqueID {};
	EffectBaseType* m_Type {};
	AttachEffect* m_ParentAE {};
	ObjectClass* m_OwnerObject {};
	bool m_IsActive {};


	Stand() = default;
	virtual ~Stand() = default;

	Stand(EffectBaseType* pType) : EffectBase { pType }
	{ }

	// 返回AE是否还存活
	virtual bool IsAlive() override { return false;  }
	virtual void Enable(AttachEffect* AE) override { }
	virtual void Disable(CoordStruct location) override { }
	virtual void ResetDuration() override { }
	virtual void OnGScreenRender(CoordStruct location) override { }
	virtual void OnUpdate(CoordStruct location, bool isDead) override { }
	virtual void OnWarpUpdate(CoordStruct location, bool isDead) override { }
	virtual void OnTemporalUpdate(TemporalClass* pTemporal) override { }
	virtual void OnTemporalEliminate(TemporalClass* pTemporal) override { }
	virtual void OnRocketExplosion() override { }
	virtual void OnPut(CoordStruct* pCoord, DirType dirType) override { }
	virtual void OnRemove() override { }
	virtual void OnReceiveDamage(int* pDamage, int DistanceFromEpicenter, WarheadTypeClass* pWH, ObjectClass* pAttacker, bool IgnoreDefenses, bool PreventPassengerEscape, HouseClass* pAttackingHouse) override { }
	virtual void OnReceiveDamage2(int* pRealDamage, WarheadTypeClass* pWH, DamageState damageState, ObjectClass* pAttacker, HouseClass* pAttackingHouse)  override { }
	virtual void OnReceiveDamageDestroy() override { }
	virtual void OnGuardCommand() override { }
	virtual void OnStopCommand() override { }

	virtual bool Load(PhobosStreamReader& stm, bool RegisterForChange) override {
		return EffectBase::LoadFromStream(stm, RegisterForChange);
	}

	virtual bool Save(PhobosStreamWriter& stm) {
		return EffectBase::SaveToStream(stm);
	}
};