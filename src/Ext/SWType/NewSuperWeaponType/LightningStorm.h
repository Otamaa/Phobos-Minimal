#pragma once

#include "NewSWType.h"
#include "SWStateMachine.h"

class SW_LightningStorm : public NewSWType
{
public:
	virtual std::vector<const char*> GetTypeString() const override;
	virtual bool HandleThisType(SuperWeaponType type) const override;
	virtual SuperWeaponFlags Flags(const SWTypeExt::ExtData* pData) const override;

	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer) override;
	virtual bool AbortFire(SuperClass* pSW, bool IsPlayer) override;

	virtual void LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI) override;
	virtual void Initialize(SWTypeExt::ExtData* pData) override;
	virtual bool IsLaunchSite(const SWTypeExt::ExtData* pData, BuildingClass* pBuilding) const override;

	virtual WarheadTypeClass* GetWarhead(const SWTypeExt::ExtData* pData) const override;
	virtual int GetDamage(const SWTypeExt::ExtData* pData) const override;
	virtual SWRange GetRange(const SWTypeExt::ExtData* pData) const override;

	static SuperClass* CurrentLightningStorm;

	using TStateMachine = CloneableLighningStormStateMachine;

	void newStateMachine(int Duration, int Deferment, CellStruct XY, SuperClass* pSuper , TechnoClass* pFirer)
	{
		SWStateMachine::Register(std::make_unique<TStateMachine>(Duration, Deferment, XY, pSuper, pFirer, this));
	}
};