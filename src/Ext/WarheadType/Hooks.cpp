#include "Body.h"

#include <BulletClass.h>
#include <ScenarioClass.h>
#include <HouseClass.h>

#include <Ext/Anim/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/House/Body.h>
#include <Ext/Scenario/Body.h>

#include <Utilities/EnumFunctions.h>
#include <Utilities/Helpers.h>

#include <VeinholeMonsterClass.h>
#include <Misc/PhobosGlobal.h>

#include <TacticalClass.h>

#pragma region DETONATION

//ASMJIT_PATCH(0x46920B, BulletClass_Logics, 0x6)
//{
//	//GET(BulletClass* const, pThis, ESI);
//	//GET_BASE(const CoordStruct*, pCoords, 0x8);
//
//	//if (pThis && pThis->WH)
//	//{
//	//	auto const pExt = BulletExtContainer::Instance.Find(pThis);
//	//	auto const pTechno = pThis->Owner ? pThis->Owner : nullptr;
//	//	auto const pHouse = pTechno ? pTechno->Owner : pExt->Owner ? pExt->Owner : nullptr;
//	//
//	//	WarheadTypeExtContainer::Instance.Find(pThis->WH)->Detonate(pTechno, pHouse, pThis, *pCoords);
//	//}
//
//	PhobosGlobal::Instance()->DetonateDamageArea = false;
//
//	return 0;
//}

void ApplyExtraWarheads(
	BulletClass* pBullet,
	std::vector<WarheadTypeClass*>& exWH,
	std::vector<int>& exWHDamageOverrides,
	std::vector<double>& exWHChances,
	std::vector<bool>& exWHFull,
	CoordStruct* coords, HouseClass* pOwner) {

	const size_t damageoverride_size = exWHDamageOverrides.size();
	const size_t fulldetonation_size = exWHFull.size();
	const size_t chance_size = exWHChances.size();
	int damage = pBullet->WeaponType ? pBullet->WeaponType->Damage : 0;

	for (size_t i = 0; i < exWH.size(); i++)
	{
		auto const pWH = exWH[i];

		if (damageoverride_size > 0 && damageoverride_size > i)
			damage = exWHDamageOverrides[i];
		else if (damageoverride_size > 0)
			damage = exWHDamageOverrides[damageoverride_size - 1];

		bool detonate = true;

		if (chance_size > 0 && chance_size > i)
			detonate = exWHChances[i] >= ScenarioClass::Instance->Random.RandomDouble();
		else if (chance_size > 0)
			detonate = exWHChances[chance_size - 1] >= ScenarioClass::Instance->Random.RandomDouble();

		bool isFull = true;

		if (fulldetonation_size > 0 && fulldetonation_size > i)
			isFull = exWHFull[i];
		else if (fulldetonation_size > 0)
			isFull = exWHFull[fulldetonation_size - 1];

		if (detonate) {

			if(isFull)
				WarheadTypeExtData::DetonateAt(pWH, pBullet->Target ? pBullet->Target : MapClass::Instance->GetCellAt(coords), *coords, pBullet->Owner, damage , pOwner);
			else
				WarheadTypeExtContainer::Instance.Find(pWH)->DamageAreaWithTarget(*coords, damage, pBullet->Owner, pWH, true, pOwner,
				flag_cast_to<TechnoClass*>(pBullet->Target));
		}
	}
}

void ApplyLogics(WarheadTypeClass* pWH , WeaponTypeClass*pWeapon ,BulletClass * pThis , CoordStruct* coords) {
	auto const pBulletExt = BulletExtContainer::Instance.Find(pThis);
	auto const pOwner = pThis->Owner ? pThis->Owner->Owner : pBulletExt->Owner;
	const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWH);

	if(pThis->WeaponType){
		auto const pWeaponExt = WeaponTypeExtContainer::Instance.Find(pThis->WeaponType);
		ApplyExtraWarheads(pThis , pWeaponExt->ExtraWarheads, pWeaponExt->ExtraWarheads_DamageOverrides, pWeaponExt->ExtraWarheads_DetonationChances, pWeaponExt->ExtraWarheads_FullDetonation, coords, pOwner);
	}

		// Return to sender
	if (pThis->Type && pThis->Owner)
	{
		auto const pTypeExt = BulletTypeExtContainer::Instance.Find(pThis->Type);
		if (pThis->Owner) {
			auto const pExt = TechnoExtContainer::Instance.Find(pThis->Owner);

			if (pExt->AE.flags.HasExtraWarheads) {
				for (auto const& aE : pExt->AeData.Data) {
					if (aE.Type->ExtraWarheads.size() > 0)
						ApplyExtraWarheads(pThis , aE.Type->ExtraWarheads, aE.Type->ExtraWarheads_DamageOverrides, aE.Type->ExtraWarheads_DetonationChances, aE.Type->ExtraWarheads_FullDetonation, coords, pOwner);
				}

				for (auto const& pAE : pExt->PhobosAE) {

					if(!pAE || !pAE->IsActive())
						continue;

					auto const pType = pAE->GetType();

					if (pType->ExtraWarheads.size() > 0)
						ApplyExtraWarheads(pThis , pType->ExtraWarheads, pType->ExtraWarheads_DamageOverrides, pType->ExtraWarheads_DetonationChances, pType->ExtraWarheads_FullDetonation, coords, pOwner);
				}
			}
		}

		if (pTypeExt->ReturnWeapon)
		{
			auto const RpWeapon = pTypeExt->ReturnWeapon.Get();
			int damage = RpWeapon->Damage;

			if (pTypeExt->ReturnWeapon_ApplyFirepowerMult)
			 	damage = static_cast<int>(damage * pThis->Owner->FirepowerMultiplier * TechnoExtContainer::Instance.Find(pThis->Owner)->AE.FirepowerMultiplier);

			if (BulletClass* pBullet = RpWeapon->Projectile->CreateBullet(pThis->Owner, pThis->Owner,
				damage, RpWeapon->Warhead, RpWeapon->Speed, RpWeapon->Bright))
			{
				pBullet->WeaponType = RpWeapon;
				auto const pRBulletExt = BulletExtContainer::Instance.Find(pBullet);
				pRBulletExt->Owner = BulletExtContainer::Instance.Find(pThis)->Owner;

				BulletExtData::SimulatedFiringUnlimbo(pBullet, pThis->Owner->Owner, pWeapon, pThis->Location, false);
				BulletExtData::SimulatedFiringEffects(pBullet, pThis->Owner->Owner, nullptr, false, false);
			}
		}
	}

	PhobosGlobal::Instance()->DetonateDamageArea = true;

	if (pThis->Owner && pThis->Owner->IsAlive && pThis->Owner->InLimbo && !pWH->Parasite && pWHExt->UnlimboDetonate)
	{
		CoordStruct location = *coords;
		const auto pTarget = pThis->Target;
		const bool isInAir = pTarget && pTarget->AbstractFlags & AbstractFlags::Foot ? static_cast<FootClass*>(pTarget)->IsInAir() : false;
		bool success = false;

		if (!pWHExt->UnlimboDetonate_Force)
		{
			const auto pType = GET_TECHNOTYPE(pThis->Owner);
			const auto nCell = MapClass::Instance->NearByLocation(CellClass::Coord2Cell(location),
									pType->SpeedType, ZoneType::None, pType->MovementZone, false, 1, 1, true,
									false, false, true, CellStruct::Empty, false, false);

			if (const auto pCell = MapClass::Instance->TryGetCellAt(nCell))
			{
				pThis->Owner->OnBridge = pCell->ContainsBridge();
				location = pCell->GetCoordsWithBridge();
			}
			else
			{
				pThis->Owner->OnBridge = false;
			}

			if (isInAir)
				location.Z = coords->Z;

			success = pThis->Owner->Unlimbo(location, pThis->Owner->PrimaryFacing.Current().GetDir());
		}
		else
		{
			if (const auto pCell = MapClass::Instance->TryGetCellAt(location))
				pThis->Owner->OnBridge = pCell->ContainsBridge();
			else
				pThis->Owner->OnBridge = false;

			++Unsorted::ScenarioInit;
			success = pThis->Owner->Unlimbo(location, pThis->Owner->PrimaryFacing.Current().GetDir());
			--Unsorted::ScenarioInit;
		}

		const auto pTechnoExt = TechnoExtContainer::Instance.Find(pThis->Owner);

		if (success)
		{
			if (isInAir)
			{
				pThis->Owner->IsFallingDown = true;
				pThis->Owner->FallRate = 0;
			}

			if (pWHExt->UnlimboDetonate_KeepTarget
				&& pTarget && pTarget->AbstractFlags & AbstractFlags::Object)
			{
				pThis->Owner->SetTarget(pTarget);
			}
			else
			{
				pThis->Owner->SetTarget(nullptr);
			}

			if (pTechnoExt->IsSelected)
			{
				ScenarioExtData::Instance()->LimboLaunchers.erase(pThis->Owner);
				pThis->Owner->Select();
				pTechnoExt->IsSelected = false;
			}
		}
		else
		{
			if (pTechnoExt->IsSelected)
			{
				ScenarioExtData::Instance()->LimboLaunchers.erase(pThis->Owner);
				pTechnoExt->IsSelected = false;
			}

			pThis->Owner->SetLocation(location);
			pThis->Owner->ReceiveDamage(&pThis->Owner->Health, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, pThis->Owner->Owner);
		}
	}
}

// ASMJIT_PATCH(0x46A2A1, BulletClass_Logics_ReturnB, 0x5){
// 	GET(BulletClass* , pThis ,ESI);
// 	GET_BASE(CoordStruct*, coords, 0x8);
//
// 	if(auto pNullify = RulesClass::Instance->WeaponNullifyAnim){
// 		GameCreate<AnimClass>(pNullify , coords ,0,1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200,-15,0);
// 	}
//
// 	ApplyLogics(pThis, coords);
//
// 	return 0x46A2FB;
// }

ASMJIT_PATCH(0x469AA4, BulletClass_Logics_Extras, 0x5)
{
	GET(BulletClass* , pThis ,ESI);
	GET_BASE(CoordStruct*, coords, 0x8);
	ApplyLogics(pThis->WH , pThis->WeaponType, pThis,  coords);

	return 0;
}

#pragma endregion

#include <Ext/SWType/NewSuperWeaponType/LightningStorm.h>

DEFINE_FUNCTION_JUMP(LJMP , 0x48A4F0 , WarheadTypeExtData::SelectCombatAnim)

/*
void TechnoExt::RemoveParasite(TechnoClass* pThis, HouseClass* sourceHouse, WarheadTypeClass* wh)
{
	if (!pThis || !wh)
		return;

	const auto pFoot = abstract_cast<FootClass*>(pThis);
	if (!pFoot)
		return;

	bool isParasiteEatingMe = pFoot->ParasiteEatingMe && pThis->WhatAmI() != AbstractType::Infantry	&& pThis->WhatAmI() != AbstractType::Building;

	// Ignore other cases that aren't useful for this logic
	if (!isParasiteEatingMe)
		return;

	const auto pWHExt = WarheadTypeExt::ExtMap.Find(wh);
	auto parasite = pFoot->ParasiteEatingMe;

	if (!pWHExt || !pWHExt->CanRemoveParasites.Get() || !pWHExt->CanTargetHouse(parasite->Owner, pThis))
		return;

	if (pWHExt->CanRemoveParasites_ReportSound.isset() && pWHExt->CanRemoveParasites_ReportSound.Get() >= 0)
		VocClass::SafeImmedietelyPlayAtpWHExt->CanRemoveParasites_ReportSound.Get(), parasite->GetCoords());

	// Kill the parasite
	CoordStruct coord = TechnoExt::PassengerKickOutLocation(pThis, parasite, 10);

	auto deleteParasite = [parasite]()
	{
		auto parasiteOwner = parasite->Owner;
		parasite->IsAlive = false;
		parasite->IsOnMap = false;
		parasite->Health = 0;

		parasiteOwner->RegisterLoss(parasite, false);
		parasiteOwner->RemoveTracking(parasite);
		parasite->UnInit();
	};

	if (!pWHExt->CanRemoveParasites_KickOut.Get() || coord == CoordStruct::Empty)
	{
		deleteParasite;
		return;
	}

	// Kick the parasite outside
	pFoot->ParasiteEatingMe = nullptr;

	if (!parasite->Unlimbo(coord, parasite->PrimaryFacing.Current().GetDir()))
	{
		// Failed to kick out the parasite, remove it instead
		deleteParasite;
		return;
	}

	parasite->Target = nullptr;
	int paralysisCountdown = pWHExt->CanRemoveParasites_KickOut_Paralysis.Get() < 0 ? 15 : pWHExt->CanRemoveParasites_KickOut_Paralysis.Get();

	if (paralysisCountdown > 0)
	{
		parasite->ParalysisTimer.Start(paralysisCountdown);
		parasite->RearmTimer.Start(paralysisCountdown);
	}

	if (pWHExt->CanRemoveParasites_KickOut_Anim.isset())
	{
		if (auto const pAnim = GameCreate<AnimClass>(pWHExt->CanRemoveParasites_KickOut_Anim.Get(), parasite->GetCoords()))
		{
			pAnim->Owner = sourceHouse ? sourceHouse : parasite->Owner;
			pAnim->SetOwnerObject(parasite);
		}
	}

	return;
}

		Valueable<bool> CanRemoveParasites { false };
		Valueable<bool> CanRemoveParasites_KickOut { false };
		Valueable<int> CanRemoveParasites_KickOut_Paralysis { 15 };
		NullableIdx<VocClass> CanRemoveParasites_ReportSound { };
		Nullable<AnimTypeClass*> CanRemoveParasites_KickOut_Anim { };


	this->CanRemoveParasites.Read(exINI, pSection, "CanRemoveParasites");
	this->CanRemoveParasites_KickOut.Read(exINI, pSection, "CanRemoveParasites.KickOut");
	this->CanRemoveParasites_KickOut_Paralysis.Read(exINI, pSection, "CanRemoveParasites.KickOut.Paralysis");
	this->CanRemoveParasites_ReportSound.Read(exINI, pSection, "CanRemoveParasites.ReportSound");
	this->CanRemoveParasites_KickOut_Anim.Read(exINI, pSection, "CanRemoveParasites.KickOut.Anim");

	.Process(this->CanRemoveParasites)
	.Process(this->CanRemoveParasites_KickOut)
	.Process(this->CanRemoveParasites_KickOut_Paralysis)
	.Process(this->CanRemoveParasites_ReportSound)
	.Process(this->CanRemoveParasites_KickOut_Anim)

*/
