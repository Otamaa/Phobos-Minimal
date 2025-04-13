#pragma once

#include "NewSWType.h"
#include "SWStateMachine.h"

class SW_GenericWarhead : public NewSWType
{
public:
	virtual std::vector<const char*> GetTypeString() const override;

	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer) override;
	virtual void Initialize(SWTypeExtData* pData) override;
	virtual bool IsLaunchSite(const SWTypeExtData* pData, BuildingClass* pBuilding) const override;

	virtual void LoadFromINI(SWTypeExtData* pData, CCINIClass* pINI) override;

	virtual WarheadTypeClass* GetWarhead(const SWTypeExtData* pData) const override;
	virtual int GetDamage(const SWTypeExtData* pData) const override;

protected:
	void newStateMachine(int Deferment, CellStruct XY, SuperClass* pSuper, TechnoClass* pfirer) {
		SWStateMachine::Array.push_back(std::move(new(GenericWarheadStateMachine::GenericWarheadStateMachine_GLUE_NOT_IMPLEMENTED) GenericWarheadStateMachine(Deferment, XY, pSuper , pfirer, this)));
	}
};
