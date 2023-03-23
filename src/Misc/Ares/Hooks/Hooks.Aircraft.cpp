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

#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>

DEFINE_OVERRIDE_SKIP_HOOK(0x414D36, AircraftClass_Update_DontloseTargetInAir, 0x5 , 414D4D)

//TODO:
//DEFINE_HOOK(0x416C3A, AircraftClass_Carryall_Unload_Facing, 0x5)
//{
//	enum
//	{
//		RetFailed = 0x416C49,
//		RetSucceeded = 0x416C5A
//	};
//
//	GET(FootClass*, pCargo, ESI);
//	GET(CoordStruct*, pCoord, ECX);
//	GET(AircraftClass*, pThis, EDI);
//
//	auto const nFacing = pThis->TurretFacing();
//	if (!pCargo->Unlimbo(*pCoord, (((nFacing.Raw >> 7) + 1) >> 1)))
//		return RetFailed;
//
//	auto const nRot = pCargo->GetTechnoType()->ROT;
//	pCargo->PrimaryFacing.Set_ROT(nRot);
//	auto const pCargoTypeExt = TechnoTypeExt::ExtMap.Find(pCargo->GetTechnoType());
//	pCargo->SecondaryFacing.Set_ROT(pCargoTypeExt->TurretROT.Get(nRot));
//	return RetSucceeded;
//}

DEFINE_OVERRIDE_HOOK(0x416CF4, AircraftClass_Carryall_Unload_Guard, 0x5)
{
	GET(FootClass*, pCargo, ESI);

	pCargo->Transporter = 0;
	pCargo->QueueMission(Mission::Guard, true);

	if (auto pTeam = pCargo->Team)
		pTeam->AddMember(pCargo, false);

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x416C94, AircraftClass_Carryall_Unload_UpdateCargo, 0x6)
{
	GET(UnitClass*, pCargo, ESI);

	pCargo->UpdatePosition(2);

	if (pCargo->Deactivated && pCargo->Locomotor->Is_Powered())
	{
		pCargo->Locomotor->Power_Off();
	}

	return 0;
}

// skip the check for UnitRepair, as it does not play well with UnitReload and
// Factory=AircraftType at all. in fact, it's prohibited, and thus docking to
// other structures was never allowed.
DEFINE_OVERRIDE_HOOK(0x417E16, AircraftClass_GetActionOnObject_Dock, 0x6)
{
	// target is known to be a building
	GET(AircraftClass* const, pThis, ESI);
	GET(BuildingClass* const, pBuilding, EDI);

	// enter and no-enter cursors only if aircraft can dock
	if (pThis->Type->Dock.FindItemIndex(pBuilding->Type) != -1)
	{
		return 0x417E4B;
	}

	// select cursor
	return 0x417E7D;
}

DEFINE_OVERRIDE_HOOK(0x413F98, AircraftClass_Init_Cloakable, 0x6)
{
	GET(AircraftClass*, Item, ESI);
	GET(AircraftTypeClass*, pType, EDI);

	if (pType->Cloakable)
	{
		Item->Cloakable = true;
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x418478, AircraftClass_Mi_Attack_Untarget1, 0x6)
{
	GET(AircraftClass*, A, ESI);
	return A->Target
		? 0
		: 0x4184C2
		;
}

DEFINE_OVERRIDE_HOOK(0x4186D7, AircraftClass_Mi_Attack_Untarget2, 0x6)
{
	GET(AircraftClass*, A, ESI);
	return A->Target
		? 0
		: 0x418720
		;
}

DEFINE_OVERRIDE_HOOK(0x418826, AircraftClass_Mi_Attack_Untarget3, 0x6)
{
	GET(AircraftClass*, A, ESI);
	return A->Target
		? 0
		: 0x418883
		;
}

DEFINE_OVERRIDE_HOOK(0x418935, AircraftClass_Mi_Attack_Untarget4, 0x6)
{
	GET(AircraftClass*, A, ESI);
	return A->Target
		? 0
		: 0x418992
		;
}

DEFINE_OVERRIDE_HOOK(0x418A44, AircraftClass_Mi_Attack_Untarget5, 0x6)
{
	GET(AircraftClass*, A, ESI);
	return A->Target
		? 0
		: 0x418AA1
		;
}

DEFINE_OVERRIDE_HOOK(0x418B40, AircraftClass_Mi_Attack_Untarget6, 0x6)
{
	GET(AircraftClass*, A, ESI);
	return A->Target
		? 0
		: 0x418B8A
		;
}

DEFINE_OVERRIDE_HOOK(0x415544, AircraftClass_Mi_Unload_Blocked, 0xB)
{
	GET(AircraftClass*, pThis, ESI);
	GET(FootClass*, pCargo, EDI);
	GET(bool, bExitObjStatus, EAX);

	if (bExitObjStatus)
	{
		pCargo->Transporter = nullptr;
		pCargo->IsOnCarryall = false;
	}
	else
	{
		pThis->AddPassenger(pCargo);
		pThis->EnterIdleMode(false, 1);
	}

	return 0x41554F;
}

// #1232: fix for dropping units out of flying Carryalls
DEFINE_OVERRIDE_HOOK(0x415DF6, AircraftClass_Paradrop_Carryall, 0x6)
{
	GET(FootClass*, pTechno, ESI);
	pTechno->Transporter = nullptr;
	pTechno->IsOnCarryall = false;
	return 0;
}

// fix for vehicle paradrop alignment
DEFINE_OVERRIDE_HOOK(0x415CA6, AircraftClass_Paradrop_Units, 0x6)
{
	GET(AircraftClass*, A, EDI);
	GET(FootClass*, P, ESI);

	if (P->WhatAmI() == AbstractType::Unit)
	{
		const CoordStruct SrcXYZ = A->GetCoords();
		LEA_STACK(CoordStruct*, XYZ, 0x20);
		XYZ->X = (SrcXYZ.X & ~0xFF) + 0x80;
		XYZ->Y = (SrcXYZ.Y & ~0xFF) + 0x80;
		XYZ->Z = SrcXYZ.Z - 1;
		R->ECX(XYZ);
		return 0x415DE3;
	}

	return 0;
}

// flying aircraft carriers
// allow spawned units to spawn above ground
DEFINE_OVERRIDE_HOOK(0x414338, AircraftClass_Put_SpawnHigh, 0x6)
{
	GET(AircraftClass*, pThis, ESI);
	GET(AircraftTypeClass*, pType, ECX);

	R->EAX(pType->MissileSpawn || pThis->SpawnOwner);
	return 0x41433E;
}

// aim for the cell for flying carriers
DEFINE_OVERRIDE_HOOK(0x6B783B, SpawnManagerClass_Update_SpawnHigh, 0x5)
{
	GET(SpawnManagerClass*, pThis, ESI);

	AbstractClass* pDest = pThis->Owner;
	if (pThis->Owner->GetHeight() > 0) {
		pDest = pThis->Owner->GetCell();
	}

	R->EAX<AbstractClass*>(pDest);
	return 0;
}

/* #1354 - Aircraft and empty SovParaDropInf list */
DEFINE_OVERRIDE_HOOK(0x41D887, AirstrikeClass_Fire, 0x6)
{
	if (!RulesClass::Instance->SovParaDropInf.Count)
	{
		R->ECX(-1);
		return 0x41D895;
	}

	return 0x0;
}

DEFINE_OVERRIDE_HOOK(0x416C4D, AircraftClass_Carryall_Unload_DestroyCargo, 0x5)
{
	GET(AircraftClass*, pCarryall, EDI);
	GET(UnitClass*, pCargo, ESI);

	int Damage = pCargo->Health;
	pCargo->ReceiveDamage(&Damage, 0, RulesClass::Instance->C4Warhead, nullptr, true, true, nullptr);

	Damage = pCarryall->Health;
	pCarryall->ReceiveDamage(&Damage, 0, RulesClass::Instance->C4Warhead, nullptr, true, true, nullptr);

	return 0x416C53;
}
