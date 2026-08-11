#include "Body.h"

#include <RectangleStruct.h>
#include <CellStruct.h>
#include <Unsorted.h>

#include <Ext/BuildingType/Body.h>

ASMJIT_PATCH(0x6D50FB , TacticalClass_DrawPlacement_CustomFoundation, 0x5)
{
	RectangleStruct bounds {};
	const bool bOnFB = R->Origin() == 0x6D50FB;

	// Get bounding rectangle of foundation cells
	BuildingTypeExtData::GetDisplayRect(&bounds, (!bOnFB ?
		Unsorted::CursorSizeSecond() : Unsorted::CursorSize()));

	// Calculate actual dimensions
    // bounds.Width is MaxX, bounds.X is MinX
    // bounds.Height is MaxY, bounds.Y is MinY
    CellStruct size{
		.X = (short)std::max(0, bounds.Width - bounds.X) + 1,   // Width = MaxX - MinX + 1
		.Y = (short)std::max(0, bounds.Height - bounds.Y) + 1  // Height = MaxY - MinY + 1
	};

    CellStruct origin_cell {
		.X = (short)bounds.X ,// MinX
    	.Y = (short)bounds.Y  // MinY
	};

	R->Stack(0x14, origin_cell.Pack());
	R->Stack(0x18, size.Pack());
	R->EAX(size.Pack());
	R->ESI(bounds.Y);

	return (!bOnFB) ? 0x6D558F : 0x6D5116 ;
}ASMJIT_PATCH_AGAIN(0x6D5573, TacticalClass_DrawPlacement_CustomFoundation, 0x6)


#include <DisplayClass.h>
#include <TacticalClass.h>

enum class BuildingRangeMode
{
	NONE = 0, LINE = 1, CELL = 2, SHP = 3
};

bool InRect(Point2D point, RectangleStruct bound)
{
	return point.X >= bound.X && point.X <= bound.X + bound.Width && point.Y >= bound.Y && point.Y <= bound.Y + bound.Height;
}

bool DrawLine(DSurface* pSurface,
	Point2D point1, Point2D point2, int dwColor, RectangleStruct bound)
{
	if (bound.IsEmpty()) {
		bound = pSurface->Get_Rect();
	}

	Game::Clip_Line(&point1, &point2, &bound);
	// point in rect then draw
	if (InRect(point1, bound) && InRect(point2, bound)) {
		return pSurface->Draw_Line(point1, point2, dwColor);
	}
	return false;
}

bool DrawDashedLine(DSurface* pSurface,
	Point2D point1, Point2D point2, int dwColor, RectangleStruct bound,
	bool blink = false)
{
	if (bound.IsEmpty()) {
		bound = pSurface->Get_Rect();
	}

	Game::Clip_Line(&point1, &point2, &bound);
	// point in rect then draw
	if (InRect(point1, bound) && InRect(point2, bound)) {
		int offset = 0;
		if (blink) {
			offset = 7 * Unsorted::CurrentFrame() % 16;
		}
		return pSurface->DrawDashedLine(point1, point2, dwColor, offset);
	}

	return false;
}

void DrawTheMarker(
	BuildingTypeClass* pBuildingType,
	BuildingRangeMode mode, 
	ColorStruct col ,
	bool dashed,
	SHPCaches* pShape,
	int ZeroFrameIndex
)
{
	switch (mode)
	{
	case BuildingRangeMode::NONE:
		return;


	case BuildingRangeMode::LINE:
	{
		int width = pBuildingType->GetFoundationWidth();
		int height = pBuildingType->GetFoundationHeight(false);
		CellStruct zoneCell = Unsorted::Display_ZoneCell();
		CellStruct zoneOffset = Unsorted::Display_ZoneOffset();
		CellStruct center = zoneCell + zoneOffset;
		int cellX = center.X;
		int cellY = center.Y;
		int adjust = pBuildingType->Adjacent + 1;
		// 北
		CellStruct nCell { static_cast<short>(cellX - adjust), static_cast<short>(cellY - adjust) };
		// 东
		CellStruct eCell { static_cast<short>(cellX + adjust + width - 1), static_cast<short>(cellY - adjust) };
		// 南
		CellStruct sCell { static_cast<short>(cellX + adjust + width - 1), static_cast<short>(cellY + adjust + height - 1) };
		// 西
		CellStruct wCell { static_cast<short>(cellX - adjust), static_cast<short>(cellY + adjust + height - 1) };
		// 可视范围
		DSurface* pSurface = DSurface::Temp;
		RectangleStruct rect = pSurface->Get_Rect();
		rect.Height -= 34;
		int color = col.ToInit();

		// 北
		CoordStruct nPos = CellClass::Cell2Coord(nCell);
		if (CellClass* pNCell = MapClass::Instance->TryGetCellAt(nCell))
		{
			nPos = pNCell->GetCenterCoords();
		}
		nPos.X -= 128;
		nPos.Y -= 128;
		Point2D n = TacticalClass::Instance->CoordsToScreen(nPos);
		// 东
		CoordStruct ePos = CellClass::Cell2Coord(eCell);
		if (CellClass* pECell = MapClass::Instance->TryGetCellAt(eCell))
		{
			ePos = pECell->GetCenterCoords();
		}
		ePos.X += 128;
		ePos.Y -= 128;
		Point2D e = TacticalClass::Instance->CoordsToScreen(ePos);
		// 南
		CoordStruct sPos = CellClass::Cell2Coord(sCell);
		if (CellClass* pSCell = MapClass::Instance->TryGetCellAt(sCell))
		{
			sPos = pSCell->GetCenterCoords();
		}
		sPos.X += 128;
		sPos.Y += 128;
		Point2D s = TacticalClass::Instance->CoordsToScreen(sPos);
		// 西
		CoordStruct wPos = CellClass::Cell2Coord(wCell);
		if (CellClass* pWCell = MapClass::Instance->TryGetCellAt(wCell))
		{
			wPos = pWCell->GetCenterCoords();
		}
		wPos.X -= 128;
		wPos.Y += 128;
		Point2D w = TacticalClass::Instance->CoordsToScreen(wPos);
		if (dashed)
		{
			// 处理四角越界并绘制
			DrawDashedLine(pSurface, n, e, color, rect);
			DrawDashedLine(pSurface, e, s, color, rect);
			DrawDashedLine(pSurface, s, w, color, rect);
			DrawDashedLine(pSurface, w, n, color, rect);
		}
		else
		{
			// 处理四角越界并绘制
			DrawLine(pSurface, n, e, color, rect);
			DrawLine(pSurface, e, s, color, rect);
			DrawLine(pSurface, s, w, color, rect);
			DrawLine(pSurface, w, n, color, rect);
		}

		return;

	}
	case BuildingRangeMode::CELL:
	{
		int width = pBuildingType->GetFoundationWidth();
		int height = pBuildingType->GetFoundationHeight(false);
		CellStruct zoneCell = Unsorted::Display_ZoneCell();
		CellStruct zoneOffset = Unsorted::Display_ZoneOffset();
		CellStruct center = zoneCell + zoneOffset;
		int cellX = center.X;
		int cellY = center.Y;
		int adjust = pBuildingType->Adjacent + 1;
		// 北
		CellStruct nCell { static_cast<short>(cellX - adjust), static_cast<short>(cellY - adjust) };
		// 东
		CellStruct eCell { static_cast<short>(cellX + adjust + width - 1), static_cast<short>(cellY - adjust) };
		// 南
		CellStruct sCell { static_cast<short>(cellX + adjust + width - 1), static_cast<short>(cellY + adjust + height - 1) };
		// 西
		CellStruct wCell { static_cast<short>(cellX - adjust), static_cast<short>(cellY + adjust + height - 1) };
		// 可视范围
		DSurface* pSurface = DSurface::Temp;
		RectangleStruct rect = pSurface->Get_Rect();
		rect.Height -= 34;
		int color = col.ToInit();

		// 有效范围
		int minX = nCell.X;
		int minY = nCell.Y;
		int maxX = eCell.X;
		int maxY = wCell.Y;
		// 可视范围
		int minVX = rect.X;
		int maxVX = rect.X + rect.Width;
		int minVY = rect.Y;
		int maxVY = rect.Y + rect.Height;
		// Logger.Log($"{Game.CurrentFrame} 可视范围 [{minVX} - {maxVX}], [{minVY} - {maxVY}]");
		// 获取所有的Cell，捡出在视野范围内的Cell
		std::set<CellClass*> cells {};
		for (int y = minY; y <= maxY; y++)
		{
			for (int x = minX; x <= maxX; x++)
			{
				CellStruct cellPos { static_cast<short>(x), static_cast<short>(y) };
				CoordStruct location = CellClass::Cell2Coord(cellPos);
				Point2D point = TacticalClass::Instance->CoordsToScreen(location);
				// 在可视范围内
				if (point.X >= minVX && point.X <= maxVX && point.Y >= minVY && point.Y <= maxVY)
				{
					if (CellClass* pCell = MapClass::Instance->TryGetCellAt(cellPos))
					{
						cells.insert(pCell);
					}
				}
			}
		}

		for (CellClass* pCell : cells)
		{
			if (pCell->SlopeIndex == 0)
			{
				CoordStruct cellPos = pCell->GetCoordsWithBridge();
				CoordStruct pE = cellPos + CoordStruct { 128, -128, 0 };
				Point2D e = TacticalClass::Instance->CoordsToScreen(pE);
				CoordStruct pW = cellPos + CoordStruct { -128, 128, 0 };
				Point2D w = TacticalClass::Instance->CoordsToScreen(pW);
				CoordStruct pN = cellPos + CoordStruct { -128, -128, 0 };
				Point2D n = TacticalClass::Instance->CoordsToScreen(pN);
				CoordStruct pS = cellPos + CoordStruct { 128, 128, 0 };
				Point2D s = TacticalClass::Instance->CoordsToScreen(pS);
				if (dashed)
				{
					// 处理四角越界并绘制
					DrawDashedLine(pSurface, n, e, color, rect);
					DrawDashedLine(pSurface, e, s, color, rect);
					DrawDashedLine(pSurface, s, w, color, rect);
					DrawDashedLine(pSurface, w, n, color, rect);
				}
				else
				{
					// 处理四角越界并绘制
					DrawLine(pSurface, n, e, color, rect);
					DrawLine(pSurface, e, s, color, rect);
					DrawLine(pSurface, s, w, color, rect);
					DrawLine(pSurface, w, n, color, rect);
				}
			}
		}

		return;

	}
	case BuildingRangeMode::SHP:
	{
		if (!pShape)
			return;

		int width = pBuildingType->GetFoundationWidth();
		int height = pBuildingType->GetFoundationHeight(false);
		CellStruct zoneCell = Unsorted::Display_ZoneCell();
		CellStruct zoneOffset = Unsorted::Display_ZoneOffset();
		CellStruct center = zoneCell + zoneOffset;
		int cellX = center.X;
		int cellY = center.Y;
		int adjust = pBuildingType->Adjacent + 1;
		// 北
		CellStruct nCell { static_cast<short>(cellX - adjust), static_cast<short>(cellY - adjust) };
		// 东
		CellStruct eCell { static_cast<short>(cellX + adjust + width - 1), static_cast<short>(cellY - adjust) };
		// 南
		CellStruct sCell { static_cast<short>(cellX + adjust + width - 1), static_cast<short>(cellY + adjust + height - 1) };
		// 西
		CellStruct wCell { static_cast<short>(cellX - adjust), static_cast<short>(cellY + adjust + height - 1) };
		// 可视范围
		DSurface* pSurface = DSurface::Temp;
		RectangleStruct rect = pSurface->Get_Rect();
		rect.Height -= 34;

		// 有效范围
		int minX = nCell.X;
		int minY = nCell.Y;
		int maxX = eCell.X;
		int maxY = wCell.Y;
		// 可视范围
		int minVX = rect.X;
		int maxVX = rect.X + rect.Width;
		int minVY = rect.Y;
		int maxVY = rect.Y + rect.Height;
		// Logger.Log($"{Game.CurrentFrame} 可视范围 [{minVX} - {maxVX}], [{minVY} - {maxVY}]");
		// 获取所有的Cell，捡出在视野范围内的Cell
		std::set<CellClass*> cells {};
		for (int y = minY; y <= maxY; y++)
		{
			for (int x = minX; x <= maxX; x++)
			{
				CellStruct cellPos { static_cast<short>(x), static_cast<short>(y) };
				CoordStruct location = CellClass::Cell2Coord(cellPos);
				Point2D point = TacticalClass::Instance->CoordsToScreen(location);
				// 在可视范围内
				if (point.X >= minVX && point.X <= maxVX && point.Y >= minVY && point.Y <= maxVY)
				{
					if (CellClass* pCell = MapClass::Instance->TryGetCellAt(cellPos))
					{
						cells.insert(pCell);
					}
				}
			}
		}

		ConvertClass* pPalette = FileSystem::PALETTE_PAL;
		for (CellClass* pCell : cells) {
			// WWSB
			CellStruct cell = pCell->MapCoords;
			CoordStruct newPos { ((((cell.X << 8) + 128) / 256) << 8), ((((cell.Y << 8) + 128) / 256) << 8), 0 };
			Point2D position = TacticalClass::Instance->CoordsToScreen(newPos);
			position -= TacticalClass::Instance->TacticalPos;
			int zAdjust = 15 * pCell->Level;
			position.Y += -1 - zAdjust;
			int frame = pCell->SlopeIndex + 2;
			// 显示对应的帧
			pSurface->DrawSHP(pPalette, pShape, ZeroFrameIndex + frame, &position);
		}
	}
	default:
		return;
	}
}

ASMJIT_PATCH(0x6D5116, TacticalClass_Draw_Placement_Recheck, 0x5)
{
	ObjectTypeClass* pBuildingType = DisplayClass::Instance->CurrentBuildingType;

	if (pBuildingType->WhatAmI() == AbstractType::BuildingType) {	
		DrawTheMarker((BuildingTypeClass*)pBuildingType, BuildingRangeMode::LINE, ColorStruct::White, true, nullptr, 0);
		Unsorted::Display_PassedProximityCheck = DisplayClass::Instance->PassesProximityCheck();
	}

	return 0;
}