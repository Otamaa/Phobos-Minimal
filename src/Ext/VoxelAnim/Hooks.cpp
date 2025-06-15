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

#include <Misc/DynamicPatcher/Trails/TrailsManager.h>


ASMJIT_PATCH(0x74A70E, VoxelAnimClass_AI_Additional, 0x6) // C
{
	GET(VoxelAnimClass* const, pThis, EBX);

	const auto pThisExt = VoxelAnimExtContainer::Instance.Find(pThis);
	const auto pTypeExt = VoxelAnimTypeExtContainer::Instance.Find(pThis->Type);

	if (!pThisExt->LaserTrails.empty())
	{
		CoordStruct location = pThis->GetCoords();
		CoordStruct drawnCoords = location;

		for (auto& trail : pThisExt->LaserTrails)
		{
			if (!trail.LastLocation.isset())
				trail.LastLocation = location;

			trail.Visible = pThis->IsVisible;
			trail.Update(drawnCoords);
		}
	}

	TrailsManager::AI(pThis);

	if (pThis->Type->TrailerAnim && pTypeExt->TrailerAnim_SpawnDelay >= 1) {
		auto& timer = pThisExt->TrailerSpawnDelayTimer;

		if(timer.Expired()) {

			timer.Start(pTypeExt->TrailerAnim_SpawnDelay);

			CoordStruct _coord = pThis->Bounce.GetCoords();

			if (!_coord.IsValid()) //yeah , fuck , lagging the game
				return 0x74A7AB;

			TechnoClass* const pInvoker = VoxelAnimExtData::GetTechnoOwner(pThis);
			auto const pOwner = pThis->OwnerHouse ? pThis->OwnerHouse : pInvoker ? pInvoker->GetOwningHouse() : HouseExtData::FindFirstCivilianHouse();

			AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pThis->Type->TrailerAnim, _coord, 1, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0), pOwner, nullptr, pInvoker, false, false);
		}
	}

	return 0x74A7AB;
}

#pragma region Otamaa

ASMJIT_PATCH(0x74A021, VoxelAnimClass_AI_Expired, 0x6)
{
	enum { SkipGameCode = 0x74A22A };

	GET(VoxelAnimClass* const, pThis, EBX);

	auto const pTypeExt = VoxelAnimTypeExtContainer::Instance.TryFind(pThis->Type);

	GET8(bool, LandIsWater, CL);
	GET8(bool, EligibleHeight, AL);

	//overridden instruction
	R->Stack(0x12, R->AL());

	TechnoClass* const pInvoker = VoxelAnimExtData::GetTechnoOwner(pThis);

	auto const pOwner = pThis->OwnerHouse ? pThis->OwnerHouse : pInvoker ? pInvoker->GetOwningHouse() : HouseExtData::FindFirstCivilianHouse();
	CoordStruct nLocation = pThis->GetCoords();

	if (!LandIsWater || EligibleHeight)
	{
		Helper::Otamaa::Detonate(pTypeExt->Weapon, pThis->Type->Damage, pThis->Type->Warhead, pTypeExt->Warhead_Detonate, pThis->Bounce.GetCoords(), pInvoker, pOwner, pTypeExt->ExpireDamage_ConsiderInvokerVet);

		if (auto const pExpireAnim = pThis->Type->ExpireAnim) {
			AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pExpireAnim, nLocation, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200 | AnimFlag::AnimFlag_2000, -30, 0),
				pOwner,
				nullptr,
				pInvoker,
				false, false
			);
		}
	}
	else {
		if (!pTypeExt->ExplodeOnWater.Get())
		{
			if (auto pSplashAnim = Helper::Otamaa::PickSplashAnim(pTypeExt->SplashList, pTypeExt->WakeAnim, pTypeExt->SplashList_Pickrandom.Get(), pThis->Type->IsMeteor)) {
				AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pSplashAnim, nLocation, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, false),
					pOwner,
					nullptr,
					pInvoker,
					false, false
				);
			}
		}else
		{
			auto const& [bPlayWHAnim , nDamage] = Helper::Otamaa::Detonate(pTypeExt->Weapon, pThis->Type->Damage, pThis->Type->Warhead, pTypeExt->Warhead_Detonate, pThis->GetCoords(), pInvoker, pOwner, pTypeExt->ExpireDamage_ConsiderInvokerVet);

			if (bPlayWHAnim) {
				if(auto pSplashAnim = MapClass::SelectDamageAnimation(nDamage,pThis->Type->Warhead, pThis->GetCell()->LandType , pThis->GetCoords())) {
					AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pSplashAnim, nLocation, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200 | AnimFlag::AnimFlag_2000, -30),
						pOwner,
						nullptr,
						pInvoker,
						false, false
					);
				}
			}
		}
	}

	return SkipGameCode;
}

ASMJIT_PATCH(0x74A83C, VoxelAnimClass_BounceAnim, 0x5) // A
{
	GET(VoxelAnimClass*, pThis, EBX);

	TechnoClass* pInvoker = VoxelAnimExtData::GetTechnoOwner(pThis);
	auto nCoords = pThis->GetCoords();
	auto const pOwner = pInvoker ? pInvoker->GetOwningHouse() : pThis->OwnerHouse;

	AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pThis->Type->BounceAnim, nCoords, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0),
		pOwner,
		nullptr,
		pInvoker,
		false, false
	);

	return 0x74A884;
}

#include <Misc/Hooks.Otamaa.h>

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F6410, FakeVoxelAnimClass::_RemoveThis);

#pragma endregion