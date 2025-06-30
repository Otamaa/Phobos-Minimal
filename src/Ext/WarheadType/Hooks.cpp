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

			if (pExt->AE.HasExtraWarheads) {
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

ASMJIT_PATCH(0x48A4F0, CombatAnimSelect, 0x5)
{
	GET(int, damage, ECX);
	GET(WarheadTypeClass*, pWarhead, EDX);
	GET_STACK(LandType, land, 0x4);
	GET_STACK(CoordStruct*, pCoord, 0x8);

	if (pWarhead) {

		const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWarhead);
		pWHExt->Splashed = false;

		//allowing zero damage to pass ,..
		//hopefully it wont do any harm to these thing , ...

		if ((damage == 0 && pWHExt->AnimList_ShowOnZeroDamage) || damage) {

			if (damage < 0)
				damage = -damage;

			if (land == LandType::Water
				&& pWarhead->Conventional
				&& !MapClass::Instance->GetCellAt(pCoord)->ContainsBridge()
				&& pCoord->Z < (MapClass::Instance->GetCellFloorHeight(pCoord) + Unsorted::CellHeight)
				) {
				pWHExt->Splashed = true;

				if (const auto Vec = pWHExt->SplashList.GetElements(RulesClass::Instance->SplashList)) {

					size_t idx = pWHExt->SplashList_PickRandom ?
						ScenarioClass::Instance->Random.RandomFromMax(Vec.size() - 1) :
						MinImpl(Vec.size() * 35 - 1, (size_t)damage) / 35;

					R->EAX(Vec[idx < Vec.size() ? idx : 0]);
					return 0x48A615;
				}

				R->EAX<AnimTypeClass*>(nullptr);
				return 0x48A615;
			}

			if (auto const pSuper = SW_LightningStorm::CurrentLightningStorm) {
				auto const pData = SWTypeExtContainer::Instance.Find(pSuper->Type);

				if (pData->GetNewSWType()->GetWarhead(pData) == pWarhead) {
					if (auto const pAnimType = pData->Weather_BoltExplosion.Get(
						RulesClass::Instance->WeatherConBoltExplosion))
					{
						R->EAX(pAnimType);
						return 0x48A615;
					}
				}
			}

			if (pWHExt->HasCrit && !pWHExt->Crit_AnimList.empty() && !pWHExt->Crit_AnimOnAffectedTargets) {
				const size_t idx = pWHExt->Crit_AnimList_PickRandom.Get(pWHExt->AnimList_PickRandom.Get(pWarhead->EMEffect)) ?
					ScenarioClass::Instance->Random.RandomFromMax(pWHExt->Crit_AnimList.size() - 1) :
					(MinImpl(pWHExt->Crit_AnimList.size() * 25 - 1, (size_t)damage) / 25);

				R->EAX(pWHExt->Crit_AnimList[idx < pWHExt->Crit_AnimList.size() ? idx : 0]);
				return 0x48A615;
			}

			if (!pWarhead->AnimList.Empty()) {
				const size_t idx = pWHExt->AnimList_PickRandom.Get(pWarhead->EMEffect) ?
					ScenarioClass::Instance->Random.RandomFromMax(pWarhead->AnimList.Count - 1) :
					MinImpl(pWarhead->AnimList.Count * 25 - 1, damage) / 25;

				R->EAX(pWarhead->AnimList.Items[idx < pWarhead->AnimList.size() ? idx : 0]);
				return 0x48A615;
			}
		}
	}

	R->EAX<AnimTypeClass*>(nullptr);
	return 0x48A615;
}

#ifdef TODO_for_DamageArea

// Cylinder CellSpread
ASMJIT_PATCH(0x489430, MapClass_DamageArea_Cylinder_1, 0x7)
{
	//GET(int, nDetoCrdZ, EDX);
	GET_BASE(FakeWarheadTypeClass* const, pWH, 0x0C);
	GET_STACK(int, nVictimCrdZ, STACK_OFFSET(0xE0, -0x5C));

	if (pWH->_GetExtData()->CellSpread_Cylinder)
	{
		R->EDX(nVictimCrdZ);
	}

	return 0;
}

ASMJIT_PATCH(0x4894C1, MapClass_DamageArea_Cylinder_2, 0x5)
{
	//GET(int, nDetoCrdZ, EDX);
	GET_BASE(FakeWarheadTypeClass* const, pWH, 0x0C);
	GET(int, nVictimCrdZ, ESI);

	auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWH);

	if (pWH->_GetExtData()->CellSpread_Cylinder)
	{
		R->EDX(nVictimCrdZ);
	}

	return 0;
}

ASMJIT_PATCH(0x48979C, MapClass_DamageArea_Cylinder_3, 0x8)
{
	//GET(int, nDetoCrdZ, ECX);
	GET_BASE(FakeWarheadTypeClass* const, pWH, 0x0C);
	GET(int, nVictimCrdZ, EDX);

	if (pWH->_GetExtData()->CellSpread_Cylinder)
	{
		R->ECX(nVictimCrdZ);
	}

	return 0;
}

ASMJIT_PATCH(0x4897C3, MapClass_DamageArea_Cylinder_4, 0x5)
{
	//GET(int, nDetoCrdZ, ECX);
	GET_BASE(FakeWarheadTypeClass* const, pWH, 0x0C);
	GET(int, nVictimCrdZ, EDX);

	if (pWH->_GetExtData()->CellSpread_Cylinder)
	{
		R->ECX(nVictimCrdZ);
	}

	return 0;
}

ASMJIT_PATCH(0x48985A, MapClass_DamageArea_Cylinder_5, 0x5)
{
	//GET(int, nDetoCrdZ, ECX);
	GET_BASE(FakeWarheadTypeClass* const, pWH, 0x0C);
	GET(int, nVictimCrdZ, EDX);

	if (pWH->_GetExtData()->CellSpread_Cylinder)
	{
		R->ECX(nVictimCrdZ);
	}

	return 0;
}

ASMJIT_PATCH(0x4898BF, MapClass_DamageArea_Cylinder_6, 0x5)
{
	//GET(int, nDetoCrdZ, EDX);
	GET_BASE(FakeWarheadTypeClass* const, pWH, 0x0C);
	GET(int, nVictimCrdZ, ECX);

	if (pWH->_GetExtData()->CellSpread_Cylinder) {
		R->EDX(nVictimCrdZ);
	}

	return 0;
}

// AffectsInAir and AffectsOnFloor
ASMJIT_PATCH(0x489416, MapClass_DamageArea_CheckHeight_AircraftTarcker, 0x6)
{
	enum { SkipThisObject = 0x489547 };

	GET_BASE(FakeWarheadTypeClass* const, pWH, 0x0C);
	GET(ObjectClass*, pObject, EBX);

	auto pWHExt = pWH->_GetExtData();

	if (!pObject ||
		((pWHExt->AffectsInAir && pObject->IsInAir()) ||
			(pWHExt->AffectsOnFloor && !pObject->IsInAir())))
	{
		return 0;
	}

	return SkipThisObject;
}

ASMJIT_PATCH(0x489710, MapClass_DamageArea_CheckHeight_2, 0x7)
{
	enum { SkipThisObject = 0x4899B3 };

	GET_BASE(FakeWarheadTypeClass* const, pWH, 0x0C);
	GET(ObjectClass*, pObject, ESI);

	auto pWHExt = pWH->_GetExtData();

	if (!pObject ||
		((pWHExt->AffectsInAir && pObject->IsInAir()) ||
			(pWHExt->AffectsOnFloor && !pObject->IsInAir())))
	{
		return 0;
	}

	return SkipThisObject;
}
#endif

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
