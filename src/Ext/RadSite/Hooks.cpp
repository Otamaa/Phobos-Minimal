#include "Body.h"

#include <BulletClass.h>
#include <HouseClass.h>
#include <InfantryClass.h>
#include <WarheadTypeClass.h>
#include <ScenarioClass.h>

#include <Ext/BuildingType/Body.h>
#include <Ext/Building/Body.h>
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

		BulletExtContainer::Instance.Find(pThis)->ApplyRadiationToCell(*pCoords, static_cast<int>(pWeapon->Warhead->CellSpread), nAmount);

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

		if (!pCell)
			return Handled;

		if (!pThis) {
			const auto pDefault = RadTypeClass::Array[0].get();
			auto const it = RadSiteClass::Array->find_if([=](auto const pSite) {
				 auto const pRadExt = RadSiteExtContainer::Instance.Find(pSite);
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

				if (nCurrent + amount > nMax)
				{
					nAmount = nMax - nCurrent;
				}

				RadSiteExtContainer::Instance.Find((*it))->Add(nAmount);
				return Handled;
			}

			RadSiteExtData::CreateInstance(pCell->GetCoordsWithBridge(), spread, amount, nullptr, nullptr);

		}
		else
		{
			BulletExtContainer::Instance.Find(pThis)->ApplyRadiationToCell(pCell->GetCoordsWithBridge(), spread, amount);
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
				const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);
				const auto currentCoord = pThis->InlineMapCoords();

				auto const it = RadSiteClass::Array->find_if([=](auto const pPair)
				{
					auto const pRadExt = RadSiteExtContainer::Instance.Find(pPair);

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

				if (it != RadSiteClass::Array->end())
				{
					radLevel = static_cast<int>(RadSiteExtContainer::Instance.Find((*it))->GetRadLevelAt(currentCoord));
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

	const auto pWeaponstr = pThis->GetDeployWeapon();

	if (pWeaponstr)
	{
		const auto pWeapon = pWeaponstr->WeaponType;

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
	}

	pThis->SetTarget(pTarget); //Here we go

	return 0x521484;
}

DEFINE_HOOK(0x43FB29, BuildingClass_AI_Radiation, 0x8)
{
	enum { Dead = 0x440573, Continue = 0x0 };

	GET(BuildingClass* const, pBuilding, ECX);

	if (!pBuilding->IsAlive)
		return Continue;

	const auto pExt = BuildingExtContainer::Instance.Find(pBuilding);

	if (!Phobos::Otamaa::DisableCustomRadSite)
	{
		if (!RadSiteClass::Array->Count)
			return Continue;

		if (pBuilding->InLimbo ||
			TechnoExtData::IsRadImmune(pBuilding))
			return Continue;

		if (pExt->LimboID != -1)
			return Continue;

		if (pBuilding->IsIronCurtained() ||
			pBuilding->BeingWarpedOut ||
			pBuilding->TemporalTargetingMe ||
			pBuilding->Type->Immune
			)
		{
			return Continue;
		}

		const auto nCurCoord = pBuilding->InlineMapCoords();
		for (auto pFoundation = pBuilding->GetFoundationData(false);
			*pFoundation != CellStruct { 0x7FFF, 0x7FFF }; ++pFoundation)
		{
			const auto nLoc = nCurCoord + (*pFoundation);

			// Loop for each different radiation stored in the RadSites container
			for (auto pRadSite : *RadSiteClass::Array())
			{
				const auto pRadExt = RadSiteExtContainer::Instance.Find(pRadSite);

				// Check the distance, if not in range, just skip this one
				const double orDistance = pRadSite->BaseCell.DistanceFrom(nLoc);
				if (static_cast<int>(orDistance) > pRadSite->Spread)
					continue;

				const RadTypeClass* pType = pRadExt->Type;
				const int delay = pType->GetBuildingApplicationDelay();
				if ((delay <= 0)
					|| (Unsorted::CurrentFrame % delay))
					continue;

				const auto nRadLevel = pRadExt->GetRadLevelAt(orDistance);
				if (nRadLevel == 0.0 || !pType->GetWarhead())
					continue;

				const auto damage = static_cast<int>((nRadLevel)*pType->GetLevelFactor());

				if (damage == 0)
					continue;


				switch (pRadExt->ApplyRadiationDamage(pBuilding, damage, static_cast<int>(orDistance)))
				{
				case RadSiteExtData::DamagingState::Dead:
					return Dead;
				default:
					break;
				}
			}
		}
	}

	return Continue;
}

// skip Frame % RadApplicationDelay
//DEFINE_JUMP(LJMP,0x4DA554, 0x4DA56E);
//DEFINE_HOOK(0x4DA554, FootClass_AI_SkipForCustomRad, 0x5)
//{
//	return !Phobos::Otamaa::DisableCustomRadSite ? 0x4DA56E : 0x0;
//}

// Hook Adjusted to support Ares RadImmune Ability check
DEFINE_HOOK(0x4DA554, FootClass_AI_ReplaceRadiationDamageProcessing, 0x5)
{
	enum
	{
		CheckOtherState = 0x4DA63B,
		SkipEverything = 0x4DAF00,
		//Continue = 0x0,
		ProcessRadSiteCheckVanilla = 0x4DA59F,
	};

	GET(FootClass* const, pThis, ESI);

	auto pExt = TechnoExtContainer::Instance.Find(pThis);

	if (pThis->SpawnOwner && !pExt->IsMissisleSpawn)
	{
		auto pSpawnTechnoType = pThis->SpawnOwner->GetTechnoType();
		auto pSpawnTechnoTypeExt = TechnoTypeExtContainer::Instance.Find(pSpawnTechnoType);

		if (const auto pTargetTech = abstract_cast<TechnoClass*>(pThis->Target))
		{
			//Spawnee trying to chase Aircraft that go out of map until it reset
			//fix this , so reset immedietely if target is not on map
			if (!MapClass::Instance->IsValid(pTargetTech->Location)
				|| pTargetTech->TemporalTargetingMe
				|| (pSpawnTechnoTypeExt->MySpawnSupportDatas.Enable && pThis->SpawnOwner->GetCurrentMission() != Mission::Attack && pThis->GetCurrentMission() == Mission::Attack)
				)
			{
				if (pThis->SpawnOwner->Target == pThis->Target)
					pThis->SpawnOwner->SetTarget(nullptr);

				pThis->SpawnOwner->SpawnManager->ResetTarget();
			}

		}
		else if (pSpawnTechnoTypeExt->MySpawnSupportDatas.Enable && pThis->SpawnOwner->GetCurrentMission() != Mission::Attack && pThis->GetCurrentMission() == Mission::Attack)
		{
			if (pThis->SpawnOwner->Target == pThis->Target)
				pThis->SpawnOwner->SetTarget(nullptr);

			pThis->SpawnOwner->SpawnManager->ResetTarget();
		}
	}

	const auto nLoc = pThis->InlineMapCoords();
	auto const pUnit = specific_cast<UnitClass*>(pThis);

	//R->BL(false);

	if ((pUnit && pUnit->DeathFrameCounter > 0) || !RadSiteClass::Array->Count)
		return (CheckOtherState);

	if (pThis->TemporalTargetingMe ||pThis->InLimbo || !pThis->Health || pThis->IsSinking || pThis->IsCrashing)
		return (CheckOtherState);

	if (pThis->IsInAir())
		return (CheckOtherState);

	if (pThis->GetTechnoType()->Immune || pThis->IsIronCurtained())
		return (CheckOtherState);

	if (pThis->IsBeingWarpedOut() || TechnoExtData::IsChronoDelayDamageImmune(pThis))
		return (CheckOtherState);

	if (TechnoExtData::IsRadImmune(pThis))
		return (CheckOtherState);

	if (!Phobos::Otamaa::DisableCustomRadSite)
	{
		// Loop for each different radiation stored in the RadSites container
		for (auto pRadSite : *RadSiteClass::Array())
		{
			const auto pRadExt = RadSiteExtContainer::Instance.Find(pRadSite);
			// Check the distance, if not in range, just skip this one
			const double orDistance = pRadSite->BaseCell.DistanceFrom(nLoc);
			if (static_cast<int>(orDistance) > pRadSite->Spread)
				continue;

			const RadTypeClass* pType = pRadExt->Type;
			const int RadApplicationDelay = pType->GetApplicationDelay();
			if ((RadApplicationDelay <= 0)
				|| (Unsorted::CurrentFrame % RadApplicationDelay))
				continue;

			// for more precise dmg calculation
			const double nRadLevel = pRadExt->GetRadLevelAt(orDistance);
			if (nRadLevel <= 0.0 || !pType->GetWarhead())
				continue;

			const int damage = static_cast<int>(nRadLevel * pType->GetLevelFactor());

			if (damage == 0)
				continue;

			switch (pRadExt->ApplyRadiationDamage(pThis, damage, static_cast<int>(orDistance)))
			{
			case RadSiteExtData::DamagingState::Dead:
				return SkipEverything;
			case RadSiteExtData::DamagingState::Ignore:
				return (CheckOtherState);
			default:
				break;
			}
		}

		return (CheckOtherState);
	}

	return (ProcessRadSiteCheckVanilla);
}

#define GET_RADSITE(reg, value)\
	GET(RadSiteClass* const, pThis, reg);\
	auto output = RadSiteExtContainer::Instance.Find(pThis)->Type->## value ##;

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

DEFINE_JUMP(VTABLE, 0x7F0858, GET_OFFSET(RadSiteExtData::GetAltCoords_Wrapper));
DEFINE_JUMP(VTABLE, 0x7F0868, GET_OFFSET(RadSiteExtData::GetAltCoords_Wrapper));