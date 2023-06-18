#pragma once

#include "NewSWType.h"

class SW_IonCannon : public NewSWType
{
public:
	virtual std::vector<const char*> GetTypeString() const override;
	virtual SuperWeaponFlags Flags() const override;

	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer) override;
	virtual void Initialize(SWTypeExt::ExtData* pData) override;
	virtual void LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI) override;
	virtual bool IsLaunchSite(SWTypeExt::ExtData* pSWType, BuildingClass* pBuilding) const;
	virtual std::pair<double, double> GetLaunchSiteRange(SWTypeExt::ExtData* pSWType, BuildingClass* pBuilding = nullptr) const;
	virtual SWRange GetRange(const SWTypeExt::ExtData* pData) const;
};
