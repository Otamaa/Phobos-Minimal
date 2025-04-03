#include "Body.h"

#include <Helpers/Macro.h>

#include <CellClass.h>
#include <ConvertClass.h>

ASMJIT_PATCH(0x544E70, IsometricTileTypeClass_Init_Drawer, 0x8)
{
	GET(int, r, ECX);
	GET(int, g, EDX);
	GET_STACK(int, b, 0x4);
	R->EAX(IsometricTileTypeExtData::GetLightConvert(nullptr, r, g, b ));
	return 0x544E70;
}

ASMJIT_PATCH(0x483F5B, CellClass_InitDrawer_custom_1,0x8)
{
	GET(CellClass*, pCell, ESI);
	GET_STACK(int, b, 0x18 - 0x10);
	GET_STACK(int, g, 0x18 - 0x14);
	GET_STACK(int, r, 0x18 + 0x4);

	R->EAX(IsometricTileTypeExtData::GetLightConvert(IsometricTileTypeClass::Array->GetItemOrDefault(pCell->IsoTileTypeIndex),
		r,
		g,
		b
	));
	return 0x483F6D;
}

ASMJIT_PATCH(0x483FE5, CellClass_InitDrawer_custom_2, 0x5)
{
	GET(CellClass*, pCell, ESI);
	R->EAX(IsometricTileTypeExtData::GetLightConvert(IsometricTileTypeClass::Array->GetItemOrDefault(pCell->IsoTileTypeIndex),
		1000,
		1000,
		1000
	));
	return 0x483FEF;
}

ASMJIT_PATCH(0x484135, CellClass_UpdateCellLightning_custom_1, 0x8)
{
	GET(CellClass*, pCell, ESI);
	GET_STACK(int, b, 0x14 + 0x10);
	GET_STACK(int, g, 0x14 + 0x14);
	GET_STACK(int, r, 0x14 - 0x4);

	R->EAX(IsometricTileTypeExtData::GetLightConvert(IsometricTileTypeClass::Array->GetItemOrDefault(pCell->IsoTileTypeIndex),
		r,
		g,
		b
	));
	return 0x484147;
}

ASMJIT_PATCH(0x484155, CellClass_UpdateCellLightning_custom_2, 0x5)
{
	GET(CellClass*, pCell, ESI);
	R->EAX(IsometricTileTypeExtData::GetLightConvert(IsometricTileTypeClass::Array->GetItemOrDefault(pCell->IsoTileTypeIndex),
		1000,
		1000,
		1000
	));
	return 0x484166;
}
