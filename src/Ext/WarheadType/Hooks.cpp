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

DEFINE_HOOK(0x46A290, BulletClass_Logics_Return, 0x5)
{
	GET(BulletClass*, pThis, ESI);
	GET_BASE(CoordStruct*, coords, 0x8);
	PhobosGlobal::Instance()->DetonateDamageArea = true;

	if (pThis->WeaponType )
	{
		auto const pWeaponExt = WeaponTypeExtContainer::Instance.Find(pThis->WeaponType);
		int defaultDamage = pThis->WeaponType->Damage;

		for (size_t i = 0; i < pWeaponExt->ExtraWarheads.size(); i++)
		{
			auto const pWH = pWeaponExt->ExtraWarheads[i];
			auto const pOwner = pThis->Owner ? pThis->Owner->Owner : BulletExtContainer::Instance.Find(pThis)->Owner;
			int damage = defaultDamage;

			if (pWeaponExt->ExtraWarheads_DamageOverrides.size() > i)
				damage = pWeaponExt->ExtraWarheads_DamageOverrides[i];

			AbstractClass* pTarget = pThis->Target ? pThis->Target : MapClass::Instance->GetCellAt(coords);
			WarheadTypeExtData::DetonateAt(pWH, pThis->Target, *coords, pThis->Owner, damage , pOwner);
		}
	}

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

		Point2D screenCoords {};
		bool const ShakeAllow = pWHExt->ShakeIsLocal ? TacticalClass::Instance->CoordsToClient(pCoords, &screenCoords) : true;

		if (ShakeAllow)
		{
			if (pWH->ShakeXhi || pWH->ShakeXlo)
				GeneralUtils::CalculateShakeVal(GScreenClass::Instance->ScreenShakeX, Random2Class::NonCriticalRandomNumber->RandomRanged(pWH->ShakeXhi, pWH->ShakeXlo));

			if (pWH->ShakeYhi || pWH->ShakeYlo)
				GeneralUtils::CalculateShakeVal(GScreenClass::Instance->ScreenShakeY, Random2Class::NonCriticalRandomNumber->RandomRanged(pWH->ShakeYhi, pWH->ShakeYlo));
		}

		auto const pDecidedOwner = !pHouse && pOwner ? pOwner->Owner : pHouse;

		if (!pWHExt->Launchs.empty())
		{
			for (const auto& Lauch : pWHExt->Launchs)
			{
				if (Lauch.LaunchWhat)
				{
					Helpers::Otamaa::LauchSW(Lauch, pDecidedOwner, *pCoords, pOwner);
				}
			}
		}

		if (PhobosGlobal::Instance()->DetonateDamageArea)
			pWHExt->Detonate(pOwner, pDecidedOwner, nullptr, *pCoords , Damage);
	}

	return 0;
}

#pragma endregion

DEFINE_HOOK(0x48A551, WarheadTypeClass_AnimList_SplashList, 0x6)
{
	GET(WarheadTypeClass* const, pThis, ESI);
	auto pWHExt = WarheadTypeExtContainer::Instance.Find(pThis);

	if (const auto Vec = pWHExt->SplashList.GetElements(RulesClass::Instance->SplashList))
	{
		GET(int, nDamage, ECX);
		int idx = pWHExt->SplashList_PickRandom ?
			ScenarioClass::Instance->Random.RandomFromMax(Vec.size() - 1) :
			MinImpl(Vec.size() * 35 - 1, (size_t)nDamage) / 35;

		R->EAX(Vec[idx]);
		return 0x48A5AD;
	}

	return 0;
}

DEFINE_HOOK(0x48A5BD, WarheadTypeClass_AnimList_PickRandom, 0x6)
{
	GET(WarheadTypeClass* const, pThis, ESI);
	const auto& random = WarheadTypeExtContainer::Instance.Find(pThis)->AnimList_PickRandom;
	return random.Get(pThis->EMEffect)
		? 0x48A5C7 : 0x48A5EB;
}

DEFINE_HOOK(0x48A5B3, WarheadTypeClass_AnimList_CritAnim, 0x6)
{
	GET(WarheadTypeClass* const, pThis, ESI);
	auto pWHExt = WarheadTypeExtContainer::Instance.Find(pThis);

	if (pWHExt->HasCrit && !pWHExt->Crit_AnimList.empty() && !pWHExt->Crit_AnimOnAffectedTargets)
	{
		GET(int, nDamage, ECX);
		int idx = pThis->EMEffect || pWHExt->Crit_AnimList_PickRandom.Get(pWHExt->AnimList_PickRandom) ?
			ScenarioClass::Instance->Random.RandomFromMax(pWHExt->Crit_AnimList.size() - 1) :
			MinImpl(pWHExt->Crit_AnimList.size() * 25 - 1, (size_t)nDamage) / 25;
		R->EAX(pWHExt->Crit_AnimList[idx]);
		return 0x48A5AD;
	}

	return 0;
}

DEFINE_HOOK(0x4896EC, Explosion_Damage_DamageSelf, 0x6)
{
	enum { SkipCheck = 0x489702 };

	GET_BASE(WarheadTypeClass*, pWarhead, 0xC);
	auto const pWHExt = WarheadTypeExtContainer::Instance.Find(pWarhead);
	return (pWHExt->AllowDamageOnSelf.isset() && pWHExt->AllowDamageOnSelf.Get()) ? SkipCheck : 0;
}

DEFINE_HOOK(0x48A4F3, SelectDamageAnimation_NegativeZeroDamage, 0x6)
{
	enum { SkipGameCode = 0x48A507, NoAnim = 0x48A618 };

	GET(int, damage, ECX);
	GET(WarheadTypeClass* const, warhead, EDX);

	if (!warhead)
		return NoAnim;

	if (damage == 0 && !WarheadTypeExtContainer::Instance.Find(warhead)->AnimList_ShowOnZeroDamage)
		return NoAnim;
	else if (damage < 0)
		damage = -damage;

	R->EDI(damage);
	R->ESI(warhead);
	return SkipGameCode;
}

DEFINE_HOOK(0x4891AF, GetTotalDamage_NegativeDamageModifiers, 0x6)
{
	enum { ApplyModifiers = 0x4891C6 };

	GET(WarheadTypeClass* const, pWarhead, EDI);

	auto const pWHExt = WarheadTypeExtContainer::Instance.Find(pWarhead);

	if (pWHExt->ApplyModifiersOnNegativeDamage)
		return ApplyModifiers;

	return 0;
}

//DEFINE_HOOK(0x489B49, MapClass_DamageArea_Rocker, 0xA)
//{
//	GET_BASE(WarheadTypeClass*, pWH, 0xC);
//	GET_STACK(int, damage, 0xE0 - 0xBC);
//
//	const int rocker = WarheadTypeExtContainer::Instance.Find(pWH)->Rocker_Damage.Get(damage);
//	_asm fild rocker;
//
//	return 0x489B4D;
//}