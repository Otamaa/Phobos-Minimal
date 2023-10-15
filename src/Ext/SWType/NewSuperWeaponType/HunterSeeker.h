#pragma once

#include "NewSWType.h"

class UnitTypeClass;
class SW_HunterSeeker : public NewSWType
{
public:
	virtual std::vector<const char*> GetTypeString() const override;

	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer) override;
	virtual void Initialize(SWTypeExtData* pData) override;
	virtual void LoadFromINI(SWTypeExtData* pData, CCINIClass* pINI) override;
	virtual bool IsLaunchSite(const SWTypeExtData* pData, BuildingClass* pBuilding) const override;
	CellStruct GetLaunchCell(SWTypeExtData* pSWType, BuildingClass* pBuilding, UnitTypeClass* pHunter) const;

	// ignore all building state and keep lauching the sw regardless ,..
	bool IsLaunchSite_HS(const SWTypeExtData* pData, BuildingClass* pBuilding) const;
};
