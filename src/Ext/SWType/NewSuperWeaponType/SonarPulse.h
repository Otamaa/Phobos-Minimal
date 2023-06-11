#pragma once

#include "NewSWType.h"

class SW_SonarPulse : public NewSWType
{
public:
	virtual const char* GetTypeString() const override;

	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer) override;
	virtual void Deactivate(SuperClass* pThis, CellStruct cell, bool isPlayer) override { }

	virtual void Initialize(SWTypeExt::ExtData* pData) override;
	virtual void LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI) override;

	virtual SWRange GetRange(const SWTypeExt::ExtData* pData) const override;
};