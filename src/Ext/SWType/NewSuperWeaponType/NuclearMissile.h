#pragma once

#include "NewSWType.h"

class SW_NuclearMissile : public NewSWType
{
public:
	virtual std::vector<const char*> GetTypeString() const override;
	virtual bool HandleThisType(SuperWeaponType type) const override;
	virtual SuperWeaponFlags Flags(const SWTypeExtData* pData) const override;

	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer) override;

	virtual void Initialize(SWTypeExtData* pData) override;
	virtual void LoadFromINI(SWTypeExtData* pData, CCINIClass* pINI) override;

	virtual bool IsLaunchSite(const SWTypeExtData* pData, BuildingClass* pBuilding) const override;

	virtual WarheadTypeClass* GetWarhead(const SWTypeExtData* pData) const override;
	virtual int GetDamage(const SWTypeExtData* pData) const override;

	BuildingClass* GetAlternateLauchSite(const SWTypeExtData* pData, SuperClass* pThis);

	static SuperWeaponTypeClass* CurrentNukeType;
	static bool DropNukeAt(SuperWeaponTypeClass* pSuper , CoordStruct const& to, TechnoClass* Owner , HouseClass* OwnerHouse , WeaponTypeClass* pPayload);
};
