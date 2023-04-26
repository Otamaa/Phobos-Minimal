#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

#include <HoverLocomotionClass.h>

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/Terrain/Body.h>

#include <TerrainTypeClass.h>
#include <New/Type/ArmorTypeClass.h>

#include <Notifications.h>

DEFINE_OVERRIDE_HOOK(0x424538, AnimClass_AI_DamageDelay, 0x6)
{
	enum { SkipDamageDelay = 0x42465D, CheckIsAlive = 0x42464C };

	GET(AnimClass*, pThis, ESI);

	if (pThis->InLimbo)
		return CheckIsAlive;

	return AnimExt::DealDamageDelay(pThis);
}

DEFINE_OVERRIDE_HOOK(0x4232CE, AnimClass_Draw_SetPalette, 6)
{
	GET(AnimClass*, pThis, ESI);
	//GET(AnimTypeClass*, AnimType, EAX);

	const auto pData = AnimTypeExt::ExtMap.TryFind(pThis->Type);

	if (pData ) {
		if(const auto pConvertData = pData->Palette) {
			R->ECX<ConvertClass*>(pConvertData->GetConvert<PaletteManager::Mode::Temperate>());
			return 0x4232D4;
		}
	}

	return 0;
}


// MakeInfantry that fails to place will just end the source animation and cleanup instead of memleaking to game end
DEFINE_OVERRIDE_HOOK(0x424B23, AnimClass_Update_FailedToUnlimboInfantry, 0x6)
{
	GET(AnimClass*, pThis, ESI);
	GET(InfantryClass*, pInf, EDI);

	pInf->UnInit();
	pThis->TimeToDie = 1;
	pThis->UnInit();

	return 0x424B29;
}

DEFINE_OVERRIDE_HOOK(0x4239F0, AnimClass_UpdateBounce_Damage, 0x8)
{
	enum
	{
		DoNotDealDamage = 0x423A92,
		DealDamage = 0x4239F8,
		GoToNext = 0x423A83,
	};

	GET(ObjectClass*, pObj, EDI);
	GET(AnimClass*, pThis, EBP);

	const auto pType = pThis->Type;
	const auto nRadius = pType->DamageRadius;

	if (!pObj || nRadius < 0 || CLOSE_ENOUGH(pType->Damage, 0.0) || !pType->Warhead)
		return DoNotDealDamage;

	const auto nCoord = pThis->Bounce.GetCoords();
	const auto pAnimTypeExt = AnimTypeExt::ExtMap.Find(pType);
	TechnoClass* const pInvoker = AnimExt::GetTechnoInvoker(pThis, pAnimTypeExt->Damage_DealtByInvoker);
	const auto nLoc = pObj->Location;
	const auto nDist = abs(nLoc.Y - nCoord.Y) + abs(nLoc.X - nCoord.X);

	if (nDist < nRadius) {
		auto nDamage = (int)pType->Damage;
		pObj->ReceiveDamage(&nDamage, TacticalClass::AdjustForZ(nDist), pType->Warhead,
					  pInvoker, false, false, pInvoker ? pInvoker->Owner : pThis->Owner);
	}

	//return !pObj || !pType->Warhead ||
	//	pType->DamageRadius < 0 || pType->Damage == 0.0 ?
	//	DoNotDealDamage : DealDamage;
	return GoToNext;
}

DEFINE_OVERRIDE_HOOK(0x4242CA, AnimClass_Update_FixIE_TrailerSeperation, 0x6)
{
	enum
	{
		PlayTrail = 0x4242D5,
		SkopTrail = 0x424322,
	};

	GET(AnimTypeClass*, AT, EAX);
	int trailSep = AT->TrailerSeperation;

	R->ECX(trailSep);

	return trailSep >= 1
		? PlayTrail : SkopTrail;
}

DEFINE_OVERRIDE_HOOK_AGAIN(0x42511B, AnimClass_Expired_ScorchFlamer, 0x7)
DEFINE_OVERRIDE_HOOK_AGAIN(0x4250C9, AnimClass_Expired_ScorchFlamer, 0x7)
DEFINE_OVERRIDE_HOOK(0x42513F, AnimClass_Expired_ScorchFlamer, 0x7)
{
	GET(AnimClass*, pThis, ESI);
	auto pType = pThis->Type;

	CoordStruct crd = pThis->GetCoords();

	auto SpawnAnim = [&crd](AnimTypeClass* pType, int dist)
	{
		if (!pType)
		{
			return static_cast<AnimClass*>(nullptr);
		}

		CoordStruct crdAnim = crd;
		if (dist > 0)
		{
			const auto crdNear = MapClass::GetRandomCoordsNear(crd, dist, false);
			crdAnim = MapClass::PickInfantrySublocation(crdNear, true);
		}

		const auto count = ScenarioClass::Instance->Random.RandomRanged(1, 2);
		return GameCreate<AnimClass>(pType, crdAnim, 0, count, 0x600u, 0, false);
	};

	if (pType->Flamer)
	{
		// always create at least one small fire
		if (auto const pAnim1 = SpawnAnim(RulesClass::Instance->SmallFire, 64))
			AnimExt::SetAnimOwnerHouseKind(pAnim1, pAnim1->Owner, nullptr);

		// 50% to create another small fire
		if (ScenarioClass::Instance->Random.RandomFromMax(99) < 50)
		{
			if (auto const pAnim2 = SpawnAnim(RulesClass::Instance->SmallFire, 160))
				AnimExt::SetAnimOwnerHouseKind(pAnim2, pAnim2->Owner, nullptr);
		}

		// 50% chance to create a large fire
		if (ScenarioClass::Instance->Random.RandomFromMax(99) < 50)
		{
			if (auto const pAnim3 = SpawnAnim(RulesClass::Instance->LargeFire, 112))
				AnimExt::SetAnimOwnerHouseKind(pAnim3, pAnim3->Owner, nullptr);
		}

	}
	else if (pType->Scorch)
	{
		// creates a SmallFire anim that is attached to the same object
		// this anim is attached to.
		if (pThis->GetHeight() < 10)
		{
			switch (pThis->GetCell()->LandType)
			{
			case LandType::Water:
			case LandType::Beach:
			case LandType::Ice:
			case LandType::Rock:
				break;
			default:
				if (auto pAnim = SpawnAnim(RulesClass::Instance->SmallFire, 0))
				{
					if (pThis->OwnerObject)
					{
						pAnim->SetOwnerObject(pThis->OwnerObject);
					}
				}
			}
		}
	}

	return 0;
}