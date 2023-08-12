#include "Body.h"


void TacticalExt::DrawDebugOverlay()
{
	RGBClass rgb_black(0, 0, 0);
	unsigned color_black = DSurface::RGB_To_Pixel(0, 0, 0);
	ColorScheme* text_color = ColorScheme::Find("White");

	int padding = 2;

	wchar_t buffer[256];
	_swprintf(buffer,
		L"[%s] %3d %3d 0x%08X",
		strupr(ScenarioGlobal->FileName),
		SessionGlobal.DesiredFrameRate,
		FPSCounter::CurrentFrameRate(),
		ObjectClass::CurrentObjects->Count == 1 ? ObjectClass::CurrentObjects->Items[0] : 0
	);

	/**
	 * Fetch the text occupy area.
	 */
	Rect text_rect = Drawing::GetTextDimensions(buffer);

	/**
	 *  Fill the background area.
	 */
	Rect fill_rect;
	fill_rect.X = 160; // Width of Options tab, so we draw from there.
	fill_rect.Y = 0;
	fill_rect.Width = text_rect.Width + (padding + 1);
	fill_rect.Height = 16; // Tab bar height
	auto const pSurface = DSurface::Composite();
	pSurface->Fill_Rect(fill_rect, color_black);

	/**
	 *  Move rects into position.
	 */
	text_rect.X = fill_rect.X + padding;
	text_rect.Y = 0;
	text_rect.Width += padding;
	text_rect.Height += 3;

	/**
	 *  Draw the overlay text.
	 */
	Point2D nDummy { };
	Point2D nLoc { text_rect.X, text_rect.Y };
	auto nSurfaceRect = pSurface->Get_Rect();
	Fancy_Text_Print_Wide(&nDummy, buffer, pSurface, &nSurfaceRect,
		&nLoc, text_color, 0, TextPrintType::Point6Grad | TextPrintType::NoShadow);

	/**
	 *  Draw the current frame number.
	 */
	_swprintf(buffer, L"%d", Unsorted::CurrentFrame);
	text_rect = Drawing::GetTextDimensions(buffer);

	fill_rect.Width = text_rect.Width + (padding + 1);
	fill_rect.Height = 16;
	fill_rect.X = pSurface->Get_Width() - fill_rect.Width;
	fill_rect.Y = 0;
	pSurface->Fill_Rect(fill_rect, color_black);

	text_rect.X = pSurface->Get_Width();
	text_rect.Y = 0;
	text_rect.Width += padding;
	text_rect.Height += 3;

	nSurfaceRect = pSurface->Get_Rect();
	nLoc.X = text_rect.X;
	nLoc.Y = text_rect.Y;

	Fancy_Text_Print_Wide(&nDummy, buffer, pSurface, &nSurfaceRect,
		&nLoc, text_color, 0, TextPrintType::Right | TextPrintType::Point6Grad | TextPrintType::NoShadow);
}

bool TacticalExt::DrawCurrentCell()
{
	unsigned color_yellow = DSurface::RGB_To_Pixel(255, 255, 0);

	ColorScheme* text_color = ColorScheme::Find("White");

	static unsigned r;
	static unsigned g;
	static unsigned b;
	static unsigned w;
	static unsigned x;

	/**
	*  Colours taken from CELLSEL.SHP, each entry matches a cell level.
	*/
	static unsigned _CellLevelColors[16];
	static bool _onetime = false;

	if (!_onetime)
	{
		_CellLevelColors[0] = DSurface::RGB_To_Pixel(255, 255, 255);  // 0
		_CellLevelColors[1] = DSurface::RGB_To_Pixel(170, 0, 170);    // 1
		_CellLevelColors[2] = DSurface::RGB_To_Pixel(0, 170, 170);    // 2
		_CellLevelColors[3] = DSurface::RGB_To_Pixel(0, 170, 0);      // 3
		_CellLevelColors[4] = DSurface::RGB_To_Pixel(89, 255, 85);    // 4
		_CellLevelColors[5] = DSurface::RGB_To_Pixel(255, 255, 85);   // 5
		_CellLevelColors[6] = DSurface::RGB_To_Pixel(255, 85, 85);    // 6
		_CellLevelColors[7] = DSurface::RGB_To_Pixel(170, 85, 0);     // 7
		_CellLevelColors[8] = DSurface::RGB_To_Pixel(170, 0, 0);      // 8
		_CellLevelColors[9] = DSurface::RGB_To_Pixel(85, 255, 255);   // 9
		_CellLevelColors[10] = DSurface::RGB_To_Pixel(80, 80, 255);   // 10
		_CellLevelColors[11] = DSurface::RGB_To_Pixel(0, 0, 170);     // 11
		_CellLevelColors[12] = DSurface::RGB_To_Pixel(0, 0, 0);       // 12
		_CellLevelColors[13] = DSurface::RGB_To_Pixel(85, 85, 85);    // 13
		_CellLevelColors[14] = DSurface::RGB_To_Pixel(170, 170, 170); // 14
		_CellLevelColors[15] = DSurface::RGB_To_Pixel(255, 255, 255); // 15

		r = DSurface::RGB_To_Pixel(255, 0, 0);
		g = DSurface::RGB_To_Pixel(0, 255, 0);
		b = DSurface::RGB_To_Pixel(0, 0, 255);
		w = DSurface::RGB_To_Pixel(255, 255, 255);
		x = DSurface::RGB_To_Pixel(255, 255, 0);
	}

	auto nMouseXY = Point2D { WWMouseClass::Instance->GetX() , WWMouseClass::Instance->GetY() };
	CellStruct cell {};
	Map.vt_entry_CC(&cell, nMouseXY);
	auto cellptr = Map[cell];

	if (!cellptr)
	{
		return false;
	}

	RectangleStruct cellrect = cellptr->GetTileRect();

	/**
	*  Get the center point of the cell.
	*/
	Point2D cell_center;
	//cell_center.X = cellrect.X + CELL_PIXEL_W/2;
	//cell_center.Y = cellrect.Y + CELL_PIXEL_H/2;
	cell_center.X = cellrect.X + cellrect.Width / 2;
	cell_center.Y = cellrect.Y + cellrect.Height / 2;

	/**
	*  Determine if the cell draw rect is within the viewport.
	*/
	Rect intersect = Drawing::Intersect(cellrect, TacticalClass::Instance->VisibleArea());
	if (!intersect.Is_Valid())
	{
		return false;
	}


	/**
	*  Fetch the highlight color based on cell height.
	*/
	unsigned color = _CellLevelColors[cellptr->Level];
	auto pSurface = DSurface::Temp();

#define CELL_PIXEL_W 48
#define CELL_PIXEL_H 24

	/**
	*  Draw the cell selection.
	*/
	enum class TileRampType : BYTE
	{
		/**
		 *  A flat tile (i.e. no ramp).
		 */
		RAMP_NONE = 0,

		/**
		 *  Basic, two adjacent corners raised.
		 */
		 RAMP_WEST = 1,      // The west corner of the tile is raised.
		 RAMP_NORTH = 2,     // The south corner of the tile is raised.
		 RAMP_EAST = 3,      // The east corner of the tile is raised.
		 RAMP_SOUTH = 4,     // The north corner of the tile is raised.

		 /**
		  *  Tile outside corners (One corner raised by half a cell).
		  */
		  RAMP_CORNER_NW = 5,
		  RAMP_CORNER_NE = 6,
		  RAMP_CORNER_SE = 7,
		  RAMP_CORNER_SW = 8,

		  /**
		   *  Tile inside corners (three corners raised by half a cell).
		   */
		   RAMP_MID_NW = 9,
		   RAMP_MID_NE = 10,
		   RAMP_MID_SE = 11,
		   RAMP_MID_SW = 12,

		   /**
			*  Full tile sloped (mid corners raised by half cell, far corner by full cell).
			*/
			RAMP_STEEP_SE = 13,     // Almost a invisible tile, but actually a fixup tile for ??.
			RAMP_STEEP_SW = 14,     // Full steep tile that faces NW (on the screen).
			RAMP_STEEP_NW = 15,     // Full steep tile that faces SE (on the screen).
			RAMP_STEEP_NE = 16,     // Full steep tile that faces SE (on the screen).

			/**
			 *  Double ramps (two corners raised, alternating).
			 */
			 RAMP_DOUBLE_UP_SW_NE = 17,      // Double ramp tile that slopes up, faces SW-NE (in the game world).
			 RAMP_DOUBLE_DOWN_SW_NE = 18,    // Double ramp tile that slopes down, faces SW-NE (in the game world).
			 RAMP_DOUBLE_UP_NW_SE = 19,      // Double ramp tile that slopes up, faces NW-SE (in the game world).
			 RAMP_DOUBLE_DOWN_NW_SE = 20,    // Double ramp tile that slopes down, faces NW-SE (in the game world).
	};

	Point2D nStart { };
	Point2D nEnd { };
	switch (static_cast<TileRampType>(cellptr->SlopeIndex))
	{

	default:
		break;

		/**
		*  No ramp is a flat tile.
		*/
	case TileRampType::RAMP_NONE:
	{
		nStart = Point2D(0, CELL_PIXEL_H / 2);
		nEnd = Point2D(CELL_PIXEL_H, 0);
		pSurface->Draw_Line_Rect(cellrect, nStart, nEnd, color);

		nStart = Point2D(CELL_PIXEL_H, 0);
		nEnd = Point2D(CELL_PIXEL_W, (CELL_PIXEL_H / 2));
		pSurface->Draw_Line_Rect(cellrect, nStart, nEnd, color);

		nStart = Point2D(CELL_PIXEL_W, (CELL_PIXEL_H / 2));
		nEnd = Point2D(CELL_PIXEL_H, CELL_PIXEL_H);
		pSurface->Draw_Line_Rect(cellrect, nStart, nEnd, color);

		nStart = Point2D(CELL_PIXEL_H, CELL_PIXEL_H);
		nEnd = Point2D(0, (CELL_PIXEL_H / 2));
		pSurface->Draw_Line_Rect(cellrect, nStart, nEnd, color);
		break;
	}
	case TileRampType::RAMP_WEST:
	{
		nStart = Point2D(0, (CELL_PIXEL_H / 2));
		nEnd = Point2D(CELL_PIXEL_H, 0);
		pSurface->Draw_Line_Rect(cellrect, nStart, nEnd, color);

		nStart = Point2D(CELL_PIXEL_H, 0);
		nEnd = Point2D(CELL_PIXEL_W, 0);
		pSurface->Draw_Line_Rect(cellrect, nStart, nEnd, color);

		nStart = Point2D(CELL_PIXEL_W, 0);
		nEnd = Point2D(CELL_PIXEL_H, (CELL_PIXEL_H / 2));
		pSurface->Draw_Line_Rect(cellrect, nStart, nEnd, color);

		nStart = Point2D(CELL_PIXEL_H, (CELL_PIXEL_H / 2));
		nEnd = Point2D(0, (CELL_PIXEL_H / 2));
		pSurface->Draw_Line_Rect(cellrect, nStart, nEnd, color);
		break;
	}
	case TileRampType::RAMP_NORTH:
	{
		nStart = Point2D(CELL_PIXEL_H, CELL_PIXEL_H);
		nEnd = Point2D(0, (CELL_PIXEL_H / 2));
		pSurface->Draw_Line_Rect(cellrect, Point2D(0, 0), Point2D(CELL_PIXEL_H, 0), r);

		nStart = Point2D(CELL_PIXEL_H, CELL_PIXEL_H);
		nEnd = Point2D(0, (CELL_PIXEL_H / 2));
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_H, 0), Point2D(CELL_PIXEL_W, (CELL_PIXEL_H / 2)), g);

		nStart = Point2D(CELL_PIXEL_H, CELL_PIXEL_H);
		nEnd = Point2D(0, (CELL_PIXEL_H / 2));
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_W, (CELL_PIXEL_H / 2)), Point2D(CELL_PIXEL_H, 12), b);

		nStart = Point2D(CELL_PIXEL_H, CELL_PIXEL_H);
		nEnd = Point2D(0, (CELL_PIXEL_H / 2));
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_H, (CELL_PIXEL_H / 2)), Point2D(0, 0), w);
		break;
	}
	case TileRampType::RAMP_EAST:
	{
		pSurface->Draw_Line_Rect(cellrect, Point2D(0, 0), Point2D(CELL_PIXEL_H, -(CELL_PIXEL_H / 2)), r);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_H, -(CELL_PIXEL_H / 2)), Point2D(CELL_PIXEL_W, (CELL_PIXEL_H / 2)), g);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_W, (CELL_PIXEL_H / 2)), Point2D(CELL_PIXEL_H, CELL_PIXEL_H), b);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_H, CELL_PIXEL_H), Point2D(0, 0), w);
		break;
	}
	case TileRampType::RAMP_SOUTH:
	{
		pSurface->Draw_Line_Rect(cellrect, Point2D(0, (CELL_PIXEL_H / 2)), Point2D(CELL_PIXEL_H, -(CELL_PIXEL_H / 2)), r);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_H, -(CELL_PIXEL_H / 2)), Point2D(CELL_PIXEL_W, 0), g);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_W, 0), Point2D(CELL_PIXEL_H, CELL_PIXEL_H), b);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_H, CELL_PIXEL_H), Point2D(0, (CELL_PIXEL_H / 2)), w);
		break;
	}
	case TileRampType::RAMP_CORNER_NW:
	{
		pSurface->Draw_Line_Rect(cellrect, Point2D(0, (CELL_PIXEL_H / 2)), Point2D(CELL_PIXEL_H, 0), r);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_H, 0), Point2D(CELL_PIXEL_W, (CELL_PIXEL_H / 2)), g);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_W, (CELL_PIXEL_H / 2)), Point2D(0, (CELL_PIXEL_H / 2)), b);
		break;
	}
	case TileRampType::RAMP_CORNER_NE:
	{
		pSurface->Draw_Line_Rect(cellrect, Point2D(0, 0), Point2D(CELL_PIXEL_H, 0), r);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_H, 0), Point2D(CELL_PIXEL_W, (CELL_PIXEL_H / 2)), g);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_W, (CELL_PIXEL_H / 2)), Point2D(CELL_PIXEL_H, CELL_PIXEL_H), b);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_H, CELL_PIXEL_H), Point2D(0, 0), w);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_H, 0), Point2D(CELL_PIXEL_H, CELL_PIXEL_H), x);
		break;
	}
	case TileRampType::RAMP_CORNER_SE:
	{
		pSurface->Draw_Line_Rect(cellrect, Point2D(0, (CELL_PIXEL_H / 2)), Point2D(CELL_PIXEL_H, -(CELL_PIXEL_H / 2)), r);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_H, -(CELL_PIXEL_H / 2)), Point2D(CELL_PIXEL_W, (CELL_PIXEL_H / 2)), g);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_W, (CELL_PIXEL_H / 2)), Point2D(CELL_PIXEL_H, CELL_PIXEL_H), b);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_H, CELL_PIXEL_H), Point2D(0, (CELL_PIXEL_H / 2)), w);
		pSurface->Draw_Line_Rect(cellrect, Point2D(0, (CELL_PIXEL_H / 2)), Point2D(CELL_PIXEL_W, (CELL_PIXEL_H / 2)), x);
		break;
	}
	case TileRampType::RAMP_CORNER_SW:
		pSurface->Draw_Line_Rect(cellrect, Point2D(0, (CELL_PIXEL_H / 2)), Point2D(CELL_PIXEL_H, 0), r);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_H, 0), Point2D(CELL_PIXEL_W, 0), g);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_W, 0), Point2D(CELL_PIXEL_H, CELL_PIXEL_H), b);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_H, CELL_PIXEL_H), Point2D(0, (CELL_PIXEL_H / 2)), w);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_H, 0), Point2D(CELL_PIXEL_H, CELL_PIXEL_H), x);
		break;

	case TileRampType::RAMP_MID_NW:
	{
		pSurface->Draw_Line_Rect(cellrect, Point2D(0, 0), Point2D(CELL_PIXEL_W, 0), r);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_W, 0), Point2D(CELL_PIXEL_H, (CELL_PIXEL_H / 2)), g);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_H, (CELL_PIXEL_H / 2)), Point2D(0, 0), b);
		break;
	}
	case TileRampType::RAMP_MID_NE:
	{
		pSurface->Draw_Line_Rect(cellrect, Point2D(0, 0), Point2D(CELL_PIXEL_H, -(CELL_PIXEL_H / 2)), r);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_H, -(CELL_PIXEL_H / 2)), Point2D(CELL_PIXEL_W, (CELL_PIXEL_H / 2)), g);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_W, (CELL_PIXEL_H / 2)), Point2D(CELL_PIXEL_H, (CELL_PIXEL_H / 2)), b);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_H, (CELL_PIXEL_H / 2)), Point2D(0, 0), w);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_H, -(CELL_PIXEL_H / 2)), Point2D(CELL_PIXEL_H, (CELL_PIXEL_H / 2)), x);
		break;
	}
	case TileRampType::RAMP_MID_SE:
	{
		pSurface->Draw_Line_Rect(cellrect, Point2D(0, 0), Point2D(CELL_PIXEL_H, -(CELL_PIXEL_H / 2)), r);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_H, -(CELL_PIXEL_H / 2)), Point2D(CELL_PIXEL_W, 0), g);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_W, 0), Point2D(CELL_PIXEL_H, CELL_PIXEL_H), b);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_H, CELL_PIXEL_H), Point2D(0, 0), w);
		pSurface->Draw_Line_Rect(cellrect, Point2D(0, 0), Point2D(CELL_PIXEL_W, 0), x);
		break;
	}
	case TileRampType::RAMP_MID_SW:
	{
		pSurface->Draw_Line_Rect(cellrect, Point2D(0, (CELL_PIXEL_H / 2)), Point2D(CELL_PIXEL_H, -(CELL_PIXEL_H / 2)), r);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_H, -(CELL_PIXEL_H / 2)), Point2D(CELL_PIXEL_W, 0), g);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_W, 0), Point2D(CELL_PIXEL_H, (CELL_PIXEL_H / 2)), b);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_H, (CELL_PIXEL_H / 2)), Point2D(0, (CELL_PIXEL_H / 2)), w);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_H, -(CELL_PIXEL_H / 2)), Point2D(CELL_PIXEL_H, (CELL_PIXEL_H / 2)), x);
		break;
	}
	case TileRampType::RAMP_STEEP_SE:
	{
		/**
		*  PLACEHOLDER:
		*  This tile is normally only 3 pixels graphically.
		*/
		pSurface->Draw_Line_Rect(cellrect, Point2D(0, (CELL_PIXEL_H / 2)), Point2D(CELL_PIXEL_H, 0), r);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_H, 0), Point2D(CELL_PIXEL_W, (CELL_PIXEL_H / 2)), g);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_W, (CELL_PIXEL_H / 2) - 1), Point2D(CELL_PIXEL_H, (CELL_PIXEL_H - 1)), b);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_H, (CELL_PIXEL_H - 1)), Point2D(0, (CELL_PIXEL_H / 2) - 1), w);
		break;
	}

	case TileRampType::RAMP_STEEP_SW:
	{
		pSurface->Draw_Line_Rect(cellrect, Point2D(0, -(CELL_PIXEL_H / 2)), Point2D(CELL_PIXEL_H, -(CELL_PIXEL_H / 2)), r);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_H, -(CELL_PIXEL_H / 2)), Point2D(CELL_PIXEL_W, (CELL_PIXEL_H / 2)), g);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_W, (CELL_PIXEL_H / 2)), Point2D(CELL_PIXEL_H, (CELL_PIXEL_H / 2)), b);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_H, (CELL_PIXEL_H / 2)), Point2D(0, -(CELL_PIXEL_H / 2)), w);
		break;
	}
	case TileRampType::RAMP_STEEP_NW:
	{
		pSurface->Draw_Line_Rect(cellrect, Point2D(0, 0), Point2D(CELL_PIXEL_H, 0 - CELL_PIXEL_H), r);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_H, 0 - CELL_PIXEL_H), Point2D(CELL_PIXEL_W, 0), g);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_W, 0), Point2D(CELL_PIXEL_H, CELL_PIXEL_W - CELL_PIXEL_H), b);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_H, CELL_PIXEL_W - CELL_PIXEL_H), Point2D(0, 0), w);
		break;
	}
	case TileRampType::RAMP_STEEP_NE:
	{
		pSurface->Draw_Line_Rect(cellrect, Point2D(0, (CELL_PIXEL_H / 2)), Point2D(CELL_PIXEL_H, -(CELL_PIXEL_H / 2)), r);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_H, -(CELL_PIXEL_H / 2)), Point2D(CELL_PIXEL_W, -(CELL_PIXEL_H / 2)), g);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_W, -(CELL_PIXEL_H / 2)), Point2D(CELL_PIXEL_H, (CELL_PIXEL_H / 2)), b);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_H, (CELL_PIXEL_H / 2)), Point2D(0, (CELL_PIXEL_H / 2)), w);
		break;
	}
	case TileRampType::RAMP_DOUBLE_UP_SW_NE:
	{
		pSurface->Draw_Line_Rect(cellrect, Point2D(0, 0), Point2D(CELL_PIXEL_W, 0), r);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_W, 0), Point2D(CELL_PIXEL_H, CELL_PIXEL_H), g);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_H, CELL_PIXEL_H), Point2D(0, 0), b);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_H, 0), Point2D(CELL_PIXEL_H, CELL_PIXEL_H), w);
		break;
	}
	case TileRampType::RAMP_DOUBLE_DOWN_SW_NE:
	{
		pSurface->Draw_Line_Rect(cellrect, Point2D(0, (CELL_PIXEL_H / 2)), Point2D(CELL_PIXEL_H, -(CELL_PIXEL_H / 2)), r);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_H, -(CELL_PIXEL_H / 2)), Point2D(CELL_PIXEL_W, (CELL_PIXEL_H / 2)), g);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_W, (CELL_PIXEL_H / 2)), Point2D(0, (CELL_PIXEL_H / 2)), b);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_H, -(CELL_PIXEL_H / 2)), Point2D(CELL_PIXEL_H, (CELL_PIXEL_H / 2)), w);
		break;
	}
	case TileRampType::RAMP_DOUBLE_UP_NW_SE:
	{
		pSurface->Draw_Line_Rect(cellrect, Point2D(0, 0), Point2D(CELL_PIXEL_W, 0), r);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_W, 0), Point2D(CELL_PIXEL_H, CELL_PIXEL_H), g);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_H, CELL_PIXEL_H), Point2D(0, 0), b);
		break;
	}
	case TileRampType::RAMP_DOUBLE_DOWN_NW_SE:
	{
		pSurface->Draw_Line_Rect(cellrect, Point2D(0, (CELL_PIXEL_H / 2)), Point2D(CELL_PIXEL_H, -(CELL_PIXEL_H / 2)), r);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_H, -(CELL_PIXEL_H / 2)), Point2D(CELL_PIXEL_W, (CELL_PIXEL_H / 2)), g);
		pSurface->Draw_Line_Rect(cellrect, Point2D(CELL_PIXEL_W, (CELL_PIXEL_H / 2)), Point2D(0, (CELL_PIXEL_H / 2)), b);
		break;
	}

	};

#undef CELL_PIXEL_W
#undef CELL_PIXEL_H
	return true;
}