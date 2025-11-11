#pragma once

#include "SWTypeHandler.h"

class SuperWeaponTypeClass;
class SWTypeExtData;
class WarheadTypeClass;
class WeaponTypeClass;
class CCINIClass;
class TechnoClass;
class BuildingClass;
class BuildingClass;
class SW_NuclearMissile : public SWTypeHandler
{
public:

	virtual SuperWeaponType GetSWType() { return SuperWeaponType::Nuke;  };
	virtual SuperWeaponFlags Flags(const SWTypeExtData* pData) const override;

	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer);

	virtual void Initialize(SWTypeExtData* pData) override;
	virtual void LoadFromINI(SWTypeExtData* pData, CCINIClass* pINI) override;

	virtual bool IsLaunchSite(const SWTypeExtData* pData, BuildingClass* pBuilding) const override;

	virtual WarheadTypeClass* GetWarhead(const SWTypeExtData* pData) const override;
	virtual int GetDamage(const SWTypeExtData* pData) const override;

	BuildingClass* GetAlternateLauchSite(const SWTypeExtData* pData, SuperClass* pThis) const;

	static SuperWeaponTypeClass* CurrentNukeType;
	static bool DropNukeAt(SuperWeaponTypeClass* pSuper , CoordStruct const& to, TechnoClass* Owner , HouseClass* OwnerHouse , WeaponTypeClass* pPayload);
};
