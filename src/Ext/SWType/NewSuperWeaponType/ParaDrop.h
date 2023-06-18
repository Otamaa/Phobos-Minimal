#pragma once

#include "NewSWType.h"

class SW_ParaDrop : public NewSWType
{
public:

	virtual bool HandleThisType(SuperWeaponType type) const override;

	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer) override;

	virtual void Initialize(SWTypeExt::ExtData* pData) override;
	virtual void LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI) override;

	bool SendParadrop(SuperClass* pThis, CellClass* pCell);

	static void SendPDPlane(HouseClass* pOwner, CellClass* pDestination,
		AircraftTypeClass* pPlaneType, Iterator<TechnoTypeClass*> Types,
		Iterator<int> Nums);
};
