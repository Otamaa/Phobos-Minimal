#pragma once

#include "SWTypeHandler.h"
#include "SWStateMachine.h"

class SW_GeneticMutator : public SWTypeHandler
{
public:

	virtual SuperWeaponType GetSWType() { return SuperWeaponType::GeneticMutator; };

	virtual SuperWeaponFlags Flags(const SWTypeExtData* pData) const
	{
		return SuperWeaponFlags::None;
	}

	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer);

	virtual void Initialize(SWTypeExtData* pData) override;
	virtual void LoadFromINI(SWTypeExtData* pData, CCINIClass* pINI) override;

	virtual WarheadTypeClass* GetWarhead(const SWTypeExtData* pData) const override;
	virtual AnimTypeClass* GetAnim(const SWTypeExtData* pData) const override;
	virtual int GetSound(const SWTypeExtData* pData) const override;
	virtual int GetDamage(const SWTypeExtData* pData) const override;
	virtual SWRange GetRange(const SWTypeExtData* pData) const override;

protected:
	void newStateMachine(int Deferment, CellStruct XY, SuperClass* pSuper, TechnoClass* pfirer) {
		SWStateMachine::Array.push_back(std::move(std::make_unique<GeneticMutatorStateMachine>(Deferment, XY, pSuper , pfirer, this)));
	}
};
