#include "Body.h"

DEFINE_HOOK(0x4409F4, BuildingClass_Put_Upgrade_Add, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	//GET(BuildingClass*, pToUpgrade, EDI);

	if (auto const pOwner = pThis->Owner)
	{
		if (pOwner->Type->MultiplayPassive)
			return 0x0;

		if (auto const pInfantrySelfHeal = pThis->Type->InfantryGainSelfHeal)
			pOwner->InfantrySelfHeal += pInfantrySelfHeal;

		if (auto const pUnitSelfHeal = pThis->Type->UnitsGainSelfHeal)
			pOwner->UnitsSelfHeal += pUnitSelfHeal;
	}

	return 0;
}

DEFINE_HOOK(0x445A9F, BuildingClass_Remove_Upgrades, 0x8)
{
	GET(BuildingClass*, pThis, ESI);

	for (int i = 0; i < (int)pThis->Upgrades.size(); ++i)
	{
		auto const upgrade = pThis->Upgrades[i];

		if (!upgrade)
			continue;

		if (auto const pTargetHouse = pThis->Owner)
		{
			if (auto const pInfantrySelfHeal = upgrade->InfantryGainSelfHeal)
			{
				pTargetHouse->InfantrySelfHeal -= pInfantrySelfHeal;
				if (pTargetHouse->InfantrySelfHeal < 0)
					pTargetHouse->InfantrySelfHeal = 0;
			}

			if (auto const pUnitSelfHeal = upgrade->UnitsGainSelfHeal)
			{
				pTargetHouse->UnitsSelfHeal -= pUnitSelfHeal;
				if (pTargetHouse->UnitsSelfHeal < 0)
					pTargetHouse->UnitsSelfHeal = 0;
			}
		}

		if (upgrade->IsThreatRatingNode)
		{
			Debug::Log("Removing Upgrade [%d][%s] With IsTreatRatingNode = true ! \n", i, upgrade->get_ID());
		}
	}

	R->Stack(0x13, true);
	return 0x445AC6;
}

DEFINE_HOOK(0x4516B1, BuildingClass_RemoveUpgrades_Add, 0x7)
{
	GET(BuildingTypeClass*, pUpgrades, EAX);
	GET(BuildingClass*, pThis, ESI);

	if (pThis->Owner)
	{
		if (auto const pInfantrySelfHeal = pUpgrades->InfantryGainSelfHeal)
		{
			pThis->Owner->InfantrySelfHeal -= pInfantrySelfHeal;
			if (pThis->Owner->InfantrySelfHeal < 0)
				pThis->Owner->InfantrySelfHeal = 0;
		}

		if (auto const pUnitSelfHeal = pUpgrades->UnitsGainSelfHeal)
		{
			pThis->Owner->UnitsSelfHeal -= pUnitSelfHeal;
			if (pThis->Owner->UnitsSelfHeal < 0)
				pThis->Owner->UnitsSelfHeal = 0;
		}
	}

	return 0;
}

DEFINE_HOOK(0x4492D7, BuildingClass_SetOwningHouse_Upgrades, 0x5)
{
	GET(BuildingClass*, pThis, ESI);
	GET(HouseClass*, pOld, EBX);
	GET(HouseClass*, pNew, EBP);

	// Somewhat upgrades were removed for AI after ownership chages
	for (auto const& upgrade : pThis->Upgrades)
	{
		if (!upgrade)
			continue;

		if (auto const pInfantrySelfHeal = upgrade->InfantryGainSelfHeal)
		{
			pOld->InfantrySelfHeal -= pInfantrySelfHeal;
			if (pOld->InfantrySelfHeal < 0)
				pOld->InfantrySelfHeal = 0;

			if (!pNew->Type->MultiplayPassive)
				pNew->InfantrySelfHeal += pInfantrySelfHeal;
		}

		if (auto const pUnitSelfHeal = upgrade->UnitsGainSelfHeal)
		{
			pOld->UnitsSelfHeal -= pUnitSelfHeal;
			if (pOld->UnitsSelfHeal < 0)
				pOld->UnitsSelfHeal = 0;

			if (!pNew->Type->MultiplayPassive)
				pNew->InfantrySelfHeal += pUnitSelfHeal;
		}
	}

	return 0;
}