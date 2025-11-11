#pragma once

#include "SWTypeHandler.h"

class SW_EMPField : public SWTypeHandler
{
public:

	virtual SuperWeaponType GetSWType() { return SuperWeaponType(NewSuperType::EMPField); };
	virtual SuperWeaponFlags Flags(const SWTypeExtData* pData) const
	{
		return SuperWeaponFlags::None;
	}

	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer);

	virtual void Initialize(SWTypeExtData* pData) override;
	virtual void LoadFromINI(SWTypeExtData* pData, CCINIClass* pINI) override;

	virtual bool IsLaunchSite(const SWTypeExtData* pData, BuildingClass* pBuilding) const override;

};