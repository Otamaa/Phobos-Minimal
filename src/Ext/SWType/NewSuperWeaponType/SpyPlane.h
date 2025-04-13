#pragma once

#include "NewSWType.h"
#include "SWStateMachine.h"

class SW_SpyPlane : public NewSWType
{
public:
	virtual std::vector<const char*> GetTypeString() const override;
	virtual bool HandleThisType(SuperWeaponType type) const override;

	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer) override;

	virtual void Initialize(SWTypeExtData* pData) override;
	virtual void LoadFromINI(SWTypeExtData* pData, CCINIClass* pINI) override;
	virtual bool IsLaunchSite(const SWTypeExtData* pData, BuildingClass* pBuilding) const override;

protected:
	void newStateMachine(int Deferment, CellStruct XY, SuperClass* pSuper ,CellClass* pTarget) {
		SWStateMachine::Array.push_back(std::move(new(SpyPlaneStateMachine::SpyPlaneStateMachine_GLUE_NOT_IMPLEMENTED) SpyPlaneStateMachine(Deferment, XY, pSuper, this, pTarget)));
	}
};
