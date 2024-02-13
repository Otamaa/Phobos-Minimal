#include "Body.h"

#include <TiberiumClass.h>
#include <OverlayTypeClass.h>
#include <OverlayClass.h>
#include <FileSystem.h>
#include <IsometricTileTypeClass.h>

#include <Ext/Tiberium/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/IsometricTileType/Body.h>

DEFINE_HOOK(0x47FDF9, CellClass_GetOverlayShadowRect, 0xA)
{
	GET(CellClass*, pThis, EDI);
	GET(OverlayTypeClass*, pOverlay, ESI);

	if (pOverlay->Tiberium)
		pOverlay = OverlayTypeClass::Array->Items[CellExtData::GetOverlayIndex(pThis)];

	R->EBX(pOverlay->GetImage());

	return 0x47FE05;
}

DEFINE_HOOK(0x47F641, CellClass_DrawShadow_Tiberium, 0x6)
{
	enum { SkipDrawing = 0x47F637, ContinueDrawing = 0x0 };
	GET(CellClass*, pThis, ESI);

	return (TiberiumClass::FindIndex(pThis->OverlayTypeIndex) >= 0) ? SkipDrawing : ContinueDrawing;
}

//seems causing large FPS drop
DEFINE_STRONG_HOOK(0x6D7A46, TacticalClass_DrawPixelFX_Tiberium, 0x7)
{
	GET(CellClass*, pCell, ESI);

	bool bDraw = false;

	if (const auto pTiberium = CellExtData::GetTiberium(pCell)) {
		if (TiberiumExtExtContainer::Instance.Find(pTiberium)->EnablePixelFXAnim)
			bDraw = pTiberium->Value;
	}

	R->EAX(bDraw);
	return 0x6D7A4D;
}

DEFINE_HOOK(0x47F860, CellClass_DrawOverlay_Tiberium, 0x8) // B
{
	GET(CellClass*, pThis, ESI);

	const auto pTiberium = CellExtData::GetTiberium(pThis);

	if (!pTiberium)
		return 0x47FB86;

	const auto pTibExt = TiberiumExtExtContainer::Instance.Find(pTiberium);

	if (!pTibExt) {
		Debug::Log("CellClass_DrawOverlay_Tiberium TiberiumExt for [%s] is missing ! \n", pTiberium->ID);
		R->EBX(pTiberium);
		return 0x47F882;
	}

	if (!pTibExt->EnableLighningFix.Get()) {
		R->EBX(pTiberium);
		return 0x47F882;
	}

	GET_STACK(Point2D, nPos, 0x14);
	GET(RectangleStruct*, pBound, EBP);

	auto nIndex = CellExtData::GetOverlayIndex(pThis, pTiberium);
	const auto pShape = OverlayTypeClass::Array->Items[nIndex]->GetImage();

	if (!pShape)
		return 0x47FB86;

	const auto nZAdjust = -2 - 15 * (pThis->Level + 4 * (((int)pThis->Flags >> 7) & 1));
	auto nTint = pTibExt->Ore_TintLevel.Get(pTibExt->UseNormalLight.Get() ? 1000 : pThis->Intensity_Terrain);
	const int nOreTint = std::clamp(nTint, 0, 1000);
	auto nShadowFrame = (nIndex + pShape->Frames / 2);
	ConvertClass* pDecided = FileSystem::x_PAL();
	if (const auto pCustom = pTibExt->Palette) {
		pDecided = pCustom->GetConvert<PaletteManager::Mode::Temperate>();
	}

	SHPStruct* pZShape = nullptr;
	if (auto nSlope = (int)pThis->SlopeIndex)
		pZShape = IsometricTileTypeClass::SlopeZshape[nSlope];	//this is just pointers to files in ram, no vector #tomsons26

	DSurface::Temp->DrawSHP(pDecided, pShape, pThis->OverlayData, &nPos, pBound, BlitterFlags(0x4E00), 0, nZAdjust, ZGradient::Ground, nOreTint, 0, pZShape, 0, 0, 0);
	DSurface::Temp->DrawSHP(pDecided, pShape, nShadowFrame, &nPos, pBound, BlitterFlags(0x4E01), 0, nZAdjust, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);

	return 0x47FB86;
}

DEFINE_HOOK(0x47F661, CellClass_DrawOverlay_Rubble_Shadow, 0x8)
{
	GET(CellClass*, pCell, ESI);
	GET_STACK(SHPStruct*, pImage, STACK_OFFSET(0x28, 0x8));
	GET_STACK(int, nFrame, STACK_OFFSET(0x28, 0x4));
	LEA_STACK(Point2D*, pPoint, STACK_OFFS(0x28, 0x10));
	GET_STACK(int, nOffset, STACK_OFFS(0x28, 0x18));
	GET(RectangleStruct*, pRect, EBX);

	if (!R->AL())
		return 0x47F637;

	auto const pBTypeExt = BuildingTypeExtContainer::Instance.Find(pCell->Rubble);

	ConvertClass* pDecided = pCell->LightConvert;
	if (const auto pCustom = pBTypeExt->RubblePalette) {
		pDecided = pCustom->GetConvert<PaletteManager::Mode::Temperate>();
	}

	auto const zAdjust = - 2 - nOffset;

	DSurface::Temp()->DrawSHP(pDecided, pImage, nFrame, pPoint, pRect, BlitterFlags(0x4601),
	0, zAdjust, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);

	return 0x47F637;
}

DEFINE_HOOK(0x47FADB, CellClass_DrawOverlay_Rubble, 0x5)
{
	GET(OverlayTypeClass*, pOvl, ECX);
	GET(CellClass*, pCell, ESI);

	auto const pRubble = pCell->Rubble;
	if(!pRubble)
		return 0x47FB86;

	LEA_STACK(SHPStruct**, pImage, STACK_OFFS(0x24, 0x14));
	LEA_STACK(int*, pFrame, STACK_OFFSET(0x24, 0x8));

	if (!pRubble->CanLeaveRubble(pImage,pFrame))
		return 0x47FB86;

	LEA_STACK(Point2D*, pPoint, STACK_OFFS(0x24, 0x10));
	GET_STACK(int, nOffset, STACK_OFFSET(0x24, 0x4));
	GET(RectangleStruct*, pRect, EBP);
	GET(int, nVal, EDI);

	//if (!R->AL())
	//	return 0x47FB86;

	auto const pBTypeExt = BuildingTypeExtContainer::Instance.Find(pRubble);
	ConvertClass* pDecided = pCell->LightConvert;
	if (const auto pCustom = pBTypeExt->RubblePalette) {
		pDecided = pCustom->GetConvert<PaletteManager::Mode::Temperate>();
	}

	const auto zAdjust = nVal - nOffset - 2;

	DSurface::Temp()->DrawSHP(pDecided, *pImage, *pFrame, pPoint, pRect, BlitterFlags(0x4E00),
	0, zAdjust, pOvl->DrawFlat != 0 ? ZGradient::Ground : ZGradient::Deg90, pCell->Intensity_Terrain, 0, nullptr, 0, 0, 0);

	return 0x47FB86;
}

/*
*    v3 = this->TileType;
	if ( v3 >= 0 && v3 < (int)*(&IsometricTileTypes + 4) && !(*(&IsometricTileTypes + 1))[v3]->AllowBurrowing
*/
//DEFINE_HOOK(0x487022, CellClass_CanEnterCell_Add, 0x6)
//{
//	enum
//	{
//		allowed = 0x487093,
//		notallowed = 0x4870A1,
//		continue_check = 0x48702C,
//	};
//
//	GET(IsometricTileTypeClass*, pTile, EDX);
//
//	if (!pTile->AllowBurrowing)
//		return notallowed;
//	else
//		return continue_check;
//}

//DEFINE_HOOK(0x4D9C41, FootClass_CanEnterCell_Restricted, 0x6)
//{
//	GET(FootClass*, pFoot, ESI);
//
//	if (auto pCell = pFoot->GetCell())
//	{
//		if (auto pILoco = pFoot->Locomotor.get())
//		{
//			auto const pLocoClass = static_cast<LocomotionClass*>(pILoco);
//			CLSID nID { };
//			if (SUCCEEDED(pLocoClass->GetClassID(&nID)))
//			{
//				if (nID == LocomotionClass::CLSIDs::Jumpjet)
//				{
//					const auto nTile = pCell->IsoTileTypeIndex;
//					if (nTile >= 0 && nTile < IsometricTileTypeClass::Array->Count
//					 )
//					{
//						if (auto const pIsoTileExt = IsometricTileTypeExt::ExtMap.Find(IsometricTileTypeClass::Array->Items[(nTile))))
//						{
//							if (pIsoTileExt->BlockJumpjet.Get())
//							{
//								R->EAX(Move::No);
//								return 0x4D9C4E;
//							}
//						}
//					}
//				}
//			}
//		}
//	}
//
//	return 0x0;
//}

// Aircraft pathfinding is shit
//DEFINE_HOOK(0x4196B0, AircraftClass_CanEnterCell_Restricted, 0x5)
//{
//	GET(AircraftClass*, pThis, ECX);
//
//	if (auto pCell = pThis->GetCell()) {
//		const auto nTile = pCell->IsoTileTypeIndex;
//		if (nTile >= 0 && nTile < IsometricTileTypeClass::Array->Count && nTile == 1499) {
//			R->EAX(Move::No);
//			return 0x4197A7;
//		}
//	}
//
//	return 0x0;
//}

//UniqueGamePtrB<LightConvertClass> SpawnTiberiumTreeConvert {};
//
//DEFINE_HOOK(0x52C046, InitGame_CreateTiberiumDrawer, 0x5)
//{
//	LEA_STACK(BytePalette*, pUnitSnoPal, 0x2F40 - 0x2BC8);
//
//	SpawnTiberiumTreeConvert.reset(GameCreate<LightConvertClass>(
//		pUnitSnoPal, &FileSystem::TEMPERAT_PAL,
//		DSurface::Primary,1000, 1000, 1000,false , nullptr , 53));
//
//	return 0;
//}
//
//DEFINE_HOOK(0x53AD00, ScenarioClass_RecalcLighting_TintTiberiumDrawer, 5)
//{
//	GET(int, red, ECX);
//	GET(int, green, EDX);
//	GET_STACK(int, blue, STACK_OFFSET(0x0, 0x4));
//	GET_STACK(bool, tint, STACK_OFFSET(0x0, 0x8));
//	SpawnTiberiumTreeConvert->UpdateColors(red, green, blue, tint);
//	return 0;
//}
//
//DEFINE_HOOK(0x71C294, TerrainClass_DrawIt_TiberiumSpawn_Palette, 0x6)
//{
//	R->EDX(SpawnTiberiumTreeConvert.get());
//	return 0x71C29A;
//}