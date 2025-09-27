#include "Body.h"

#include <Helpers\Macro.h>

ASMJIT_PATCH(0x47F71D, CellClass_DrawOverlay_ZAdjust, 0x5)
{
	GET(int, zAdjust, EDI);
	GET_STACK(OverlayTypeClass*, pOverlayType, STACK_OFFSET(0x24, -0x14));

	auto const pTypeExt = OverlayTypeExtContainer::Instance.Find(pOverlayType);

	if (pTypeExt->ZAdjust != 0)
		R->EDI(zAdjust - pTypeExt->ZAdjust);

	return 0;
}

// Replaces an Ares hook at 0x47F9A4
ASMJIT_PATCH(0x47F974, CellClass_DrawOverlay_Walls, 0x5)
{
	enum { SkipGameCode = 0x47FB86 };

	GET(CellClass*, pThis, ESI);
	GET(SHPStruct*, pShape, EAX);
	GET(RectangleStruct*, pBounds, EBP);
	GET(int, zAdjust, EDI);
	GET_STACK(OverlayTypeClass*, pOverlayType, STACK_OFFSET(0x24, -0x14));
	REF_STACK(Point2D, pLocation, STACK_OFFSET(0x24, -0x10));

	int colorSchemeIndex = HouseClass::CurrentPlayer->ColorSchemeIndex;
	if (pThis->WallOwnerIndex >= 0)
		colorSchemeIndex = HouseClass::Array->Items[pThis->WallOwnerIndex]->ColorSchemeIndex;

	LightConvertClass* pConvert = nullptr;
	const auto pTypeExt = OverlayTypeExtContainer::Instance.Find(pOverlayType);

	if (auto pConvertVec = pTypeExt->Palette.ColorschemeDataVector)
		pConvert = pConvertVec->Items[colorSchemeIndex]->LightConvert;
	else
		pConvert = ColorScheme::Array->Items[colorSchemeIndex]->LightConvert;

	DSurface::Temp->DrawSHP(pConvert, pShape, pThis->OverlayData, &pLocation, pBounds,
		BlitterFlags(0x4E00), 0, -2 - zAdjust, ZGradient::Deg90, pThis->Color1.Red, 0, 0, 0, 0, 0);

	return SkipGameCode;
}