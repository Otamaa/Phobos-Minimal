#pragma once

#include "NewSWType.h"
#include "SWStateMachine.h"

class SW_GeneticMutator : public NewSWType
{
public:
	virtual bool HandleThisType(SuperWeaponType type) const override;

	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer) override;

	virtual void Initialize(SWTypeExtData* pData) override;
	virtual void LoadFromINI(SWTypeExtData* pData, CCINIClass* pINI) override;

	virtual WarheadTypeClass* GetWarhead(const SWTypeExtData* pData) const override;
	virtual AnimTypeClass* GetAnim(const SWTypeExtData* pData) const override;
	virtual int GetSound(const SWTypeExtData* pData) const override;
	virtual int GetDamage(const SWTypeExtData* pData) const override;
	virtual SWRange GetRange(const SWTypeExtData* pData) const override;

	using TStateMachine = GeneticMutatorStateMachine;

protected:
	void newStateMachine(int Deferment, CellStruct XY, SuperClass* pSuper, TechnoClass* pfirer) {
		SWStateMachine::Register<TStateMachine>(Deferment, XY, pSuper, pfirer, this);
	}
};
