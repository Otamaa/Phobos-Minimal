#pragma once

#include "EMPulse.h"

class SW_MultiLauncher : public SW_EMPulse
{
public:
	virtual const char* GetTypeString() const override;

	virtual void Initialize(SWTypeExt::ExtData* pData) override;

	//virtual AresNewSuperType GetTypeIndex() const { return AresNewSuperType::EMPulse; }
};