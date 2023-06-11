#pragma once

#include "NewSWType.h"

class SW_Protect : public NewSWType
{
public:
	virtual const char* GetTypeString() const override;
	virtual bool HandleThisType(SuperWeaponType type) const override;

	virtual bool CanFireAt(TargetingData const& data, const CellStruct& cell, bool manual) const override;

	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer) override;
	
	virtual void Initialize(SWTypeExt::ExtData* pData) override;
	virtual void LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI) override;

	virtual AnimTypeClass* GetAnim(const SWTypeExt::ExtData* pData) const override;
	virtual SWRange GetRange(const SWTypeExt::ExtData* pData) const override;
};
