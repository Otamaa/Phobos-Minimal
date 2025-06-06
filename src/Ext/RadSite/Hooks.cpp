#include "Body.h"

#include <BulletClass.h>
#include <HouseClass.h>
#include <InfantryClass.h>
#include <WarheadTypeClass.h>
#include <ScenarioClass.h>

#include <Ext/BuildingType/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/Cell/Body.h>

#include <Ext/Rules/Body.h>
#include <Ext/Techno/Body.h>

#include <Ext/WarheadType/Body.h>

#include <Utilities/Macro.h>
#include <Misc/Hooks.Otamaa.h>

#include <SpawnManagerClass.h>

/*
	Custom Radiations
	Worked out from old uncommented Ares RadSite Hook , adding some more hook
	and rewriting some in order to make this working perfectly
	Credit : Ares Team , for unused/uncommented source of Hook.RadSite
						,RulesData_LoadBeforeTypeData Hook
			 Alex-B : GetRadSiteAt ,Helper that used at FootClass_AI & BuildingClass_AI
					  Radiate , Uncommented
			 me(Otamaa) adding some more stuffs and rewriting hook that cause crash

*/

ASMJIT_PATCH(0x469150, BulletClass_Logics_ApplyRadiation, 0x5)
{
	enum { Handled = 0x46920B, Continue = 0x0 };

	GET(BulletClass* const, pThis, ESI);
	GET_BASE(CoordStruct*, pCoords, 0x8);
	GET(WeaponTypeClass*, pWeapon, ECX);
	GET(int, nAmount, EDI);

	if (!MapClass::Instance->IsWithinUsableArea(*pCoords))
		return Handled;

	const auto pCell = MapClass::Instance->TryGetCellAt(*pCoords);

	if (!pCell) {
		return Handled;
	}


	BulletExtContainer::Instance.Find(pThis)->ApplyRadiationToCell(pCell, static_cast<int>(pWeapon->Warhead->CellSpread), nAmount);

	return Handled;
}

//unused function , safeguard
ASMJIT_PATCH(0x46ADE0, BulletClass_ApplyRadiation_NoBullet, 0x5)
{
	enum { Handled = 0x46AE5E, Continue = 0x0 };

	GET(BulletClass* const, pThis, ECX);
	GET_STACK(CellStruct, location, 0x4);
	GET_STACK(int, spread, 0x8);
	GET_STACK(int, amount, 0xC);

	const auto pCell = MapClass::Instance->TryGetCellAt(location);

	if (!pCell)
		return Handled;

	if (!pThis)
		Debug::FatalError(__FUNCTION__" require BulletClass !\n");

	BulletExtContainer::Instance.Find(pThis)->ApplyRadiationToCell(pCell, spread , amount);
	return Handled;

}

// Fix for desolator
ASMJIT_PATCH(0x5213B4, InfantryClass_AIDeployment_CheckRad, 0x7)
{
	enum { FireCheck = 0x5213F4, SetMissionRate = 0x521484, Continue = 0x0 };

		GET(InfantryClass*, pThis, ESI);
		int radLevel = 0;
		int weaponRadLevel = 0;

		if (const auto pWeaponStruct = pThis->GetDeployWeapon())
		{
			if (const auto pWeapon = pWeaponStruct->WeaponType)
			{
				const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);
				const auto currentCoord = pThis->InlineMapCoords();
				if (const auto pCell = MapClass::Instance->TryGetCellAt(currentCoord))
				{
					auto pCellExt = CellExtContainer::Instance.Find(pCell);

					auto const it = pCellExt->RadSites.find_if([=](auto const pPair) {

						auto const pRadExt = RadSiteExtContainer::Instance.Find(pPair);

						if (pRadExt->Type != pWeaponExt->RadType)
							return false;

						if (static_cast<int>(pWeapon->Warhead->CellSpread) != pPair->Spread)
							return false;

						if (pWeapon != pRadExt->Weapon)
							return false;

						if (pRadExt->TechOwner)
							return pRadExt->TechOwner == pThis;

						return true;

					});

					if (it != pCellExt->RadSites.end()) {
						radLevel = static_cast<int>((*it)->GetCurrentRadLevel());
					}
				}

				weaponRadLevel = pWeapon->RadLevel;
			}
		}

		return (!radLevel || (radLevel < (weaponRadLevel / 3))) ? FireCheck : SetMissionRate;
}

// Fix for desolator unable to fire his deploy weapon when cloaked
ASMJIT_PATCH(0x521478, InfantryClass_AIDeployment_FireNotOKCloakFix, 0x6) // 4
{
	GET(InfantryClass* const, pThis, ESI);

	AbstractClass* pTarget = nullptr; //default WWP nullptr

	const auto pWeaponstr = pThis->GetDeployWeapon();

	if (pWeaponstr)
	{
		const auto pWeapon = pWeaponstr->WeaponType;

		if (pWeapon
			&& pWeapon->DecloakToFire
			&& pThis->IsInCloakState())
		{
			// FYI this are hack to immediately stop the Cloaking
			// since this function is always failing to decloak and set target when cell is occupied
			// something is wrong somewhere  # Otamaa
			pThis->CloakDelayTimer.Start(
				static_cast<int>(pThis->Type->Sequence->GetSequence(DoType::DeployedFire).CountFrames * 900.0));

			pTarget = pThis->GetCell();
		}
	}

	pThis->SetTarget(pTarget); //Here we go

	return 0x521484;
}

// ASMJIT_PATCH(0x43FB29, BuildingClass_AI_Radiation, 0x8)
// {
// 	enum { Dead = 0x440573, Continue = 0x0 };

// 	GET(BuildingClass* const, pBuilding, ECX);

// 	if (!pBuilding->IsAlive)
// 		return Dead;

// 	const auto pExt = BuildingExtContainer::Instance.Find(pBuilding);

// 	if (!Phobos::Otamaa::DisableCustomRadSite)
// 	{
// 		if (!RadSiteClass::Array->Count)
// 			return Continue;

// 		if (pBuilding->InLimbo ||
// 			TechnoExtData::IsRadImmune(pBuilding))
// 			return Continue;

// 		if (pExt->LimboID != -1)
// 			return Continue;

// 		if (pBuilding->BeingWarpedOut ||
// 			pBuilding->TemporalTargetingMe ||
// 			pBuilding->Type->Immune
// 			)
// 		{
// 			return Continue;
// 		}

// 		PhobosMap<RadSiteClass*, int> damageCounts;

// 		const auto nCurCoord = pBuilding->InlineMapCoords();
// 		for (auto pFoundation = pBuilding->GetFoundationData(false);
// 			*pFoundation != CellStruct::EOL; ++pFoundation)
// 		{
// 			if (!pBuilding->IsAlive)
// 				return Dead;

// 			const auto nLoc = nCurCoord + (*pFoundation);

// 			// Loop for each different radiation stored in the RadSites container
// 			for (auto pRadSite : *RadSiteClass::Array())
// 			{
// 				if (!pBuilding->IsAlive)
// 					return Dead;

// 				const auto pRadExt = RadSiteExtContainer::Instance.Find(pRadSite);

// 				int maxDamageCount = pRadExt->Type->GetBuildingDamageMaxCount();

// 				if (maxDamageCount > 0 && damageCounts[pRadSite] >= maxDamageCount)
// 					continue;

// 				// Check the distance, if not in range, just skip this one
// 				const double orDistance = pRadSite->BaseCell.DistanceFrom(nLoc);
// 				if (static_cast<int>(orDistance) > pRadSite->Spread)
// 					continue;

// 				const RadTypeClass* pType = pRadExt->Type;
// 				const int delay = RulesExtData::Instance()->UseGlobalRadApplicationDelay ? pType->GetBuildingApplicationDelay() : RulesExtData::Instance()->RadApplicationDelay_Building;
// 				if ((delay <= 0)
// 					|| (Unsorted::CurrentFrame % delay))
// 					continue;

// 				const auto nRadLevel = pRadExt->GetRadLevelAt(orDistance);
// 				if (nRadLevel == 0.0 || !pType->GetWarhead())
// 					continue;

// 				const auto damage = static_cast<int>((nRadLevel) * pType->GetLevelFactor());

// 				if (maxDamageCount > 0)
// 					damageCounts[pRadSite]++;

// 				if (damage == 0)
// 					continue;

// 				switch (pRadExt->ApplyRadiationDamage(pBuilding, damage, static_cast<int>(orDistance)))
// 				{
// 				case RadSiteExtData::DamagingState::Dead:
// 					return Dead;
// 				default:
// 					break;
// 				}
// 			}
// 		}
// 	}

// 	return Continue;
// }

// skip Frame % RadApplicationDelay
//DEFINE_JUMP(LJMP,0x4DA554, 0x4DA56E);
//ASMJIT_PATCH(0x4DA554, FootClass_AI_SkipForCustomRad, 0x5)
//{
//	return !Phobos::Otamaa::DisableCustomRadSite ? 0x4DA56E : 0x0;
//}

#define GET_RADSITE(reg, value)\
	GET(RadSiteClass* const, pThis, reg);\
	auto output = RadSiteExtContainer::Instance.Find(pThis)->Type->## value ##;

ASMJIT_PATCH(0x65B843, RadSiteClass_AI_LevelDelay, 0x6)
{
	enum { SetTimer = 0x65B849, Continue = 0x0 };

	GET_RADSITE(ESI, GetLevelDelay());
	R->ECX(output);
	return SetTimer;
}

ASMJIT_PATCH(0x65B8B9, RadSiteClass_AI_LightDelay, 0x6)
{
	enum { SetTimer = 0x65B8BF, Continue = 0x0 };

	GET_RADSITE(ESI, GetLightDelay());
	R->ECX(output);
	return SetTimer;
}

ASMJIT_PATCH(0x65BB67, RadSite_Deactivate, 0x6)
{
	enum { DevideValue = 0x65BB6D, Continue = 0x0 };

	GET_RADSITE(ECX, GetLevelDelay());
	GET(int, val, EAX);

	if (output <= 0)
		output = 1;

	R->EAX(val / output);
	R->EDX(val % output);

	return DevideValue;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F0858,  FakeRadSiteClass::__GetAltCoords);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F0868,  FakeRadSiteClass::__GetAltCoords);