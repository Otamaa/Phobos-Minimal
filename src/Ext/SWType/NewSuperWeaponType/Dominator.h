#pragma once

#include "NewSWType.h"
#include "SWStateMachine.h"

class SW_PsychicDominator : public NewSWType
{
public:
	virtual bool HandleThisType(SuperWeaponType type) const override;
	virtual SuperWeaponFlags Flags() const override;

	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer) override;
	virtual bool AbortFire(SuperClass* pSW, bool IsPlayer) override;

	virtual void Initialize(SWTypeExt::ExtData* pData) override;
	virtual void LoadFromINI(SWTypeExt::ExtData* pData,  CCINIClass* pINI) override;

	virtual WarheadTypeClass* GetWarhead(const SWTypeExt::ExtData* pData) const override;
	virtual int GetDamage(const SWTypeExt::ExtData* pData) const override;
	virtual SWRange GetRange(const SWTypeExt::ExtData* pData) const override;

	static SuperClass* CurrentPsyDom;

	void newStateMachine(CellStruct XY, SuperClass* pSuper) {
		SWStateMachine::Register(std::make_unique<PsychicDominatorStateMachine>(XY, pSuper, this));
	}
};