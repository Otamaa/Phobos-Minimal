#include <Ext/Wave/Body.h>

#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <HouseClass.h>
#include <VocClass.h>
#include <VoxClass.h>

#include <Utilities/Debug.h>
#include <Utilities/Helpers.h>
#include <Utilities/Macro.h>

#include <Helpers/Macro.h>

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Aircraft/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/Bomb/Body.h>
#include <Ext/Mouse/Body.h>

#include <New/Type/ArmorTypeClass.h>

#include <Notifications.h>
#include <algorithm>

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

// 6F523C, 5
// custom ivan bomb drawing
ASMJIT_PATCH(0x6F523C, TechnoClass_DrawExtras_IvanBombImage_Shape, 5)
{
	GET(TechnoClass*, pThis, EBP);

	if (SHPCaches* pImage = BombExtContainer::Instance.Find(pThis->AttachedBomb)
		->Weapon->Ivan_Image.Get(RulesClass::Instance->BOMBCURS_SHP))
	{
		R->ECX(pImage);
		return 0x6F5247;
	}

	return 0;
}

// 51E488, 5
ASMJIT_PATCH(0x51E488, InfantryClass_GetCursorOverObject2, 5)
{
	GET(TechnoClass* const, Target, ESI);
	return !BombExtContainer::Instance.Find(Target->AttachedBomb)
		->Weapon->Ivan_Detachable
		? 0x51E49E : 0x0;
}

//new
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E3D4C, FakeBombClass::_GetOwningHouse);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E3D38, FakeBombClass::_Detach);

//TechnoClass_Maybe_Draw_It_TechnoClass_DrawExtras_CheckFog
DEFINE_PATCH(0x6F51FD, 0x58);

ASMJIT_PATCH(0x46934D, IvanBombs_Spread, 6)
{
	GET(BulletClass* const, pBullet, ESI);

	if (!pBullet->Owner)
		return 0x469AA4;

	if (const auto pWeapon = pBullet->WeaponType)
	{
		// single target or spread switch
		if (pBullet->WH->CellSpread < 0.5f)
		{
			if (!pBullet->Target || !(pBullet->Target->AbstractFlags & AbstractFlags::Object))
				return 0x469AA4;

			// single target
			TechnoExtData::PlantBomb(pBullet->Owner, (ObjectClass*)pBullet->Target, pWeapon);
		}
		else
		{
			// cell spread
			CoordStruct tgtCoords = pBullet->GetTargetCoords();
			auto pWHExt = WarheadTypeExtContainer::Instance.Find(pBullet->WH);

			Helpers::Alex::ApplyFuncToCellSpreadItems(tgtCoords, pBullet->WH->CellSpread,
				true, pWHExt->CellSpread_Cylinder, false, pWHExt->AffectsInAir, pWHExt->AffectsGround, false, [=](TechnoClass* pTarget)
 {
	 TechnoExtData::PlantBomb(pBullet->Owner, pTarget, pWeapon);
			});
		}
	}
	else
	{
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

	if (pThis->Deactivated)
	{
		R->EAX(TechnoExtData::GetAction(pThis, pThat));
		return 0x447273;
	}

	return 0;
}

ASMJIT_PATCH(0x73FD5A, UnitClass_GetActionOnObject_Deactivated, 5)
{
	GET(UnitClass* const, pThis, ECX);
	GET_STACK(ObjectClass*, pThat, 0x20);

	if (pThis->Deactivated)
	{
		R->EAX(TechnoExtData::GetAction(pThis, pThat));
		return 0x73FD72;
	}

	return 0;
}

ASMJIT_PATCH(0x51E440, InfantryClass_GetActionOnObject_Deactivated, 8)
{
	GET(InfantryClass* const, pThis, EDI);
	GET_STACK(ObjectClass*, pThat, 0x3C);

	if (pThis->Deactivated)
	{
		R->EAX(TechnoExtData::GetAction(pThis, pThat));
		return 0x51E458;
	}

	return 0;
}

ASMJIT_PATCH(0x6FFEC0, TechnoClass_GetActionOnObject_Additionals, 5)
{
	GET(TechnoClass*, pThis, ECX);
	GET_STACK(ObjectClass*, pObject, 0x4);
	//GET_STACK(DWORD , caller , 0x0);

	if (TechnoExtData::CanDetonate(pThis, pObject))
	{
		R->EAX(Action::Detonate);
		return 0x7005EF;
	}

	if (!pThis->IsAlive)
	{
		R->EAX(Action::None);
		return 0x7005EF;
	}

	const auto pType = GET_TECHNOTYPE(pThis);

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	// Cursor Move
	MouseCursorFuncs::SetMouseCursorAction(pTypeExt->Cursor_Move.Get(), Action::Move, false);

	// Cursor NoMove
	MouseCursorFuncs::SetMouseCursorAction(pTypeExt->Cursor_NoMove.Get(), Action::NoMove, false);

	if (auto pTech = flag_cast_to<TechnoClass*>(pObject))
	{
		const auto pTargetType = GET_TECHNOTYPE(pTech);

		{
			auto pTargetTypeExt = TechnoTypeExtContainer::Instance.Find(pTargetType);
			// Cursor Enter
			MouseCursorFuncs::SetMouseCursorAction(pTargetTypeExt->Cursor_Enter.Get(), Action::Repair, false);
			MouseCursorFuncs::SetMouseCursorAction(pTargetTypeExt->Cursor_Enter.Get(), Action::Enter, false);
			//

			// Cursor NoEnter
			MouseCursorFuncs::SetMouseCursorAction(pTargetTypeExt->Cursor_NoEnter.Get(), Action::NoEnter, false);
		}
	}

	return 0x0;
}

ASMJIT_PATCH(0x44A1FF, BuildingClass_Mission_Selling_DetonatePostBuildup, 6)
{
	GET(BuildingClass* const, pStructure, EBP);

	if (const auto pBomb = pStructure->AttachedBomb)
	{
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
	const auto pTypeExt = GET_TECHNOTYPEEXT(pThis);
	const auto pUnit = cast_to<UnitClass*, false>(pThis);
	const int money = pUnit && FakeRulesClass::Instance()->UnitsUnsellable ? 0 : pThis->GetRefund();

	//distribute the money
	pThis->Owner->GiveMoney(money);

	if (const auto pBomb = pThis->AttachedBomb)
	{
		if (BombExtContainer::Instance.Find(pBomb)->Weapon->Ivan_DetonateOnSell.Get())
			pBomb->Detonate(); // Otamaa : detonate may kill the techno before this function
		// so this can possibly causing some weird crashes if that happening
	}

	if (pThis->Owner->ControlledByCurrentPlayer())
	{
		VoxClass::PlayIndex(pTypeExt->EVA_Sold);
		//WW used VocClass::PlayGlobal to play the SellSound, why did they do that?
		VocClass::SafeImmedietelyPlayAt(pTypeExt->SellSound, &loc);
	}

	FlyingStrings::Instance.AddMoneyString(FakeRulesClass::Instance()->DisplayIncome, money, pThis->Owner, FakeRulesClass::Instance()->DisplayIncome_Houses, loc, Point2D::Empty, ColorStruct::Empty);

	//this thing may already death , just
	return pThis->IsAlive ? 0x4D9FCB : 0x4D9FE9;
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


#ifdef _Hook

ASMJIT_PATCH(0x438D44, BombListClass_AI_Visibility, 0x5)
{
	enum { SkipGameCode = 0x438E2B };

	GET(BombListClass*, pBombList, EDI);
	GET(FakeBombClass*, pBomb, EBX);
	AffectedHouse visibility = AffectedHouse::Owner;

	if (const auto pWeaponExt = pBomb->_GetExtData()->Weapon)
		visibility = pWeaponExt->IvanBomb_Visibility.Get(FakeRulesClass::Instance->IvanBomb_Visibility);
	else
		visibility = FakeRulesClass::Instance->IvanBomb_Visibility;

	const auto pCurrent = HouseClass::CurrentPlayer();
	bool visible = false;

	if (EnumFunctions::CanTargetHouse(visibility, pBomb->OwnerHouse, pCurrent)
		|| std::ranges::find_if(pBombList->Detectors, [=](TechnoClass* pDetector)
			{
				if (!EnumFunctions::CanTargetHouse(visibility, pDetector->Owner, pCurrent))
					return false;

				const int sight = pDetector->GetTechnoType()->BombSight * Unsorted::LeptonsPerCell;
				return pDetector->GetCoords().DistanceFromSquared(pBomb->Target->GetCoords()) <= static_cast<double>(sight) * sight;
			}) != pBombList->Detectors.end()
				)
	{
		visible = true;
	}

	R->AL(visible);
	return SkipGameCode;
}
#else 

void FakeBombListClass::__AI()
{// the vectors are not allocated yet for static objects , so this weird code checking is used
	if(this->Bombs.IsAllocated) {
		this->Bombs.erase_if([](BombClass* pBomb){ 
			if (!pBomb || !pBomb->Target)
			{
				if(pBomb && !pBomb->Target)
				{ GameDelete<true, false>(pBomb); }
				return true;
			}

			else  if (pBomb->TickSound != -1) {
				ObjectClass* target = pBomb->Target;

				if (target->InLimbo) {
					pBomb->TickAudioController.AudioEventHandleStop();
					pBomb->ShouldPlayTickingSound = 0;
				} else {
					if (pBomb->ShouldPlayTickingSound) {
						VocClass::PlayIfInRange(target->Location, &pBomb->TickAudioController);
					} else {
						VocClass::SafeImmedietelyPlayAt(pBomb->TickSound, target->Location, &pBomb->TickAudioController);
						pBomb->ShouldPlayTickingSound = 1;
					}
				}
			}
			return false;
		});
	}

	if (this->UpdateDelay > 0) {
		--this->UpdateDelay;
		return;
	}

	this->UpdateDelay = 45;

	if (this->Bombs.IsAllocated) {
		const auto pCurrent = HouseClass::CurrentPlayer();
		const bool isObserverLooking = pCurrent == HouseClass::Observer();

		this->Bombs.for_each([this, isObserverLooking, pCurrent](BombClass* pBomb) {
			 bool wasVisible = pBomb->Target->BombVisible;
			 bool nowVisible = false;

			 if (!isObserverLooking) {
				 AffectedHouse visibility = FakeRulesClass::Instance->IvanBomb_Visibility;
				 if (const auto pWeaponExt = ((FakeBombClass*)pBomb)->_GetExtData()->Weapon)
					 visibility = pWeaponExt->IvanBomb_Visibility.Get(visibility);

				 nowVisible = pBomb->Target->IsOnMyView() && (EnumFunctions::CanTargetHouse(visibility, pBomb->OwnerHouse, pCurrent)
					 || (this->Detectors.IsAllocated && this->Detectors.find_if([pBomb, visibility, pCurrent](TechnoClass* pDetector)
						 {
							 const auto pDetectorOwner = pDetector->GetOwningHouse();

							 if (!pDetectorOwner->ControlledByCurrentPlayer())
								 return false;

							 if (!EnumFunctions::CanTargetHouse(visibility, pDetectorOwner, pCurrent))
								 return false;

							 const int sight = pDetector->GetTechnoType()->BombSight * Unsorted::LeptonsPerCell;
							 return pDetector->GetCoords().DistanceFrom(pBomb->Target->GetCoords()) <= static_cast<double>(sight) * sight;
						 }) != this->Detectors.end()));
			 }
			 else nowVisible = pBomb->Target->IsOnMyView();

			 pBomb->Target->BombVisible = nowVisible;
			 if (wasVisible != nowVisible) {
				 pBomb->Target->NeedsRedraw = 1;
			 }
		});
	}
}

void  FakeBombListClass::__PlantB(TechnoClass* pOwner, ObjectClass* pTarget, WeaponTypeClass* pWeapon)
{
	if (!pOwner || !pTarget || pTarget->AttachedBomb)
		return;

	const auto pPlantOwner = pOwner->GetOwningHouse();
	auto pBomb = (FakeBombClass*)GameCreate<BombClass>();
	const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);
	pBomb->_GetExtData()->Weapon = pWeaponExt;
	pBomb->OwnerHouse = pPlantOwner;
	pBomb->Owner = pOwner;
	pBomb->Target = pTarget;
	pTarget->AttachedBomb = pBomb;
	pBomb->PlantingFrame = Unsorted::CurrentFrame();
	pBomb->Type = BombType::NormalBomb;
	pBomb->DetonationFrame = Unsorted::CurrentFrame.get() + pWeaponExt->Ivan_Delay.Get(RulesClass::Instance->IvanTimedDelay);
	this->Bombs.push_back(pBomb);
	this->UpdateDelay = 1;

	pBomb->TickSound = pWeaponExt->Ivan_TickingSound.Get(RulesClass::Instance->BombTickingSound);
	const auto IsAlly = pPlantOwner->IsAlliedWith(pTarget);
	pBomb->Type = BombType((!IsAlly && pWeaponExt->Ivan_DeathBomb) || (IsAlly && pWeaponExt->Ivan_DeathBombOnAllies));

	if (pPlantOwner->ControlledByCurrentPlayer()) {
		VocClass::SafeImmedietelyPlayAt(pWeaponExt->Ivan_AttachSound.Get(RulesClass::Instance->BombAttachSound)
		, &pBomb->Target->Location);
	}
}

void FakeBombListClass::__Plant(TechnoClass* pOwner, ObjectClass* pTarget)
{
	Debug::FatalErrorAndExit("Use Proper Function");
}

DEFINE_FUNCTION_JUMP(LJMP, 0x438E70, FakeBombListClass::__Plant)

DEFINE_FUNCTION_JUMP(LJMP, 0x438BF0, FakeBombListClass::__AI)
#endif