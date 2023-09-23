#pragma once

#include "NewSWType.h"

class SW_Firewall : public NewSWType
{
public:
	~SW_Firewall();

	virtual std::vector<const char*> GetTypeString() const override;

	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer) override;
	virtual void Initialize(SWTypeExt::ExtData* pData) override;
	virtual void Deactivate(SuperClass* pThis, CellStruct cell, bool isPlayer) override;
	virtual void LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI) override;

	static SuperWeaponType FirewallType;
};