#pragma once

#include <Utilities/Enum.h>
#include <CoordStruct.h>

class PhobosStreamReader;
class PhobosStreamWriter;
class AbstractClass;
struct args_ReceiveDamage;
struct IEntity
{
	EntityType m_Type { EntityType::None };
	AbstractClass* m_OwnerObject { nullptr };

	static void SetUP(IEntity* ett , EntityType type, AbstractClass* Owner) {
		ett->m_Type = type;
		ett->m_OwnerObject = Owner;
	}

	virtual void OnInit() = 0;
	virtual void OnUnInit() = 0;
	virtual void OnDetonate(CoordStruct* location) = 0;
	virtual void OnPut(CoordStruct pCoord, short faceDirValue8) = 0;
	virtual void OnRemove() = 0;
	virtual void OnReceiveDamage(args_ReceiveDamage* args) = 0;
	virtual void OnFire(AbstractClass* pTarget, int weaponIndex) = 0;
	virtual void OnUpdate() = 0;

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) = 0;
	virtual bool Save(PhobosStreamWriter& Stm) const = 0;
	virtual void InvalidatePointer(AbstractClass* ptr, bool bDetach) = 0;
};
