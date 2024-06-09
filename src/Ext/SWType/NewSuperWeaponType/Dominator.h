#pragma once

#include "NewSWType.h"
#include "SWStateMachine.h"

class SW_PsychicDominator : public NewSWType
{
public:
	virtual std::vector<const char*> GetTypeString() const override;
	virtual bool HandleThisType(SuperWeaponType type) const override;
	virtual SuperWeaponFlags Flags(const SWTypeExtData* pData) const override;

	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer) override;
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
		SWStateMachine::Register(std::make_unique<PsychicDominatorStateMachine>(XY, pSuper, this));
	}
};
