#pragma once

#include "SWTypeHandler.h"
#include "SWStateMachine.h"

class SW_SonarPulse : public SWTypeHandler
{
public:
	virtual SuperWeaponFlags Flags(const SWTypeExtData* pData) const override;

	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer);

	virtual void Initialize(SWTypeExtData* pData) override;
	virtual void LoadFromINI(SWTypeExtData* pData, CCINIClass* pINI) override;

	virtual bool IsLaunchSite(const SWTypeExtData* pData, BuildingClass* pBuilding) const override;

	virtual SWRange GetRange(const SWTypeExtData* pData) const override;

protected:
	void newStateMachine(int Deferment, CellStruct XY, SuperClass* pSuper) {
		SWStateMachine::Array.push_back(std::move(std::make_unique<SonarPulseStateMachine>(Deferment, XY, pSuper, this)));
	}
};