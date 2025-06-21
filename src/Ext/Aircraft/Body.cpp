#include "Body.h"
#include <Ext/WeaponType/Body.h>

#include <Ext/AnimType/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>

#include <AircraftClass.h>
#include <Misc/DynamicPatcher/Techno/AircraftDive/AircraftDiveFunctional.h>
#include <Misc/DynamicPatcher/Techno/AircraftPut/AircraftPutDataFunctional.h>

AbstractClass* FakeAircraftClass::_GreatestThreat(ThreatType threatType, CoordStruct* pSelectCoords, bool onlyTargetHouseEnemy)
{
	if (RulesExtData::Instance()->ExpandAircraftMission && !this->Team && this->Ammo && !this->Airstrike && !this->Spawned)
	{
		if (WeaponTypeClass* const pPrimaryWeapon = this->GetWeapon(0)->WeaponType)
			threatType |= pPrimaryWeapon->AllowedThreats();

		if (WeaponTypeClass* const pSecondaryWeapon = this->GetWeapon(1)->WeaponType)
			threatType |= pSecondaryWeapon->AllowedThreats();
	}

	return this->FootClass::GreatestThreat(threatType, pSelectCoords, onlyTargetHouseEnemy); // FootClass_GreatestThreat (Prevent circular calls)
}

void FakeAircraftClass::_FootClass_Update_Wrapper()
{
	auto pExt = TechnoExtContainer::Instance.Find(this);

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(this->Type);


	pExt->UpdateAircraftOpentopped();
	AircraftPutDataFunctional::AI(pExt, pTypeExt);
	AircraftDiveFunctional::AI(pExt, pTypeExt);
	//FighterAreaGuardFunctional::AI(pExt, pTypeExt);

	//if (pThis->IsAlive && pThis->SpawnOwner != nullptr)
	//{
	//
	//	/**
	//	 *  If we are close enough to our owner, delete us and return true
	//	 *  to signal to the challer that we were deleted.
	//	 */
	//	if (Spawned_Check_Destruction(pThis))
	//	{
	//		pThis->UnInit();
	//		return 0x414F99;
	//	}
	//}

	this->FootClass::Update();
}

void FakeAircraftClass::_SetTarget(AbstractClass* pTarget)
{
	this->TechnoClass::SetTarget(pTarget);
	TechnoExtContainer::Instance.Find(this)->CurrentAircraftWeaponIndex = -1;
}

void FakeAircraftClass::_Destroyed(int mult)
{
	AircraftExt::TriggerCrashWeapon(this, mult);
}

WeaponStruct* FakeAircraftClass::_GetWeapon(int weaponIndex)
{
	auto const pExt = TechnoExtContainer::Instance.Find(this);

	if (pExt->CurrentAircraftWeaponIndex >= 0)
		return this->TechnoClass::GetWeapon(pExt->CurrentAircraftWeaponIndex);
	else
		return this->TechnoClass::GetWeapon(this->SelectWeapon(this->Target));
}

// Spy plane, airstrike etc.
bool AircraftExt::PlaceReinforcementAircraft(AircraftClass* pThis, CellStruct edgeCell)
{
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);

	auto coords = CellClass::Cell2Coord(edgeCell);
	AbstractClass* pTarget = nullptr;

	if (pTypeExt->SpawnDistanceFromTarget.isset())
	{
		pTarget = pThis->Target ? pThis->Target : pThis->Destination;

		if (pTarget)
			coords = GeneralUtils::CalculateCoordsFromDistance(CellClass::Cell2Coord(edgeCell), pTarget->GetCoords(), pTypeExt->SpawnDistanceFromTarget.Get());
	}

	++Unsorted::ScenarioInit;
	const bool result = pThis->Unlimbo(coords, DirType::North);
	--Unsorted::ScenarioInit;

	pThis->SetHeight(pTypeExt->SpawnHeight.Get(pThis->Type->GetFlightLevel()));

	if (pTarget)
		pThis->PrimaryFacing.Set_Desired(pThis->GetDirectionOverObject(pTarget));

	return result;
}

void AircraftExt::TriggerCrashWeapon(AircraftClass* pThis, int nMult)
{
	const auto pType = pThis->GetTechnoType();
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	auto pCrashWeapon = pTypeExt->CrashWeapon.Get(pThis);
	if (!pCrashWeapon)
		pCrashWeapon = pTypeExt->CrashWeapon_s.Get();

	if (!TechnoExtData::FireWeaponAtSelf(pThis, pCrashWeapon))
		pThis->FireDeathWeapon(nMult);

	AnimTypeExtData::ProcessDestroyAnims(pThis, nullptr);
}

void AircraftExt::FireBurst(AircraftClass* pThis, AbstractClass* pTarget, AircraftFireMode shotNumber)
{
	if (!pTarget)
		return;

	AircraftExt::FireBurst(pThis, pTarget, shotNumber, pThis->SelectWeapon(pTarget));
}

void AircraftExt::FireBurst(AircraftClass* pThis, AbstractClass* pTarget, AircraftFireMode shotNumber, int WeaponIdx)
{
	const auto pWeaponStruct = pThis->GetWeapon(WeaponIdx);

	if (!pWeaponStruct)
		return;

	const auto weaponType = pWeaponStruct->WeaponType;

	if (!weaponType)
		return;

	AircraftExt::FireBurst(pThis , pTarget, shotNumber, WeaponIdx, weaponType);
}

void AircraftExt::FireBurst(AircraftClass* pThis, AbstractClass* pTarget, AircraftFireMode shotNumber, int WeaponIdx, WeaponTypeClass* pWeapon)
{
	if (!pWeapon->Burst)
		return;

	for (int i = 0; i < pWeapon->Burst; i++)
	{
		if (pWeapon->Burst < 2 && WeaponTypeExtContainer::Instance.Find(pWeapon)->Strafing_SimulateBurst)
			pThis->CurrentBurstIndex = (int)shotNumber;

		pThis->Fire(pTarget, WeaponIdx);
	}
}

#include <Ext/TerrainType/Body.h>
#include "Body.h"

bool AircraftExt::IsValidLandingZone(AircraftClass* pThis)
{
	if (const auto pPassanger = pThis->Passengers.GetFirstPassenger())
	{
		if (const auto pDest = pThis->Destination)
		{
			const auto pDestCell = MapClass::Instance->GetCellAt(pDest->GetCoords());

			return pDestCell->IsClearToMove(pPassanger->GetTechnoType()->SpeedType, false, false, ZoneType::None, pPassanger->GetTechnoType()->MovementZone, -1, false)
				&& pDestCell->OverlayTypeIndex == -1;
		}
	}

	return false;

}

#ifdef ENABLE_NEWHOOKS
ASMJIT_PATCH(0x413F6A, AircraftClass_CTOR, 0x7)
{
	GET(AircraftClass*, pItem, ESI);

	AircraftExt::ExtMap.JustAllocate(pItem, !pItem, "Invalid !");

	return 0;
}

ASMJIT_PATCH(0x41426F, AircraftClass_DTOR, 0x7)
{
	GET(AircraftClass*, pItem, EDI);

	AircraftExt::ExtMap.Remove(pItem);

	return 0;
}

ASMJIT_PATCH_AGAIN(0x41B430, AircraftClass_SaveLoad_Prefix, 0x6)
ASMJIT_PATCH(0x41B5C0, AircraftClass_SaveLoad_Prefix, 0x8)
{
	GET_STACK(AircraftClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	AircraftExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

ASMJIT_PATCH(0x41B5B5, AircraftClass_Load_Suffix, 0x6)
{
	AircraftExt::ExtMap.LoadStatic();

	return 0;
}

ASMJIT_PATCH(0x41B5D4, AircraftClass_Save_Suffix, 0x5)
{
	AircraftExt::ExtMap.SaveStatic();

	return 0;
}

ASMJIT_PATCH(0x41B685, AircraftClass_Detach, 0x6)
{
	GET(AircraftClass*, pThis, ESI);
	GET(void*, target, EDI);
	GET_STACK(bool, all, STACK_OFFSET(0x8, 0x8));

	if (const auto pExt = AircraftExt::ExtMap.Find(pThis))
		pExt->InvalidatePointer(target, all);

	return 0x0;
}
#endif

//ASMJIT_PATCH(0x418478, AircraftClass_Mi_Attack_Untarget1, 6)
//{
//	GET(AircraftClass*, A, ESI);
//	return A->Target
//		? 0
//		: 0x4184C2
//		;
//}
//
//ASMJIT_PATCH(0x4186D7, AircraftClass_Mi_Attack_Untarget2, 6)
//{
//	GET(AircraftClass*, A, ESI);
//	return A->Target
//		? 0
//		: 0x418720
//		;
//}
//
//ASMJIT_PATCH(0x418826, AircraftClass_Mi_Attack_Untarget3, 6)
//{
//	GET(AircraftClass*, A, ESI);
//	return A->Target
//		? 0
//		: 0x418883
//		;
//}
//
//ASMJIT_PATCH(0x418935, AircraftClass_Mi_Attack_Untarget4, 6)
//{
//	GET(AircraftClass*, A, ESI);
//	return A->Target
//		? 0
//		: 0x418992
//		;
//}
//
//ASMJIT_PATCH(0x418A44, AircraftClass_Mi_Attack_Untarget5, 6)
//{
//	GET(AircraftClass*, A, ESI);
//	return A->Target
//		? 0
//		: 0x418AA1
//		;
//}
//
//ASMJIT_PATCH(0x418B40, AircraftClass_Mi_Attack_Untarget6, 6)
//{
//	GET(AircraftClass*, A, ESI);
//	return A->Target
//		? 0
//		: 0x418B8A
//		;
//}
