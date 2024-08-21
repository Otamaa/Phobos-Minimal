#include "Body.h"

#include <UnitClass.h>
#include <Utilities/Macro.h>

#include <Ext/BuildingType/Body.h>

#pragma region Otamaa
DEFINE_HOOK(0x4518CF, BuildingClass_AnimLogic_check, 0x9)
{
	GET(BuildingClass*, pThis, ESI);
	GET_STACK(const char*, pDecidedName, STACK_OFFS(0x34, -0x4));
	GET_STACK(BuildingAnimSlot, nSlot, STACK_OFFS(0x34, -0x8));
	R->EAX(BuildingTypeExtData::GetBuildingAnimTypeIndex(pThis, nSlot, pDecidedName));
	return 0x4518D8;
}

DEFINE_HOOK(0x45234B, BuildingClass_TurnOn_EVA, 0x5)
{
	GET(BuildingClass*, pThis, ESI);
	VoxClass::PlayIndex(BuildingTypeExtContainer::Instance.Find(pThis->Type)->EVA_Online);
	return 0x45235A;
}

DEFINE_HOOK(0x4523D4, BuildingClass_TurnOff_EVA, 0x5)
{
	GET(BuildingClass*, pThis, ESI);
	VoxClass::PlayIndex(BuildingTypeExtContainer::Instance.Find(pThis->Type)->EVA_Offline);
	return 0x4523E3;
}

/*
DEFINE_HOOK(0x442243, BuildingClass_ReceiveDamage_AddEarly, 0xA)
{
	R->Stack(STACK_OFFS(0x9C, 0x6C), DamageState::Unaffected);

	GET(BuildingClass*, pThis, ESI);
	GET(TechnoClass*, pSource, EBP);

	if (pThis == pSource && !pSource->GetTechnoType()->DamageSelf) {
		return 0x442C06;
	}

	return 0x442268;
}*/

DEFINE_HOOK(0x44E85F, BuildingClass_Power_UntieStregth, 0x7)
{
	GET(BuildingClass*, pThis, ESI);
	GET_STACK(int, nPowMult, 0x8);

	R->EAX((int)(!BuildingTypeExtContainer::Instance.Find(pThis->Type)->Power_DegradeWithHealth.Get()
		? (nPowMult) : (nPowMult * pThis->GetHealthPercentage_())));

	return 0x44E86F;
}

/*
namespace Temp_BuildingClass_GetStaticImage_Sell
{
	Valueable<SHPStruct*> SellImage;
}

DEFINE_HOOK(0x43F000, BuildingClass_GetStaticImage_Sell, 0x6)
{
	GET(BuildingClass*, pThis, ESI);

	if (auto pShape = Temp_BuildingClass_GetStaticImage_Sell::SellImage.Get())
	{
		int const nFrame = pShape->Frames / 2;
		int const nFrameOut = nFrame > 1 ? nFrame : 1;
		int const nFrameBld = pThis->Type->BuildingAnimFrame[0].FrameCount + pThis->Type->BuildingAnimFrame[0].dwUnknown;
		R->EAX((nFrameBld - pThis->Animation.Value - 1) / nFrameBld * nFrameOut);
		return 0x43F029;
	}

	return 0x0;
}
*/

DEFINE_HOOK_AGAIN(0x6D5EB1, BuildingClass_PlaceCementGrid_Shape, 0x6)
DEFINE_HOOK(0x47EF52, BuildingClass_PlaceCementGrid_Shape, 0x6)
{
	if (auto const pBuilding = specific_cast<BuildingClass*>(DisplayClass::Instance->CurrentBuilding)) {
		R->EDX(BuildingTypeExtContainer::Instance.Find(pBuilding->Type)->BuildingPlacementGrid_Shape.Get(FileSystem::PLACE_SHP()));
		return R->Origin() + 0x6;
	}

	return 0x0;
}

// DEFINE_HOOK(0x441EFC, BuildingClass_Destroy_PreventRubble, 0xB)
// {
// 	GET(BuildingClass*, pThis, ESI);
// 	//GET_STACK(TechnoClass*, pKiller, STACK_OFFS(0x64, -0x8));
// 	//GET_STACK(void*, pPointer, STACK_OFFS(0x64, 0x14));
//
// 	if (pThis->GetCurrentMission() == Mission::Selling)
// 	{
// 		pThis->Health = 0;
// 		if (R->AL()) {
// 			pThis->NoCrew = true;
// 		}
//
// 		pThis->DestroyExtrasFunctions(pThis->C4AppliedBy);
// 		return 0x441F20;
// 	}

// 	return 0x0;
// }

DEFINE_JUMP(VTABLE, 0x7E4140, GET_OFFSET(BuildingTypeExtData::IsFactory));

/*
#ifdef ENABLE_NEWHOOKS
DEFINE_HOOK(0x443FF9,BuildingClass_ExitObject_Aircraft,0xA)
{
	GET(BuildingClass*, pThis, ESI);
	GET(AircraftClass*, pProduct, EBP);
	//GET(CoordStruct*, pCoord,EAX);

	if (!pProduct->Type->AirportBound)
	{
		CellStruct nCell;
		pProduct->NearbyLocation(&nCell, pThis);
		if(auto pCell = Map[nCell])
		{
			CoordStruct nBuff = pCell->GetCoords();
			nBuff.Z += Map.GetCellFloorHeight(nBuff);
			R->EAX(&nBuff);
			return 0x444003;
		}
	}

	return 0x0;
}
#endif
*/

#pragma endregion

// use GetCenterCoords instead of GetCoords for
// TechnoClass::FireLaser
DEFINE_PATCH(0x6FD338 ,0x58);