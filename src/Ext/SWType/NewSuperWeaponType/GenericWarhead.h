#pragma once

#include "NewSWType.h"
#include "SWStateMachine.h"

class SW_GenericWarhead : public NewSWType
{
public:
	virtual std::vector<const char*> GetTypeString() const override;

	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer) override;
	virtual void Initialize(SWTypeExt::ExtData* pData) override;
	virtual bool IsLaunchSite(const SWTypeExt::ExtData* pData, BuildingClass* pBuilding) const override;

	virtual void LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI) override;

	virtual WarheadTypeClass* GetWarhead(const SWTypeExt::ExtData* pData) const override;
	virtual int GetDamage(const SWTypeExt::ExtData* pData) const override;

	using TStateMachine = GenericWarheadStateMachine;

	void newStateMachine(int Deferment, CellStruct XY, SuperClass* pSuper, TechnoClass* pfirer) {
		SWStateMachine::Register(std::make_unique<TStateMachine>(Deferment, XY, pSuper , pfirer, this));
	}

};
