#pragma once

#include "SWTypeHandler.h"
#include "SWStateMachine.h"

class SW_ParaDrop : public SWTypeHandler
{
public:

	virtual SuperWeaponType GetSWType() { return SuperWeaponType::ParaDrop; };

	virtual SuperWeaponFlags Flags(const SWTypeExtData* pData) const
	{
		return SuperWeaponFlags::None;
	}

	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer);

	virtual void Initialize(SWTypeExtData* pData) override;
	virtual void LoadFromINI(SWTypeExtData* pData, CCINIClass* pINI) override;

	virtual bool IsLaunchSite(const SWTypeExtData* pData, BuildingClass* pBuilding) const override;

	bool SendParadrop(SuperClass* pThis, CellClass* pCell);

	static void SendPDPlane(HouseClass* pOwner, CellClass* pDestination,
		AircraftTypeClass* pPlaneType, Iterator<TechnoTypeClass*> Types,
		Iterator<int> Nums);

protected:
	void newStateMachine(int Deferment, CellStruct XY, SuperClass* pSuper, CellClass* pTarget) {
		SWStateMachine::Array.push_back(std::move(std::make_unique<ParaDropStateMachine>(Deferment, XY, pSuper, this, pTarget)));
	}
};


class SW_AmericanParaDrop : public SW_ParaDrop
{
public:
	virtual SuperWeaponType GetSWType() override { return SuperWeaponType::AmerParaDrop; };
};
