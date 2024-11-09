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

//DEFINE_HOOK(0x46920B, BulletClass_Logics, 0x6)
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

void ApplyLogics(BulletClass* pThis , CoordStruct* coords) {

	if (pThis->WeaponType)
	{
		auto const pWeaponExt = WeaponTypeExtContainer::Instance.Find(pThis->WeaponType);
		const size_t size = pWeaponExt->ExtraWarheads_DamageOverrides.size();
		const size_t chance_size = pWeaponExt->ExtraWarheads_DetonationChances.size();

		for (size_t i = 0; i < pWeaponExt->ExtraWarheads.size(); i++)
		{
			auto const pWH = pWeaponExt->ExtraWarheads[i];
			auto const pOwner = pThis->Owner ? pThis->Owner->Owner : BulletExtContainer::Instance.Find(pThis)->Owner;
			int damage = pThis->WeaponType->Damage;

			if (size > i)
				damage = pWeaponExt->ExtraWarheads_DamageOverrides[i];
			else if (size > 0)
				damage = pWeaponExt->ExtraWarheads_DamageOverrides[size - 1];

			bool detonate = true;


			if (chance_size > i)
				detonate = pWeaponExt->ExtraWarheads_DetonationChances[i] >= ScenarioClass::Instance->Random.RandomDouble();
			if (chance_size > 0)
				detonate = pWeaponExt->ExtraWarheads_DetonationChances[chance_size - 1] >= ScenarioClass::Instance->Random.RandomDouble();

			if (detonate) {
				AbstractClass* pTarget = pThis->Target ? pThis->Target : MapClass::Instance->GetCellAt(coords);
				WarheadTypeExtData::DetonateAt(pWH, pThis->Target, *coords, pThis->Owner, damage , pOwner);
			}
		}
	}

		// Return to sender
	if (pThis->Type && pThis->Owner)
	{
		auto const pTypeExt = BulletTypeExtContainer::Instance.Find(pThis->Type);

		if (pTypeExt->ReturnWeapon)
		{
			auto const pWeapon = pTypeExt->ReturnWeapon.Get();

			if (BulletClass* pBullet = pWeapon->Projectile->CreateBullet(pThis->Owner, pThis->Owner,
				pWeapon->Damage, pWeapon->Warhead, pWeapon->Speed, pWeapon->Bright))
			{
				pBullet->WeaponType = pWeapon;
				auto const pBulletExt = BulletExtContainer::Instance.Find(pBullet);
				pBulletExt->Owner = BulletExtContainer::Instance.Find(pThis)->Owner;

				pBullet->MoveTo(pThis->Location, {});
			}
		}
	}

	PhobosGlobal::Instance()->DetonateDamageArea = true;
}

// DEFINE_HOOK(0x46A2A1, BulletClass_Logics_ReturnB, 0x5){
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

DEFINE_HOOK(0x469AA4, BulletClass_Logics_Extras, 0x5)
{
	GET(BulletClass* , pThis ,ESI);
	GET_BASE(CoordStruct*, coords, 0x8);
	ApplyLogics(pThis , coords);

	return 0;
}

DEFINE_HOOK(0x489286, MapClass_DamageArea, 0x6)
{
	GET_BASE(WarheadTypeClass*, pWH, 0x0C);

	if (auto const pWHExt = WarheadTypeExtContainer::Instance.TryFind(pWH))
	{
		 GET(const int, Damage, EDX);
		// GET_BASE(const bool, AffectsTiberium, 0x10);
		GET(CoordStruct*, pCoords, ECX);
		GET_BASE(TechnoClass*, pOwner, 0x08);
		GET_BASE(HouseClass*, pHouse, 0x14);

		if (!pWHExt->ShakeIsLocal || TacticalClass::Instance->IsCoordsToClientVisible(*pCoords)) {

			if (pWH->ShakeXhi || pWH->ShakeXlo)
				GeneralUtils::CalculateShakeVal(GScreenClass::Instance->ScreenShakeX, Random2Class::NonCriticalRandomNumber->RandomRanged(pWH->ShakeXhi, pWH->ShakeXlo) , pWHExt->Shake_UseAlternativeCalculation);

			if (pWH->ShakeYhi || pWH->ShakeYlo)
				GeneralUtils::CalculateShakeVal(GScreenClass::Instance->ScreenShakeY, Random2Class::NonCriticalRandomNumber->RandomRanged(pWH->ShakeYhi, pWH->ShakeYlo) , pWHExt->Shake_UseAlternativeCalculation);
		}

		auto const pDecidedOwner = !pHouse && pOwner ? pOwner->Owner : pHouse;

		for (const auto& Lauch : pWHExt->Launchs) {
			if (Lauch.LaunchWhat) {
				Helpers::Otamaa::LauchSW(Lauch, pDecidedOwner, *pCoords, pOwner);
			}
		}

		if (PhobosGlobal::Instance()->DetonateDamageArea)
			pWHExt->Detonate(pOwner, pDecidedOwner, nullptr, *pCoords , Damage);
	}

	return 0;
}

#pragma endregion

#include <Ext/SWType/NewSuperWeaponType/LightningStorm.h>

DEFINE_HOOK(0x48A4F0, CombatAnimSelect, 0x5)
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

					int idx = pWHExt->SplashList_PickRandom ?
						ScenarioClass::Instance->Random.RandomFromMax(Vec.size() - 1) :
						MinImpl(Vec.size() * 35 - 1, (size_t)damage) / 35;

					R->EAX(Vec[idx]);
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
				const size_t idx = pWarhead->EMEffect || pWHExt->Crit_AnimList_PickRandom.Get(pWHExt->AnimList_PickRandom) ?
					ScenarioClass::Instance->Random.RandomFromMax(pWHExt->Crit_AnimList.size() - 1) :
					(MinImpl(pWHExt->Crit_AnimList.size() * 25 - 1, (size_t)damage) / 25);

				R->EAX(pWHExt->Crit_AnimList[idx < pWHExt->Crit_AnimList.size() ? idx : pWHExt->Crit_AnimList.size() - 1]);
				return 0x48A615;
			}

			if (pWarhead->AnimList.Count > 0) {
				const int idx = pWHExt->AnimList_PickRandom.Get(pWarhead->EMEffect) ?
					ScenarioClass::Instance->Random.RandomFromMax(pWarhead->AnimList.Count - 1) :
					MinImpl(pWarhead->AnimList.Count * 25 - 1, damage) / 25;

				R->EAX(pWarhead->AnimList.Items[idx < pWarhead->AnimList.Count ? idx : pWarhead->AnimList.Count - 1]);
				return 0x48A615;
			}
		}
	}

	R->EAX<AnimTypeClass*>(nullptr);
	return 0x48A615;
}

DEFINE_HOOK(0x4896EC, Explosion_Damage_DamageSelf, 0x6)
{
	enum { SkipCheck = 0x489702 };

	GET_BASE(WarheadTypeClass*, pWarhead, 0xC);
	auto const pWHExt = WarheadTypeExtContainer::Instance.Find(pWarhead);
	return (pWHExt->AllowDamageOnSelf.isset() && pWHExt->AllowDamageOnSelf.Get()) ? SkipCheck : 0;
}

// Cylinder CellSpread
DEFINE_HOOK(0x489430, MapClass_DamageArea_Cylinder_1, 0x7)
{
	//GET(int, nDetoCrdZ, EDX);
	GET_BASE(WarheadTypeClass* const, pWH, 0x0C);
	GET_STACK(int, nVictimCrdZ, STACK_OFFSET(0xE0, -0x5C));

	auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWH);

	if (pWHExt->CellSpread_Cylinder)
	{
		R->EDX(nVictimCrdZ);
	}

	return 0;
}

DEFINE_HOOK(0x4894C1, MapClass_DamageArea_Cylinder_2, 0x5)
{
	//GET(int, nDetoCrdZ, EDX);
	GET_BASE(WarheadTypeClass* const, pWH, 0x0C);
	GET(int, nVictimCrdZ, ESI);

	auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWH);

	if (pWHExt->CellSpread_Cylinder)
	{
		R->EDX(nVictimCrdZ);
	}

	return 0;
}

DEFINE_HOOK(0x48979C, MapClass_DamageArea_Cylinder_3, 0x8)
{
	//GET(int, nDetoCrdZ, ECX);
	GET_BASE(WarheadTypeClass* const, pWH, 0x0C);
	GET(int, nVictimCrdZ, EDX);

	auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWH);

	if (pWHExt->CellSpread_Cylinder)
	{
		R->ECX(nVictimCrdZ);
	}

	return 0;
}

DEFINE_HOOK(0x4897C3, MapClass_DamageArea_Cylinder_4, 0x5)
{
	//GET(int, nDetoCrdZ, ECX);
	GET_BASE(WarheadTypeClass* const, pWH, 0x0C);
	GET(int, nVictimCrdZ, EDX);

	auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWH);

	if (pWHExt->CellSpread_Cylinder)
	{
		R->ECX(nVictimCrdZ);
	}

	return 0;
}

DEFINE_HOOK(0x48985A, MapClass_DamageArea_Cylinder_5, 0x5)
{
	//GET(int, nDetoCrdZ, ECX);
	GET_BASE(WarheadTypeClass* const, pWH, 0x0C);
	GET(int, nVictimCrdZ, EDX);

	auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWH);

	if (pWHExt->CellSpread_Cylinder)
	{
		R->ECX(nVictimCrdZ);
	}

	return 0;
}

DEFINE_HOOK(0x4898BF, MapClass_DamageArea_Cylinder_6, 0x5)
{
	//GET(int, nDetoCrdZ, EDX);
	GET_BASE(WarheadTypeClass* const, pWH, 0x0C);
	GET(int, nVictimCrdZ, ECX);

	auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWH);

	if (pWHExt->CellSpread_Cylinder)
	{
		R->EDX(nVictimCrdZ);
	}

	return 0;
}

// AffectsInAir and AffectsOnFloor
DEFINE_HOOK(0x489416, MapClass_DamageArea_CheckHeight_1, 0x6)
{
	enum { SkipThisObject = 0x489547 };

	GET_BASE(WarheadTypeClass* const, pWH, 0x0C);
	GET(ObjectClass*, pObject, EBX);

	auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWH);

	if (!pObject ||
		((pWHExt->AffectsInAir && pObject->IsInAir()) ||
			(pWHExt->AffectsOnFloor && !pObject->IsInAir())))
	{
		return 0;
	}

	return SkipThisObject;
}

DEFINE_HOOK(0x489710, MapClass_DamageArea_CheckHeight_2, 0x7)
{
	enum { SkipThisObject = 0x4899B3 };

	GET_BASE(WarheadTypeClass* const, pWH, 0x0C);
	GET(ObjectClass*, pObject, ESI);

	auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWH);

	if (!pObject ||
		((pWHExt->AffectsInAir && pObject->IsInAir()) ||
			(pWHExt->AffectsOnFloor && !pObject->IsInAir())))
	{
		return 0;
	}

	return SkipThisObject;
}

// DEFINE_HOOK(0x4891AF, GetTotalDamage_NegativeDamageModifiers, 0x6)
// {
// 	enum { ApplyModifiers = 0x4891C6 };
//
// 	GET(WarheadTypeClass* const, pWarhead, EDI);
//
// 	auto const pWHExt = WarheadTypeExtContainer::Instance.Find(pWarhead);
//
// 	if (pWHExt->ApplyModifiersOnNegativeDamage)
// 		return ApplyModifiers;
//
// 	return 0;
// }

DEFINE_HOOK(0x489B49, MapClass_DamageArea_Rocker, 0xA)
{
	GET_BASE(WarheadTypeClass*, pWH, 0xC);
	GET_STACK(int, damage, 0xE0 - 0xBC);

	//dont do any calculation when it is not even a rocker
	R->EBX(pWH);
	if (!pWH->Rocker) {
		return 0x489E87;
	}

	const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWH);
	double rocker = pWHExt->Rocker_AmplitudeOverride.Get(damage);

	if (pWHExt->Rocker_AmplitudeMultiplier.isset())
		rocker *= pWHExt->Rocker_AmplitudeMultiplier;

	if (rocker >= 4.0)
		rocker = 4.0;

	R->Stack(0x88, rocker);
	return 0x489B92;
}

DEFINE_HOOK(0x4D73DE, FootClass_ReceiveDamage_RemoveParasites, 0x5)
{
	enum { Continue = 0x4D73E3, Skip = 0x4D7413 };
	GET(WarheadTypeClass*, pWarhead, EBP);
	GET(int*, damage, EDI);
	auto const pTypeExt = WarheadTypeExtContainer::Instance.Find(pWarhead);
	return pTypeExt->RemoveParasites.Get(*damage < 0) ? Continue : Skip;
}