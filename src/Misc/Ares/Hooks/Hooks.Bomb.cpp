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

#include <HoverLocomotionClass.h>

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

namespace Funcs
{
	void PlantBomb(TechnoClass* pSource, ObjectClass* pTarget , WeaponTypeClass* pWeapon)
	{
		// ensure target isn't rigged already
		if (pTarget && !pTarget->AttachedBomb)
		{
			BombListClass::Instance->Plant(pSource, pTarget);

			// if target has a bomb, planting was successful
			if (auto pBomb = pTarget->AttachedBomb)
			{
				const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
				BombExt::ExtMap.Find(pBomb)->Weapon = pWeaponExt;
				pBomb->DetonationFrame = Unsorted::CurrentFrame + pWeaponExt->Ivan_Delay.Get(RulesClass::Instance->IvanTimedDelay);
				pBomb->TickSound = pWeaponExt->Ivan_TickingSound.Get(RulesClass::Instance->BombTickingSound);

				int index = pWeaponExt->Ivan_AttachSound.Get(RulesClass::Instance->BombAttachSound);
				if (index != -1 && pSource->Owner->ControlledByPlayer())
				{
					VocClass::PlayAt(index, pBomb->Target->Location, nullptr);
				}
			}
		}
	}

	bool CanDetonate(TechnoClass* pThis, ObjectClass* pThat)
	{
		if (pThis == pThat && ObjectClass::CurrentObjects->Count == 1)
		{

			if (auto const pBomb = pThis->AttachedBomb)
			{
				if (pBomb->OwnerHouse->ControlledByPlayer())
				{
					const auto pData = BombExt::ExtMap.Find(pBomb);
					const bool bCanDetonateDeathBomb =
						pData->Weapon->Ivan_CanDetonateDeathBomb.Get(RulesClass::Instance->CanDetonateDeathBomb);
					const bool bCanDetonateTimeBomb =
						pData->Weapon->Ivan_CanDetonateTimeBomb.Get(RulesClass::Instance->CanDetonateTimeBomb);

					if (pBomb->Type == BombType::DeathBomb ?
						bCanDetonateDeathBomb : bCanDetonateTimeBomb)
						return true;
				}
			}
		}

		return false;
	}

	Action GetAction(TechnoClass* pThis, ObjectClass* pThat)
	{
		if (CanDetonate(pThis, pThat))
			return Action::Detonate;

		if (pThis == pThat && ObjectClass::CurrentObjects->Count == 1) {

			if (pThat->AbstractFlags & AbstractFlags::Techno) {
				if (pThis->Owner->IsAlliedWith(pThat) && pThat->IsSelectable()) {
						return Action::Select;
				}
			}
		}

		return Action::None;
	}
};

DEFINE_OVERRIDE_HOOK(0x438FD7, BombListClass_Plant_AttachSound, 7)
{
	return 0x439022;
}

DEFINE_OVERRIDE_HOOK(0x438A00, BombClass_GetCurrentFrame, 6)
{
	GET(BombClass*, pThis, ECX);

	const auto pData = BombExt::ExtMap.Find(pThis)->Weapon;
	const SHPStruct* pSHP = pData->Ivan_Image.Get(RulesClass::Instance->BOMBCURS_SHP);
	int frame = 0;

	if (pSHP->Frames >= 2)
	{
		if (pThis->Type == BombType::NormalBomb)
		{
			// -1 so that last iteration has room to flicker. order is important
			int delay = pData->Ivan_Delay.Get(RulesClass::Instance->IvanTimedDelay);
			int lifetime = (Unsorted::CurrentFrame - pThis->PlantingFrame);
			frame = lifetime / (delay / (pSHP->Frames - 1));

			// flicker over a time period
			int rate = pData->Ivan_FlickerRate.Get(RulesClass::Instance->IvanIconFlickerRate);
			int period = 2 * rate;
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
DEFINE_OVERRIDE_HOOK(0x6F523C, TechnoClass_DrawExtras_IvanBombImage_Shape, 5)
{
	GET(TechnoClass*, pThis, EBP);

	if (SHPStruct* pImage = BombExt::ExtMap.Find(pThis->AttachedBomb)
		->Weapon->Ivan_Image.Get(RulesClass::Instance->BOMBCURS_SHP))
	{
		R->ECX(pImage);
		return 0x6F5247;
	}

	return 0;
}

// 6FCBAD, 6
// custom ivan bomb disarm 1
DEFINE_OVERRIDE_HOOK(0x6FCBAD, TechnoClass_GetObjectActivityState_IvanBomb, 6)
{
	GET(TechnoClass*, Target, EBP);
	GET(WarheadTypeClass*, Warhead, EDI);

	if (Warhead->BombDisarm) {
		if (BombClass* Bomb = Target->AttachedBomb) {
			if (!BombExt::ExtMap.Find(Bomb)->Weapon->Ivan_Detachable) {
				return 0x6FCBBE;
			}
		}
	}

	return 0;
}

// 51E488, 5
DEFINE_OVERRIDE_HOOK(0x51E488, InfantryClass_GetCursorOverObject2, 5)
{
	GET(TechnoClass*, Target, ESI);
	return !BombExt::ExtMap.Find(Target->AttachedBomb)->Weapon->Ivan_Detachable 
		? 0x51E49E : 0x0;
}

DEFINE_HOOK(0x438761, BombClass_Detonate_fetch, 0x7)
{
	GET(BombClass*, pThis, ESI);
	GET(ObjectClass*, pTarget, ECX);

	pTarget->BombVisible = false;
	pThis->State = BombState::Removed;
	// Also adjust detonation coordinate.
	const CoordStruct coords = pTarget->GetCenterCoords();
	const auto pExt = BombExt::ExtMap.Find(pThis);
	const auto pBombWH = pExt->Weapon->Ivan_WH.Get(RulesClass::Instance->IvanWarhead);
	const auto nDamage = pExt->Weapon->Ivan_Damage.Get(RulesClass::Instance->IvanDamage);
	const auto OwningHouse = pThis->GetOwningHouse();

	MapClass::Instance->DamageArea(coords, nDamage, pThis->Owner, pBombWH, pBombWH->Tiberium, OwningHouse);
	MapClass::Instance->FlashbangWarheadAt(nDamage, pBombWH, coords);
	const auto pCell = MapClass::Instance->GetCellAt(coords);

	if (auto pAnimType = MapClass::Instance->SelectDamageAnimation(nDamage, pBombWH, pCell->LandType, coords)) {
		if (auto pAnim = GameCreate<AnimClass>(pAnimType, coords, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200 | AnimFlag::AnimFlag_2000, -15, false)) {
			AnimExt::SetAnimOwnerHouseKind(pAnim, OwningHouse, pThis->Target ? pThis->Target->GetOwningHouse() : nullptr, pThis->Owner, false);
		}
	}

	return 0x438857;
}

//mew
DEFINE_JUMP(VTABLE, 0x7E3D4C, GET_OFFSET(BombExt::GetOwningHouse));

//mew
DEFINE_HOOK(0x6F51F8, TechnoClass_DrawExtras_IvanBombImage_Pos, 0x9)
{
	GET(TechnoClass*, pThis, EBP);
	GET(CoordStruct*, pCoordBuffA, ECX);
	R->EAX(pThis->GetCenterCoords(pCoordBuffA));
	return 0x6F5201;
}

// 438879, 6
// custom ivan bomb detonation 3
DEFINE_OVERRIDE_HOOK(0x438879, BombClass_Detonate_CanKillBridge, 6)
{
	GET(BombClass*, Bomb, ESI);
	return  BombExt::ExtMap.Find(Bomb)->Weapon->Ivan_KillsBridges 
		? 0 : 0x438989;
}

DEFINE_OVERRIDE_HOOK(0x46934D, IvanBombs_Spread, 6)
{
	GET(BulletClass*, pBullet, ESI);

	if (TechnoClass* pOwner = generic_cast<TechnoClass*>(pBullet->Owner)) {
		if (const auto pWeapon = pBullet->WeaponType) {
			// single target or spread switch
			if (pBullet->WH->CellSpread < 0.5f) {
				// single target
				if (const auto pTarget = generic_cast<TechnoClass*>(pBullet->Target)) {
					Funcs::PlantBomb(pOwner, pTarget, pWeapon);
				}
			} else {

				// cell spread
				CoordStruct tgtCoords = pBullet->GetTargetCoords();
				CellStruct centerCoords = CellClass::Coord2Cell(tgtCoords);

				CellSpreadIterator<TechnoClass>{}(centerCoords, 
					static_cast<int>(pBullet->WH->CellSpread),
					[pOwner, pWeapon](TechnoClass* pTechno) {
						if (pTechno != pOwner && !pTechno->AttachedBomb) {
							Funcs::PlantBomb(pOwner, pTechno, pWeapon);
						}
						return true;
					});
			}
		}
		else
		{
			Debug::Log("IvanBomb bullet without attached WeaponType.\n");
		}
	}

	return 0x469AA4;
}

// deglobalized manual detonation settings
DEFINE_OVERRIDE_HOOK(0x6FFFB1, TechnoClass_GetCursorOverObject_IvanBombs, 8)
{
	GET(TechnoClass*, pThis, EDI);
	const auto pExt = BombExt::ExtMap.Find(pThis->AttachedBomb);

	const bool canDetonate = (pExt->Get()->Type == BombType::NormalBomb)
		? pExt->Weapon->Ivan_CanDetonateTimeBomb.Get(RulesClass::Instance->CanDetonateTimeBomb)
		: pExt->Weapon->Ivan_CanDetonateDeathBomb.Get(RulesClass::Instance->CanDetonateDeathBomb);

	return canDetonate ? 0x6FFFCC : 0x700006;
}

DEFINE_OVERRIDE_HOOK(0x447218, BuildingClass_GetActionOnObject_Deactivated, 6)
{
	GET(BuildingClass*, pThis, ESI);
	GET_STACK(ObjectClass*, pThat, 0x1C);
	if (pThis->Deactivated)
	{
		R->EAX(Funcs::GetAction(pThis, pThat));
		return 0x447273;
	}
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x73FD5A, UnitClass_GetActionOnObject_Deactivated, 5)
{
	GET(UnitClass*, pThis, ECX);
	GET_STACK(ObjectClass*, pThat, 0x20);
	if (pThis->Deactivated)
	{
		R->EAX(Funcs::GetAction(pThis, pThat));
		return 0x73FD72;
	}
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x51E440, InfantryClass_GetActionOnObject_Deactivated, 8)
{
	GET(InfantryClass*, pThis, EDI);
	GET_STACK(ObjectClass*, pThat, 0x3C);
	if (pThis->Deactivated)
	{
		R->EAX(Funcs::GetAction(pThis, pThat));
		return 0x51E458;
	}
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x417CCB, AircraftClass_GetActionOnObject_Deactivated, 5)
{
	GET(AircraftClass*, pThis, ECX);
	GET_STACK(ObjectClass*, pThat, 0x20);
	if (pThis->Deactivated)
	{
		R->EAX(Funcs::GetAction(pThis, pThat));
		return 0x417CDF;
	}
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x6FFEC0, TechnoClass_GetActionOnObject_IvanBombsA, 5)
{
	GET(TechnoClass*, pThis, ECX);
	GET_STACK(ObjectClass*, pObject, 0x4);

	if (Funcs::CanDetonate(pThis, pObject))
	{
		R->EAX(Action::Detonate);
		return 0x7005EF;
	}

	return 0x0;
}

DEFINE_OVERRIDE_HOOK(0x4471D5, BuildingClass_Sell_DetonateNoBuildup, 6)
{
	GET(BuildingClass*, pStructure, ESI);

	if (auto pBomb = pStructure->AttachedBomb) {
		if (BombExt::ExtMap.Find(pBomb)->Weapon->Ivan_DetonateOnSell.Get())
			pBomb->Detonate();
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x44A1FF, BuildingClass_Mi_Selling_DetonatePostBuildup, 6)
{
	GET(BuildingClass*, pStructure, EBP);

	if (auto pBomb = pStructure->AttachedBomb) {
		if (BombExt::ExtMap.Find(pBomb)->Weapon->Ivan_DetonateOnSell.Get())
			pBomb->Detonate();
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x4D9F7B, FootClass_Sell_Detonate, 6)
{
	GET(FootClass*, pSellee, ESI);

	if (auto pBomb = pSellee->AttachedBomb) {
		if (BombExt::ExtMap.Find(pBomb)->Weapon->Ivan_DetonateOnSell.Get())
			pBomb->Detonate();
	}

	return 0;
}