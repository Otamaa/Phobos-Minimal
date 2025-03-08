#pragma once

#include "NewSWType.h"
#include "SWStateMachine.h"

class SW_SonarPulse : public NewSWType
{
public:
	virtual std::vector<const char*> GetTypeString() const override;
	virtual SuperWeaponFlags Flags(const SWTypeExtData* pData) const override;

	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer) override;
	virtual void Deactivate(SuperClass* pThis, CellStruct cell, bool isPlayer) override { }

	virtual void Initialize(SWTypeExtData* pData) override;
	virtual void LoadFromINI(SWTypeExtData* pData, CCINIClass* pINI) override;
	virtual bool IsLaunchSite(const SWTypeExtData* pData, BuildingClass* pBuilding) const override;

	virtual SWRange GetRange(const SWTypeExtData* pData) const override;

	using TStateMachine = SonarPulseStateMachine;

protected:
	void newStateMachine(int Deferment, CellStruct XY, SuperClass* pSuper) {
		SWStateMachine::Register<TStateMachine>(Deferment, XY, pSuper, this);
	}
};