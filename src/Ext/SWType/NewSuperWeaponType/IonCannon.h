#pragma once

#include "SWTypeHandler.h"
#include "SWStateMachine.h"

class SW_IonCannon : public SWTypeHandler
{
public:
	virtual SuperWeaponFlags Flags(const SWTypeExtData* pData) const override;

	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer);

	virtual void Initialize(SWTypeExtData* pData) override;
	virtual void LoadFromINI(SWTypeExtData* pData, CCINIClass* pINI) override;

	virtual bool IsLaunchSite(const SWTypeExtData* pData, BuildingClass* pBuilding) const override;

	virtual WarheadTypeClass* GetWarhead(const SWTypeExtData* pData) const override;
	virtual int GetDamage(const SWTypeExtData* pData) const override;

protected:
	void newStateMachine(CellStruct XY, SuperClass* pSuper, TechnoClass* pFirer) {
		SWStateMachine::Array.push_back(std::move(std::make_unique<IonCannonStateMachine>(XY, pSuper, pFirer, this)));
	}
};
