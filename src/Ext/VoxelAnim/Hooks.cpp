#include "Body.h"

#include <HouseClass.h>
#include <AnimClass.h>
#include <AnimTypeClass.h>
#include <ScenarioClass.h>
#include <WarheadTypeClass.h>

#include <Utilities/Macro.h>
#include <Utilities/GeneralUtils.h>

#include <Ext/Anim/Body.h>
#include <Ext/House/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/VoxelAnimType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Utilities/AnimHelpers.h>
#ifdef COMPILE_PORTED_DP_FEATURES
#include <Misc/DynamicPatcher/Trails/TrailsManager.h>
#endif

DEFINE_HOOK(0x74A70E, VoxelAnimClass_AI_Additional, 0x6) // C
{
	GET(VoxelAnimClass* const, pThis, EBX);

	auto pThisExt = VoxelAnimExt::ExtMap.Find(pThis);

	if (!pThisExt->LaserTrails.empty())
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

	//overridden instruction
	R->Stack(0x12, R->AL());

	if (!pThis->Type) {
		pThis->Type = VoxelAnimTypeClass::Find(VoxelAnimExt::ExtMap.Find(pThis)->ID.data());
	}

	CoordStruct nLocation = pThis->GetCoords();

	auto const pTypeExt = VoxelAnimTypeExt::ExtMap.Find(pThis->Type);
	TechnoClass* const pInvoker = VoxelAnimExt::GetTechnoOwner(pThis, pTypeExt->Damage_DealtByOwner.Get());
	auto const pOwner = pThis->OwnerHouse ? pThis->OwnerHouse : pInvoker ? pInvoker->GetOwningHouse() : HouseExt::FindCivilianSide();

	GET8(bool, LandIsWater, CL);
	GET8(bool, EligibleHeight, AL);

	if (!LandIsWater || EligibleHeight)
	{
		Helper::Otamaa::Detonate(pTypeExt->Weapon, pThis->Type->Damage, pThis->Type->Warhead, pTypeExt->Warhead_Detonate, pThis->Bounce.GetCoords(), pInvoker, pOwner, pTypeExt->ExpireDamage_ConsiderInvokerVet);

		if (auto const pExpireAnim = pThis->Type->ExpireAnim) {
			if (auto pAnim = GameCreate<AnimClass>(pExpireAnim, nLocation, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200 | AnimFlag::AnimFlag_2000, -30, 0)) {
				AnimExt::SetAnimOwnerHouseKind(pAnim, pOwner, nullptr, pInvoker, false);
			}
		}
	}
	else {
		if (!pTypeExt->ExplodeOnWater.Get())
		{
			if (auto pSplashAnim = Helper::Otamaa::PickSplashAnim(pTypeExt->SplashList, RulesClass::Instance->Wake, pTypeExt->SplashList_Pickrandom.Get(), pThis->Type->IsMeteor)) {
				if (auto const pSplashAnimCreated = GameCreate<AnimClass>(pSplashAnim, nLocation, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, false)) {
					AnimExt::SetAnimOwnerHouseKind(pSplashAnimCreated, pOwner, nullptr, pInvoker, false);
				}
			}
		}else
		{
			auto const [bPlayWHAnim , nDamage] = Helper::Otamaa::Detonate(pTypeExt->Weapon, pThis->Type->Damage, pThis->Type->Warhead, pTypeExt->Warhead_Detonate, pThis->GetCenterCoord(), pInvoker, pOwner, pTypeExt->ExpireDamage_ConsiderInvokerVet);

			if (bPlayWHAnim) {
				if(auto pSplashAnim = MapClass::SelectDamageAnimation(nDamage,pThis->Type->Warhead, pThis->GetCell()->LandType , pThis->GetCenterCoord())) {
					if (auto const pSplashAnimCreated = GameCreate<AnimClass>(pSplashAnim, nLocation, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200 | AnimFlag::AnimFlag_2000, -30)) {
						AnimExt::SetAnimOwnerHouseKind(pSplashAnimCreated, pOwner, nullptr, pInvoker, false) ;
					}
				}
			}
		}
	}


	return SkipGameCode;
}

DEFINE_HOOK(0x74A83C, VoxelAnimClass_BounceAnim, 0x5) // A
{
	GET(VoxelAnimClass*, pThis, EBX);

	auto nCoords = pThis->GetCoords();
	auto TypeExt = VoxelAnimTypeExt::ExtMap.Find(pThis->Type);
	TechnoClass* pInvoker = VoxelAnimExt::GetTechnoOwner(pThis, TypeExt->Damage_DealtByOwner.Get());
	auto const pOwner = pInvoker ? pInvoker->GetOwningHouse() : pThis->OwnerHouse;

	if (auto pAnim = GameCreate<AnimClass>(pThis->Type->BounceAnim, nCoords, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0)) {
		AnimExt::SetAnimOwnerHouseKind(pAnim, pOwner, nullptr, pInvoker, false);
	}

	return 0x74A884;
}

static void __fastcall VoxelAnimClass_Remove_This(VoxelAnimClass* pThis)
{
	if (pThis->Type && pThis->Type->StopSound != -1)
		VocClass::PlayAt(pThis->Type->StopSound, pThis->Location, nullptr);

	pThis->ObjectClass::UnInit();

}

DEFINE_JUMP(VTABLE, 0x7F6410, GET_OFFSET(VoxelAnimClass_Remove_This));

#pragma endregion