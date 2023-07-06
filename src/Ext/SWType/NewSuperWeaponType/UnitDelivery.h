#pragma once

#include "NewSWType.h"
#include "SWStateMachine.h"

class SW_UnitDelivery : public NewSWType
{
public:
	virtual std::vector<const char*> GetTypeString() const override;

	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer) override;

	virtual void Initialize(SWTypeExt::ExtData* pData) override;
	virtual void LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI) override;
	virtual bool IsLaunchSite(const SWTypeExt::ExtData* pData, BuildingClass* pBuilding) const override;

protected:
	void newStateMachine(int Duration, CellStruct XY, SuperClass* pSuper) {
		SWStateMachine::Register(std::make_unique<UnitDeliveryStateMachine>(Duration, XY, pSuper, this));
	}
};
