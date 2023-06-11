#pragma once

#include "NewSWType.h"

class SW_Firewall : public NewSWType
{
public:
	virtual const char* GetTypeString() const override;

	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer) override;
	virtual void Deactivate(SuperClass* pThis, CellStruct cell, bool isPlayer) override;

	virtual void Initialize(SWTypeExt::ExtData* pData) { }
	virtual void LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI) override;

};