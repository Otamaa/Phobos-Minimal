#pragma once

#include "SWTypeHandler.h"
#include "SWStateMachine.h"

class SW_ChronoWarp : public SWTypeHandler
{
public:
	virtual SuperWeaponFlags Flags(const SWTypeExtData* pData) const;

	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer);

	virtual void Initialize(SWTypeExtData* pData) override;
	virtual void LoadFromINI(SWTypeExtData* pData, CCINIClass* pINI) { };

	virtual bool IsLaunchSite(const SWTypeExtData* pData, BuildingClass* pBuilding) const override;

protected:
	void newStateMachine(int Duration, const CellStruct& XY, SuperClass* pSuper, SWTypeHandler* pSWType,
	HelperedVector<ChronoWarpStateMachine::ChronoWarpContainer> Buildings) {
		SWStateMachine::Array.push_back(std::move(std::make_unique<ChronoWarpStateMachine>(Duration, XY, pSuper, this, std::move(Buildings))));
	}

};
