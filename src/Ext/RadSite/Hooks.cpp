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

#include <Utilities/Macro.h>
/*
	Custom Radiations
	Worked out from old uncommented Ares RadSite Hook , adding some more hook
	and rewriting some in order to make this working perfecly
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

		if(auto pExt = BulletExt::GetExtData(pThis)) {
			auto cell = CellClass::Coord2Cell(*pCoords);
			auto spread = Game::F2I(pWeapon->Warhead->CellSpread);
			pExt->ApplyRadiationToCell(cell, spread, nAmount);
		}

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

		if (!pThis) {

			auto pDefault = RadTypeClass::Find(RADIATION_SECTION);

			if (RadSiteClass::Array_Constant->Count > 0) {
				auto const it = std::find_if(RadSiteClass::Array_Constant->begin(), RadSiteClass::Array_Constant->end(),
					[=](RadSiteClass* const pSite) {

						auto pSiteExt = RadSiteExt::GetExtData(pSite);

						{
							if (pSiteExt->Type != pDefault)
								return false;

							if (pSite->BaseCell != location)
								return false;

							if (spread != pSite->Spread)
								return false;

							if (pThis->WeaponType != pSiteExt->Weapon)
								return false;
						}

						return true;
					});

				if (it != RadSiteClass::Array_Constant->end()) {
					auto nAmount = amount;
					if ((*it)->GetRadLevel() + amount >= pDefault->GetLevelMax()) {
						nAmount = pDefault->GetLevelMax() - (*it)->GetRadLevel();
					}

					(*it)->Add(nAmount);
					return Handled;
				}
			}

			RadSiteExt::CreateInstance(location, spread, amount, nullptr, nullptr);
		}

		return Handled;
	}

	return Continue;
}

// Fix for desolator
DEFINE_HOOK(0x5213B4, InfantryClass_AIDeployment_CheckRad, 0x7)
{
	enum { FireCheck = 0x5213F4, SetMissionRate = 0x521484, Continue = 0x0 };

	if (!Phobos::Otamaa::DisableCustomRadSite) {
		GET(InfantryClass*, pThis, ESI);
		int radLevel = 0;
		int weaponRadLevel = 0;

		if (auto pWeaponStruct = pThis->GetDeployWeapon()) {
			if (auto pWeapon = pWeaponStruct->WeaponType) {
				auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
				auto currentCoord = pThis->GetMapCoords();

			if (RadSiteClass::Array_Constant->Count > 0) {
				auto const it = std::find_if(RadSiteClass::Array_Constant->begin(), RadSiteClass::Array_Constant->end(),
					[=](RadSiteClass* const pSite) {
						auto pSiteExt = RadSiteExt::GetExtData(pSite);

						if (pSiteExt->Type != pWeaponExt->RadType)
							return false;

						if (pSite->BaseCell != currentCoord)
							return false;

						if (Game::F2I(pWeapon->Warhead->CellSpread) != pSite->Spread)
							return false;

						if (pWeapon != pSiteExt->Weapon)
							return false;

						if (pSiteExt->TechOwner)
							return pSiteExt->TechOwner == pThis;

						return true ;

					});

					if (it != RadSiteClass::Array_Constant->end()) {
						auto pSiteExt = RadSiteExt::GetExtData((*it));
						radLevel = Game::F2I(pSiteExt->GetRadLevelAt(currentCoord));
					}
				}

				weaponRadLevel = pWeapon->RadLevel;
			}
		}

		return (!radLevel || (radLevel < (weaponRadLevel / 3))) ? FireCheck : SetMissionRate;
	}

	return Continue;
}

// Fix for desolator unable to fire his deploy weapon when cloaked
DEFINE_HOOK(0x521478, InfantryClass_AIDeployment_FireNotOKCloakFix, 0x4)
{
	GET(InfantryClass* const, pThis, ESI);

	AbstractClass* pTarget = nullptr; //default WWP nullptr

	auto pWeapon = pThis->GetDeployWeapon()->WeaponType;

	if (pWeapon
		&& pWeapon->DecloakToFire
		&& (pThis->CloakState == CloakState::Cloaked || pThis->CloakState == CloakState::Cloaking))
	{
		// FYI this are hack to immedietely stop the Cloaking
		// since this function is always failing to decloak and set target when cell is occupied
		// something is wrong somewhere  # Otamaa
		pThis->CloakDelayTimer.Start(static_cast<int>(pThis->Type->Sequence->GetSequence(DoType::DeployedFire).CountFrames * 900.0));
		pTarget = pThis->GetCell();
	}

	pThis->SetTarget(pTarget); //Here we go

	return 0x521484;
}

// Too OP, be aware
DEFINE_HOOK(0x43FB23, BuildingClass_AI, 0x5)
{
	if (!Phobos::Otamaa::DisableCustomRadSite) {
		GET(BuildingClass* const, pBuilding, ECX);

		if (pBuilding->IsIronCurtained() ||
			pBuilding->Type->ImmuneToRadiation ||
			pBuilding->InLimbo ||
			pBuilding->BeingWarpedOut ||
			pBuilding->TemporalTargetingMe ||
			pBuilding->Type->Immune
			)
		{
			return 0;
		}

		auto buildingCoords = pBuilding->GetMapCoords();
		for (auto pFoundation = pBuilding->GetFoundationData(false);
			*pFoundation != CellStruct{ 0x7FFF, 0x7FFF }; ++pFoundation)
		{
			auto nCellStruct = buildingCoords + *pFoundation;
			auto nCurrentCoord = Map[nCellStruct]->GetCoords();

			for (const auto pRad : *RadSiteClass::Array_Constant)
			{
				RadSiteExt::ExtData* pRadExt = RadSiteExt::GetExtData(pRad);
				RadSiteClass* pRadSite = pRad;
				RadTypeClass* pType = pRadExt->Type;

				// Check the distance, if not in range, just skip this one
				double orDistance = Map[pRadSite->BaseCell]->GetCoords().DistanceFrom(nCurrentCoord);
				if (static_cast<int>(orDistance) > pRadSite->SpreadInLeptons)
					continue;

				int delay = pType->GetBuildingApplicationDelay();
				if ((delay <= 0) || (Unsorted::CurrentFrame % delay != 0))
					continue;

				auto nRadLevel = pRadExt->GetRadLevelAt(nCellStruct);
				if (nRadLevel <= 0.0|| !pType->GetWarhead())
					continue;

				auto damage = Game::F2I((nRadLevel)*pType->GetLevelFactor());

				if (damage == 0)
					continue;

				auto pWarhead = pType->GetWarhead();
				auto absolute = pWarhead->WallAbsoluteDestroyer;
				bool ignore = pBuilding->Type->Wall && absolute;

				auto pTechnoOwner = !pRadExt->NoOwner ? pRadExt->TechOwner : nullptr;
				auto pHouseOwner = !pRadExt->NoOwner ? pRadExt->RadHouse : nullptr;

				if (pBuilding->IsAlive) // simple fix for previous issues
					if (pBuilding->ReceiveDamage(&damage, static_cast<int>(orDistance), pWarhead, pTechnoOwner, ignore, absolute, pHouseOwner) == DamageState::NowDead)
						break; //dont continue , meaningless
			}
		}
	}
	return 0;
}
// skip Frame % RadApplicationDelay
//DEFINE_LJMP(0x4DA554, 0x4DA56E);
DEFINE_HOOK(0x4DA554, FootClass_AI_SkipForCustomRad, 0x5)
{
	return !Phobos::Otamaa::DisableCustomRadSite ? 0x4DA56E : 0x0;
}

// Hook Adjusted to support Ares RadImmune Ability check
DEFINE_HOOK(0x4DA59F, FootClass_AI_Radiation, 0x6)
{
	enum { CheckOtherState = 0x4DA63B, SkipEverything = 0x4DAF00, Continue = 0x0 };

	if (!Phobos::Otamaa::DisableCustomRadSite)
	{
		GET(FootClass* const, pFoot, ESI);

		if (!pFoot->GetTechnoType()->Immune &&
			!pFoot->IsIronCurtained() &&
			pFoot->IsInPlayfield &&
			!pFoot->TemporalTargetingMe
			) {

			auto CurrentCoord = pFoot->GetCoords();

			// Loop for each different radiation stored in the RadSites container
			for (const auto pRad : *RadSiteClass::Array_Constant)
			{
				RadSiteExt::ExtData* pRadExt = RadSiteExt::GetExtData(pRad);

				if (auto pUnit = specific_cast<UnitClass*>(pFoot))
					if (pUnit->DeathFrameCounter > 0)
						break;

				RadSiteClass* pRadSite = pRad;

				// Check the distance, if not in range, just skip this one
				double orDistance = Map[pRadSite->BaseCell]->GetCoords().DistanceFrom(CurrentCoord);
				if ( static_cast<int>(orDistance) > pRadSite->SpreadInLeptons)
					continue;

				RadTypeClass* pType = pRadExt->Type;
				int RadApplicationDelay = pType->GetApplicationDelay();
				if ((RadApplicationDelay <= 0) || (Unsorted::CurrentFrame % RadApplicationDelay != 0))
					continue;

				// for more precise dmg calculation
				double nRadLevel = pRadExt->GetRadLevelAt(pFoot->GetMapCoords());
				if (nRadLevel <= 0.0 || !pType->GetWarhead())
					continue;

				int damage = Game::F2I(nRadLevel * pType->GetLevelFactor());

				if (damage == 0)
					continue;

				auto pWarhead = pType->GetWarhead();
				auto absolute = pWarhead->WallAbsoluteDestroyer;
				auto pTechnoOwner = !pRadExt->NoOwner ? pRadExt->TechOwner : nullptr;
				auto pHouseOwner = !pRadExt->NoOwner ? pRadExt->RadHouse : nullptr;

				if (pFoot->IsAlive &&  !pFoot->IsSinking && !pFoot->IsCrashing) {
					if (pFoot->ReceiveDamage(&damage,static_cast<int>(orDistance), pWarhead, pTechnoOwner, false, absolute, pHouseOwner) == DamageState::NowDead)
						break; //dont continue , meaningless
				}
			}

			return pFoot->IsAlive ? CheckOtherState : SkipEverything;
		}
	}
	return Continue;
}

#define GET_RADSITE(reg, value)\
	GET(RadSiteClass* const, pThis, reg);\
	auto pExt = RadSiteExt::GetExtData(pThis);\
	auto output = pExt->Type->## value ##;

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