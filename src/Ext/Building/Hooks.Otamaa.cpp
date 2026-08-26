#include "Body.h"

#include <UnitClass.h>
#include <Utilities/Macro.h>

#include <VocClass.h>
#include <VoxClass.h>

#include <Ext/BuildingType/Body.h>

#pragma region Otamaa
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

ASMJIT_PATCH(0x47EF52, BuildingClass_PlaceCementGrid_Shape, 0x6)
{
	if (auto const pBuilding = cast_to<BuildingClass*>(DisplayClass::Instance->CurrentBuilding)) {
		R->EDX(BuildingTypeExtContainer::Instance.Find(pBuilding->Type)->BuildingPlacementGrid_Shape.Get(FileSystem::PLACE_SHP()));
		return R->Origin() + 0x6;
	}

	return 0x0;
}ASMJIT_PATCH_AGAIN(0x6D5EB1, BuildingClass_PlaceCementGrid_Shape, 0x6)

bool FakeBuildingClass::_IsFactory() {
	return this->Type->Factory == AbstractType::AircraftType || this->IsFactory();
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7E4140, FakeBuildingClass::_IsFactory);

#pragma endregion

// use GetCenterCoords instead of GetCoords for
// TechnoClass_FireLaser
DEFINE_PATCH(0x6FD338 ,0x58);