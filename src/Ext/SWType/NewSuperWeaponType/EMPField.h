#pragma once

#include "NewSWType.h"

class SW_EMPField : public NewSWType
{
public:
	virtual std::vector<const char*> GetTypeString() const override;

	virtual void Initialize(SWTypeExtData* pData) override;
	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer) override;
	virtual void LoadFromINI(SWTypeExtData* pData, CCINIClass* pINI) override;
	virtual bool IsLaunchSite(const SWTypeExtData* pData, BuildingClass* pBuilding) const override;

};
