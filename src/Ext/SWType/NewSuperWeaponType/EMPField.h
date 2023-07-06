#pragma once

#include "NewSWType.h"

class SW_EMPField : public NewSWType
{
public:
	virtual std::vector<const char*> GetTypeString() const override;

	virtual void Initialize(SWTypeExt::ExtData* pData) override;
	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer) override;
	virtual void LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI) override;
	virtual bool IsLaunchSite(const SWTypeExt::ExtData* pData, BuildingClass* pBuilding) const override;

};