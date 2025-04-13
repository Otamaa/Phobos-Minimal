#pragma once

#include "NewSWType.h"
#include "SWStateMachine.h"

class SW_ChronoWarp : public NewSWType
{
public:
	virtual bool HandleThisType(SuperWeaponType type) const override;
	virtual SuperWeaponFlags Flags(const SWTypeExtData* pData) const override;

	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer) override;

	virtual void Initialize(SWTypeExtData* pData) override;
	virtual bool IsLaunchSite(const SWTypeExtData* pData, BuildingClass* pBuilding) const override;

protected:
	void newStateMachine(int Duration, const CellStruct& XY, SuperClass* pSuper, NewSWType* pSWType,
	HelperedVector<ChronoWarpStateMachine::ChronoWarpContainer> Buildings) {
		SWStateMachine::Array.push_back(std::move(new(ChronoWarpStateMachine::ChronoWarpStateMachine_GLUE_NOT_IMPLEMENTED) ChronoWarpStateMachine(Duration, XY, pSuper, this, std::move(Buildings))));
	}

};
