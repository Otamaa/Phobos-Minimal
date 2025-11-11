#pragma once

#include "SWTypeHandler.h"
#include "SWStateMachine.h"

class SW_EMPulse : public SWTypeHandler
{
public:

	virtual SuperWeaponType GetSWType() { return SuperWeaponType(NewSuperType::EMPulse); };
	virtual SuperWeaponFlags Flags(const SWTypeExtData* pData) const
	{
		return SuperWeaponFlags::None;
	}

	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer);

	virtual void Initialize(SWTypeExtData* pData) override;
	virtual void LoadFromINI(SWTypeExtData* pData, CCINIClass* pINI) override;

	virtual bool IsLaunchSite(const SWTypeExtData* pData, BuildingClass* pBuilding) const override;
	virtual std::pair<double, double> GetLaunchSiteRange(const SWTypeExtData* pData, BuildingClass* pBuilding) const override;

};