#pragma once

#include <New/Interfaces/IEntity.h>
#include <string>
#include <vector>
#include "SavegameDef.h"

struct EntityClass : public IEntity
{
	static void SetUP(EntityClass* ett , EntityType type, AbstractClass* Owner, const char* name) {
		IEntity::SetUP(ett ,type, Owner);
		ett->m_Name = name;
	}

	virtual void OnInit() { };
	virtual void OnUnInit() { };
	virtual void OnDetonate(CoordStruct* location) { };
	virtual void OnPut(CoordStruct pCoord, short faceDirValue8) { };
	virtual void OnRemove() { };
	virtual void OnReceiveDamage(args_ReceiveDamage* args) { };
	virtual void OnFire(AbstractClass* pTarget, int weaponIndex) { };
	virtual void OnUpdate() { };

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		Stm
			.Process(m_Type)
			.Process(m_OwnerObject)
			.Process(m_Name);
	}

	virtual bool Save(PhobosStreamWriter& Stm) const
	{
		Stm
			.Process(m_Type)
			.Process(m_OwnerObject)
			.Process(m_Name);
	}

	virtual void InvalidatePointer(AbstractClass* ptr, bool bDetach) { };

	std::string  m_Name { "NONE" };
};
