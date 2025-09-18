#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

#include <Ext/WarheadType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/BuildingType/Body.h>

#include <InfantryClass.h>

ASMJIT_PATCH(0x5f4fe7, ObjectClass_Put, 8)
{
	GET(ObjectClass*, pThis, ESI);
	GET(ObjectTypeClass*, pType, EBX);

	if(pType) {
		if(auto pBullet = cast_to<BulletClass*, false>(pThis)) {
			BulletExtContainer::Instance.Find(pBullet)->CreateAttachedSystem();
		}

		return 0x5F4FEF;
	}

	return 0x5F5210;
}

ASMJIT_PATCH(0x469467, BulletClass_Logics_CanTemporalTarget, 0x5)
{
	GET(TechnoClass* const, Target, ECX);

	switch (Target->InWhichLayer())
	{
	case Layer::Ground:
	case Layer::Air:
	case Layer::Top:
		return 0x469475;
	}

	return 0x469AA4;
}

// Overpowerer no longer just infantry
ASMJIT_PATCH(0x4693B0, BulletClass_Logics_Overpower, 0x6)
{
	GET(TechnoClass* const, pT, ECX);
	switch (pT->WhatAmI())
	{
	case InfantryClass::AbsID:
	case UnitClass::AbsID:
	case BuildingClass::AbsID:
		return 0x4693BC;
	default:
		return 0x469AA4;
	}
}

ASMJIT_PATCH(0x4664FB, BulletClass_Initialize_Ranged, 0x6)
{
	GET(BulletClass*, pThis, ECX);
	// conservative approach for legacy-initialized bullets
	pThis->Range = std::numeric_limits<int>::max();
	return 0;
}

ASMJIT_PATCH(0x46837F, BulletClass_DrawSHP_SetAnimPalette, 6)
{
	GET(BulletTypeClass* const, pType, EAX);

	if (!pType)
		return 0x0;

	const auto pTypeExt = BulletTypeExtContainer::Instance.Find(pType);

	if (const auto pConvert = pTypeExt->GetBulletConvert()) {
		R->EBX(pConvert);
		return 0x4683D7;
	}

	return 0x0;
}

#include <Ext/Techno/Body.h>
#include <Utilities/DebrisSpawners.h>

ASMJIT_PATCH(0x469C46, BulletClass_Logics_ApplyMoreLogics, 0x8)
{
	GET(FakeBulletClass* const, pThis, ESI);
	GET(AnimTypeClass* const, AnimType, EBX);
	LEA_STACK(CoordStruct*, XYZ, 0x64);

	if(AnimType){

		const auto pWarheadExt = WarheadTypeExtContainer::Instance.Find(pThis->WH);

		int cellHeight = MapClass::Instance->GetCellFloorHeight(XYZ);
		auto newCrds = pWarheadExt->PlayAnimAboveSurface ? CoordStruct{ XYZ->X, XYZ->Y, MaxImpl(cellHeight, XYZ->Z) } : *XYZ;

		if (!(cellHeight > newCrds.Z && !pWarheadExt->PlayAnimUnderground))
		{
			bool createdAnim = false;

			int creationInterval = pWarheadExt->Splashed ?
					pWarheadExt->SplashList_CreationInterval : pWarheadExt->AnimList_CreationInterval;

			int* remainingInterval = &pWarheadExt->RemainingAnimCreationInterval;
			int scatterMin = pWarheadExt->Splashed ? pWarheadExt->SplashList_ScatterMin.Get() : pWarheadExt->AnimList_ScatterMin.Get();
			int scatterMax = pWarheadExt->Splashed ? pWarheadExt->SplashList_ScatterMax.Get() : pWarheadExt->AnimList_ScatterMax.Get();
			bool allowScatter = scatterMax != 0 || scatterMin != 0;

			if (creationInterval > 0 && pThis->Owner)
					remainingInterval = &TechnoExtContainer::Instance.Find(pThis->Owner)->WHAnimRemainingCreationInterval;

			if (creationInterval < 1 || *remainingInterval <= 0)
			{
				HouseClass* pInvoker = nullptr ;
				HouseClass* pVictim = nullptr;

				if (const TechnoClass* Target = flag_cast_to<TechnoClass*>(pThis->Target)) {
					pVictim = Target->Owner;
				}

				if (const auto pTech = pThis->Owner) {
					pInvoker = pThis->Owner->GetOwningHouse();

				} else {
					if (auto const pBulletExt = BulletExtContainer::Instance.Find(pThis))
						pInvoker = pBulletExt->Owner;
				}

				auto types = make_iterator_single(AnimType);

				if (pWarheadExt->SplashList_CreateAll && pWarheadExt->Splashed)
					types = pWarheadExt->SplashList.GetElements(RulesClass::Instance->SplashList);
				else if (!pWarheadExt->Splashed){
					bool createAll = pWarheadExt->AnimList_CreateAll;
					if (pWarheadExt->CritActive && !pWarheadExt->Crit_AnimList.empty() && !pWarheadExt->Crit_AnimOnAffectedTargets) {
						createAll = pWarheadExt->Crit_AnimList_CreateAll.Get(createAll);
						if (createAll)
							types = pWarheadExt->Crit_AnimList;
					}
					else if (createAll) {
							types = pWarheadExt->This()->AnimList;
					}
				}

					for (auto pType : types)
					{
						if (!pType)
							continue;

							auto animCoords = newCrds;

							if (allowScatter) {
								int distance = ScenarioClass::Instance->Random.RandomRanged(scatterMin, scatterMax);
								animCoords = MapClass::GetRandomCoordsNear(animCoords, distance, false);
							}

							{
								auto const pAnim = GameCreate<AnimClass>(pType, animCoords, 0, 1, (AnimFlag)0x2600, -15, false);
								createdAnim = true;

								if (const auto pTech = pThis->Owner) {
									((FakeAnimClass*)pAnim)->_GetExtData()->Invoker = pTech;
								}

								if (pAnim->Type->MakeInfantry > -1)
								{
									AnimExtData::SetAnimOwnerHouseKind(pAnim, pInvoker, pVictim, nullptr, true ,false);
								}
								else
								{
									AnimExtData::SetAnimOwnerHouseKind(pAnim, pInvoker, pVictim, pInvoker);
								}
							}
					}
			}else
			{
				(*remainingInterval)--;
			}

			if (!createdAnim && pWarheadExt->IsNukeWarhead.Get()) {
				int HouseIdx = pThis->Owner ? pThis->Owner->Owner->ArrayIndex : -1;
				auto _loc = pThis->GetCoords();
				auto _loc_result = CellClass::Coord2Cell(_loc);
				MapClass::AtomDamage(HouseIdx, _loc_result);
			}
		}
	}

	auto pWHExt = pThis->_GetWarheadTypeExtData();
	auto pWarhead = pWHExt->This();

	if (pWarhead->MaxDebris > 0)
	{

		auto const pCell = pThis->GetCell();
		const bool isLand = !pCell ? true : pCell->LandType != LandType::Water || pCell->ContainsBridge();

		if (isLand || !pWHExt->Debris_Conventional)
		{

			std::optional<bool> limited {};
			if (pWHExt->DebrisTypes_Limit.isset())
			{
				limited = pWHExt->DebrisTypes_Limit.Get();
			}

			auto pExt = pThis->_GetExtData();
			HouseClass* const pOwner = pThis->Owner ? pThis->Owner->GetOwningHouse() : (pExt->Owner ? pExt->Owner : HouseExtData::FindFirstCivilianHouse());
			HouseClass* const pVictim = (pThis->Target) ? pThis->Target->GetOwningHouse() : nullptr;
			auto spawn_coords = pThis->GetCoords();
			DebrisSpawners::Spawn(pWarhead->MinDebris, pWarhead->MaxDebris,
				spawn_coords, pWarhead->DebrisTypes,
				pWHExt->DebrisAnimTypes.GetElements(RulesClass::Instance->MetallicDebris)
				, pWarhead->DebrisMaximums, pWHExt->DebrisMinimums, limited, pThis->Owner, pOwner, pVictim);
		}
	}

	if (pThis->_GetTypeExtData()->HasSplitBehavior())
	{
		BulletExtData::ApplyAirburst(pThis);
	}

	return 0x46A290;
}

ASMJIT_PATCH(0x46670F, BulletClass_Update_PreImpactAnim, 6)
{
	GET(BulletClass*, pThis, EBP);

	const auto pWarheadTypeExt = WarheadTypeExtContainer::Instance.Find(pThis->WH);

	if (!pThis->NextAnim)
		return 0x46671D;

	if (pWarheadTypeExt->PreImpact_Moves.Get())
	{
		auto coords = pThis->NextAnim->GetCoords();
		pThis->Location = coords;
		pThis->Target = MapClass::Instance->TryGetCellAt(coords);
	}

	return 0x467FEE;
}

ASMJIT_PATCH(0x46867F , BulletClass_SetMovement_Parachute, 5)
{
	GET(CoordStruct*, XYZ, EAX);
	GET(BulletClass*, Bullet, ECX);
	//	GET_BASE(VelocityClass *, Trajectory, 0xC);

	R->EBX<BulletClass*>(Bullet);
	// if (!Bullet->Target) {
	// 	Debug::LogInfo("Bullet [%s - %x] Missing Target Pointer when Unlimbo! , Fallback To CreationCoord to Prevent Crash",
	// 		Bullet->get_ID(), Bullet);
	//
	// 	Bullet->Target = MapClass::Instance->GetCellAt(XYZ);
	// }

	const auto pBulletData = BulletTypeExtContainer::Instance.Find(Bullet->Type);

	bool result = false;
	if (pBulletData->Parachuted) {
		result = Bullet->SpawnParachuted(*XYZ);
		//Bullet->IsABomb = true; dev phobos remove this
	} else {
		result = Bullet->ObjectClass::Unlimbo(*XYZ, DirType::North);
	}

	if (!Bullet->Target) {
		Bullet->Target = MapClass::Instance->GetCellAt(XYZ);
	}

	R->EAX(result);
	return 0x468689;
}

ASMJIT_PATCH(0x468EB9, BulletClass_Fire_SplitsA, 6)
{
	//GET(BulletClass*, pThis, ESI);
	GET(BulletTypeClass* const, pType, EAX);
	return !BulletTypeExtContainer::Instance.Find(pType)->HasSplitBehavior()
		? 0x468EC7u : 0x468FF4u;
}

ASMJIT_PATCH(0x468FFA, BulletClass_Fire_SplitsB, 6)
{
	GET(BulletTypeClass* const, pType, EAX);
	return BulletTypeExtContainer::Instance.Find(pType)->HasSplitBehavior()
		? 0x46909Au : 0x469008u;
}

ASMJIT_PATCH(0x468000, BulletClass_GetAnimFrame, 6)
{
	GET(BulletClass*, pThis, ECX);

	int frame = 0;
	if (pThis->Type->AnimLow || pThis->Type->AnimHigh)
	{
		frame = pThis->AnimFrame;
	}
	else if (pThis->Type->Rotates())
	{
		DirStruct dir(-pThis->Velocity.Y, pThis->Velocity.X);
		const auto ReverseFacing32 = *reinterpret_cast<int(*)[8]>(0x7F4890);
		const auto facing = ReverseFacing32[(short)dir.GetValue(5)];
		const int length = BulletTypeExtContainer::Instance.Find(pThis->Type)->AnimLength.Get();

		if (length > 1)
		{
			frame = facing * length + ((Unsorted::CurrentFrame / pThis->Type->AnimRate) % length);
		}
		else
		{
			frame = facing;
		}
	}

	R->EAX(frame);
	return 0x468088;
}