#include <Ext/Wave/Body.h>

#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>
#include <Utilities/Helpers.h>

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/Bomb/Body.h>

#include <New/Type/ArmorTypeClass.h>

#include <Notifications.h>
#include <algorithm>

#include "Header.h"

#include <New/Entity/FlyingStrings.h>
#include <InfantryClass.h>

#include <Misc/DamageArea.h>

//BombListClass_Plant_AttachSound
DEFINE_JUMP(LJMP, 0x438FD7, 0x439022);

// #896027: do not announce pointers as expired to bombs
// if the pointed to object is staying in-game.
ASMJIT_PATCH(0x725961, AnnounceInvalidPointer_BombCloak, 0x6)
{
	GET(bool, remove, EDI);
	return remove ? 0 : 0x72596C;
}

// ASMJIT_PATCH(0x438A00, BombClass_GetCurrentFrame, 6)
// {
// 	GET(BombClass*, pThis, ECX);
//     const auto ext = BombExtContainer::Instance.Find(pThis);
//     const auto pData = ext->Weapon;

//     const SHPStruct* shp = pData->Ivan_Image.Get(RulesClass::Instance->BOMBCURS_SHP);
//     const int frames = shp->Frames;

//     int result = 0;

//     if(frames >= 2) {
//         if(pThis->Type != BombType::NormalBomb) {
//             // DeathBomb → last frame
//             result = frames - 1;
//         }  else {
//             const int delay = pData->Ivan_Delay.Get(RulesClass::Instance->IvanTimedDelay);
//             const int flickerRate = pData->Ivan_FlickerRate.Get(RulesClass::Instance->IvanIconFlickerRate);
//             const int elapsed = Unsorted::CurrentFrame - pThis->PlantingFrame;

//             const int half = frames / 2;
//             int capped = half - 1;

//             if(flickerRate <= 0) {
//                 // no flicker: use only half the frames
//                 int frame = elapsed / (delay / (2 * half));
//                 if(frame > capped) frame = capped;
//                 result = frame;
//             }  else  {
//                 // flicker: use full even/odd pattern
//                 int frame = elapsed / (delay / half);
//                 if(frame > capped) frame = capped;

//                 int even = frame * 2;
//                 int odd  = even + 1;

//                 bool flick = (Unsorted::CurrentFrame % (2 * flickerRate)) < flickerRate;
//                 result = flick ? even : odd;
//             }
//         }
//     }

//     R->EAX(result);
//     return 0x438A62;
// }

// 6F523C, 5
// custom ivan bomb drawing
ASMJIT_PATCH(0x6F523C, TechnoClass_DrawExtras_IvanBombImage_Shape, 5)
{
	GET(TechnoClass*, pThis, EBP);

	if (SHPStruct* pImage = BombExtContainer::Instance.Find(pThis->AttachedBomb)
		->Weapon->Ivan_Image.Get(RulesClass::Instance->BOMBCURS_SHP))
	{
		R->ECX(pImage);
		return 0x6F5247;
	}

	return 0;
}

// 6FCBAD, 6
// custom ivan bomb disarm 1
//ASMJIT_PATCH(0x6FCB8D , TechnoClass_GetObjectActivityState_IvanBomb, 6)
//{
//	enum { ContinueCheck = 0x6FCBCD , ReturnFireIllegal = 0x6FCBBE };
//
//	GET(TechnoClass* const, pTarget, EBP);
//	GET(WarheadTypeClass* const, pWarhead, EDI);
//
//	if(pWarhead->BombDisarm) {
//		if (!pTarget->AttachedBomb)
//			return ReturnFireIllegal;
//
//		if(!BombExt::ExtMap.Find(pTarget->AttachedBomb)->Weapon->Ivan_Detachable) {
//			return ReturnFireIllegal;
//		}
//	}
//	if(pWarhead->IvanBomb && pTarget->AttachedBomb) {
//		return ReturnFireIllegal;
//	}
//
//	return ContinueCheck;
//}

// 51E488, 5
ASMJIT_PATCH(0x51E488, InfantryClass_GetCursorOverObject2, 5)
{
	GET(TechnoClass* const, Target, ESI);
	return !BombExtContainer::Instance.Find(Target->AttachedBomb)
			->Weapon->Ivan_Detachable
		? 0x51E49E : 0x0;
}

// ASMJIT_PATCH(0x438761, BombClass_Detonate_Handle, 0x7)
// {
// 	GET(BombClass*, pThis, ESI);
// 	GET(ObjectClass*, pTarget, ECX);

// 	pTarget->BombVisible = false;
// 	pThis->State = BombState::Removed;
// 	// Also adjust detonation coordinate.
// 	const auto pExt = BombExtContainer::Instance.Find(pThis);

// 	CoordStruct coords = pExt->Weapon->Ivan_AttachToCenter.Get(RulesExtData::Instance()->IvanBombAttachToCenter) ?
// 		pTarget->GetCenterCoords() : pTarget->Location;

// 	const auto pBombWH = pExt->Weapon->Ivan_WH.Get(RulesClass::Instance->IvanWarhead);
// 	const auto nDamage = pExt->Weapon->Ivan_Damage.Get(RulesClass::Instance->IvanDamage);
// 	const auto OwningHouse = pThis->GetOwningHouse();

// 	/*WarheadTypeExtData::DetonateAt(pBombWH, pTarget, coords, pThis->Owner, nDamage);*/
// 	DamageArea::Apply(&coords, nDamage, pThis->Owner, pBombWH, pBombWH->Tiberium, OwningHouse);
// 	MapClass::Instance->FlashbangWarheadAt(nDamage, pBombWH, coords);
// 	const auto pCell = MapClass::Instance->GetCellAt(coords);

// 	if (auto pAnimType = MapClass::Instance->SelectDamageAnimation(nDamage, pBombWH, pCell->LandType, coords))
// 	{
// 		AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnimType, coords, 0, 1, AnimFlag::AnimFlag_2600, -15, false),
// 			OwningHouse,
// 			pThis->Target ? pThis->Target->GetOwningHouse() : nullptr,
// 			pThis->Owner,
// 			false, false
// 		);
// 	}

// 	return pExt->Weapon->Ivan_KillsBridges ? 0x438857 : 0x438989;
// }

//new
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E3D4C, FakeBombClass::_GetOwningHouse);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E3D38, FakeBombClass::_Detach);

//new
//ASMJIT_PATCH(0x6F51F8, TechnoClass_DrawExtras_IvanBombImage_Pos, 0x9)
//{
//	GET(TechnoClass* const , pThis, EBP);
//	GET(CoordStruct*, pCoordBuffA, ECX);
//	R->EAX(pThis->GetCenterCoords(pCoordBuffA));
//	return 0x6F5201;
//}
DEFINE_PATCH(0x6F51FD, 0x58);

ASMJIT_PATCH(0x46934D, IvanBombs_Spread, 6)
{
	GET(BulletClass* const, pBullet, ESI);

	if (!pBullet->Owner)
		return 0x469AA4;

	if (const auto pWeapon = pBullet->WeaponType)
	{
		// single target or spread switch
		if (pBullet->WH->CellSpread < 0.5f) {
			if(!pBullet->Target || !(pBullet->Target->AbstractFlags & AbstractFlags::Object))
				return 0x469AA4;

			// single target
			TechnoExt_ExtData::PlantBomb(pBullet->Owner, (ObjectClass*)pBullet->Target, pWeapon);
		} else {
			// cell spread
			CoordStruct tgtCoords = pBullet->GetTargetCoords();
			auto pWHExt = WarheadTypeExtContainer::Instance.Find(pBullet->WH);

			Helpers::Alex::ApplyFuncToCellSpreadItems(tgtCoords, pBullet->WH->CellSpread,
				true, pWHExt->CellSpread_Cylinder , false , pWHExt->AffectsInAir, pWHExt->AffectsGround , false, [=](TechnoClass* pTarget) {
				TechnoExt_ExtData::PlantBomb(pBullet->Owner, pTarget, pWeapon);
			});
		}
	} else {
		Debug::LogInfo("IvanBomb bullet without attached WeaponType.");
	}

	return 0x469AA4;
}

// deglobalized manual detonation settings
ASMJIT_PATCH(0x6FFFB1, TechnoClass_GetCursorOverObject_IvanBombs, 8)
{
	GET(TechnoClass* const, pThis, EDI);

	const auto pExt = BombExtContainer::Instance.Find(pThis->AttachedBomb);

	const bool canDetonate = (pThis->AttachedBomb->Type == BombType::NormalBomb)
		? pExt->Weapon->Ivan_CanDetonateTimeBomb.Get(RulesClass::Instance->CanDetonateTimeBomb)
		: pExt->Weapon->Ivan_CanDetonateDeathBomb.Get(RulesClass::Instance->CanDetonateDeathBomb);

	return canDetonate ? 0x6FFFCC : 0x700006;
}

ASMJIT_PATCH(0x447218, BuildingClass_GetActionOnObject_Deactivated, 6)
{
	GET(BuildingClass* const, pThis, ESI);
	GET_STACK(ObjectClass*, pThat, 0x1C);

	if (pThis->Deactivated) {
		R->EAX(TechnoExt_ExtData::GetAction(pThis, pThat));
		return 0x447273;
	}

	return 0;
}

ASMJIT_PATCH(0x73FD5A, UnitClass_GetActionOnObject_Deactivated, 5)
{
	GET(UnitClass* const, pThis, ECX);
	GET_STACK(ObjectClass*, pThat, 0x20);

	if (pThis->Deactivated) {
		R->EAX(TechnoExt_ExtData::GetAction(pThis, pThat));
		return 0x73FD72;
	}

	return 0;
}

ASMJIT_PATCH(0x51E440, InfantryClass_GetActionOnObject_Deactivated, 8)
{
	GET(InfantryClass* const, pThis, EDI);
	GET_STACK(ObjectClass*, pThat, 0x3C);

	if (pThis->Deactivated) {
		R->EAX(TechnoExt_ExtData::GetAction(pThis, pThat));
		return 0x51E458;
	}

	return 0;
}

ASMJIT_PATCH(0x417CCB, AircraftClass_GetActionOnObject_Deactivated, 5)
{
	GET(AircraftClass* const, pThis, ECX);
	GET_STACK(ObjectClass*, pThat, 0x20);

	if (pThis->Deactivated) {
		R->EAX(TechnoExt_ExtData::GetAction(pThis, pThat));
		return 0x417CDF;
	}

	return 0;
}

ASMJIT_PATCH(0x6FFEC0, TechnoClass_GetActionOnObject_Additionals, 5)
{
	GET(TechnoClass* , pThis, ECX);
	GET_STACK(ObjectClass*, pObject, 0x4);
	//GET_STACK(DWORD , caller , 0x0);

	if (TechnoExt_ExtData::CanDetonate(pThis, pObject)) {
		R->EAX(Action::Detonate);
		return 0x7005EF;
	}

	if (!pThis->IsAlive) {
		R->EAX(Action::None);
		return 0x7005EF;
	}

	const auto pType = pThis->GetTechnoType();

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	// Cursor Move
	MouseCursorFuncs::SetMouseCursorAction(pTypeExt->Cursor_Move.Get(), Action::Move, false);

	// Cursor NoMove
	MouseCursorFuncs::SetMouseCursorAction(pTypeExt->Cursor_NoMove.Get(), Action::NoMove, false);

	if(!pObject)
		return 0x0;

	if (const auto pTargetType = pObject->GetTechnoType())
	{
		auto pTargetTypeExt = TechnoTypeExtContainer::Instance.Find(pTargetType);
		// Cursor Enter
		MouseCursorFuncs::SetMouseCursorAction(pTargetTypeExt->Cursor_Enter.Get(), Action::Repair, false);
		MouseCursorFuncs::SetMouseCursorAction(pTargetTypeExt->Cursor_Enter.Get(), Action::Enter, false);
		//

		// Cursor NoEnter
		MouseCursorFuncs::SetMouseCursorAction(pTargetTypeExt->Cursor_NoEnter.Get(), Action::NoEnter, false);
	}

	return 0x0;
}

ASMJIT_PATCH(0x44A1FF, BuildingClass_Mi_Selling_DetonatePostBuildup, 6)
{
	GET(BuildingClass* const, pStructure, EBP);

	if (const auto pBomb = pStructure->AttachedBomb) {
		if (BombExtContainer::Instance.Find(pBomb)->Weapon->Ivan_DetonateOnSell.Get())
			pBomb->Detonate();// Otamaa : detonate may kill the techno before this function
			// so this can possibly causing some weird crashes if that happening
	}

	return 0;
}


ASMJIT_PATCH(0x4D9F7B, FootClass_Sell_Detonate, 6)
{
	GET(FootClass* const, pThis, ESI);

	const auto& loc = pThis->Location;
	int money = pThis->GetRefund();
	pThis->Owner->GiveMoney(money);

	if (const auto pBomb = pThis->AttachedBomb) {
		if (BombExtContainer::Instance.Find(pBomb)->Weapon->Ivan_DetonateOnSell.Get())
			pBomb->Detonate(); // Otamaa : detonate may kill the techno before this function
			// so this can possibly causing some weird crashes if that happening
	}

	if (pThis->Owner->ControlledByCurrentPlayer())
	{
		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

		VoxClass::PlayIndex(pTypeExt->EVA_Sold);
		//WW used VocClass::PlayGlobal to play the SellSound, why did they do that?
		VocClass::SafeImmedietelyPlayAt(pTypeExt->SellSound, &loc);
	}

	FlyingStrings::Instance.AddMoneyString(RulesExtData::Instance()->DisplayIncome  , money, pThis->Owner, RulesExtData::Instance()->DisplayIncome_Houses, loc, Point2D::Empty, ColorStruct::Empty);

	//this thing may already death , just
	return pThis->IsAlive  ? 0x4D9FCB : 0x4D9FE9;
}

// custom ivan bomb attachment
// bugfix #385: Only InfantryTypes can use Ivan Bombs
ASMJIT_PATCH(0x438E86, BombListClass_Plant_AllTechnos, 5)
{
	GET(TechnoClass*, Source, EBP);
	switch (Source->WhatAmI())
	{
	case AbstractType::Aircraft:
	case AbstractType::Infantry:
	case AbstractType::Unit:
	case AbstractType::Building:
		return 0x438E97;
	default:
		return 0x439022;
	}
}