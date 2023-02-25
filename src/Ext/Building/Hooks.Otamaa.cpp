#include "Body.h"

#include <UnitClass.h>
#include <Utilities/Macro.h>

#include <Ext/BuildingType/Body.h>

#pragma region Otamaa

DEFINE_HOOK(0x44D0C3, BuildingClass_Missile_EMPFire_WeaponType, 0x5)
{
	GET(BulletClass*, pBullet, EAX);
	GET(WeaponTypeClass*, pWeapon, EBP);

	if (pBullet && pWeapon && !pBullet->GetWeaponType())
		pBullet->SetWeaponType(pWeapon);

	return 0;
}

DEFINE_HOOK(0x4518CF, BuildingClass_AnimLogic_check, 0x9)
{
	GET(BuildingClass*, pThis, ESI);
	GET_STACK(const char*, pDecidedName, STACK_OFFS(0x34, -0x4));
	GET_STACK(BuildingAnimSlot, nSlot, STACK_OFFS(0x34, -0x8));
	R->EAX(BuildingTypeExt::GetBuildingAnimTypeIndex(pThis, nSlot, pDecidedName));
	return 0x4518D8;
}

/*
DEFINE_HOOK(0x442243, BuildingClass_ReceiveDamage_AddEarly, 0xA)
{
	R->Stack(STACK_OFFS(0x9C, 0x6C), DamageState::Unaffected);

	GET(BuildingClass*, pThis, ESI);
	GET(TechnoClass*, pSource, EBP);

	if (pThis == pSource && !pSource->GetTechnoType()->DamageSelf) {
		return 0x442C06;
	}

	return 0x442268;
}*/

DEFINE_HOOK(0x44E85F, BuildingClass_Power_UntieStregth, 0x7)
{
	GET(BuildingClass*, pThis, ESI);
	GET_STACK(int, nPowMult, STACK_OFFS(0xC, 0x4));

	R->EAX(Game::F2I(!BuildingTypeExt::ExtMap.Find(pThis->Type)->Power_DegradeWithHealth.Get()
		? (nPowMult) : (nPowMult * pThis->GetHealthPercentage())));

	return 0x44E86F;
}

/*
namespace Temp_BuildingClass_GetStaticImage_Sell
{
	Valueable<SHPStruct*> SellImage;
}

DEFINE_HOOK(0x43F000, BuildingClass_GetStaticImage_Sell, 0x6)
{
	GET(BuildingClass*, pThis, ESI);

	if (auto pShape = Temp_BuildingClass_GetStaticImage_Sell::SellImage.Get())
	{
		int const nFrame = pShape->Frames / 2;
		int const nFrameOut = nFrame > 1 ? nFrame : 1;
		int const nFrameBld = pThis->Type->BuildingAnimFrame[0].FrameCount + pThis->Type->BuildingAnimFrame[0].dwUnknown;
		R->EAX((nFrameBld - pThis->Animation.Value - 1) / nFrameBld * nFrameOut);
		return 0x43F029;
	}

	return 0x0;
}
*/

DEFINE_HOOK_AGAIN(0x6D5EB1, BuildingClass_PlaceCementGrid_Shape, 0x6)
DEFINE_HOOK(0x47EF52, BuildingClass_PlaceCementGrid_Shape, 0x6)
{
	if (auto const pBuilding = specific_cast<BuildingClass*>(DisplayClass::Instance->CurrentBuilding)) {
		R->EDX(BuildingTypeExt::ExtMap.Find(pBuilding->Type)->BuildingPlacementGrid_Shape.Get(FileSystem::PLACE_SHP()));
		return R->Origin() + 0x6;
	}

	return 0x0;
}

DEFINE_HOOK(0x441EFC, BuildingClass_Destroy_PreventRubble, 0xB)
{
	GET(BuildingClass*, pThis, ESI);
	//GET_STACK(TechnoClass*, pKiller, STACK_OFFS(0x64, -0x8));
	//GET_STACK(void*, pPointer, STACK_OFFS(0x64, 0x14));

	if (pThis->GetCurrentMission() == Mission::Selling)
	{
		pThis->Health = 0;
		if (R->AL()) {
			pThis->NoCrew = true;
		}

		pThis->DestroyExtrasFunctions(pThis->C4AppliedBy);
		return 0x441F20;
	}

	return 0x0;
}

DEFINE_JUMP(VTABLE, 0x7E4140, GET_OFFSET(BuildingTypeExt::IsFactory));

/*
#ifdef ENABLE_NEWHOOKS
DEFINE_HOOK(0x443FF9,BuildingClass_ExitObject_Aircraft,0xA)
{
	GET(BuildingClass*, pThis, ESI);
	GET(AircraftClass*, pProduct, EBP);
	//GET(CoordStruct*, pCoord,EAX);

	if (!pProduct->Type->AirportBound)
	{
		CellStruct nCell;
		pProduct->NearbyLocation(&nCell, pThis);
		if(auto pCell = Map[nCell])
		{
			CoordStruct nBuff = pCell->GetCoords();
			nBuff.Z += Map.GetCellFloorHeight(nBuff);
			R->EAX(&nBuff);
			return 0x444003;
		}
	}

	return 0x0;
}
#endif
*/

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

		if (upgrade->IsThreatRatingNode) {
			Debug::Log("Removing Upgrade [%d][%s] With IsTreatRatingNode = true ! \n", i, upgrade->get_ID());
		}
	}

	R->Stack(0x13, true);
	return 0x445AC6;
}

DEFINE_HOOK(0x4516B1, BuildingClass_RemoveUpgrades_Add , 0x7)
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

			if(!pNew->Type->MultiplayPassive)
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
#pragma endregion

