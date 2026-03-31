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
