#pragma once

#include "SWTypeHandler.h"
#include "SWStateMachine.h"

class SW_Protect : public SWTypeHandler
{
public:
	virtual SuperWeaponFlags Flags(const SWTypeExtData* pData) const { return SuperWeaponFlags::None; }

	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer);

	virtual void Initialize(SWTypeExtData* pData) override;
	virtual void LoadFromINI(SWTypeExtData* pData, CCINIClass* pINI) override;

	virtual bool IsLaunchSite(const SWTypeExtData* pData, BuildingClass* pBuilding) const override;

	virtual AnimTypeClass* GetAnim(const SWTypeExtData* pData) const override;
	virtual SWRange GetRange(const SWTypeExtData* pData) const override;

	virtual bool CanTargetingFireAt(const TargetingData* pTargeting, const CellStruct& cell, bool manual) const override;

	static void ApplyProtect(SuperClass* pThis, const CellStruct& Coords, SWRange range);

protected:
	void newStateMachine(int Deferment, CellStruct XY, SuperClass* pSuper) {
		SWStateMachine::Array.push_back(std::move(std::make_unique<ProtectStateMachine>(Deferment, XY, pSuper, this)));
	}
};
