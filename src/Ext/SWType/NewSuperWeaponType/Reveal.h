#pragma once

#include "NewSWType.h"
#include "SWStateMachine.h"

class SW_Reveal : public NewSWType
{
public:
	virtual std::vector<const char*> GetTypeString() const override;
	virtual bool HandleThisType(SuperWeaponType type) const override;

	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer) override;
	virtual void Deactivate(SuperClass* pThis, CellStruct cell, bool isPlayer) override;

	virtual void Initialize(SWTypeExtData* pData) override;
	virtual void LoadFromINI(SWTypeExtData* pData, CCINIClass* pINI) override;
	virtual bool IsLaunchSite(const SWTypeExtData* pData, BuildingClass* pBuilding) const override;

	virtual int GetSound(const SWTypeExtData* pData) const override;
	virtual SWRange GetRange(const SWTypeExtData* pData) const override;

	static void RevealMap(const CellStruct& Coords , float range , int height , HouseClass* Owner);

protected:
	void newStateMachine(int Deferment, CellStruct XY, SuperClass* pSuper) {
		SWStateMachine::Array.push_back(std::move(new(RevealStateMachine::RevealStateMachine_GLUE_NOT_IMPLEMENTED) RevealStateMachine(Deferment, XY, pSuper, this)));
	}
};