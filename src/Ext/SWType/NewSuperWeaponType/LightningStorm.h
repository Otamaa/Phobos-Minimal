#pragma once

#include "SWTypeHandler.h"
#include "SWStateMachine.h"

class SW_LightningStorm : public SWTypeHandler
{
public:

	virtual SuperWeaponFlags Flags(const SWTypeExtData* pData) const override;

	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer);
	virtual bool AbortFire(SuperClass* pSW, bool IsPlayer) override;

	virtual void LoadFromINI(SWTypeExtData* pData, CCINIClass* pINI) override;
	virtual void Initialize(SWTypeExtData* pData) override;

	virtual bool IsLaunchSite(const SWTypeExtData* pData, BuildingClass* pBuilding) const override;

	virtual WarheadTypeClass* GetWarhead(const SWTypeExtData* pData) const override;
	virtual int GetDamage(const SWTypeExtData* pData) const override;
	virtual SWRange GetRange(const SWTypeExtData* pData) const override;

	static SuperClass* CurrentLightningStorm;

protected:
	void newStateMachine(int Duration, int Deferment, CellStruct XY, SuperClass* pSuper , TechnoClass* pFirer)
	{
		SWStateMachine::Array.push_back(std::move(std::make_unique<CloneableLighningStormStateMachine>(Duration, Deferment, XY, pSuper, pFirer, this)));
	}
};