#pragma once

#include "NewSWType.h"

class SW_ChronoSphere : public NewSWType
{
public:
	virtual bool HandleThisType(SuperWeaponType type) const override;
	virtual SuperWeaponFlags Flags(const SWTypeExtData* pData) const override;

	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer) override;

	virtual void Initialize(SWTypeExtData* pData) override;
	virtual void LoadFromINI(SWTypeExtData* pData, CCINIClass* pINI) override;
	virtual bool IsLaunchSite(const SWTypeExtData* pData, BuildingClass* pBuilding) const override;

	virtual AnimTypeClass* GetAnim(const SWTypeExtData* pData) const override;
	virtual SWRange GetRange(const SWTypeExtData* pData) const override;
};
