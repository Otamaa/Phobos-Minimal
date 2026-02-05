#include "Body.h"

#include <TiberiumClass.h>
#include <OverlayTypeClass.h>
#include <OverlayClass.h>
#include <FileSystem.h>
#include <IsometricTileTypeClass.h>

#include <Ext/Tiberium/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/IsometricTileType/Body.h>

#include <Ext/Rules/Body.h>

/*
void CellClass::Draw_Overlay(int xOffset, int yOffset, const Rect& viewport)
{
	// Early exit for invalid overlays
	if (Overlay == OVERLAY_USELESS || Overlay == OVERLAY_VEINHOLEDUMMY)
		return;

	OverlayTypeClass* overlayType = OverlayTypes[Overlay];

	// Calculate pixel position for overlay
	Point2D overlayPixelPos = CellClass_overlay_pixelPos_480110(this);
	Point2D drawPos;
	drawPos.X = overlayPixelPos.X + xOffset - viewport.X;
	drawPos.Y = overlayPixelPos.Y + yOffset - viewport.Y;

	// Calculate Z-offset based on level and bridge status
	int level = Level;
	int isBridge = (Bitfield2 >> 7) & 1;
	int zOffset = 15 * (level + 4 * isBridge);

	// Initialize tile drawer if needed
	if (!TileDrawer)
	{
		Init_Drawer(0, 0x10000, 0, 1000, 1000, 1000);
	}

	int* imageData = overlayType->Get_Image_Data();

	// Handle special veins overlay (Bitfield2[0] < 0 means it's veins)
	if (Bitfield2[0] < 0)
	{
		// Check if redraw is needed (caching mechanism)
		if (RedrawFrame == Frame &&
			RedrawCountMAYBE == MapClass_Redraws &&
			InViewportRect.X == viewport.X &&
			InViewportRect.Y == viewport.Y &&
			InViewportRect.Width == viewport.Width &&
			InViewportRect.Height == viewport.Height)
		{
			return; // No redraw needed
		}

		// Calculate frame index
		int frame = OverlayData;
		if (OverlayData == 0 || OverlayData == 9)
		{
			int lookupIndex = (Position.X & 3) | ((Position.Y & 3) << 2);
			frame += OverlayFrameLookup[lookupIndex];
		}

		CC_Draw_Shape(
			&TempSurface->xs.s,
			&TileDrawer->cv,
			(ShapeCache*)imageData,
			frame,
			&drawPos,
			&viewport,
			BF_USE_ZBUFFER | BF_ALPHA | BF_400 | BF_CENTER,
			0,
			-2 - zOffset,
			0,
			Color1.b,
			0, 0, 0, 0, 0);

		// Update cache state
		RedrawFrame = Frame;
		InViewportRect = ViewportBounds_TacPixel;
		RedrawCountMAYBE = MapClass_Redraws;
		return;
	}

	// Handle tiberium overlays
	if (overlayType->IsTiberium)
	{
		int tiberiumType = OverlayClass::get_tiberium_type(Overlay);
		if (tiberiumType == -1)
			return;

		TiberiumClass* tiberium = Tiberiums[tiberiumType];
		int numImages;
		int frameIndex;

		// Handle ramp tiberium
		if (Ramp)
		{
			numImages = tiberium->__NumImages;
			// Complex calculation for ramp tiberium frame
			frameIndex = tiberium->Image->ArrayIndex +
						tiberium->int_field_EC / 4 * ((unsigned char)Ramp - 1) +
						Position.X * Position.Y % (tiberium->int_field_EC / 4);
		}
		else
		{
			// Flat tiberium frame calculation
			frameIndex = Position.X * Position.Y % tiberium->__NumImages;
			numImages = tiberium->Image->ArrayIndex;
		}

		// Get the correct overlay type and image data for this tiberium frame
		imageData = OverlayTypes[numImages + frameIndex]->Get_Image_Data();

		if (Ramp)
		{
			CC_Draw_Shape(
				&TempSurface->xs.s,
				VoxelDrawer,
				(ShapeCache*)imageData,
				OverlayData,
				&drawPos,
				&viewport,
				BF_USE_ZBUFFER | BF_ALPHA | BF_400 | BF_CENTER,
				0,
				-2 - zOffset,
				0,
				1000,
				0,
				ramvals_AA105C[(unsigned char)Ramp],
				0, 0, 0);
			return;
		}

		// Flat tiberium
		CC_Draw_Shape(
			&TempSurface->xs.s,
			VoxelDrawer,
			(ShapeCache*)imageData,
			OverlayData,
			&drawPos,
			&viewport,
			BF_USE_ZBUFFER | BF_ALPHA | BF_400 | BF_CENTER,
			0,
			-2 - zOffset,
			0,
			1000,
			0, 0, 0, 0, 0);
		return;
	}

	// Handle wall overlays
	if (overlayType->IsWall)
	{
		CC_Draw_Shape(
			&TempSurface->xs.s,
			&ColorSchemes.Vector[PlayerPtr->RemapColor]->Drawer->cv,
			(ShapeCache*)imageData,
			OverlayData,
			&drawPos,
			&viewport,
			BF_USE_ZBUFFER | BF_ALPHA | BF_400 | BF_CENTER,
			0,
			-2 - zOffset,
			2,
			Color1.r,
			0, 0, 0, 0, 0);
		return;
	}

	// Handle OVERLAY_DUMMYOLD type
	if (overlayType->Type == OVERLAY_DUMMYOLD)
	{
		if (Ramp)
		{
			CC_Draw_Shape(
				&TempSurface->xs.s,
				VoxelDrawer,
				(ShapeCache*)imageData,
				OverlayData,
				&drawPos,
				&viewport,
				BF_USE_ZBUFFER | BF_ALPHA | BF_400 | BF_CENTER,
				0,
				-2 - zOffset,
				0,
				Color1.g,
				0,
				ramvals_AA105C[(unsigned char)Ramp],
				0,
				30,
				-2);
			return;
		}

		// Flat dummy overlay
		CC_Draw_Shape(
			&TempSurface->xs.s,
			VoxelDrawer,
			(ShapeCache*)imageData,
			OverlayData,
			&drawPos,
			&viewport,
			BF_USE_ZBUFFER | BF_ALPHA | BF_400 | BF_CENTER,
			0,
			-2 - zOffset,
			0,
			Color1.g,
			0, 0, 0, 0, 0);
		return;
	}

	// Handle generic overlays (crates, rubble, rocks, etc.)
	int verticalOffset = overlayType->DrawFlat ? 0 : -15;

	if (overlayType->IsARock)
	{
		verticalOffset = 0;
	}

	int zParameter = overlayType->DrawFlat ? 0 : 2;

	// Handle crates
	if (overlayType->IsCrate)
	{
		CC_Draw_Shape(
			&TempSurface->xs.s,
			&TileDrawer->cv,
			(ShapeCache*)imageData,
			0,
			&drawPos,
			&viewport,
			BF_USE_ZBUFFER | BF_ALPHA | BF_400 | BF_CENTER,
			0,
			verticalOffset - zOffset - 2,
			zParameter,
			Color1.g,
			0, 0, 0, 0, 0);
		return;
	}

	// Handle rubble
	if (overlayType->IsRubble)
	{
		BuildingTypeClass* rubbleBuilding = (BuildingTypeClass*)Rubble;
		if (rubbleBuilding)
		{
			// The decompiler shows these are passed as pointers and modified
			OverlayTypeClass* rubbleOverlayType = overlayType;
			int rubbleFrame = yOffset; // a6 parameter reused

			if (BuildingTypeClass::Can_Leave_Rubble(rubbleBuilding, (int*)&rubbleOverlayType, &rubbleFrame))
			{
				CC_Draw_Shape(
					&TempSurface->xs.s,
					&TileDrawer->cv,
					(ShapeCache*)rubbleOverlayType,
					rubbleFrame,
					&drawPos,
					&viewport,
					BF_USE_ZBUFFER | BF_ALPHA | BF_400 | BF_CENTER,
					0,
					verticalOffset - zOffset - 2,
					zParameter,
					Color1.g,
					0, 0, 0, 0, 0);
			}
		}
		return;
	}

	// Draw standard overlay
	CC_Draw_Shape(
		&TempSurface->xs.s,
		&TileDrawer->cv,
		(ShapeCache*)imageData,
		OverlayData,
		&drawPos,
		&viewport,
		BF_USE_ZBUFFER | BF_ALPHA | BF_400 | BF_CENTER,
		0,
		verticalOffset - zOffset - 2,
		zParameter,
		Color1.g,
		0, 0, 0, 0, 0);
}
*/

ASMJIT_PATCH(0x480EA8, CellClass_DamageWall_AdjacentWallDamage, 0x5)
{
	GET(CellClass*, pThis, EAX);
	pThis->ReduceWall(RulesExtData::Instance()->AdjacentWallDamage);

	if (pThis->OverlayTypeIndex == -1)
		TechnoClass::ClearWhoTargetingThis(pThis);

	return 0x480EB4;
}

ASMJIT_PATCH(0x480E27, CellClass_DamageWall_DamageWallRecursivly, 0x5)
{
	enum { SkipGameCode = 0x480EBC };
	return RulesExtData::Instance()->DamageWallRecursivly ? 0 : SkipGameCode;
}

ASMJIT_PATCH(0x47FDF9, CellClass_GetOverlayShadowRect, 0xA)
{
	GET(CellClass*, pThis, EDI);
	GET(OverlayTypeClass*, pOverlay, ESI);

	if (pOverlay->Tiberium)
		pOverlay = OverlayTypeClass::Array->Items[CellExtData::GetOverlayIndex(pThis)];

	R->EBX(pOverlay->GetImage());

	return 0x47FE05;
}

ASMJIT_PATCH(0x47F641, CellClass_DrawShadow_Tiberium, 0x6)
{
	enum { SkipDrawing = 0x47F637, ContinueDrawing = 0x0 };
	GET(CellClass*, pThis, ESI);

	return (TiberiumClass::FindIndex(pThis->OverlayTypeIndex) >= 0) ? SkipDrawing : ContinueDrawing;
}

ASMJIT_PATCH(0x47F852, CellClass_DrawOverlay_Tiberium_, 0x6) // B
{
	GET(FakeCellClass*, pThis, ESI);
	GET(OverlayTypeClass*, pOverlay, EBX);

	if (!pOverlay->Tiberium) {
		return 0x47F96A;
	}

	const auto pTiberium = CellExtData::GetTiberium(pThis);

	if (!pTiberium) {
		return 0x47FB86;
	}

	GET_STACK(Point2D, nPos, 0x14);
	GET(RectangleStruct*, pBound, EBP);
	//GET(int, zOffs, EDI);
	GET_STACK(int, zOffs, 0x28);

	const auto pTibExt = TiberiumExtContainer::Instance.Find(pTiberium);

	ConvertClass* pDecided = FileSystem::x_PAL();
	if (const auto pCustom = pTibExt->Palette.GetConvert()) {
		pDecided = pCustom;
	}

	SHPStruct* pZShape = nullptr;
	if (auto nSlope = (int)pThis->SlopeIndex)
		pZShape = IsometricTileTypeClass::SlopeZshape[pThis->SlopeIndex];	//this is just pointers to files in ram, no vector #tomsons26

	//if (!pTibExt->EnableLighningFix.Get()) {
	//	int numImages;
	//	int frameIndex;
	//
	//	// Handle ramp tiberium
	//	if (pThis->SlopeIndex) {
	//		numImages = pTiberium->NumImages;
	//		// Complex calculation for ramp tiberium frame
	//		frameIndex = pTiberium->Image->ArrayIndex +
	//			pTiberium->SlopeFrames / 4 * ((unsigned char)pThis->SlopeIndex - 1) +
	//			nPos.X * nPos.Y % (pTiberium->SlopeFrames / 4);
	//	} else {
	//		// Flat tiberium frame calculation
	//		frameIndex = nPos.X * nPos.Y % pTiberium->NumImages;
	//		numImages = pTiberium->Image->ArrayIndex;
	//	}
	//
	//	// Get the correct overlay type and image data for this tiberium frame
	//	SHPStruct* imageData = OverlayTypeClass::Array->Items[numImages + frameIndex]->GetImage();
	//
	//	if (!imageData) {
	//		return 0x47FB86;
	//	}
	//
	//	if (pThis->SlopeIndex)
	//	{
	//		DSurface::Temp->DrawSHP(
	//			pDecided,
	//			imageData,
	//			pThis->OverlayData,
	//			&nPos,
	//			pBound,
	//			BlitterFlags(0x4E00),
	//			0,
	//			-2 - zOffs,
	//			ZGradient::Ground,
	//			1000,
	//			0,
	//			pZShape,
	//			0, 0, 0);
	//
	//		return 0x47FB86;
	//	}
	//
	//	// Flat tiberium
	//	DSurface::Temp->DrawSHP(
	//		pDecided,
	//		imageData,
	//		pThis->OverlayData,
	//		&nPos,
	//		pBound,
	//		BlitterFlags(0x4E00),
	//		0,
	//		-2 - zOffs,
	//		ZGradient::Ground,
	//		1000,
	//		0, 0, 0, 0, 0);
	//
	//	return 0x47FB86;
	//}

	auto nIndex = CellExtData::GetOverlayIndex(pThis, pTiberium);
	const auto pShape = OverlayTypeClass::Array->Items[nIndex]->GetImage();

	if (!pShape) {
		return 0x47FB86;
	}

	const auto nZAdjust = -2 - zOffs;
	auto nTint = pTibExt->Ore_TintLevel.Get(pTibExt->UseNormalLight.Get() ? 1000 : pThis->Color1.Green);
	const int nOreTint = std::clamp(nTint, 0, 1000);

	DSurface::Temp->DrawSHP(pDecided, pShape, pThis->OverlayData, &nPos, pBound, BlitterFlags(0x4E00), 0, nZAdjust, ZGradient::Ground, nOreTint, 0, pZShape, 0, 0, 0);

	if(pTibExt->EnableLighningFix){
		auto nShadowFrame = (nIndex + pShape->Frames / 2);
		DSurface::Temp->DrawSHP(pDecided, pShape, nShadowFrame, &nPos, pBound, BlitterFlags(0x4E01), 0, nZAdjust, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
	}

	return 0x47FB86;
}

ASMJIT_PATCH(0x47F661, CellClass_DrawOverlay_Rubble_Shadow, 0x8)
{
	GET(CellClass*, pCell, ESI);
	GET_STACK(SHPStruct*, pImage, STACK_OFFSET(0x28, 0x8));
	GET_STACK(int, nFrame, STACK_OFFSET(0x28, 0x4));
	LEA_STACK(Point2D*, pPoint, STACK_OFFS(0x28, 0x10));
	GET_STACK(int, nOffset, STACK_OFFS(0x28, 0x18));
	GET(RectangleStruct*, pRect, EBX);

	if (R->AL()) {
		auto const pBTypeExt = BuildingTypeExtContainer::Instance.Find(pCell->Rubble);

		ConvertClass* pDecided = pCell->LightConvert;
		if (const auto pCustom = pBTypeExt->RubblePalette.GetConvert()) {
			pDecided = pCustom;
		}

		auto const zAdjust = - 2 - nOffset;

		DSurface::Temp()->DrawSHP(pDecided, pImage, nFrame, pPoint, pRect, BlitterFlags(0x4601),
		0, zAdjust, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
	}
	return 0x47F637;
}

ASMJIT_PATCH(0x47FADB, CellClass_DrawOverlay_Rubble, 0x5)
{
	GET(OverlayTypeClass*, pOvl, ECX);
	GET(CellClass*, pCell, ESI);
	LEA_STACK(SHPStruct**, pImage, STACK_OFFS(0x24, 0x14));
	LEA_STACK(int*, pFrame, STACK_OFFSET(0x24, 0x8));
	LEA_STACK(Point2D*, pPoint, STACK_OFFS(0x24, 0x10));
	GET_STACK(int, nOffset, STACK_OFFSET(0x24, 0x4));
	GET(RectangleStruct*, pRect, EBP);
	GET(int, nVal, EDI);

	if (auto const pRubble = pCell->Rubble) {
		if (pRubble->CanLeaveRubble(pImage, pFrame)) {
			auto const pBTypeExt = BuildingTypeExtContainer::Instance.Find(pRubble);
			ConvertClass* pDecided = pCell->LightConvert;
			if (const auto pCustom = pBTypeExt->RubblePalette.GetConvert()) {
				pDecided = pCustom;
			}

			const auto zAdjust = nVal - nOffset - 2;

			DSurface::Temp()->DrawSHP(pDecided, *pImage, *pFrame, pPoint, pRect, BlitterFlags(0x4E00),
			0, zAdjust, pOvl->DrawFlat != 0 ? ZGradient::Ground : ZGradient::Deg90, pCell->Color1.Green, 0, nullptr, 0, 0, 0);

		}
	}

	return 0x47FB86;
}

#include <TerrainTypeClass.h>
#include <TerrainClass.h>

bool FakeCellClass::_CanTiberiumGerminate(TiberiumClass* tiberium)
{
	if (!MapClass::Instance->IsWithinUsableArea(this->MapCoords, true)) return false;

	if (this->ContainsBridgeEx()) return false;

	/*
	**  Don't allow Tiberium to grow on a cell with a building unless that building is
	**  invisible. In such a case, the Tiberium must grow or else the location of the
	**  building will be revealed.
	*/
	BuildingClass const* building = this->GetBuilding();
	if (building && building->Health > 0 && !building->Type->Invisible && !building->Type->InvisibleInGame) return false;

	TerrainClass* terrain = this->GetTerrain(false);
	if (terrain && terrain->Type->SpawnsTiberium) return false;

	if (!GroundType::Get(this->LandType)->Build) return false;

	if (this->OverlayTypeIndex != -1 || this->SlopeIndex > 0) return false;

	if (this->IsoTileTypeIndex >= 0 && this->IsoTileTypeIndex < IsometricTileTypeClass::Array->Count)
	{
		IsometricTileTypeClass* ittype = IsometricTileTypeClass::Array->Items[this->IsoTileTypeIndex];
		if (!ittype->AllowTiberium) return false;

		const auto ittype_ext = IsometricTileTypeExtContainer::Instance.Find(ittype);

		if (!ittype_ext->AllowedTiberiums.empty() && !ittype_ext->AllowedTiberiums.Contains(tiberium)) return false;
	}

	return true;

}

bool FakeCellClass::_CanPlaceVeins()
{
	if (this->SlopeIndex <= 4)
	{
		if (this->LandType != LandType::Water
			&& this->LandType != LandType::Rock
			&& this->LandType != LandType::Ice
			&& this->LandType != LandType::Beach)
		{
			if (this->OverlayTypeIndex == -1 || OverlayTypeClass::Array->Items[this->OverlayTypeIndex]->IsVeins)
			{
				int ittype = this->IsoTileTypeIndex;
				if (ittype < 0 || ittype >= IsometricTileTypeClass::Array->Count) {
					ittype = 0;
				}

				const auto isotype_ext = IsometricTileTypeExtContainer::Instance.Find(IsometricTileTypeClass::Array->Items[ittype]);

				if (isotype_ext->AllowVeins)
				{
					for (int dir = 0; dir < 8; dir += 2)
					{
						CellStruct adjacent;
						MapClass::GetAdjacentCell(&adjacent, &this->MapCoords, static_cast<FacingType>(dir));
						auto adjacent_cell = MapClass::Instance->TryGetCellAt(adjacent);

						if (adjacent_cell->SlopeIndex > 4 && this->SlopeIndex == 0)
						{
							if (adjacent_cell->OverlayTypeIndex == -1 || !OverlayTypeClass::Array->Items[adjacent_cell->OverlayTypeIndex]->IsVeins)
							{
								return false;
							}
						}

						if (adjacent_cell->LandType == LandType::Water
							|| adjacent_cell->LandType == LandType::Rock
							|| adjacent_cell->LandType == LandType::Ice
							|| adjacent_cell->LandType == LandType::Beach)
						{
							return false;
						}

						if (adjacent_cell->OverlayTypeIndex != -1 && !OverlayTypeClass::Array->Items[adjacent_cell->OverlayTypeIndex]->IsVeins)
						{
							return false;
						}
					}
					return true;
				}
			}
		}
	}
	return false;
}

//DEFINE_FUNCTION_JUMP(LJMP, 0x4838E0, FakeCellClass::_CanTiberiumGerminate);

//seems causing large FPS drop
//ASMJIT_PATCH(0x6D7A46, TacticalClass_DrawPixelFX_Tiberium, 0x7)
//{
//	GET(CellClass*, pCell, ESI);
//
//	bool bDraw = false;
//
//	if (const auto pTiberium = CellExtData::GetTiberium(pCell)) {
//		if (TiberiumExtContainer::Instance.Find(pTiberium)->EnablePixelFXAnim)
//			bDraw = pTiberium->Value;
//	}
//
//	R->EAX(bDraw);
//	return 0x6D7A4D;
//}


/*
*    v3 = this->TileType;
	if ( v3 >= 0 && v3 < (int)*(&IsometricTileTypes + 4) && !(*(&IsometricTileTypes + 1))[v3]->AllowBurrowing
*/
//ASMJIT_PATCH(0x487022, CellClass_CanEnterCell_Add, 0x6)
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

//ASMJIT_PATCH(0x4D9C41, FootClass_CanEnterCell_Restricted, 0x6)
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
//ASMJIT_PATCH(0x4196B0, AircraftClass_CanEnterCell_Restricted, 0x5)
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
//ASMJIT_PATCH(0x52C046, InitGame_CreateTiberiumDrawer, 0x5)
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
//ASMJIT_PATCH(0x53AD00, ScenarioClass_RecalcLighting_TintTiberiumDrawer, 5)
//{
//	GET(int, red, ECX);
//	GET(int, green, EDX);
//	GET_STACK(int, blue, STACK_OFFSET(0x0, 0x4));
//	GET_STACK(bool, tint, STACK_OFFSET(0x0, 0x8));
//	SpawnTiberiumTreeConvert->UpdateColors(red, green, blue, tint);
//	return 0;
//}
//
//ASMJIT_PATCH(0x71C294, TerrainClass_DrawIt_TiberiumSpawn_Palette, 0x6)
//{
//	R->EDX(SpawnTiberiumTreeConvert.get());
//	return 0x71C29A;
//}