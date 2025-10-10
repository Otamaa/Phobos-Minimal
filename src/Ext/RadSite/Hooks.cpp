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

static NOINLINE int CalculateRadiationDamage(
	int baseLevel,
	double levelFactor,
	double distanceInCells,
	int cellSpreadInCells,
	double minFalloff = 0.1
)
{
	double maxDistance = std::max(1.0, static_cast<double>(cellSpreadInCells));

	// Linear falloff, clamped between minFalloff and 1.0
	double falloff = std::clamp(1.0 - (distanceInCells / maxDistance), minFalloff, 1.0);

	// Base damage before falloff
	double rawDamage = static_cast<double>(baseLevel) * levelFactor;

	// Final damage (may be negative for healing)
	return static_cast<int>(rawDamage * falloff);
}

static NOINLINE void ApplyRadDamage(RadSiteClass* pRad, FootClass* pObj, CellClass* pCell)
{
	if (pObj->IsAlive && !pObj->InLimbo && pObj->Health > 0 && !pObj->TemporalTargetingMe && !TechnoExtData::IsRadImmune(pObj))
	{
		const auto pRadExt = RadSiteExtContainer::Instance.Find(pRad);
		RadTypeClass* pRadType = pRadExt->Type;

		const int RadApplicationDelay = RulesExtData::Instance()->UseGlobalRadApplicationDelay ? pRadType->GetApplicationDelay() : RulesClass::Instance->RadApplicationDelay;
		if ((RadApplicationDelay <= 0)
			|| (Unsorted::CurrentFrame % RadApplicationDelay))
			return;

		if (pObj->GetTechnoType()->Immune || !pRadType->GetWarhead())
			return;

		auto it = CellExtContainer::Instance.Find(pCell)->RadLevels.find_if([pRad](auto& pair) { return pair.Rad == pRad; });

		if (it == CellExtContainer::Instance.Find(pCell)->RadLevels.end() || it->Level <= 0)
			return;

		const double distance = pRad->BaseCell.DistanceFrom(pCell->MapCoords);
		const auto damage = CalculateRadiationDamage(it->Level, pRadType->GetLevelFactor(), distance, pRad->Spread);

		if (damage == 0)
			return;

		UnitClass* pUnit = cast_to<UnitClass*, false>(pObj);
		FootClass* pFoot = pObj;

		if ((pUnit && pUnit->DeathFrameCounter > 0) || !RadSiteClass::Array->Count)
			return;

		if (pObj->IsSinking || pObj->IsCrashing)
			return;

		if (pObj->IsInAir())
			return;

		if (pObj->IsBeingWarpedOut() || TechnoExtData::IsChronoDelayDamageImmune(pFoot))
			return;

		if (pRadExt->ApplyRadiationDamage(pObj, damage, static_cast<int>(distance)) == RadSiteExtData::DamagingState::Dead)
			return;

	}
}

struct BuildingRadiationExposure
{
	int Damage;
	int BestRadLevel;
	double BestDistance;
	CellStruct SourceCell;
};

static NOINLINE BuildingRadiationExposure CalculateBuildingRadiationDamage(
	RadSiteClass* pRad,
	RadTypeClass* pRadType,
	BuildingClass* pBld
)
{
	BuildingRadiationExposure result {};

	const auto baseCell = pRad->BaseCell;
	const auto nCurCoord = pBld->InlineMapCoords();

	for (auto* pFoundation = pBld->GetFoundationData(false); *pFoundation != CellStruct::EOL; ++pFoundation)
	{

		const auto nLoc = nCurCoord + (*pFoundation);
		auto pCell = MapClass::Instance->TryGetCellAt(nLoc);

		if (!pCell) continue;

		auto* pCellExt = CellExtContainer::Instance.Find(pCell);

		auto it = pCellExt->RadLevels.find_if([pRad](auto& pair) {
			return pair.Rad == pRad;
		});

		if (it == pCellExt->RadLevels.end() || it->Level <= 0)
			continue;

		const int radLevel = it->Level;
		double distance = static_cast<double>(baseCell.DistanceFrom(nLoc));

		// Use best exposed cell logic (max level or min distance)
		if (radLevel > result.BestRadLevel || result.BestRadLevel == 0)
		{
			result.BestRadLevel = radLevel;
			result.BestDistance = distance;
			result.SourceCell = nLoc;
		}
	}

	if (result.BestRadLevel > 0)
	{
		result.Damage = CalculateRadiationDamage(result.BestRadLevel, pRadType->GetLevelFactor(), result.BestDistance, pRad->Spread);
	}

	return result;
}

static NOINLINE void ApplyRadDamage(RadSiteClass* pRad, BuildingClass* pObj, CellClass* pCell)
{
	if (pObj->IsAlive && !pObj->InLimbo && pObj->Health > 0 && !pObj->TemporalTargetingMe && !TechnoExtData::IsRadImmune(pObj))
	{

		const auto pRadExt = RadSiteExtContainer::Instance.Find(pRad);

		RadTypeClass* pRadType = pRadExt->Type;

		const int delay = RulesExtData::Instance()->UseGlobalRadApplicationDelay ? pRadType->GetBuildingApplicationDelay() : RulesExtData::Instance()->RadApplicationDelay_Building;

		if ((delay <= 0) || (Unsorted::CurrentFrame % delay))
			return;

		if (pObj->GetTechnoType()->Immune || !pRadType->GetWarhead() || pObj->IsBeingWarpedOut())
			return;

		auto& damageCount = pRadExt->damageCounts[pObj];
		const int maxDamageCount = pRadType->GetBuildingDamageMaxCount();
		if (maxDamageCount > 0 && damageCount >= maxDamageCount)
			return;

		const auto damage = CalculateBuildingRadiationDamage(pRad, pRadType, pObj);

		if (damage.Damage == 0)
			return;

		if (pRadExt->ApplyRadiationDamage(pObj, damage.Damage, static_cast<int>(damage.BestDistance)) == RadSiteExtData::DamagingState::Dead)
			return;
	}
}

ASMJIT_PATCH(0x65B8C8, RadSiteClass_AI_cond, 0x5)
{
	GET(RadSiteClass*, pThis, ESI);

	if (pThis->RadTimeLeft <= 0 || pThis->RadLevel <= 0)
	{
		return 0x65B8D3;
	}

	for (CellRangeEnumerator it(pThis->BaseCell, pThis->Spread + 0.5); it; it++)
	{
		if (const auto pCell = MapClass::Instance->TryGetCellAt(*it))
		{
			if (auto pObj = pCell->Cell_Occupier())
			{
				if (auto pFoot = flag_cast_to<FootClass*, false>(pObj))
					ApplyRadDamage(pThis, pFoot, pCell);
				else if (auto pBld = cast_to<BuildingClass*, false>(pObj))
					ApplyRadDamage(pThis, pBld, pCell);
			}
		}
	}

	return 0x65B8DC;
}

template<bool reduce = false>
void PopulateCellRadVector(RadSiteClass* pRad, CellStruct* cell, int distance, int timeParam)
{
	const auto max = pRad->SpreadInLeptons;

	if (distance <= max)
	{
		if (auto pCell = MapClass::Instance->TryGetCellAt(cell))
		{
			const auto pCellExt = CellExtContainer::Instance.Find(pCell);
			auto it = pCellExt->RadLevels.find_if([pRad](auto& pair) { return pair.Rad == pRad; });

			if constexpr (!reduce)
			{
				const int amount = int(static_cast<double>(max - distance) / max * pRad->RadLevel);


				if (it != pCellExt->RadLevels.end())
					it->Level += MinImpl(it->Level + amount, RadSiteExtContainer::Instance.Find(pRad)->Type->GetLevelMax());
				else
					pCellExt->RadLevels.emplace_back(pRad, amount);
			}
			else
			{
				if (it != pCellExt->RadLevels.end())
				{
					it->Level -= int(static_cast<double>(max - distance) / max * pRad->RadLevel / pRad->LevelSteps * timeParam);
				}
			}
		}
	}
}

ASMJIT_PATCH(0x65BAC1, RadSiteClass_Radiate_Increase, 0x8)
{
	enum { SkipGameCode = 0x65BB11 };

	GET(RadSiteClass*, pThis, EDX);
	GET(int, distance, EAX);
	LEA_STACK(CellStruct*, cell, STACK_OFFSET(0x60, -0x4C));
	PopulateCellRadVector<false>(pThis, cell, distance, 0);
	return SkipGameCode;
}

ASMJIT_PATCH(0x65BC6E, RadSiteClass_Deactivate_Decrease, 0x6)
{
	enum { SkipGameCode = 0x65BCBD };

	GET(RadSiteClass*, pThis, EDX);
	GET(int, distance, EAX);
	LEA_STACK(CellStruct*, cell, STACK_OFFSET(0x70, -0x5C));
	GET_STACK(int, timeParam, STACK_OFFSET(0x70, -0x30));
	PopulateCellRadVector<true>(pThis, cell, distance, timeParam);
	return SkipGameCode;
}

ASMJIT_PATCH(0x65BE01, RadSiteClass_DecreaseRadiation_Decrease, 0x6)
{
	enum { SkipGameCode = 0x65BE4C };

	GET(RadSiteClass*, pThis, EDX);
	GET(int, distance, EAX);
	LEA_STACK(CellStruct*, cell, STACK_OFFSET(0x60, -0x50));
	PopulateCellRadVector<true>(pThis, cell, distance, 0);
	return SkipGameCode;
}
