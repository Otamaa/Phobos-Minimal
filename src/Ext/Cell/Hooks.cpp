#include "Body.h"

#include <TiberiumClass.h>
#include <OverlayTypeClass.h>
#include <OverlayClass.h>
#include <FileSystem.h>
#include <IsometricTileTypeClass.h>

#include <Ext/Tiberium/Body.h>
#include <Ext/BuildingType/Body.h>

DEFINE_HOOK(0x47FDF9, CellClass_GetOverlayShadowRect, 0xA)
{
	GET(CellClass*, pThis, EDI);
	GET(OverlayTypeClass*, pOverlay, ESI);

	if (pOverlay->Tiberium)
		pOverlay = OverlayTypeClass::Array->GetItem(CellExt::GetOverlayIndex(pThis));

	R->EBX(pOverlay->GetImage());

	return 0x47FE05;
}

DEFINE_HOOK(0x47F641, CellClass_DrawShadow_Tiberium, 0x6)
{
	enum { SkipDrawing = 0x47F637, ContinueDrawing = 0x0 };
	GET(CellClass*, pThis, ESI);

	return (TiberiumClass::FindIndex(pThis->OverlayTypeIndex) >= 0) ? SkipDrawing : ContinueDrawing;
}

DEFINE_HOOK(0x47F860, CellClass_DrawOverlay_Tiberium, 0x8)
{

	GET(CellClass*, pThis, ESI);
	GET_STACK(Point2D, nPos, 0x14);
	GET(RectangleStruct*, pBound, EBP);

	const auto nIndex = CellExt::GetOverlayIndex(pThis);
	const auto pShape = OverlayTypeClass::Array->GetItem(nIndex)->GetImage();

	if (!pShape)
		return 0x47FB86;

	const auto nZAdjust = -2 - 15 * (pThis->Level + 4 * (((int)pThis->Flags >> 7) & 1));
	const auto pPalette = FileSystem::x_PAL.get();
	int nOreTint = 1000;
	auto nShadowFrame = (nIndex + pShape->Frames / 2);
	//this is just pointers to files in ram, no vector #tomsons26

	if (const auto pTibExt = TiberiumExt::ExtMap.Find(TiberiumClass::Array->GetItem(pThis->GetContainedTiberiumIndex()))) {
			//pPalette = pTibExt->Ore_Palette.GetOrDefaultConvert(pPalette);
		if (pTibExt->Ore_TintLevel.isset())
			nOreTint = Math::min(pTibExt->Ore_TintLevel.Get(), 1000);
	}

	SHPStruct* pZShape = nullptr;
	if (auto nSlope = (int)pThis->SlopeIndex)
		pZShape = IsometricTileTypeClass::SlopeZshape[nSlope];

	DSurface::Temp->DrawSHP(pPalette, pShape, pThis->OverlayData, &nPos, pBound, BlitterFlags(0x4E00), 0, nZAdjust, ZGradient::Ground, nOreTint, 0, pZShape, 0, 0, 0);
	DSurface::Temp->DrawSHP(pPalette, pShape, nShadowFrame, &nPos, pBound, BlitterFlags(0x4E01), 0, nZAdjust, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);

	return 0x47FB86;
}

/*
DEFINE_HOOK(0x47F629, CellClass_DrawOverlay_Shadow_Rubble_Palette, 0x9	)
{
	GET(CellClass* const, pCell, ESI);

	if (auto pBuildingType = pCell->Rubble) {
		auto pBuildingExt = BuildingTypeExt::ExtMap.Find(pCell->Rubble);
		R->EDX(pBuildingExt->RubblePalette.GetOrDefaultConvert(pCell->LightConvert));
		return 0x47F62C;
	}

	return 0x0;
}

DEFINE_HOOK(0x47FB77 , CellClass_DrawOverlay_Rubble_Palette , 0x0)
{
	GET(CellClass* const, pCell, ESI);

	constexpr static unsigned char ret_bytes[] = {
		0x50                    // push  eax
	};

	if (auto pBuildingType = pCell->Rubble) {
		auto pBuildingExt = BuildingTypeExt::ExtMap.Find(pCell->Rubble);
		R->EDX(pBuildingExt->RubblePalette.GetOrDefaultConvert(pCell->LightConvert));
	}
	else {
		R->EDX(pCell->LightConvert);
	}

	//return 0x47FB7A;
	return (int)ret_bytes;
}*/
