#pragma once

#include "NewSWType.h"
#include "SWStateMachine.h"

class SW_DropPod : public NewSWType
{
public:
	virtual std::vector<const char*> GetTypeString() const override;
	virtual SuperWeaponFlags Flags(const SWTypeExt::ExtData* pData) const override;

	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer) override;
	virtual void Initialize(SWTypeExt::ExtData* pData) override;
	virtual void LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI) override;

	using TStateMachine = DroppodStateMachine;

	void newStateMachine(int Duration, int Deferment, CellStruct XY, SuperClass* pSuper) {
		SWStateMachine::Register(std::make_unique<TStateMachine>(Duration, Deferment, XY, pSuper, this));
	}
};