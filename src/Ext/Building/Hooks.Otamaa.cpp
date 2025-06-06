#include "Body.h"

#include <UnitClass.h>
#include <Utilities/Macro.h>

#include <Ext/BuildingType/Body.h>

#include <Misc/Hooks.Otamaa.h>

#pragma region Otamaa
ASMJIT_PATCH(0x4518CF, BuildingClass_AnimLogic_check, 0x9)
{
	GET(BuildingClass*, pThis, ESI);
	GET_STACK(const char*, pDecidedName, STACK_OFFS(0x34, -0x4));
	GET_STACK(BuildingAnimSlot, nSlot, STACK_OFFS(0x34, -0x8));
	R->EAX(BuildingTypeExtData::GetBuildingAnimTypeIndex(pThis, nSlot, pDecidedName));
	return 0x4518D8;
}

ASMJIT_PATCH(0x45234B, BuildingClass_TurnOn_EVA, 0x5)
{
	GET(FakeBuildingClass*, pThis, ESI);
	VoxClass::PlayIndex(pThis->_GetTypeExtData()->EVA_Online);
	return 0x45235A;
}

ASMJIT_PATCH(0x4523D4, BuildingClass_TurnOff_EVA, 0x5)
{
	GET(FakeBuildingClass*, pThis, ESI);
	VoxClass::PlayIndex(pThis->_GetTypeExtData()->EVA_Offline);
	return 0x4523E3;
}

ASMJIT_PATCH(0x44E85F, BuildingClass_Power_UntieStregth, 0x7)
{
	GET(FakeBuildingClass*, pThis, ESI);
	GET_STACK(int, nPowMult, 0x8);

	R->EAX((int)(!pThis->_GetTypeExtData()->Power_DegradeWithHealth.Get()
		? (nPowMult) :
		 MaxImpl(
			 ((nPowMult * pThis->_GetTypeExtData()->PowerPlant_DamageFactor) * pThis->GetHealthPercentage_()), 0)));

	return 0x44E86F;
}

/*
namespace Temp_BuildingClass_GetStaticImage_Sell
{
	Valueable<SHPStruct*> SellImage;
}

ASMJIT_PATCH(0x43F000, BuildingClass_GetStaticImage_Sell, 0x6)
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

ASMJIT_PATCH(0x47EF52, BuildingClass_PlaceCementGrid_Shape, 0x6)
{
	if (auto const pBuilding = cast_to<BuildingClass*>(DisplayClass::Instance->CurrentBuilding)) {
		R->EDX(BuildingTypeExtContainer::Instance.Find(pBuilding->Type)->BuildingPlacementGrid_Shape.Get(FileSystem::PLACE_SHP()));
		return R->Origin() + 0x6;
	}

	return 0x0;
}ASMJIT_PATCH_AGAIN(0x6D5EB1, BuildingClass_PlaceCementGrid_Shape, 0x6)

// ASMJIT_PATCH(0x441EFC, BuildingClass_Destroy_PreventRubble, 0xB)
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
bool FakeBuildingClass::_IsFactory() {
	return this->Type->Factory == AbstractType::AircraftType || this->IsFactory();
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7E4140, FakeBuildingClass::_IsFactory);

/*
#ifdef ENABLE_NEWHOOKS
ASMJIT_PATCH(0x443FF9,BuildingClass_ExitObject_Aircraft,0xA)
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