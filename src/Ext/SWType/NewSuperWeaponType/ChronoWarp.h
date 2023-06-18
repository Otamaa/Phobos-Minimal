#pragma once

#include "NewSWType.h"
#include "SWStateMachine.h"

class SW_ChronoWarp : public NewSWType
{
public:
	virtual bool HandleThisType(SuperWeaponType type) const override;
	virtual SuperWeaponFlags Flags() const override;

	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer) override;

	virtual void Initialize(SWTypeExt::ExtData* pData) override;

	void newStateMachine(int Duration, const CellStruct& XY, SuperClass* pSuper, NewSWType* pSWType,
	DynamicVectorClass<ChronoWarpStateMachine::ChronoWarpContainer> Buildings)
	{
		SWStateMachine::Register(std::make_unique<ChronoWarpStateMachine>(Duration, XY, pSuper, this, std::move(Buildings)));
	}

};
