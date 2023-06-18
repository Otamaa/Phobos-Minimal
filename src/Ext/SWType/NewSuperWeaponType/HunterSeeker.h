#pragma once

#include "NewSWType.h"

class UnitTypeClass;
class SW_HunterSeeker : public NewSWType
{
public:
	virtual std::vector<const char*> GetTypeString() const override;

	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer) override;
	virtual void Initialize(SWTypeExt::ExtData* pData) override;
	virtual void LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI) override;
	CellStruct GetLaunchCell(SWTypeExt::ExtData* pSWType, BuildingClass* pBuilding, UnitTypeClass* pHunter) const;

};
