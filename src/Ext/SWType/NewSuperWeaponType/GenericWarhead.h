#pragma once

#include "NewSWType.h"

class SW_GenericWarhead : public NewSWType
{
public:
	virtual const char* GetTypeString() const override;

	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer) override;
	virtual void Initialize(SWTypeExt::ExtData* pData) override;
};
