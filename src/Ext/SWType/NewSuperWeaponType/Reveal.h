#pragma once

#include "SWTypeHandler.h"
#include "SWStateMachine.h"

class SW_Reveal : public SWTypeHandler
{
public:

	virtual SuperWeaponType GetSWType() { return SuperWeaponType::PsychicReveal; };

	virtual SuperWeaponFlags Flags(const SWTypeExtData* pData) const
	{
		return SuperWeaponFlags::None;
	}

	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer);
	virtual void Deactivate(SuperClass* pThis, CellStruct cell, bool isPlayer) override;

	virtual void Initialize(SWTypeExtData* pData) override;
	virtual void LoadFromINI(SWTypeExtData* pData, CCINIClass* pINI) override;

	virtual bool IsLaunchSite(const SWTypeExtData* pData, BuildingClass* pBuilding) const override;

	virtual int GetSound(const SWTypeExtData* pData) const override;
	virtual SWRange GetRange(const SWTypeExtData* pData) const override;

	static void RevealMap(const CellStruct& Coords , float range , int height , HouseClass* Owner);

protected:
	void newStateMachine(int Deferment, CellStruct XY, SuperClass* pSuper) {
		SWStateMachine::Array.push_back(std::move(std::make_unique <RevealStateMachine>(Deferment, XY, pSuper, this)));
	}
};