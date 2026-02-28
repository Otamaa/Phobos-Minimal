#include "Body.h"

#include <Ext/Anim/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>

#include <Locomotor/Cast.h>

#include <Utilities/Macro.h>

#include <TacticalClass.h>

#define GET_LOCO(reg_Loco) \
	GET(ILocomotion*, Loco, reg_Loco); \
	TeleportLocomotionClass* pLocomotor = static_cast<TeleportLocomotionClass*>(Loco); \
	TechnoClass* pOwner =  pLocomotor->LinkedTo ? pLocomotor->LinkedTo : pLocomotor->Owner; \
	TechnoTypeClass* pType = GET_TECHNOTYPE(pOwner); \
	TechnoTypeExtData *pExt = TechnoTypeExtContainer::Instance.Find(pType);

ASMJIT_PATCH(0x7197DF, TeleportLocomotionClass_Process_ChronospherePreDelay, 0x5)
{
	//GET(TeleportLocomotionClass*, pThis, ESI);
	GET(FootClass*, pLinked, ECX);

	auto pTypeExtData = GET_TECHNOTYPEEXT(pLinked);
	R->ECX(pTypeExtData->ChronoSpherePreDelay.Get(RulesExtData::Instance()->ChronoSpherePreDelay));
	return 0x7197E4;
}

ASMJIT_PATCH(0x719BD9, TeleportLocomotionClass_Process_ChronosphereDelay2, 0x6)
{
	GET(TeleportLocomotionClass*, pThis, ESI);

	auto const pExt = TechnoExtContainer::Instance.Find(pThis->Owner);

	if (!pExt->IsBeingChronoSphered)
		return 0;

	auto pTypeExtData = GET_TECHNOTYPEEXT(pThis->Owner);
	int delay = pTypeExtData->ChronoSphereDelay.Get(RulesExtData::Instance()->ChronoSphereDelay);

	if (delay > 0)
	{
		pThis->Owner->WarpingOut = true;
		pExt->HasRemainingWarpInDelay = true;
		pExt->LastWarpInDelay = std::max(delay, pExt->LastWarpInDelay);
	}
	else
	{
		pExt->IsBeingChronoSphered = false;
	}

	return 0;
}

ASMJIT_PATCH(0x7193F6, TeleportLocomotionClass_ILocomotion_Process_WarpoutAnim, 0x6)
{
	GET_LOCO(ESI);

	TechnoExtData::PlayAnim(pExt->WarpOut.GetOrDefault(pOwner , RulesClass::Instance->WarpOut), pOwner);

	if (const auto pWeapon = pExt->WarpOutWeapon.Get(pOwner))
		WeaponTypeExtData::DetonateAt1(pWeapon, pOwner, pOwner , true , nullptr);

	const int distance = (int)Math::sqrt(pOwner->Location.DistanceFromSquared(pLocomotor->LastCoords));
	TechnoExtContainer::Instance.Find(pOwner)->LastWarpDistance = distance;

	if (auto pImage = pType->AlphaImage) {
		auto xy = TacticalClass::Instance->CoordsToClient(pOwner->Location);
		RectangleStruct Dirty = { xy.X - (pImage->Width / 2) , xy.Y - (pImage->Height / 2),
		  pImage->Width, pImage->Height };
		TacticalClass::Instance->RegisterDirtyArea(Dirty, true);
	}

	int duree = pExt->ChronoMinimumDelay.GetOrDefault(pOwner, RulesClass::Instance->ChronoMinimumDelay);
	const auto factor = pExt->ChronoRangeMinimum.GetOrDefault(pOwner, RulesClass::Instance->ChronoRangeMinimum);

	if (distance >= factor
		&& pExt->ChronoTrigger.GetOrDefault(pOwner, RulesClass::Instance->ChronoTrigger))
	{
		const auto f_factor = pExt->ChronoDistanceFactor.GetOrDefault(pOwner, RulesClass::Instance->ChronoDistanceFactor);
		duree = MaxImpl(distance / MaxImpl(f_factor, 1), duree);

	}

	pOwner->WarpingOut = true;

	if (auto pUnit = cast_to<UnitClass*, false>(pOwner)) {
		if (pUnit->Type->Harvester || pUnit->Type->Weeder) {
			duree = 0;
			pUnit->WarpingOut = false;
		}
	}

	auto const pLinkedExt = TechnoExtContainer::Instance.Find(pOwner);
	pLocomotor->Timer.Start(duree);
	pLinkedExt->LastWarpInDelay = std::max(duree, pLinkedExt->LastWarpInDelay);
	return 0x7195BC;
}

ASMJIT_PATCH(0x719742, TeleportLocomotionClass_ILocomotion_Process_WarpInAnim, 0x6)
{
	GET_LOCO(ESI);

	//WarpIn is unused , maybe a type on WW side
	TechnoExtData::PlayAnim(pExt->WarpIn.GetOrDefault(pOwner ,RulesClass::Instance->WarpOut), pOwner);

	const auto pTechnoExt = TechnoExtContainer::Instance.Find(pOwner);

	const auto Rank = pOwner->CurrentRanking;
	const auto pWarpInWeapon = pExt->WarpInWeapon.GetFromSpecificRank(Rank);

	const auto pWeapon = pTechnoExt->LastWarpDistance < pExt->ChronoRangeMinimum.GetOrDefault(pOwner ,RulesClass::Instance->ChronoRangeMinimum)
		? pExt->WarpInMinRangeWeapon.GetFromSpecificRank(Rank)->Get(pWarpInWeapon) : pWarpInWeapon;

	if (pWeapon) {
		const int damage = pExt->WarpInWeapon_UseDistanceAsDamage.Get(pOwner) ?
			(pTechnoExt->LastWarpDistance / Unsorted::LeptonsPerCell) : pWeapon->Damage;

		WeaponTypeExtData::DetonateAt2(pWeapon, pOwner, pOwner, damage, true, nullptr);
	}

	return 0x719796;
}

ASMJIT_PATCH(0x719827, TeleportLocomotionClass_ILocomotion_Process_WarpAway, 0x6)
{
	GET_LOCO(ESI);

	TechnoExtData::PlayAnim(pExt->WarpAway.GetOrDefault(pOwner , RulesClass::Instance->WarpOut), pOwner);
	return 0x719878;
}

ASMJIT_PATCH(0x71997B, TeleportLocomotionClass_ILocomotion_Process_ChronoDelay, 0x6)
{
	GET_LOCO(ESI);
	GET(RulesClass*, pRules, EAX);

	R->ECX(pExt->ChronoDelay.GetOrDefault(pOwner, pRules->ChronoDelay));

	return 0x719981;
}

ASMJIT_PATCH(0x7196BB, TeleportLocomotionClass_Process_MarkDown, 0xA)
{
	enum { SkipGameCode = 0x7196C5 };

	GET(FootClass*, pLinkedTo, ECX);
	// When Teleport units board transport vehicles on the bridge, the lack of this repair can lead to numerous problems
	// An impassable invisible barrier will be generated on the bridge (the object linked list of the cell will leave it)
	// And the transport vehicle will board on the vehicle itself (BFRT Passenger:..., BFRT)
	// If any infantry attempts to pass through this position on the bridge later, it will cause the game to freeze
	auto shouldMarkDown = [pLinkedTo]()
		{
			if (pLinkedTo->GetCurrentMission() != Mission::Enter)
				return true;

			const auto pEnter = pLinkedTo->GetNthLink();

			return (!pEnter || GET_TECHNOTYPE(pEnter)->Passengers <= 0);
		};

	if (shouldMarkDown())
		pLinkedTo->Mark(MarkType::Down);

	return SkipGameCode;
}

// Rewrite from 0x718505
ASMJIT_PATCH(0x718F1E, TeleportLocomotionClass_MovingTo_ReplaceMovementZone, 0x6)
{
	GET(TechnoTypeClass* const, pType, EAX);

	auto movementZone = pType->MovementZone;

	if (movementZone == MovementZone::Fly || movementZone == MovementZone::Destroyer)
		movementZone = MovementZone::Normal;
	else if (movementZone == MovementZone::AmphibiousDestroyer)
		movementZone = MovementZone::Amphibious;

	R->EBP(movementZone);
	return R->Origin() + 0x6;
}ASMJIT_PATCH_AGAIN(0x7190B0, TeleportLocomotionClass_MovingTo_ReplaceMovementZone, 0x6)

ASMJIT_PATCH(0x71872C, TeleportLocomotionClass_MakeRoom_OccupationFix, 0x9)
{
	enum { SkipMarkOccupation = 0x71878F };

	GET(const LocomotionClass* const, pLoco, EBP);

	const auto pFoot = pLoco->LinkedTo;

	return (pFoot && !pFoot->InLimbo && pFoot->IsAlive && pFoot->Health > 0 && !pFoot->IsSinking) ? 0 : SkipMarkOccupation;
}

// do not let deactivated teleporter units move, otherwise
// they could block a cell forever
ASMJIT_PATCH(0x71810D, TeleportLocomotionClass_ILocomotion_MoveTo_Deactivated, 6)
{
	GET(FootClass*, pFoot, ECX);
	return (!pFoot->Deactivated && pFoot->Locomotor.GetInterfacePtr()->Is_Powered() && !TechnoExtContainer::Instance.Find(pFoot)->Is_DriverKilled)
		? 0 : 0x71820F;
}

// sink stuff that simply cannot exist on water
ASMJIT_PATCH(0x7188F2, TeleportLocomotionClass_Unwarp_SinkJumpJets, 7)
{
	GET(CellClass*, pCell, EAX);
	GET(TechnoClass**, pTechno, ESI);

	if (pCell->Tile_Is_Wet() && !pCell->ContainsBridge())
	{
		if (UnitClass* pUnit = cast_to<UnitClass*>(pTechno[3]))
		{
			if (pUnit->Deactivated || TechnoExtContainer::Instance.Find(pUnit)->Is_DriverKilled)
			{
				// this thing does not float
				R->BL(0);
			}

			// manually sink it
			if (pUnit->Type->SpeedType == SpeedType::Hover && pUnit->Type->JumpJet)
			{
				return 0x718A66;
			}
		}
	}

	return 0;
}

// iron curtained units would crush themselves
ASMJIT_PATCH(0x7187DA, TeleportLocomotionClass_Unwarp_PreventSelfCrush, 6)
{
	GET(TechnoClass*, pTeleporter, EDI);
	GET(TechnoClass*, pContent, ECX);
	return (pTeleporter == pContent) ? 0x71880A : 0;
}

// bug 897
ASMJIT_PATCH(0x718871, TeleportLocomotionClass_UnfreezeObject_SinkOrSwim, 7)
{
	GET(TechnoTypeClass*, Type, EAX);

	switch (Type->MovementZone)
	{
	case MovementZone::Amphibious:
	case MovementZone::AmphibiousCrusher:
	case MovementZone::AmphibiousDestroyer:
	case MovementZone::WaterBeach:
		R->BL(1);
		return 0x7188B1;
	}
	if (Type->SpeedType == SpeedType::Hover)
	{
		// will set BL to 1 , unless this is a powered unit with no power centers <-- what if we have a powered unit that's not a hover?
		return 0x71887A;
	}
	return 0x7188B1;
}

ASMJIT_PATCH(0x7185DA, TeleportLocomotionClass_MakeRoom_DestFix, 0x6)
{
	enum { ReturnTrue = 0x71878F };

	GET(CellStruct*, pCellAt, EAX);
	GET(LocomotionClass*, pLoco, EBP);

	if (*pCellAt == CellStruct::Empty)
	{
		// cannot find location ? dont move
		pLoco->LinkedTo->ChronoDestCoords = pLoco->LinkedTo->Location;
		return ReturnTrue;
	}
	return 0;
}

ASMJIT_PATCH(0x7184CE, TeleportLocomotionClass_MakeRoom_GetMovement_CellFix, 0x7)
{
	REF_STACK(CoordStruct, coords, STACK_OFFSET(0x5C, 0x4));

	R->Stack(STACK_OFFSET(0x38, -0x18), MapClass::Instance->GetCellAt(coords));
	return 0;
}

ASMJIT_PATCH(0x718275, TeleportLocomotionClass_MakeRoom, 9)
{
	LEA_STACK(CoordStruct*, pCoord, 0x3C);
	GET(TeleportLocomotionClass*, pLoco, EBP);

	const auto pCell = MapClass::Instance->GetCellAt(pCoord);

	R->Stack(0x48, false);
	const bool bLinkedIsInfantry = pLoco->LinkedTo->WhatAmI() == AbstractType::Infantry;
	R->EBX(pCell->OverlayTypeIndex);
	R->EDI(false);

	for (auto pObj = pCell->GetContentB(); pObj; pObj = pObj->NextObject)
	{
		const auto bObjIsInfantry = pObj->WhatAmI() == AbstractType::Infantry;
		bool bIsImmune = pObj->IsIronCurtained();
		auto pType = pObj->GetTechnoType();
		const auto pTypeExt = TechnoTypeExtContainer::Instance.TryFind(pType);

		if (pTypeExt && !pTypeExt->Chronoshift_Crushable)
			bIsImmune = 1;

		if (!RulesExtData::Instance()->ChronoInfantryCrush && bLinkedIsInfantry && !bObjIsInfantry)
		{
			pLoco->LinkedTo->ReceiveDamage(&pLoco->LinkedTo->GetType()->Strength, 0, RulesClass::Instance->C4Warhead, 0, 1, 0, 0);
			break;
		}

		if (!bIsImmune && bObjIsInfantry && bLinkedIsInfantry)
		{
			auto nCoord = pObj->GetCoords();
			if (nCoord.X == pCoord->X && nCoord.Y == pCoord->Y && nCoord.Z == pCoord->Z)
			{
				pObj->ReceiveDamage(&pObj->GetType()->Strength, 0, RulesClass::Instance->C4Warhead, 0, 1, 0, 0);
			}
		}
		else if (bIsImmune || ((pObj->AbstractFlags & AbstractFlags::Foot) == AbstractFlags::None))
		{
			if ((pObj->AbstractFlags & AbstractFlags::Foot) == AbstractFlags::None)
			{
				R->Stack(0x48, true);
			}
			else if (bIsImmune)
			{
				pLoco->LinkedTo->ReceiveDamage(&pLoco->LinkedTo->GetType()->Strength, 0, RulesClass::Instance->C4Warhead, 0, 1, 0, 0);
				break;
			}
		}
		else
		{
			pObj->ReceiveDamage(&pObj->GetType()->Strength, 0, RulesClass::Instance->C4Warhead, 0, 1, 0, 0);
		}
	}

	// Check if bridge exists (0x100) but body is destroyed (0x200 not set)
	// Original: (Flags & 0x300) == 0x100 means bridge head present without body
	if ((pCell->Flags & CellFlags(0x300)) == CellFlags(0x100))
		R->Stack(0x48, true);

	R->Stack(0x20, pLoco->LinkedTo->GetCellAgain());
	R->EAX(true);
	return 0x7184CE;
}

//FORCEDINLINE std::pair<Matrix3D, Matrix3D> SimplifiedTiltingConsideration(float arf, float ars, TechnoTypeClass* linkedType)
//{
//	double scalex = linkedType->VoxelScaleX;
//	double scaley = linkedType->VoxelScaleY;
//
//	Matrix3D pre = Matrix3D::GetIdentity();
//	pre.TranslateZ(float(Math::abs(Math::sin(ars)) * scalex + Math::abs(Math::sin(arf)) * scaley));
//
//	Matrix3D post = Matrix3D::GetIdentity();
//	post.TranslateX(float(Math::signum(arf) * (scaley * (1 - Math::cos(arf)))));
//	post.TranslateY(float(Math::signum(-ars) * (scalex * (1 - Math::cos(ars)))));
//	post.RotateX(ars);
//	post.RotateY(arf);
//
//	return { pre,post };
//}

#ifndef FUCKTHESE

// Author : chaserli
Matrix3D* __stdcall LocomotionClass_Draw_Matrix(ILocomotion* pThis, Matrix3D* ret, VoxelIndexKey* pIndex)
{
	auto loco = static_cast<LocomotionClass*>(pThis);
	auto slope_idx = MapClass::Instance->GetCellAt(loco->Owner->Location)->SlopeIndex;

	if (pIndex && pIndex->Is_Valid_Key())
		*(int*)(pIndex) = slope_idx + (*(int*)(pIndex) << 6);

	Matrix3D _DrawMtx {};
	loco->LocomotionClass::Draw_Matrix(&_DrawMtx,pIndex);
	*ret = Game::VoxelRampMatrix[slope_idx] * _DrawMtx;

	float arf = loco->Owner->AngleRotatedForwards;
	float ars = loco->Owner->AngleRotatedSideways;

	if (Math::abs(ars) >= 0.005 || Math::abs(arf) >= 0.005)
	{
		//just forget about ramp here, math too complicated, not considered for other locos either
		if (pIndex)
			*(int*)pIndex = -1;

		auto pOwnerType = GET_TECHNOTYPE(loco->Owner);
		double scalex = pOwnerType->VoxelScaleX;
		double scaley = pOwnerType->VoxelScaleY;

		Matrix3D pre = Matrix3D::GetIdentity();
		pre.TranslateZ(float(Math::abs(Math::sin(ars)) * scalex + Math::abs(Math::sin(arf)) * scaley));
		ret->TranslateX(float(Math::signum(arf) * (scaley * (1 - Math::cos(arf)))));
		ret->TranslateY(float(Math::signum(-ars) * (scalex * (1 - Math::cos(ars)))));
		ret->RotateX(ars);
		ret->RotateY(arf);

		*ret = pre * *ret;
		return ret;
	}

	return ret;
}
DEFINE_FUNCTION_JUMP(VTABLE , 0x7F5024, LocomotionClass_Draw_Matrix)

ASMJIT_PATCH(0x729B5D, TunnelLocomotionClass_DrawMatrix_Tilt, 0x8)
{
	GET(ILocomotion*, iloco, ESI);
	GET_BASE(VoxelIndexKey*, pIndex, 0x10);
	GET_BASE(Matrix3D*, ret, 0xC);
	R->EAX(LocomotionClass_Draw_Matrix(iloco, ret, pIndex));
	return 0x729C09;
}
#endif
#undef GET_LOCO