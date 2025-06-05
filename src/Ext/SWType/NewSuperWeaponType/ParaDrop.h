#pragma once

#include "NewSWType.h"
#include "SWStateMachine.h"

// Forward declaration
struct ParadropPlaneData;

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
	std::vector<ParadropPlaneData> GetParadropData(SuperClass* pThis) const;

	static bool SendPDPlane(HouseClass* pOwner, CellClass* pDestination,
		AircraftTypeClass* pPlaneType, Iterator<TechnoTypeClass*> Types,
		Iterator<int> Nums);

protected:
	void newStateMachine(int Deferment, CellStruct XY, SuperClass* pSuper, CellClass* pTarget) {
		SWStateMachine::Array.push_back(std::move(std::make_unique<ParaDropStateMachine>(Deferment, XY, pSuper, this, pTarget)));
	}
};
