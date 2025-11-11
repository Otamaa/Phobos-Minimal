#pragma once

#include "SWTypeHandler.h"
#include "SWStateMachine.h"

class SW_PsychicDominator : public SWTypeHandler
{
public:

	virtual SuperWeaponType GetSWType() { return SuperWeaponType::PsychicDominator; };
	virtual SuperWeaponFlags Flags(const SWTypeExtData* pData) const;

	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer);
	virtual bool AbortFire(SuperClass* pSW, bool IsPlayer) override;

	virtual void Initialize(SWTypeExtData* pData) override;
	virtual void LoadFromINI(SWTypeExtData* pData,  CCINIClass* pINI) override;
	virtual bool IsLaunchSite(const SWTypeExtData* pData, BuildingClass* pBuilding) const override;

	virtual WarheadTypeClass* GetWarhead(const SWTypeExtData* pData) const override;
	virtual int GetDamage(const SWTypeExtData* pData) const override;
	virtual SWRange GetRange(const SWTypeExtData* pData) const override;

	static SuperClass* CurrentPsyDom;

protected:
	void newStateMachine(CellStruct XY, SuperClass* pSuper) {
		SWStateMachine::Array.push_back(std::move(std::make_unique<PsychicDominatorStateMachine>(XY, pSuper, this)));
	}
};
