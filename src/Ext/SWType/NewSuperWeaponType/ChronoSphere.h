#pragma once

#include "NewSWType.h"

class SW_ChronoSphere : public NewSWType
{
public:
	virtual bool HandleThisType(SuperWeaponType type) const override;
	virtual SuperWeaponFlags Flags(const SWTypeExt::ExtData* pData) const override;

	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer) override;

	virtual void Initialize(SWTypeExt::ExtData* pData) override;
	virtual void LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI) override;
	virtual bool IsLaunchSite(const SWTypeExt::ExtData* pData, BuildingClass* pBuilding) const override;

	virtual AnimTypeClass* GetAnim(const SWTypeExt::ExtData* pData) const override;
	virtual SWRange GetRange(const SWTypeExt::ExtData* pData) const override;
};