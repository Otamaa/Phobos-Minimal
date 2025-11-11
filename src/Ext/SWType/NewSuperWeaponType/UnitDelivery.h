#pragma once

#include "SWTypeHandler.h"
#include "SWStateMachine.h"

class SW_UnitDelivery : public SWTypeHandler
{
public:

	virtual SuperWeaponType GetSWType() { return SuperWeaponType(NewSuperType::UnitDelivery); };
	virtual SuperWeaponFlags Flags(const SWTypeExtData* pData) const
	{
		return SuperWeaponFlags::None;
	}

	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer);

	virtual void Initialize(SWTypeExtData* pData) override;
	virtual void LoadFromINI(SWTypeExtData* pData, CCINIClass* pINI) override;

	virtual bool IsLaunchSite(const SWTypeExtData* pData, BuildingClass* pBuilding) const override;

protected:
	void newStateMachine(int Duration, CellStruct XY, SuperClass* pSuper) {
		SWStateMachine::Array.push_back(std::move(std::make_unique<UnitDeliveryStateMachine>(Duration, XY, pSuper, this)));
	}
};
