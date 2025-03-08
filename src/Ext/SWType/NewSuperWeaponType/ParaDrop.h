#pragma once

#include "NewSWType.h"
#include "SWStateMachine.h"

class SW_ParaDrop : public NewSWType
{
public:
	virtual std::vector<const char*> GetTypeString() const override;
	virtual bool HandleThisType(SuperWeaponType type) const override;

	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer) override;

	virtual void Initialize(SWTypeExtData* pData) override;
	virtual void LoadFromINI(SWTypeExtData* pData, CCINIClass* pINI) override;
	virtual bool IsLaunchSite(const SWTypeExtData* pData, BuildingClass* pBuilding) const override;

	bool SendParadrop(SuperClass* pThis, CellClass* pCell);

	static void SendPDPlane(HouseClass* pOwner, CellClass* pDestination,
		AircraftTypeClass* pPlaneType, Iterator<TechnoTypeClass*> Types,
		Iterator<int> Nums);

	using TStateMachine = ParaDropStateMachine;

protected:
	TStateMachine* newStateMachine(int Deferment, CellStruct XY, SuperClass* pSuper, CellClass* pTarget) {
		return SWStateMachine::Register<TStateMachine>(Deferment, XY, pSuper, this, pTarget);
	}
};
