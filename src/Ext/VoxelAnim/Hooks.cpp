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
#include <Utilities/AnimHelpers.h>
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

DEFINE_HOOK(0x74A021, VoxelAnimClass_AI_Expired, 0x6)
{
	enum { SkipGameCode = 0x74A22A };

	GET(VoxelAnimClass* const, pThis, EBX);
	GET(int, flag, EAX);

	bool heightFlag = flag & 0xFF;

	if (!pThis || !pThis->Type)
		return SkipGameCode;

	CoordStruct nLocation;
	pThis->GetCenterCoord(&nLocation);
	auto const pOwner = pThis->OwnerHouse;
	auto const pTypeExt = VoxelAnimTypeExt::ExtMap.Find(pThis->Type);
	TechnoClass* pInvoker = VoxelAnimExt::GetTechnoOwner(pThis, pTypeExt && pTypeExt->Damage_DealtByOwner.Get());

	if (pThis->GetCell()->LandType != LandType::Water || heightFlag)
	{
		Helper::Otamaa::Detonate(pTypeExt->Weapon, pThis->Type->Damage, pThis->Type->Warhead, pTypeExt->Warhead_Detonate, pThis->Bounce.GetCoords(), pInvoker, pOwner, pTypeExt->ExpireDamage_ConsiderInvokerVet);

		if (auto const pExpireAnim = pThis->Type->ExpireAnim) {
			if (auto pAnim = GameCreate<AnimClass>(pExpireAnim, nLocation, 0, 1, 0x2600u, 0, 0))
			{
				AnimExt::SetAnimOwnerHouseKind(pAnim, pOwner, nullptr, false);
				if (auto const pAnimExt = AnimExt::GetExtData(pAnim))
					pAnimExt->Invoker = pInvoker;
			}
		}
	}
	else {
		if (!pTypeExt->ExplodeOnWater.Get())
		{
			if (auto pSplashAnim = Helper::Otamaa::PickSplashAnim(pTypeExt->SplashList, RulesClass::Instance->Wake, pTypeExt->SplashList_Pickrandom.Get(), pThis->Type->IsMeteor))
			{
				if (auto const pSplashAnimCreated = GameCreate<AnimClass>(pSplashAnim, nLocation, 0, 1, 0x600u, false))
				{
					AnimExt::SetAnimOwnerHouseKind(pSplashAnimCreated, pOwner, nullptr, false);
					if (auto const pAnimExt = AnimExt::GetExtData(pSplashAnimCreated))
						pAnimExt->Invoker = pInvoker;
				}
			}
		}else
		{
			Helper::Otamaa::Detonate(pTypeExt->Weapon, pThis->Type->Damage, pThis->Type->Warhead, pTypeExt->Warhead_Detonate, pThis->GetCenterCoord(), pInvoker, pOwner ,pTypeExt->ExpireDamage_ConsiderInvokerVet);
		}
	}


	return SkipGameCode;
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
		if (auto const pAnimExt = AnimExt::GetExtData(pAnim))
			pAnimExt->Invoker = pInvoker;
	}

	return 0x74A884;
}
#pragma endregion