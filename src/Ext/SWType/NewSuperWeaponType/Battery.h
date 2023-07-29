#pragma once

#include "NewSWType.h"

class SW_Battery : public NewSWType
{
public:
	virtual std::vector<const char*> GetTypeString() const override;

	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer) override;
	virtual void Deactivate(SuperClass* pSW, CellStruct cell, bool isPlayer) override;
	virtual void Initialize(SWTypeExt::ExtData* pData) override;
	virtual void LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI) override;
	virtual bool IsLaunchSite(const SWTypeExt::ExtData* pData, BuildingClass* pBuilding) const override;
};
