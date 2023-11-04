#include "Body.h"

#include <Helpers/Macro.h>

#include <CellClass.h>
#include <ConvertClass.h>

LightConvertClass* GetConvert(CellClass* pCell, int r, int g, int b)
{
	if (!DSurface::Primary())
		return nullptr;

	if (auto pIso = IsometricTileTypeClass::Array->GetItemOrDefault(pCell->IsoTileTypeIndex)) {
		const auto pExt = IsometricTileTypeExtContainer::Instance.Find(pIso);

		if (pExt->Palette) {
			return pExt->GetLightConvert(r, g, b);
		}
	}

	return LightConvertClass::InitLightConvert(r, g, b);
}

DEFINE_HOOK(0x483F5B, CellClass_InitDrawer_custom_1,0x8)
{
	GET(CellClass*, pCell, ESI);
	GET_STACK(int, b, 0x18 - 0x10);
	GET_STACK(int, g, 0x18 - 0x14);
	GET_STACK(int, r, 0x18 + 0x4);

	R->EAX(GetConvert(pCell, r,g,b));
	return 0x483F6D;
}

DEFINE_HOOK(0x483FE5, CellClass_InitDrawer_custom_2, 0x5)
{
	GET(CellClass*, pCell, ESI);
	R->EAX(GetConvert(pCell, 1000, 1000, 1000));
	return 0x483FEF;
}

DEFINE_HOOK(0x484135, CellClass_UpdateCellLightning_custom_1, 0x8)
{
	GET(CellClass*, pCell, ESI);
	GET_STACK(int, b, 0x14 + 0x10);
	GET_STACK(int, g, 0x14 + 0x14);
	GET_STACK(int, r, 0x14 - 0x4);

	R->EAX(GetConvert(pCell, r, g, b));
	return 0x484147;
}

DEFINE_HOOK(0x484155, CellClass_UpdateCellLightning_custom_2, 0x5)
{
	GET(CellClass*, pCell, ESI);
	R->EAX(GetConvert(pCell, 1000, 1000, 1000));
	return 0x484166;
}