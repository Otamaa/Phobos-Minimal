#pragma once

#include "SWTypeHandler.h"
#include "SWStateMachine.h"

class SW_MeteorShower : public SWTypeHandler
{
public:

	virtual SuperWeaponType GetSWType() { return SuperWeaponType(NewSuperType::MeteorShower); };
	virtual SuperWeaponFlags Flags(const SWTypeExtData* pData) const
	{
		return SuperWeaponFlags::None;
	}

	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer);

	virtual void Initialize(SWTypeExtData* pData) override;
	virtual void LoadFromINI(SWTypeExtData* pData, CCINIClass* pINI) override;

	virtual bool IsLaunchSite(const SWTypeExtData* pData, BuildingClass* pBuilding) const override;

protected:
	void newStateMachine(int Deferment, CellStruct XY, SuperClass* pSuper, TechnoClass* pfirer) {
		SWStateMachine::Array.push_back(std::move(std::make_unique<MeteorShowerStateMachine>(Deferment, XY, pSuper, pfirer, this)));
	}
};
