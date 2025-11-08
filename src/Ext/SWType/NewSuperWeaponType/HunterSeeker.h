#pragma once

#include "SWTypeHandler.h"

class UnitTypeClass;
class SW_HunterSeeker : public SWTypeHandler
{
public:
	virtual SuperWeaponFlags Flags(const SWTypeExtData* pData) const
	{
		return SuperWeaponFlags::None;
	}

	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer);

	virtual void Initialize(SWTypeExtData* pData) override;
	virtual void LoadFromINI(SWTypeExtData* pData, CCINIClass* pINI) override;

	virtual bool IsLaunchSite(const SWTypeExtData* pData, BuildingClass* pBuilding) const override;
	CellStruct GetLaunchCell(SWTypeExtData* pSWType, BuildingClass* pBuilding, UnitTypeClass* pHunter) const;

	// ignore all building state and keep lauching the sw regardless ,..
	bool IsLaunchSite_HS(const SWTypeExtData* pData, BuildingClass* pBuilding) const;
};
