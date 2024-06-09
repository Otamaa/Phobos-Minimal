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

//BombListClass_Plant_AttachSound
DEFINE_JUMP(LJMP, 0x438FD7, 0x439022);

// #896027: do not announce pointers as expired to bombs
// if the pointed to object is staying in-game.
DEFINE_HOOK(0x725961, AnnounceInvalidPointer_BombCloak, 0x6)
{
	GET(bool, remove, EDI);
	return remove ? 0 : 0x72596C;
}

DEFINE_HOOK(0x438A00, BombClass_GetCurrentFrame, 6)
{
	GET(BombClass*, pThis, ECX);

	const auto pData = BombExtContainer::Instance.Find(pThis)->Weapon;
	const SHPStruct* pSHP = pData->Ivan_Image.Get(RulesClass::Instance->BOMBCURS_SHP);
	int frame = 0;

	if (pSHP->Frames >= 2)
	{
		if (pThis->Type == BombType::NormalBomb)
		{
			// -1 so that last iteration has room to flicker. order is important
			const int delay = pData->Ivan_Delay.Get(RulesClass::Instance->IvanTimedDelay);
			const int lifetime = (Unsorted::CurrentFrame - pThis->PlantingFrame);
			frame = lifetime / (delay / (pSHP->Frames - 1));

			// flicker over a time period
			const int rate = pData->Ivan_FlickerRate.Get(RulesClass::Instance->IvanIconFlickerRate);
			const int period = 2 * rate;
			if (Unsorted::CurrentFrame % period >= rate)
			{
				++frame;
			}

			if (frame >= pSHP->Frames)
			{
				frame = pSHP->Frames - 1;
			}
			else if (frame == pSHP->Frames - 1)
			{
				--frame;
			}
		}
		else
		{
			// DeathBombs (that don't exist) use the last frame
			frame = pSHP->Frames - 1;
		}
	}

	R->EAX(frame);
	return 0x438A62;
}

// 6F523C, 5
// custom ivan bomb drawing
DEFINE_HOOK(0x6F523C, TechnoClass_DrawExtras_IvanBombImage_Shape, 5)
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
//DEFINE_HOOK(0x6FCB8D , TechnoClass_GetObjectActivityState_IvanBomb, 6)
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
DEFINE_HOOK(0x51E488, InfantryClass_GetCursorOverObject2, 5)
{
	GET(TechnoClass* const, Target, ESI);
	return !BombExtContainer::Instance.Find(Target->AttachedBomb)
			->Weapon->Ivan_Detachable
		? 0x51E49E : 0x0;
}

DEFINE_HOOK(0x438761, BombClass_Detonate_Handle, 0x7)
{
	GET(BombClass*, pThis, ESI);
	GET(ObjectClass*, pTarget, ECX);

	pTarget->BombVisible = false;
	pThis->State = BombState::Removed;
	// Also adjust detonation coordinate.
	const CoordStruct coords = pTarget->GetCenterCoords();
	const auto pExt = BombExtContainer::Instance.Find(pThis);
	const auto pBombWH = pExt->Weapon->Ivan_WH.Get(RulesClass::Instance->IvanWarhead);
	const auto nDamage = pExt->Weapon->Ivan_Damage.Get(RulesClass::Instance->IvanDamage);
	const auto OwningHouse = pThis->GetOwningHouse();

	/*WarheadTypeExtData::DetonateAt(pBombWH, pTarget, coords, pThis->Owner, nDamage);*/
	MapClass::Instance->DamageArea(coords, nDamage, pThis->Owner, pBombWH, pBombWH->Tiberium, OwningHouse);
	MapClass::Instance->FlashbangWarheadAt(nDamage, pBombWH, coords);
	const auto pCell = MapClass::Instance->GetCellAt(coords);

	if (auto pAnimType = MapClass::Instance->SelectDamageAnimation(nDamage, pBombWH, pCell->LandType, coords))
	{
		AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnimType, coords, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200 | AnimFlag::AnimFlag_2000, -15, false),
			OwningHouse,
			pThis->Target ? pThis->Target->GetOwningHouse() : nullptr,
			pThis->Owner,
			false
		);
	}

	return 0x438857;
}

//new
DEFINE_JUMP(VTABLE, 0x7E3D4C, GET_OFFSET(BombExtData::GetOwningHouse));
DEFINE_JUMP(VTABLE, 0x7E3D38 , GET_OFFSET(BombExtData::InvalidatePointer));

//new
//DEFINE_HOOK(0x6F51F8, TechnoClass_DrawExtras_IvanBombImage_Pos, 0x9)
//{
//	GET(TechnoClass* const , pThis, EBP);
//	GET(CoordStruct*, pCoordBuffA, ECX);
//	R->EAX(pThis->GetCenterCoords(pCoordBuffA));
//	return 0x6F5201;
//}
DEFINE_PATCH(0x6F51FD, 0x58);

// 438879, 6
// custom ivan bomb detonation 3
DEFINE_HOOK(0x438879, BombClass_Detonate_CanKillBridge, 6)
{
	GET(BombClass* const, Bomb, ESI);
	return BombExtContainer::Instance.Find(Bomb)->Weapon->Ivan_KillsBridges
		? 0 : 0x438989;
}

DEFINE_HOOK(0x46934D, IvanBombs_Spread, 6)
{
	GET(BulletClass* const, pBullet, ESI);

	if (!pBullet->Owner)
		return 0x469AA4;

	if (const auto pWeapon = pBullet->WeaponType)
	{
		// single target or spread switch
		if (pBullet->WH->CellSpread < 0.5f)
		{
			if(!pBullet->Target || !(pBullet->Target->AbstractFlags & AbstractFlags::Object))
				return 0x469AA4;

			// single target
			TechnoExt_ExtData::PlantBomb(pBullet->Owner, (ObjectClass*)pBullet->Target, pWeapon);
		}
		else
		{
			// cell spread
			CoordStruct tgtCoords = pBullet->GetTargetCoords();
			Helpers::Alex::ApplyFuncToCellSpreadItems(tgtCoords, pBullet->WH->CellSpread, [=](TechnoClass* pTarget) {
				TechnoExt_ExtData::PlantBomb(pBullet->Owner, pTarget, pWeapon);
			}, false, false);
		}
	}
	else
	{
		Debug::Log("IvanBomb bullet without attached WeaponType.\n");
	}

	return 0x469AA4;
}

// deglobalized manual detonation settings
DEFINE_HOOK(0x6FFFB1, TechnoClass_GetCursorOverObject_IvanBombs, 8)
{
	GET(TechnoClass* const, pThis, EDI);

	const auto pExt = BombExtContainer::Instance.Find(pThis->AttachedBomb);

	const bool canDetonate = (pThis->AttachedBomb->Type == BombType::NormalBomb)
		? pExt->Weapon->Ivan_CanDetonateTimeBomb.Get(RulesClass::Instance->CanDetonateTimeBomb)
		: pExt->Weapon->Ivan_CanDetonateDeathBomb.Get(RulesClass::Instance->CanDetonateDeathBomb);

	return canDetonate ? 0x6FFFCC : 0x700006;
}

DEFINE_HOOK(0x447218, BuildingClass_GetActionOnObject_Deactivated, 6)
{
	GET(BuildingClass* const, pThis, ESI);
	GET_STACK(ObjectClass*, pThat, 0x1C);

	if (pThis->Deactivated) {
		R->EAX(TechnoExt_ExtData::GetAction(pThis, pThat));
		return 0x447273;
	}

	return 0;
}

DEFINE_HOOK(0x73FD5A, UnitClass_GetActionOnObject_Deactivated, 5)
{
	GET(UnitClass* const, pThis, ECX);
	GET_STACK(ObjectClass*, pThat, 0x20);

	if (pThis->Deactivated) {
		R->EAX(TechnoExt_ExtData::GetAction(pThis, pThat));
		return 0x73FD72;
	}

	return 0;
}

DEFINE_HOOK(0x51E440, InfantryClass_GetActionOnObject_Deactivated, 8)
{
	GET(InfantryClass* const, pThis, EDI);
	GET_STACK(ObjectClass*, pThat, 0x3C);

	if (pThis->Deactivated) {
		R->EAX(TechnoExt_ExtData::GetAction(pThis, pThat));
		return 0x51E458;
	}

	return 0;
}

DEFINE_HOOK(0x417CCB, AircraftClass_GetActionOnObject_Deactivated, 5)
{
	GET(AircraftClass* const, pThis, ECX);
	GET_STACK(ObjectClass*, pThat, 0x20);

	if (pThis->Deactivated) {
		R->EAX(TechnoExt_ExtData::GetAction(pThis, pThat));
		return 0x417CDF;
	}

	return 0;
}

DEFINE_HOOK(0x6FFEC0, TechnoClass_GetActionOnObject_IvanBombsA, 5)
{
	GET(TechnoClass* const, pThis, ECX);
	GET_STACK(ObjectClass*, pObject, 0x4);

	if (TechnoExt_ExtData::CanDetonate(pThis, pObject)) {
		R->EAX(Action::Detonate);
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

DEFINE_HOOK(0x44A1FF, BuildingClass_Mi_Selling_DetonatePostBuildup, 6)
{
	GET(BuildingClass* const, pStructure, EBP);

	if (const auto pBomb = pStructure->AttachedBomb) {
		if (BombExtContainer::Instance.Find(pBomb)->Weapon->Ivan_DetonateOnSell.Get())
			pBomb->Detonate();// Otamaa : detonate may kill the techno before this function
			// so this can possibly causing some weird crashes if that happening
	}

	return 0;
}


DEFINE_HOOK(0x4D9F7B, FootClass_Sell_Detonate, 6)
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
		VocClass::PlayAt(pTypeExt->SellSound, loc);
	}

	FlyingStrings::AddMoneyString(RulesExtData::Instance()->DisplayIncome  , money, pThis->Owner, RulesExtData::Instance()->DisplayIncome_Houses, loc);

	//this thing may already death , just
	return pThis->IsAlive  ? 0x4D9FCB : 0x4D9FE9;
}

// custom ivan bomb attachment
// bugfix #385: Only InfantryTypes can use Ivan Bombs
DEFINE_HOOK(0x438E86, BombListClass_Plant_AllTechnos, 5)
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