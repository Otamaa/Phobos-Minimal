#pragma once

#include "NewSWType.h"

class SW_NuclearMissile : public NewSWType
{
public:
	virtual bool HandleThisType(SuperWeaponType type) const override;
	virtual SuperWeaponFlags Flags() const override;

	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer) override;
	
	virtual void Initialize(SWTypeExt::ExtData* pData) override;
	virtual void LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI) override;

	virtual bool IsLaunchSite(const SWTypeExt::ExtData* pData, BuildingClass* pBuilding) const override;

	virtual WarheadTypeClass* GetWarhead(const SWTypeExt::ExtData* pData) const override;
	virtual int GetDamage(const SWTypeExt::ExtData* pData) const override;

	static SuperClass* CurrentNukeType;

};
