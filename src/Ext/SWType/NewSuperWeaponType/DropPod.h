#pragma once

#include "NewSWType.h"
#include "SWStateMachine.h"

class SW_DropPod : public NewSWType
{
public:
	virtual std::vector<const char*> GetTypeString() const override;

	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer) override;
	virtual void Initialize(SWTypeExtData* pData) override;
	virtual void LoadFromINI(SWTypeExtData* pData, CCINIClass* pINI) override;
	virtual bool IsLaunchSite(const SWTypeExtData* pData, BuildingClass* pBuilding) const override;

	using TStateMachine = DroppodStateMachine;

protected:
	void newStateMachine(int Deferment, CellStruct XY, SuperClass* pSuper) {
		SWStateMachine::Register<TStateMachine>(Deferment, XY, pSuper, this);
	}
};
