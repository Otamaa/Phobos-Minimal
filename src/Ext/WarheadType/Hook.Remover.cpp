#ifdef ENABLE_NEWHOOKS
//TODO : rework , and desync test

#include "Body.h"

#include <Ext/TechnoType/Body.h>
#include <Ext/Anim/Body.h>

#include <SlaveManagerClass.h>

DEFINE_HOOK(0x416619, AircraftClass_ReceiveDamage_Remove, 0x6)
{
	GET_STACK(WarheadTypeClass*, pWarhead, 0x24);

	auto const pWHExt = WarheadTypeExt::ExtMap.Find(pWarhead);
	if (pWHExt->Remover.Get())
	{
		R->AL(false);
		return 0x41669A;
	}

	return 0x0;
}

DEFINE_HOOK(0x70222E, TechnoClass_ReceiveDamage_Remove, 0xA)
{
	GET(TechnoClass*, pThis, ESI);
	GET_STACK(WarheadTypeClass*, pWarhead, 0xD0);
	GET_STACK(TechnoClass*, pAttacker, 0xD4);

	auto const pWHExt = WarheadTypeExt::ExtMap.Find(pWarhead);

	if (pWHExt->Remover.Get())
	{
		if (pThis->AttachedBomb)
			pThis->AttachedBomb->Disarm();

		if (pThis->SpawnManager)
			pThis->SpawnManager->KillNodes();

		if (auto pAnimType = pWHExt->Remover_Anim.Get())
		{
			auto nCoord = pThis->GetCoords();
			if (auto pAnim = GameCreate<AnimClass>(pAnimType, nCoord))
			{
				HouseClass* pAttack = pAttacker ? pAttacker->GetOwningHouse() : nullptr;
				AnimExt::SetAnimOwnerHouseKind(pAnim, pAttack, pThis->GetOwningHouse(), false);
			}
		}

		pThis->RegisterDestruction(pAttacker);
		return 0x702684;
	}

	return 0x0;
}

DEFINE_HOOK(0x442651, BuildingClass_ReceiveDamage_Remove, 0xB)
{
	GET(BuildingClass*, pThis, ESI);
	GET_STACK(WarheadTypeClass*, pWarhead, 0xA8);
	GET_STACK(TechnoClass*, pAttacker, 0xAC);

	auto const pWHExt = WarheadTypeExt::ExtMap.Find(pWarhead);

	if (pWHExt->Remover.Get())
	{
		auto const pThisOwner = pThis->Owner;

		if (pThis->Occupants.Count)
			pThis->KillOccupants(pAttacker);

		if (pThis->BunkerLinkedItem)
			pThis->UnloadBunker();

		if (!Unsorted::IKnowWhatImDoing)
			pThis->KillCargo(pAttacker);

		if (pThis->SlaveManager)
			pThis->SlaveManager->Killed(pAttacker);

		pThis->Destroyed(pAttacker);
		pThis->UnInit();
		pThisOwner->RecheckTechTree = true;

		return 0x442905;
	}

	return 0x0;
}

DEFINE_HOOK(0x5181B5, InfantryClass_ReceiveDamage_Remove, 0xA)
{
	GET_STACK(WarheadTypeClass*, pWarhead, 0xDC);
	auto const pWHExt = WarheadTypeExt::ExtMap.Find(pWarhead);
	return pWHExt->Remover.Get() ? 0x518619 : 0x0;
}

DEFINE_HOOK(0x737D9C, UnitClass_ReceiveDamage_Remove, 0x6)
{
	GET(UnitClass*, pThis, ESI);
	GET_STACK(WarheadTypeClass*, pWarhead, 0x50);
	GET_STACK(TechnoClass*, pAttacker, 0x54);

	auto const pWHExt = WarheadTypeExt::ExtMap.Find(pWarhead);
	if (pWHExt->Remover.Get())
	{
		pThis->Destroyed(pAttacker);
		return 0x7384A1;
	}

	return 0x0;
}
#endif