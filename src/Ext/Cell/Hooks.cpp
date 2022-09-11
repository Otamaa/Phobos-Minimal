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

DEFINE_HOOK(0x47F860, CellClass_DrawOverlay_Tiberium, 0x8) // B
{
	GET(CellClass*, pThis, ESI);

	const auto pTiberium = CellExt::GetTiberium(pThis);

	if (!pTiberium)
		return 0x47FB86;

	const auto pTibExt = TiberiumExt::ExtMap.Find(pTiberium);

	if (!pTibExt || !pTibExt->EnableLighningFix.Get()){
		R->EBX(pTiberium);
		return 0x47F882;
	}

	GET_STACK(Point2D, nPos, 0x14);
	GET(RectangleStruct*, pBound, EBP);

	auto nIndex = CellExt::GetOverlayIndex(pThis ,pTiberium);
	const auto pShape = OverlayTypeClass::Array->GetItem(nIndex)->GetImage();

	if (!pShape)
		return 0x47FB86;

	const auto nZAdjust = -2 - 15 * (pThis->Level + 4 * (((int)pThis->Flags >> 7) & 1));
	auto nTint = pTibExt->Ore_TintLevel.Get(pTibExt->UseNormalLight.Get() ? 1000 : pThis->Intensity_Terrain);
	const int nOreTint =  std::clamp(nTint,0, 1000);
	auto nShadowFrame = (nIndex + pShape->Frames / 2);
	const auto pPalette = pTibExt->Palette.GetOrDefaultConvert(FileSystem::x_PAL.get());

	SHPStruct* pZShape = nullptr;
	if (auto nSlope = (int)pThis->SlopeIndex)
		pZShape = IsometricTileTypeClass::SlopeZshape[nSlope];	//this is just pointers to files in ram, no vector #tomsons26

	DSurface::Temp->DrawSHP(pPalette, pShape, pThis->OverlayData, &nPos, pBound, BlitterFlags(0x4E00), 0, nZAdjust, ZGradient::Ground, nOreTint, 0, pZShape, 0, 0, 0);
	DSurface::Temp->DrawSHP(pPalette, pShape, nShadowFrame, &nPos, pBound, BlitterFlags(0x4E01), 0, nZAdjust, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);

	return 0x47FB86;
}

//DEFINE_HOOK(0x47F62C, CellClass_DrawOverlay_Shadow_Rubble_Palette, 0x6)
//{
//	GET(CellClass* const, pCell, ESI);
//
//	if (auto pBuildingType = pCell->Rubble) {
//		auto pBuildingExt = BuildingTypeExt::ExtMap.Find(pCell->Rubble);
//		R->EDX(pBuildingExt->RubblePalette.GetOrDefaultConvert(pCell->LightConvert));
//		return 0x47F62C;
//	}
//
//	return 0x0;
//}
//
//DEFINE_HOOK(0x47FB77, CellClass_DrawOverlay_Rubble_Palette , 0xA)
//{
//	GET(CellClass* const, pCell, ESI);
//
//	if (auto pBuildingType = pCell->Rubble) {
//		auto pBuildingExt = BuildingTypeExt::ExtMap.Find(pCell->Rubble);
//		R->EDX(pBuildingExt->RubblePalette.GetOrDefaultConvert(pCell->LightConvert));
//	}
//
//	return 0x0;
//}
