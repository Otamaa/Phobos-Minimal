#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

#include <Ext/TechnoType/Body.h>

DEFINE_OVERRIDE_HOOK(0x5F6515, AbstractClass_Distance2DSquared_1, 0x8)
{
	GET(AbstractClass*, pThis, ECX);
	GET(AbstractClass*, pThat, EBX);

	auto const nThisCoord = pThis->GetCoords();
	auto const nThatCoord = pThat->GetCoords();
	auto const nXY =
		((nThisCoord.Y - nThatCoord.Y) * (nThisCoord.Y - nThatCoord.Y)) +
		((nThisCoord.X - nThatCoord.X) * (nThisCoord.X - nThatCoord.X));

	R->EAX(nXY >= INT_MAX ? INT_MAX : nXY);
	return 0x5F6559;
}

DEFINE_OVERRIDE_HOOK(0x5F6560, AbstractClass_Distance2DSquared_2, 5)
{
	GET(AbstractClass*, pThis, ECX);
	auto const nThisCoord = pThis->GetCoords();
	GET_STACK(CoordStruct*, pThatCoord, 0x4);

	auto const nXY =
		((nThisCoord.Y - pThatCoord->Y) * (nThisCoord.Y - pThatCoord->Y)) +
		((nThisCoord.X - pThatCoord->X) * (nThisCoord.X - pThatCoord->X));

	R->EAX(nXY >= INT_MAX ? INT_MAX : nXY);
	return 0x5F659B;
}

DEFINE_OVERRIDE_HOOK(0x414D36, AircraftClass_Update_DontloseTargetInAir, 0x5)
{ return 0x414D4D; }

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

	if (pCargo->Deactivated && pCargo->Locomotor->Is_Powered()) {
		pCargo->Locomotor->Power_Off();
	}

	return 0;
}