#include "Body.h"

#include <BulletClass.h>
#include <HouseClass.h>
#include <InfantryClass.h>
#include <WarheadTypeClass.h>
#include <ScenarioClass.h>

#include <Ext/BuildingType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/Rules/Body.h>
#include <Ext/Techno/Body.h>

#include <Ext/WarheadType/Body.h>

#include <Utilities/Macro.h>
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

DEFINE_HOOK(0x469150, BulletClass_Logics_ApplyRadiation, 0x5)
{
	enum { Handled = 0x46920B, Continue = 0x0 };

	if (!Phobos::Otamaa::DisableCustomRadSite)
	{
		GET(BulletClass* const, pThis, ESI);
		GET_BASE(CoordStruct const*, pCoords, 0x8);
		GET(WeaponTypeClass*, pWeapon, ECX);
		GET(int, nAmount, EDI);

		BulletExt::ExtMap.Find(pThis)->ApplyRadiationToCell(*pCoords, static_cast<int>(pWeapon->Warhead->CellSpread), nAmount);

		return Handled;
	}
	return Continue;
}

//unused function , safeguard
DEFINE_HOOK(0x46ADE0, BulletClass_ApplyRadiation_NoBullet, 0x5)
{
	enum { Handled = 0x46AE5E, Continue = 0x0 };

	if (!Phobos::Otamaa::DisableCustomRadSite)
	{
		GET(BulletClass* const, pThis, ECX);
		GET_STACK(CellStruct, location, 0x4);
		GET_STACK(int, spread, 0x8);
		GET_STACK(int, amount, 0xC);

		const auto pCell = MapClass::Instance->GetCellAt(location);

		if (!pThis) {

			const auto pDefault = RadTypeClass::FindOrAllocate(RADIATION_SECTION);
			auto const it = std::find_if(RadSiteClass::Array->begin(), RadSiteClass::Array->end(),
		
			[=](auto const pSite) {
				auto const pRadExt = RadSiteExt::ExtMap.Find(pSite);
				if (pRadExt->Type != pDefault)
					return false;

				if (pSite->BaseCell != location)
					return false;

				if (spread != pSite->Spread)
					return false;

				if (pThis->WeaponType != pRadExt->Weapon)
					return false;

				return true;
			});

			if (it != RadSiteClass::Array->end())
			{
				auto nAmount = amount;
				const auto nMax = pDefault->GetLevelMax();
				const auto nCurrent = (*it)->GetCurrentRadLevel();

				if (nCurrent + amount > nMax) {
					nAmount = nMax - nCurrent;
				}

				RadSiteExt::ExtMap.Find((*it))->Add(nAmount);
				return Handled;
			}

			RadSiteExt::CreateInstance(pCell->GetCoordsWithBridge(), spread, amount, nullptr , nullptr);
		
		} else {
			BulletExt::ExtMap.Find(pThis)->ApplyRadiationToCell(pCell->GetCoordsWithBridge(), spread, amount);
		}

		return Handled;
	}

	return Continue;
}

// Fix for desolator
DEFINE_HOOK(0x5213B4, InfantryClass_AIDeployment_CheckRad, 0x7)
{
	enum { FireCheck = 0x5213F4, SetMissionRate = 0x521484, Continue = 0x0 };

	if (!Phobos::Otamaa::DisableCustomRadSite)
	{
		GET(InfantryClass*, pThis, ESI);
		int radLevel = 0;
		int weaponRadLevel = 0;

		if (const auto pWeaponStruct = pThis->GetDeployWeapon())
		{
			if (const auto pWeapon = pWeaponStruct->WeaponType)
			{
				const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
				const auto currentCoord = pThis->InlineMapCoords();

				auto const it = std::find_if(RadSiteClass::Array->begin(), RadSiteClass::Array->end(),
					[=](auto const pPair)
					{
						auto const pRadExt = RadSiteExt::ExtMap.Find(pPair);

						if (pRadExt->Type != pWeaponExt->RadType)
							return false;

						if (pPair->BaseCell != currentCoord)
							return false;

						if (static_cast<int>(pWeapon->Warhead->CellSpread) != pPair->Spread)
							return false;

						if (pWeapon != pRadExt->Weapon)
							return false;

						if (pRadExt->TechOwner)
							return pRadExt->TechOwner == pThis;

						return true;

					});

				if (it != RadSiteClass::Array->end()) {
					radLevel = static_cast<int>(RadSiteExt::ExtMap.Find((*it))->GetRadLevelAt(currentCoord));
				}

				weaponRadLevel = pWeapon->RadLevel;
			}
		}

		return (!radLevel || (radLevel < (weaponRadLevel / 3))) ? FireCheck : SetMissionRate;
	}

	return Continue;
}

// Fix for desolator unable to fire his deploy weapon when cloaked
DEFINE_HOOK(0x521478, InfantryClass_AIDeployment_FireNotOKCloakFix, 0x6) // 4
{
	GET(InfantryClass* const, pThis, ESI);

	AbstractClass* pTarget = nullptr; //default WWP nullptr

	auto const pWeapon = pThis->GetDeployWeapon()->WeaponType;

	if (pWeapon
		&& pWeapon->DecloakToFire
		&& (pThis->CloakState == CloakState::Cloaked || pThis->CloakState == CloakState::Cloaking))
	{
		// FYI this are hack to immediately stop the Cloaking
		// since this function is always failing to decloak and set target when cell is occupied
		// something is wrong somewhere  # Otamaa
		pThis->CloakDelayTimer.Start(
			static_cast<int>(pThis->Type->Sequence->GetSequence(DoType::DeployedFire).CountFrames * 900.0));

		pTarget = pThis->GetCell();
	}

	pThis->SetTarget(pTarget); //Here we go

	return 0x521484;
}

#include <Ext/Building/Body.h>
// Too OP, be aware
DEFINE_HOOK(0x43FB23, BuildingClass_AI, 0x5)
{
	if (!Phobos::Otamaa::DisableCustomRadSite)
	{
		GET(BuildingClass* const, pBuilding, ECX);

		if (pBuilding->InLimbo  || 
			TechnoExt::IsRadImmune(pBuilding))
			return 0;

		const auto pExt = BuildingExt::ExtMap.Find(pBuilding);

		if (pExt->LimboID != -1)
			return 0;

		if (pBuilding->IsIronCurtained() ||
			pBuilding->BeingWarpedOut ||
			pBuilding->TemporalTargetingMe ||
			pBuilding->Type->Immune
			)
		{
			return 0;
		}

		if (pBuilding->IsAlive && pBuilding->Health > 0)
		{
			auto buildingCoords = pBuilding->InlineMapCoords();
			for (auto pFoundation = pBuilding->GetFoundationData(false);
				*pFoundation != CellStruct { 0x7FFF, 0x7FFF }; ++pFoundation) {

				const auto nCellStruct = buildingCoords + *pFoundation;

				// Loop for each different radiation stored in the RadSites container
				for (auto pRadSite : *RadSiteClass::Array())
				{
					const auto pRadExt = RadSiteExt::ExtMap.Find(pRadSite);
					// Check the distance, if not in range, just skip this one
					const double orDistance = pRadSite->BaseCell.DistanceFrom(nCellStruct);
					if (static_cast<int>(orDistance) > pRadSite->Spread)
						continue;

					const RadTypeClass* pType = pRadExt->Type;
					const int delay = pType->GetBuildingApplicationDelay();
					if ((delay <= 0) 
						|| (Unsorted::CurrentFrame % (delay + RadSiteClass::Array->Count)))
						continue;

					const auto nRadLevel = pRadExt->GetRadLevelAt(nCellStruct);
					if (nRadLevel == 0.0 || !pType->GetWarhead())
						continue;

					const auto damage = static_cast<int>((nRadLevel)*pType->GetLevelFactor());

					if (damage == 0)
						continue;

					if (pBuilding->IsAlive && pBuilding->Health > 0) // simple fix for previous issues
					{
						if (!pRadExt->ApplyRadiationDamage(pBuilding, damage, static_cast<int>(orDistance)) || !(pBuilding->Health > 0))
							return 0;
					}
				}
			}
		}
	}

	return 0;
}

// skip Frame % RadApplicationDelay
//DEFINE_JUMP(LJMP,0x4DA554, 0x4DA56E);
//DEFINE_HOOK(0x4DA554, FootClass_AI_SkipForCustomRad, 0x5)
//{
//	return !Phobos::Otamaa::DisableCustomRadSite ? 0x4DA56E : 0x0;
//}

// Hook Adjusted to support Ares RadImmune Ability check
DEFINE_HOOK(0x4DA554, FootClass_AI_Radiation, 0x5)
{
	enum { 
		CheckOtherState = 0x4DA63B, 
		SkipEverything = 0x4DAF00,
		Continue = 0x0 ,
		ProcessRadSiteCheckVanilla = 0x4DA59F,
	};

	GET(FootClass* const, pThis, ESI);

	if (pThis->InLimbo)
		return CheckOtherState;

	if (pThis->IsInAir())
		return CheckOtherState;

	auto const pUnit = specific_cast<UnitClass*>(pThis);

	if (pThis->GetTechnoType()->Immune ||
		pThis->IsIronCurtained() ||
		!pThis->IsInPlayfield ||
		pThis->TemporalTargetingMe || (pUnit && pUnit->DeathFrameCounter > 0))
	{
		return CheckOtherState;
	}

	if (pThis->IsBeingWarpedOut() || TechnoExt::IsChronoDelayDamageImmune(pThis))
		return CheckOtherState;

	if(TechnoExt::IsRadImmune(pThis))
		return CheckOtherState ;

	if (!Phobos::Otamaa::DisableCustomRadSite)
	{
		auto const nLoc = CellClass::Coord2Cell(pThis->Location);
		// Loop for each different radiation stored in the RadSites container
		for (auto pRadSite : *RadSiteClass::Array())
		{
			const auto pRadExt = RadSiteExt::ExtMap.Find(pRadSite);
			// Check the distance, if not in range, just skip this one
			const double orDistance = pRadSite->BaseCell.DistanceFrom(nLoc);
			if (static_cast<int>(orDistance) > pRadSite->Spread)
				continue;

			const RadTypeClass* pType = pRadExt->Type;
			const int RadApplicationDelay = pType->GetApplicationDelay();
			if ((RadApplicationDelay <= 0) 
				|| (Unsorted::CurrentFrame % (RadApplicationDelay + RadSiteClass::Array->Count)))
				continue;

			// for more precise dmg calculation
			const double nRadLevel = pRadExt->GetRadLevelAt(nLoc);
			if (nRadLevel <= 0.0 || !pType->GetWarhead())
				continue;

			const int damage = static_cast<int>(nRadLevel * pType->GetLevelFactor());

			if (damage == 0)
				continue;

			if (pThis->IsAlive && !pThis->IsSinking && !pThis->IsCrashing) {

				if (!pRadExt->ApplyRadiationDamage(pThis,damage, static_cast<int>(orDistance)))
					return SkipEverything;
			}
		}

		return pThis->IsAlive ? CheckOtherState : SkipEverything;
	}

	return ProcessRadSiteCheckVanilla;
}

#define GET_RADSITE(reg, value)\
	GET(RadSiteClass* const, pThis, reg);\
	auto output = RadSiteExt::ExtMap.Find(pThis)->Type->## value ##;

DEFINE_HOOK(0x65B843, RadSiteClass_AI_LevelDelay, 0x6)
{
	enum { SetTimer = 0x65B849, Continue = 0x0 };

	if (!Phobos::Otamaa::DisableCustomRadSite)
	{
		GET_RADSITE(ESI, GetLevelDelay());
		R->ECX(output);
		return SetTimer;
	}

	return Continue;
}

DEFINE_HOOK(0x65B8B9, RadSiteClass_AI_LightDelay, 0x6)
{
	enum { SetTimer = 0x65B8BF, Continue = 0x0 };

	if (!Phobos::Otamaa::DisableCustomRadSite)
	{
		GET_RADSITE(ESI, GetLightDelay());
		R->ECX(output);
		return SetTimer;
	}

	return Continue;
}

DEFINE_HOOK(0x65BB67, RadSite_Deactivate, 0x6)
{
	enum { DevideValue = 0x65BB6D, Continue = 0x0 };

	if (!Phobos::Otamaa::DisableCustomRadSite)
	{
		GET_RADSITE(ECX, GetLevelDelay());
		GET(int, val, EAX);

		if (output <= 0)
			output = 1;

		R->EAX(val / output);
		R->EDX(val % output);

		return DevideValue;
	}

	return Continue;
}

DEFINE_JUMP(VTABLE, 0x7F0858, GET_OFFSET(RadSiteExt::GetAltCoords_Wrapper));
DEFINE_JUMP(VTABLE, 0x7F0868, GET_OFFSET(RadSiteExt::GetAltCoords_Wrapper));