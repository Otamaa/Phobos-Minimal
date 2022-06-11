#include "Body.h"

#include <HouseClass.h>
#include <AnimClass.h>
#include <AnimTypeClass.h>
#include <ScenarioClass.h>
#include <WarheadTypeClass.h>
#include <Utilities/GeneralUtils.h>
#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/VoxelAnimType/Body.h>
#include <Ext/WeaponType/Body.h>

#ifdef COMPILE_PORTED_DP_FEATURES
#include <Misc/DynamicPatcher/Trails/TrailsManager.h>
#endif

DEFINE_HOOK(0x74A70E, VoxelAnimClass_AI_Additional, 0xC)
{
	GET(VoxelAnimClass* const, pThis, EBX);

	auto pThisExt = VoxelAnimExt::ExtMap.Find(pThis);

	if (pThisExt && !pThisExt->LaserTrails.empty())
	{
		CoordStruct location = pThis->GetCoords();
		CoordStruct drawnCoords = location;

		for (auto const& trail : pThisExt->LaserTrails)
		{
			if (!trail->LastLocation.isset())
				trail->LastLocation = location;

			trail->Visible = pThis->IsVisible;
			trail->Update(drawnCoords);

		}
	}

#ifdef COMPILE_PORTED_DP_FEATURES
	TrailsManager::AI(pThis);
#endif
	return 0;
}

#pragma region Otamaa
DEFINE_HOOK(0x74A035, VoxelAnimClass_DamageArea_Water, 0x6)
{
	GET(VoxelAnimClass* const, pThis, EBX);

	auto pType = pThis->Type;
	auto TypeExt = VoxelAnimTypeExt::ExtMap.Find(pType);
	TechnoClass* pInvoker = VoxelAnimExt::GetTechnoOwner(pThis, TypeExt && TypeExt->Damage_DealtByOwner.Get());
	auto nLocation = pThis->GetCoords();
	auto const pOwner = pInvoker ? pInvoker->GetOwningHouse() :pThis->OwnerHouse;
	AnimTypeClass* pAnimType = nullptr;
	DWORD flags = 0x600;
	int ForceZAdjust = 0;

	const auto GetOriginalAnimResult = [pType]()
	{
		if (pType->IsMeteor)
		{
			auto const splash = RulesClass::Instance->SplashList;

			if (splash.Count > 0)
			{
				return splash[ScenarioClass::Instance->Random(0, splash.Count - 1)];
			}
		}

		return RulesClass::Instance->Wake;
	};

	if (TypeExt)
	{
		if (TypeExt->ExplodeOnWater.Get())
		{
			auto nDamage = pType->Damage;

			if (auto pWeapon = TypeExt->Weapon.Get())
			{
				WeaponTypeExt::DetonateAt(pWeapon, pThis->Bounce.GetCoords(), pInvoker, nDamage);
			}
			else
				if (auto const pWarhead = pType->Warhead)
				{
					auto nLand = pThis->GetCell() ? pThis->GetCell()->LandType : LandType::Clear;
					pAnimType = MapClass::SelectDamageAnimation(nDamage, pWarhead, nLand, nLocation);
					MapClass::DamageArea(nLocation, nDamage, pInvoker, pWarhead, pWarhead->Tiberium, pOwner);
					MapClass::FlashbangWarheadAt(nDamage, pWarhead, nLocation);
					flags = 0x2600;
					ForceZAdjust = -30;

				}
		}
		else
		{
			if (pType->IsMeteor)
			{
				auto const splash = TypeExt->SplashList.GetElements(RulesClass::Instance->SplashList);
				auto const nSplashIdx = ScenarioClass::Instance->Random(0, (splash.size() - 1));
				pAnimType = splash.at(nSplashIdx);
			}
			else
			{
				pAnimType = TypeExt->WakeAnim.Get(RulesClass::Instance->Wake);
			}
		}

	} else {
		pAnimType = GetOriginalAnimResult();
	}

	if (pAnimType)
	{
		if (auto const pAnimCreated = GameCreate<AnimClass>(pAnimType, nLocation, 0, 1, flags, ForceZAdjust))
		{
			AnimExt::SetAnimOwnerHouseKind(pAnimCreated, pOwner, nullptr, false);
			if (auto const pAnimExt = AnimExtAlt::GetExtData(pAnimCreated))
				pAnimExt->Invoker = pInvoker;
		}
	}

	return 0x74A22A;
}

DEFINE_HOOK(0x74A13E, VoxelAnimClass_ExpiredOnLand_DamageArea, 0x6)
{
	GET(VoxelAnimClass*, pThis, EBX);

	if (auto pType = pThis->Type)
	{
		auto nCoords = pThis->Bounce.GetCoords();
		auto TypeExt = VoxelAnimTypeExt::ExtMap.Find(pType);
		TechnoClass* pInvoker = VoxelAnimExt::GetTechnoOwner(pThis, TypeExt && TypeExt->Damage_DealtByOwner.Get());
		auto const pOwner = pInvoker ? pInvoker->GetOwningHouse() : pThis->OwnerHouse;

		if (auto pExpireAnim = pType->ExpireAnim)
		{
			if (auto pAnim = GameCreate<AnimClass>(pExpireAnim, nCoords, 0, 1, 0x2600u, -30, 0))
			{
				AnimExt::SetAnimOwnerHouseKind(pAnim, pOwner, nullptr, false);
				if (auto const pAnimExt = AnimExtAlt::GetExtData(pAnim))
					pAnimExt->Invoker = pInvoker;
			}
		}

		auto nDamage = pType->Damage;

		if (TypeExt) {
			if (auto pWeapon = TypeExt->Weapon.Get()) {
				WeaponTypeExt::DetonateAt(pWeapon, pThis->Bounce.GetCoords(), pInvoker, nDamage);
				return 0x74A22A;
			}
		}

		if (auto pWarhead = pType->Warhead) {
				MapClass::DamageArea(nCoords, nDamage, pInvoker, pWarhead, pWarhead->Tiberium, pOwner);
				MapClass::FlashbangWarheadAt(nDamage, pWarhead, nCoords, false);
		}
	}

	return 0x74A22A;
}

DEFINE_HOOK(0x74A83C, VoxelAnimClass_BounceAnim, 0xA)
{
	GET(VoxelAnimClass*, pThis, EBX);

	auto nCoords = pThis->GetCoords();
	auto TypeExt = VoxelAnimTypeExt::ExtMap.Find(pThis->Type);
	TechnoClass* pInvoker = VoxelAnimExt::GetTechnoOwner(pThis, TypeExt && TypeExt->Damage_DealtByOwner.Get());
	auto const pOwner = pInvoker ? pInvoker->GetOwningHouse() : pThis->OwnerHouse;

	if (auto pAnim = GameCreate<AnimClass>(pThis->Type->BounceAnim, nCoords, 0, 1, 0x600, 0, 0))
	{
		AnimExt::SetAnimOwnerHouseKind(pAnim, pOwner, nullptr, false);
		if (auto const pAnimExt = AnimExtAlt::GetExtData(pAnim))
			pAnimExt->Invoker = pInvoker;
	}

	return 0x74A884;
}
#pragma endregion