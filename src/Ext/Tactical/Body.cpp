#include "Body.h"

#include <Ext/Super/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Scenario/Body.h>

#include <Utilities/Macro.h>
#include <Utilities/Cast.h>

#include <TacticalClass.h>
#include <HouseClass.h>
#include <Unsorted.h>

#include <FPSCounter.h>

#include <TextDrawing.h>

#include <Phobos.h>
enum class CollisionBoxShape : BYTE{
	Rectangle,
	Diamond,      // Isometric diamond
	Ellipse,
	Circle
};

void NOINLINE DrawRectangle(DSurface* surface, RectangleStruct& clip,
							   Point2D& center, int width, int height,
							   unsigned short color)
{
	const int halfW = width / 2;
	const int halfH = height / 2;

	Point2D topLeft = { center.X - halfW, center.Y - halfH };
	Point2D topRight = { center.X + halfW, center.Y - halfH };
	Point2D bottomLeft = { center.X - halfW, center.Y + halfH };
	Point2D bottomRight = { center.X + halfW, center.Y + halfH };

	surface->Draw_Line_Rect(clip, topLeft, topRight, color);
	surface->Draw_Line_Rect(clip, topRight, bottomRight, color);
	surface->Draw_Line_Rect(clip, bottomRight, bottomLeft, color);
	surface->Draw_Line_Rect(clip, bottomLeft, topLeft, color);
}

void DrawDiamond(DSurface* surface, RectangleStruct& clip,
							 Point2D& center, int width, int height,
							 unsigned short color)
{
	const int halfW = width / 2;
	const int halfH = height / 2;

	Point2D top = { center.X,         center.Y - halfH };
	Point2D right = { center.X + halfW, center.Y };
	Point2D bottom = { center.X,         center.Y + halfH };
	Point2D left = { center.X - halfW, center.Y };

	surface->Draw_Line_Rect(clip, top, right, color);
	surface->Draw_Line_Rect(clip, right, bottom, color);
	surface->Draw_Line_Rect(clip, bottom, left, color);
	surface->Draw_Line_Rect(clip, left, top, color);
}

void DrawEllipse(DSurface* surface, RectangleStruct& clip,
							 Point2D& center, int width, int height,
							 unsigned short color)
{
	surface->Draw_Ellipse(center, width / 2, height / 2, clip, color);
}

void NOINLINE DrawCenterCross(DSurface* surface, RectangleStruct& clip, Point2D& center)
{
	unsigned short white = DSurface::RGB_To_Pixel(255, 255, 255);

	// Small cross for visibility
	Point2D _c1_start { center.X - 2, center.Y };
	Point2D _c1_end { center.X + 2, center.Y };
	surface->Draw_Line_Rect(clip, _c1_start, _c1_end, white);

	Point2D _c2_start { center.X, center.Y - 2 };
	Point2D _c2_end { center.X, center.Y + 2 };
	surface->Draw_Line_Rect(clip, _c2_start, _c2_end, white);
}

void NOINLINE DrawArrowHead(DSurface* surface, RectangleStruct& bounds,
							 Point2D& from, Point2D& to, unsigned short color,
							 int arrowSize = 8)
{
	// Only draw if endpoint is on screen
	if (to.X < bounds.X || to.X >= bounds.X + bounds.Width ||
		to.Y < bounds.Y || to.Y >= bounds.Y + bounds.Height)
		return;

	float dx = (float)(to.X - from.X);
	float dy = (float)(to.Y - from.Y);
	float len = Math::sqrt(dx * dx + dy * dy);

	if (len < 1.0f)
		return;

	// Normalize
	dx /= len;
	dy /= len;

	// Arrow wings perpendicular to line
	float px = -dy;
	float py = dx;

	Point2D wing1 = {
		(int)(to.X - dx * arrowSize + px * arrowSize / 2),
		(int)(to.Y - dy * arrowSize + py * arrowSize / 2)
	};
	Point2D wing2 = {
		(int)(to.X - dx * arrowSize - px * arrowSize / 2),
		(int)(to.Y - dy * arrowSize - py * arrowSize / 2)
	};

	surface->Draw_Line_Rect(bounds, to, wing1, color);
	surface->Draw_Line_Rect(bounds, to, wing2, color);
}

void NOINLINE DrawDestinationLine(FootClass* obj, DSurface* surface,
								   RectangleStruct& bounds, Point2D& start,
								   unsigned short color)
{
	CoordStruct destCoords = CoordStruct::Empty;

	if (obj->Locomotor.GetInterfacePtr())
	{
		destCoords = obj->Locomotor->Destination();
	}

	if (destCoords.IsEmpty() && obj->Destination)
	{
		destCoords = obj->Destination->GetCoords();
	}

	if (destCoords.IsEmpty() || destCoords == obj->Location)
		return;

	// Just convert coordinates, don't check if visible
	Point2D end = TacticalClass::Instance->CoordsToView(destCoords);

	end.X += bounds.X;
	end.Y += bounds.Y;

	// Draw line - clipping handled by Draw_Line_Rect
	surface->Draw_Line_Rect(bounds, start, end, color);

	DrawArrowHead(surface, bounds, start, end, color, 10);

	// Only draw diamond marker if destination is on screen
	if (end.X >= bounds.X && end.X < bounds.X + bounds.Width &&
		end.Y >= bounds.Y && end.Y < bounds.Y + bounds.Height)
	{
		Point2D d1 = { end.X, end.Y - 5 };
		Point2D d2 = { end.X + 5, end.Y };
		Point2D d3 = { end.X, end.Y + 5 };
		Point2D d4 = { end.X - 5, end.Y };
		surface->Draw_Line_Rect(bounds, d1, d2, color);
		surface->Draw_Line_Rect(bounds, d2, d3, color);
		surface->Draw_Line_Rect(bounds, d3, d4, color);
		surface->Draw_Line_Rect(bounds, d4, d1, color);
	}
}

void NOINLINE DrawTargetLine(FootClass* obj, DSurface* surface,
							  RectangleStruct& bounds, Point2D& start,
							  unsigned short color)
{
	AbstractClass* target = obj->Target;
	if (!target)
		return;

	CoordStruct targetCoords = target->GetCoords();

	// Just convert coordinates, don't check if visible
	Point2D end = TacticalClass::Instance->CoordsToView(targetCoords);
	end.X += bounds.X;
	end.Y += bounds.Y;

	// Draw line - clipping is handled by Draw_Line_Rect
	surface->Draw_Line_Rect(bounds, start, end, color);

	DrawArrowHead(surface, bounds, start, end, color, 10);

	// Only draw X marker if target is on screen
	if (end.X >= bounds.X && end.X < bounds.X + bounds.Width &&
		end.Y >= bounds.Y && end.Y < bounds.Y + bounds.Height)
	{
		Point2D x1 = { end.X - 4, end.Y - 4 };
		Point2D x2 = { end.X + 4, end.Y + 4 };
		Point2D x3 = { end.X - 4, end.Y + 4 };
		Point2D x4 = { end.X + 4, end.Y - 4 };
		surface->Draw_Line_Rect(bounds, x1, x2, color);
		surface->Draw_Line_Rect(bounds, x3, x4, color);
	}
}

void NOINLINE DrawCollisionBox(FootClass* obj, DSurface* surface,
								  RectangleStruct& bounds, COLORREF color)
{
	auto tech_loc = obj->GetCoords();
	if (tech_loc.IsEmpty())
		return;

	unsigned short red = DSurface::RGB_To_Pixel(255, 0, 0);
	unsigned short yellow = DSurface::RGB_To_Pixel(255, 255, 0);

	// Convert world coords to screen
	auto [outClient, visible] = TacticalClass::Instance->GetCoordsToClientSituation(tech_loc);

	// Offset by viewport
	Point2D screen = outClient + Point2D(bounds.X, bounds.Y);

	// Draw target line with arrow (red)
	DrawTargetLine(obj, surface, bounds, screen, red);

	// Draw destination line with arrow (yellow)
	DrawDestinationLine(obj, surface, bounds, screen, yellow);

	if (!visible)
		return;

	// Get dimensions from type extension, or use defaults
	int width = 30;   // Default
	int height = 20;  // Default

	// The CENTER is visible, but box edges might not be!
	int left = screen.X - width / 2;
	int right = screen.X + width / 2;
	int top = screen.Y - height / 2;
	int bottom = screen.Y + height / 2;

	// Clamp to surface dimensions
	RectangleStruct surfaceRect = surface->Get_Rect();
	left = std::max(left, surfaceRect.X);
	right = std::min(right, surfaceRect.X + surfaceRect.Width);
	top = std::max(top, surfaceRect.Y);
	bottom = std::min(bottom, surfaceRect.Y + surfaceRect.Height);

	// Skip if degenerate
	if (left >= right || top >= bottom) return;

	CollisionBoxShape shape = CollisionBoxShape::Rectangle;

	// Draw based on shape
	switch (shape)
	{
	case CollisionBoxShape::Rectangle:
		DrawRectangle(surface, bounds, screen, width, height, color);
		break;
	case CollisionBoxShape::Diamond:
		DrawDiamond(surface, bounds, screen, width, height, color);
		break;
	case CollisionBoxShape::Ellipse:
		DrawEllipse(surface, bounds, screen, width, height, color);
		break;
	case CollisionBoxShape::Circle:
		DrawEllipse(surface, bounds, screen, width, width / 2, color);  // Circle in isometric
		break;
	default:
		break;
	}

	// Always draw center cross (white)
	DrawCenterCross(surface, bounds, screen);

}

template<typename T>
void NOINLINE Draws(COLORREF color, DSurface* pSurface , RectangleStruct& bounds) {
	unsigned short pixelColor = DSurface::RGB_To_Pixel(
		(color >> 16) & 0xFF,  // R
		(color >> 8) & 0xFF,   // G
		color & 0xFF           // B
	);

	T::Array->for_each([&](T* tech) {
		if (tech && tech->IsAlive && tech->IsOnMap && !tech->InLimbo)
			DrawCollisionBox(tech, pSurface, bounds, pixelColor);  // Green
	});
}


void NOINLINE FakeTacticalClass::DrawCollisionDebug()
{
	auto surface = DSurface::Temp();

	// Check if surface is valid and locked
	if (!surface)
		return;

	auto& bounds = DSurface::ViewBounds();

	// Units
	Draws<UnitClass>(0x00FF00, surface, bounds); // Green

	// Infantry
	Draws<InfantryClass>(0x00FFFF, surface, bounds); // Cyan

	// Aircraft
	Draws<AircraftClass>(0xFFFF00, surface, bounds); // Yellow
}

void NOINLINE DrawLines()
{
	auto surface = DSurface::Temp();

	Debug::Log("surface = %p\n", surface);

	if (!surface)
	{
		Debug::Log("Surface is NULL!\n");
		return;
	}

	Debug::Log("surface->Is_Locked() = %d\n", surface->Is_Locked());

	if (!surface->Is_Locked())
	{
		Debug::Log("Surface not locked!\n");
		return;
	}

	Debug::Log("Getting ViewBounds...\n");
	auto& bounds = DSurface::ViewBounds();
	Debug::Log("bounds: X=%d Y=%d W=%d H=%d\n", bounds.X, bounds.Y, bounds.Width, bounds.Height);

	Debug::Log("Converting color...\n");
	unsigned short green = DSurface::RGB_To_Pixel(0, 255, 0);
	Debug::Log("green = %04X\n", green);

	Debug::Log("Creating points...\n");
	Point2D p1 = { 100, 100 };
	Point2D p2 = { 200, 100 };
	Debug::Log("p1=(%d,%d) p2=(%d,%d)\n", p1.X, p1.Y, p2.X, p2.Y);

	Debug::Log("Calling Draw_Line_Rect...\n");
	surface->Draw_Line_Rect(bounds, p1, p2, green);
	Debug::Log("Draw_Line_Rect returned!\n");
}

void FakeTacticalClass::_Render_Layer(bool arg)
{
	this->Render_Layer(arg);
	/*this->DrawCollisionDebug();*/
}
DEFINE_FUNCTION_JUMP(CALL, 0x6D465F, FakeTacticalClass::_Render_Layer)

// Reversed from Is_Selectable, w/o Select call
bool FakeTacticalClass::ObjectClass_IsSelectable(ObjectClass* pThis)
{
	const auto pOwner = pThis->GetOwningHouse();
	return pOwner
		&& pOwner->ControlledByCurrentPlayer()
		&& pThis->CanBeSelected()
		&& pThis->CanBeSelectedNow();
}

void FakeTacticalClass::_Draw_Pixel_Effects(RectangleStruct* tactical_rect, RectangleStruct* effect_rect)
{
	this->Draw_Pixel_Effects(tactical_rect, effect_rect);
	this->__RenderOverlapForeignMap();
}

DEFINE_FUNCTION_JUMP(CALL, 0x6D492B, FakeTacticalClass::_Draw_Pixel_Effects)

void FakeTacticalClass::_Render_Objects_Near_Shroud(bool arg0, Point2D pos, RectangleStruct* a5) {
	// Update light sources if they have been flagged to be updated.
	if (ScenarioExtData::UpdateLightSources) {
		for (auto light : *LightSourceClass::Array)
		{
			if (light->Activated)
			{
				light->Activated = false;
				light->Activate();
			}
		}

		ScenarioExtData::UpdateLightSources = false;
	}

	this->Render_Objects_Near_Shroud(arg0, pos, a5);
}

DEFINE_FUNCTION_JUMP(CALL , 0x6D4471 , FakeTacticalClass::_Render_Objects_Near_Shroud)

// Fixes glitches if the map size is smaller than the screen resolution
// Author: Belonit
static constexpr float paddingTopInCell = 5;
static constexpr float paddingBottomInCell = 4.5;

bool FakeTacticalClass::__ClampTacticalPos(Point2D* tacticalPos) {
	bool isUpdated = false;

	const auto pMapRect = &MapClass::Instance->MapRect;
	const auto pMapVisibleRect = &MapClass::Instance->VisibleRect;
	const auto pSurfaceViewBounds = &DSurface::ViewBounds();

	{
		const int xMin = (pSurfaceViewBounds->Width / 2) + (Unsorted::CellWidthInPixels / 2) * (pMapVisibleRect->X * 2 - pMapRect->Width);
		if (tacticalPos->X < xMin)
		{
			tacticalPos->X = xMin;
			isUpdated = true;
		}
		else
		{
			const int xMax = MaxImpl(
				xMin,
				xMin + (Unsorted::CellWidthInPixels * pMapVisibleRect->Width) - pSurfaceViewBounds->Width
			);

			if (tacticalPos->X > xMax)
			{
				tacticalPos->X = xMax;
				isUpdated = true;
			}
		}
	}

	{
		const int yMin = (pSurfaceViewBounds->Height / 2) + (Unsorted::CellHeightInPixels / 2) * (pMapVisibleRect->Y * 2 + pMapRect->Width - int(paddingTopInCell));
		if (tacticalPos->Y < yMin)
		{
			tacticalPos->Y = yMin;
			isUpdated = true;
		}
		else
		{
			const int yMax = MaxImpl(
				yMin,
				yMin + (Unsorted::CellHeightInPixels * pMapVisibleRect->Height) - pSurfaceViewBounds->Height + int(Unsorted::CellHeightInPixels * paddingBottomInCell)
			);

			if (tacticalPos->Y > yMax)
			{
				tacticalPos->Y = yMax;
				isUpdated = true;
			}
		}
	}

	return isUpdated;
}

bool FakeTacticalClass::IsInSelectionRect(LTRBStruct* pRect, const TacticalSelectableStruct& selectable)
{
	if (selectable.Object
		&& selectable.Object->IsAlive
		&& !selectable.Object->InLimbo
		&& selectable.Object->AbstractFlags & AbstractFlags::Techno
		)
	{
		int nLocalX = selectable.Point.X - this->TacticalPos.X;
		int nLocalY = selectable.Point.Y - this->TacticalPos.Y;

		if ((nLocalX >= pRect->Left && nLocalX < pRect->Right + pRect->Left) &&
			(nLocalY >= pRect->Top && nLocalY < pRect->Bottom + pRect->Top))
		{
			return true;
		}
	}

	return false;
}

bool FakeTacticalClass::IsHighPriorityInRect(LTRBStruct* rect)
{
	for (const auto& selected : Array)
	{
		if (this->IsInSelectionRect(rect, selected) && ObjectClass_IsSelectable(selected.Object))
		{
			//auto const pExt = TechnoExtContainer::Instance.Find(static_cast<TechnoClass*>(selected.Object));
			auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(selected.Object->GetTechnoType());

			return !pTypeExt->LowSelectionPriority;
		}
	}

	return false;
}

// Reversed from Tactical::Select
void FakeTacticalClass::SelectFiltered(LTRBStruct* pRect, callback_type fpCheckCallback, bool bPriorityFiltering)
{
	Unsorted::MoveFeedback = true;

	if (pRect->Right <= 0 || pRect->Bottom <= 0 || this->SelectableCount <= 0)
		return;

	for (const auto& selected : Array) {

		if (this->IsInSelectionRect(pRect, selected)) {

			const auto pObj = selected.Object;
			auto pTechno = static_cast<TechnoClass*>(selected.Object);
			const auto pObjType = pObj->GetType();

			{
				const auto TypeExt = TechnoTypeExtContainer::Instance.Find((TechnoTypeClass*)pObjType);

				if (bPriorityFiltering){

					//auto const pExt = TechnoExtContainer::Instance.Find(pTechno);
					// Attached units shouldn't be selected regardless of the setting
					bool isLowPriorityByTechno = Phobos::Config::PrioritySelectionFiltering && TypeExt->LowSelectionPriority;

					if (isLowPriorityByTechno)
						continue;
				}

				if (Game::IsTypeSelecting())
					Game::UICommands_TypeSelect_7327D0(TypeExt->GetSelectionGroupID());
				else if (fpCheckCallback)
					(*fpCheckCallback)(pTechno);
				else {
					const auto pBldType = type_cast<BuildingTypeClass*>((TechnoTypeClass*)pObjType);
					const auto pOwner = pTechno->GetOwningHouse();

					if (pOwner
						&& pOwner->ControlledByCurrentPlayer()
						&& pTechno->CanBeSelected()
						&& (!pBldType || pBldType->IsUndeployable())
						) {
						Unsorted::MoveFeedback = !pTechno->Select();
					}
				}

			}
		}
	}

	Unsorted::MoveFeedback = true;
}

// Reversed from Tactical::MakeSelection
void FakeTacticalClass::Tactical_MakeFilteredSelection(callback_type fpCheckCallback)
{
	if (this->Band.Left || this->Band.Top)
	{
		int nLeft = this->Band.Left;
		int nRight = this->Band.Right;
		int nTop = this->Band.Top;
		int nBottom = this->Band.Bottom;

		if (nLeft > nRight)
			std::swap(nLeft, nRight);
		if (nTop > nBottom)
			std::swap(nTop, nBottom);

		LTRBStruct rect { nLeft , nTop, nRight - nLeft + 1, nBottom - nTop + 1 };

		const bool bPriorityFiltering = Phobos::Config::PrioritySelectionFiltering
			&& this->IsHighPriorityInRect(&rect);

		this->SelectFiltered(&rect, fpCheckCallback, bPriorityFiltering);

		this->Band.Left = 0;
		this->Band.Top = 0;
	}
}

template<class T, class U>
constexpr int8_t __CFADD__(T x, U y)
{
	using Wider = std::conditional_t<(sizeof(T) > sizeof(U)), T, U>;

	if constexpr (sizeof(Wider) == 1)
		return static_cast<uint8_t>(x) > static_cast<uint8_t>(x + y);
	else if constexpr (sizeof(Wider) == 2)
		return static_cast<uint16_t>(x) > static_cast<uint16_t>(x + y);
	else if constexpr (sizeof(Wider) == 4)
		return static_cast<uint32_t>(x) > static_cast<uint32_t>(x + y);
	else
		return static_cast<uint64_t>(x) > static_cast<uint64_t>(x + y);
}

/**
 *  Draw a radial to the screen.
 *
 *  @authors: CCHyper
 */

void FakeTacticalClass::__DrawRadialIndicator(
	bool draw_indicator,
	bool animate,
	Coordinate center_coord,
	ColorStruct color,
	float radius,
	bool concentric,
	bool round)
{
	if (round)
	{
		radius = std::round(radius);
	}

	int size;

	if (concentric)
	{
		size = (int)radius;
	}
	else
	{
		size = (int)((radius + 0.5) / Math::SQRT_TWO *double(Unsorted::CellWidthInPixels)); // should be cell size global?
	}

	Point2D center_pixel = TacticalClass::Instance->CoordsToClient(center_coord);

	center_pixel.X += DSurface::ViewBounds().X;
	center_pixel.Y += DSurface::ViewBounds().Y;

	RectangleStruct draw_area(
		center_pixel.X - size,
		center_pixel.Y - size / 2,
		size * 2,
		size
	);

	RectangleStruct intersect = draw_area.IntersectWith(DSurface::ViewBounds());
	if (!intersect.IsValid())
	{
		return;
	}

	ColorStruct draw_color = color;

	if (animate)
	{
		draw_color.Adjust(50, ColorStruct::Empty);
	}

	unsigned ellipse_color = DSurface::RGB_To_Pixel(draw_color.R, draw_color.G, draw_color.B);

	/**
	 *  Draw the main radial ellipse, then draw one slightly smaller to give a thicker impression.
	 */
	DSurface::Temp->Draw_Ellipse(center_pixel, size, size / 2, DSurface::ViewBounds(), ellipse_color);
	DSurface::Temp->Draw_Ellipse(center_pixel, size - 1, size / 2 - 1, DSurface::ViewBounds(), ellipse_color);

	/**
	 *  Draw the sweeping indicator line.
	 */
	if (!draw_indicator)
	{
		return;
	}

	double d_size = (double)size;
	double size_half = (double)size / 2;

	/**
	   *  The alpha values for the lines (producing the fall-off effect).
	   */
	static constexpr double _line_alpha[] = {
		//0.05, 0.20, 0.40, 1.0                     // original values.
		0.05, 0.10, 0.20, 0.40, 0.60, 0.80, 1.0     // new values.
	};

	static constexpr int _rate = 50;

	for (size_t i = 0; i < ARRAY_SIZE(_line_alpha); ++i)
	{

		static int _offset = 0;
		static MSTimerClass sweep_rate(_rate);

		if (sweep_rate.Expired())
		{
			sweep_rate.Start(_rate);
			++_offset;
		}

		float angle_offset = float((_offset + i) * 0.05);
		int angle_increment = int(angle_offset / Math::DEG_TO_RADF(360));
		float angle = angle_offset - (angle_increment * Math::DEG_TO_RADF(360));

		Point2D line_start {};
		Point2D line_end {};

		if (Math::abs(angle - Math::DEG_TO_RADF(90)) < 0.001)
		{

			line_start = center_pixel;
			line_end = Point2D(center_pixel.X, int(center_pixel.Y + (-size_half)));

		}
		else if (Math::abs(angle - Math::DEG_TO_RADF(270)) < 0.001)
		{

			line_start = center_pixel;
			line_end = Point2D(center_pixel.X, int(center_pixel.Y + size_half));

		}
		else
		{

			double angle_tan = std::tan(angle);
			double xdist = Math::sqrt(1.0 / ((angle_tan * angle_tan) / (size_half * size_half) + 1.0 / (d_size * d_size)));
			double ydist = Math::sqrt((1.0 - (xdist * xdist) / (d_size * d_size)) * (size_half * size_half));

			if (angle > Math::DEG_TO_RADF(90) && angle < Math::DEG_TO_RADF(270))
			{
				xdist = -xdist;
			}

			if (angle < Math::DEG_TO_RADF(180))
			{
				ydist = -ydist;
			}

			line_start = center_pixel;
			line_end = Point2D(int(center_pixel.X + xdist), int(center_pixel.Y + ydist));

		}

		line_start.X -= DSurface::ViewBounds().X;
		line_start.Y -= DSurface::ViewBounds().Y;

		line_end.X -= DSurface::ViewBounds().X;
		line_end.Y -= DSurface::ViewBounds().Y;

		bool enable_red_channel = true;
		bool enable_green_channel = true;
		bool enable_blue_channel = true;

		DSurface::Temp->DrawSubtractiveLine_AZ(DSurface::ViewBounds(),
										line_start,
										line_end,
										draw_color,
										-500,
										-500,
										false,
										enable_red_channel,
										enable_green_channel,
										enable_blue_channel,
										(float)_line_alpha[i]);

	}
}

void FakeTacticalClass::__RenderOverlapForeignMap()
{
	auto pMapVisibleRect = &MapClass::Instance->VisibleRect;
	auto pSurfaceViewBounds = &DSurface::ViewBounds();

	{
		const int maxWidth = pSurfaceViewBounds->Width - pMapVisibleRect->Width * Unsorted::CellWidthInPixels;

		if (maxWidth > 0)
		{
			RectangleStruct rect = {
				pSurfaceViewBounds->Width - maxWidth,
				0,
				maxWidth,
				pSurfaceViewBounds->Height
			};

			DSurface::Composite->Fill_Rect(rect, COLOR_BLACK);
		}
	}

	{
		const int maxHeight = pSurfaceViewBounds->Height - (Unsorted::CellHeightInPixels * pMapVisibleRect->Height) - int(Unsorted::CellHeightInPixels * paddingBottomInCell);

		if (maxHeight > 0)
		{
			RectangleStruct rect = {
				0,
				pSurfaceViewBounds->Height - maxHeight,
				pSurfaceViewBounds->Width,
				maxHeight
			};

			DSurface::Composite->Fill_Rect(rect, COLOR_BLACK);
		}
	}
}

#include <WWKeyboardClass.h>

void DrawTransRect(BitFont* pBitInst, int* width , int index ) {

	width += 6;
	const int lineSpace = pBitInst->field_1C + 2;
	Point2D location { DSurface::ViewBounds->Width, (DSurface::ViewBounds->Height - ((index + 1) * lineSpace)) };
	RectangleStruct rect { (location.X - *width), location.Y, *width, lineSpace };

	ColorStruct fillColor { 0, 0, 0 };
	DSurface::Composite->Fill_Rect_Trans(&rect, &fillColor, (InputManagerClass::Instance->IsForceMoveKeyPressed() ? 80 : 40));
	Point2D top { rect.X - 1, rect.Y };
	Point2D bot { top.X, top.Y + lineSpace - 1 };
	DSurface::Composite->Draw_Line(top, bot, COLOR_BLACK);
}

#ifndef ___test

void __fastcall FakeTacticalClass::__DrawTimersA(int value, ColorScheme* color, int interval, const wchar_t* label, LARGE_INTEGER* _arg, bool* _arg1)
{
	BitFont* pFont = BitFont::BitFontPtr(TextPrintType::UseGradPal | TextPrintType::Right | TextPrintType::NoShadow | TextPrintType::Metal12 | TextPrintType::Background);

	const int hour = interval / 60 / 60;
	const int minute = interval / 60 % 60;
	const int second = interval % 60;
	static fmt::basic_memory_buffer<wchar_t> buffer;
	buffer.clear();
	if (hour)
	{
		fmt::format_to(std::back_inserter(buffer), L"{:02}:{:02}:{:02}", hour, minute, second);
	}
	else
	{
		fmt::format_to(std::back_inserter(buffer), L"{:02}:{:02}", minute, second);
	}
	buffer.push_back(L'\0');

	static fmt::basic_memory_buffer<wchar_t> labe_buffer;
	labe_buffer.clear();
	fmt::format_to(std::back_inserter(labe_buffer), L"{}  ", !label ? L"" : label);
	labe_buffer.push_back(L'\0');

	int width = 0;
	RectangleStruct rect_bound = DSurface::ViewBounds();
	pFont->GetTextDimension(buffer.data(), &width, nullptr, rect_bound.Width);
	ColorScheme* fore = color;

	if (!interval && _arg && _arg1)
	{
		const auto now = Game::AudioGetTime();
		if (static_cast<uint64_t>(_arg->QuadPart) < static_cast<uint64_t>(now.QuadPart)) {
			_arg->QuadPart = now.QuadPart + 1000;
			*_arg1 = !*_arg1;
		}

		if (*_arg1)
		{
			fore = ColorScheme::Array->Items[RulesExtData::Instance()->TimerBlinkColorScheme];
		}
	}

	int value_plusone = value + 1;
	Point2D point {
		rect_bound.Width - width - 3 ,
		rect_bound.Height - (value_plusone) * (pFont->field_1C + 2)
	};

	auto pComposite = DSurface::Composite();
	auto rect = pComposite->Get_Rect();
	Point2D _temp {};
	ColorStruct out {};
	color->BaseColor.ToColorStruct(&out);

	TextDrawing::Simple_Text_Print_Wide(
		&_temp, 
		labe_buffer.data(),
		pComposite,
		&rect,
		&point,
		(COLORREF)out.ToInit(),
		(COLORREF)0u,
		TextPrintType::UseGradPal | TextPrintType::Right | TextPrintType::NoShadow | TextPrintType::Metal12 | TextPrintType::Background,
		true
	);

	point.X = rect_bound.Width - 3;
	point.Y = rect_bound.Height - value_plusone * (pFont->field_1C + 2);
	rect = pComposite->Get_Rect();
	fore->BaseColor.ToColorStruct(&out);

	TextDrawing::Simple_Text_Print_Wide(
	&_temp,
	buffer.data(),
	pComposite,
	&rect,
	&point,
	(COLORREF)out.ToInit(),
	(COLORREF)0u,
	TextPrintType::UseGradPal | TextPrintType::Right | TextPrintType::NoShadow | TextPrintType::Metal12 | TextPrintType::Background,
	true
	);
}

void __fastcall FakeTacticalClass::__DrawTimersB(int value, ColorScheme* color, int interval, const wchar_t* label, LARGE_INTEGER* _arg, bool* _arg1)
{
	BitFont* pFont = BitFont::BitFontPtr(TextPrintType::UseGradPal | TextPrintType::Right | TextPrintType::NoShadow | TextPrintType::Metal12 | TextPrintType::Background);

	const int hour = interval / 60 / 60;
	const int minute = interval / 60 % 60;
	const int second = interval % 60;
	static fmt::basic_memory_buffer<wchar_t> buffer;
	buffer.clear();
	if (hour)
	{
		fmt::format_to(std::back_inserter(buffer), L"{:02}:{:02}:{:02}", hour, minute, second);
	}
	else
	{
		fmt::format_to(std::back_inserter(buffer), L"{:02}:{:02}", minute, second);
	}
	buffer.push_back(L'\0');

	static fmt::basic_memory_buffer<wchar_t> labe_buffer;
	labe_buffer.clear();
	fmt::format_to(std::back_inserter(labe_buffer), L"{}  ", !label ? L"" : label);
	labe_buffer.push_back(L'\0');

	int width = 0;
	RectangleStruct rect_bound = DSurface::ViewBounds();
	pFont->GetTextDimension(buffer.data(), &width, nullptr, rect_bound.Width);
	ColorScheme* fore = color;

	if (!interval && _arg && _arg1)
	{
		const auto now = Game::AudioGetTime();
		if (static_cast<uint64_t>(_arg->QuadPart) < static_cast<uint64_t>(now.QuadPart)) {
			_arg->QuadPart = now.QuadPart + 1000;
			*_arg1 = !*_arg1;
		}

		if (*_arg1)
		{
			fore = ColorScheme::Array->Items[RulesExtData::Instance()->TimerBlinkColorScheme];
		}
	}

	int value_plusone = value + 1;
	Point2D point {
		rect_bound.Width - width - 3 ,
		rect_bound.Height - (value_plusone) * (pFont->field_1C + 2)
	};

	auto pComposite = DSurface::Composite();
	auto rect = pComposite->Get_Rect();
	Point2D _temp {};
	ColorStruct out {};
	color->BaseColor.ToColorStruct(&out);

	TextDrawing::Simple_Text_Print_Wide(
		&_temp,
		labe_buffer.data(),
		pComposite,
		&rect,
		&point,
		(COLORREF)out.ToInit(),
		(COLORREF)0u,
		TextPrintType::UseGradPal | TextPrintType::Right | TextPrintType::NoShadow | TextPrintType::Metal12 | TextPrintType::Background,
		true
	);

	point.X = rect_bound.Width - 3;
	point.Y = rect_bound.Height - value_plusone * (pFont->field_1C + 2);
	rect = pComposite->Get_Rect();
	fore->BaseColor.ToColorStruct(&out);

	TextDrawing::Simple_Text_Print_Wide(
	&_temp,
	buffer.data(),
	pComposite,
	&rect,
	&point,
	(COLORREF)out.ToInit(),
	(COLORREF)0u,
	TextPrintType::UseGradPal | TextPrintType::Right | TextPrintType::NoShadow | TextPrintType::Metal12 | TextPrintType::Background,
	true
	);
}

void __fastcall FakeTacticalClass::__DrawTimersC(int value, ColorScheme* color, int interval, const wchar_t* label, LARGE_INTEGER* _arg, bool* _arg1)
{
	BitFont* pFont = BitFont::BitFontPtr(TextPrintType::UseGradPal | TextPrintType::Right | TextPrintType::NoShadow | TextPrintType::Metal12 | TextPrintType::Background);

	const int hour = interval / 60 / 60;
	const int minute = interval / 60 % 60;
	const int second = interval % 60;
	static fmt::basic_memory_buffer<wchar_t> buffer;
	buffer.clear();

	if (hour)
	{
		fmt::format_to(std::back_inserter(buffer), L"{:02}:{:02}:{:02}", hour, minute, second);
	}
	else
	{
		fmt::format_to(std::back_inserter(buffer), L"{:02}:{:02}", minute, second);
	}
	buffer.push_back(L'\0');

	static fmt::basic_memory_buffer<wchar_t> labe_buffer;
	labe_buffer.clear();
	fmt::format_to(std::back_inserter(labe_buffer), L"{}  ", label);
	labe_buffer.push_back(L'\0');

	int width = 0;
	RectangleStruct rect_bound = DSurface::ViewBounds();
	pFont->GetTextDimension(buffer.data(), &width, nullptr, rect_bound.Width);
	ColorScheme* fore = color;

	if (!interval && _arg && _arg1)
	{
		const auto now = Game::AudioGetTime();
		if (static_cast<uint64_t>(_arg->QuadPart) < static_cast<uint64_t>(now.QuadPart)) {
			_arg->QuadPart = now.QuadPart + 1000;
			*_arg1 = !*_arg1;
		}

		if (*_arg1)
		{
			fore = ColorScheme::Array->Items[RulesExtData::Instance()->TimerBlinkColorScheme];
		}
	}

	int value_plusone = value + 1;
	Point2D point {
		rect_bound.Width - width - 3 ,
		rect_bound.Height - (value_plusone) * (pFont->field_1C + 2)
	};

	auto pComposite = DSurface::Composite();
	auto rect = pComposite->Get_Rect();
	Point2D _temp {};
	ColorStruct out {};
	color->BaseColor.ToColorStruct(&out);

	TextDrawing::Simple_Text_Print_Wide(
		&_temp,
		labe_buffer.data(),
		pComposite,
		&rect,
		&point,
		(COLORREF)out.ToInit(),
		(COLORREF)0u,
		TextPrintType::UseGradPal | TextPrintType::Right | TextPrintType::NoShadow | TextPrintType::Metal12 | TextPrintType::Background,
		true
	);

	point.X = rect_bound.Width - 3;
	point.Y = rect_bound.Height - value_plusone * (pFont->field_1C + 2);
	rect = pComposite->Get_Rect();
	fore->BaseColor.ToColorStruct(&out);

	TextDrawing::Simple_Text_Print_Wide(
	&_temp,
	buffer.data(),
	pComposite,
	&rect,
	&point,
	(COLORREF)out.ToInit(),
	(COLORREF)0u,
	TextPrintType::UseGradPal | TextPrintType::Right | TextPrintType::NoShadow | TextPrintType::Metal12 | TextPrintType::Background,
	true
	);
}

DEFINE_FUNCTION_JUMP(CALL, 0x6D49AA, FakeTacticalClass::__DrawTimersA);
DEFINE_FUNCTION_JUMP(CALL, 0x6D4B08, FakeTacticalClass::__DrawTimersB);
DEFINE_FUNCTION_JUMP(LJMP, 0x6D4B50, FakeTacticalClass::__DrawTimersC);

#else

void __fastcall FakeTacticalClass::__DrawTimers(int value, ColorScheme* color, int interval, const wchar_t* label, LARGE_INTEGER* _arg, bool* _arg1)
{
	BitFont* pFont = BitFont::BitFontPtr(TextPrintType::UseGradPal | TextPrintType::Right | TextPrintType::NoShadow | TextPrintType::Metal12 | TextPrintType::Background);

	const int hour = interval / 60 / 60;
	const int minute = interval / 60 % 60;
	const int second = interval % 60;
	fmt::basic_memory_buffer<wchar_t> buffer;

	if (hour)
	{
		fmt::format_to(std::back_inserter(buffer), L"{:02}:{:02}:{:02}", hour, minute, second);
	}
	else
	{
		fmt::format_to(std::back_inserter(buffer), L"{:02}:{:02}", minute, second);
	}

	buffer.push_back(L'\0');

	fmt::basic_memory_buffer<wchar_t> labe_buffer;
	fmt::format_to(std::back_inserter(labe_buffer), L"{}  ", label);
	labe_buffer.push_back(L'\0');

	int width = 0;
	RectangleStruct rect_bound = DSurface::ViewBounds();
	pFont->GetTextDimension(buffer.data(), &width, nullptr, rect_bound.Width);
	ColorScheme* fore = color;

	if (!interval && _arg && _arg1)
	{
		if ((unsigned __int64)_arg->QuadPart < (unsigned __int64)Game::AudioGetTime().QuadPart)
		{
			auto large = Game::AudioGetTime();
			_arg->LowPart = large.LowPart + 1000;
			_arg->HighPart = __CFADD__(large.LowPart, 1000) + large.HighPart;
			*_arg1 = !*_arg1;
		}

		if (*_arg1)
		{
			fore = ColorScheme::Array->Items[RulesExtData::Instance()->TimerBlinkColorScheme];
		}
	}

	int value_plusone = value + 1;
	Point2D point {
		rect_bound.Width - width - 3 ,
		rect_bound.Height - (value_plusone) * (pFont->field_1C + 2)
	};

	auto pComposite = DSurface::Composite();
	auto rect = pComposite->Get_Rect();
	Point2D _temp {};
	ColorStruct out {};
	color->BaseColor.ToColorStruct(&out);

	TextDrawing::Simple_Text_Print_Wide(
		&_temp,
		labe_buffer.data(),
		pComposite,
		&rect,
		&point,
		(COLORREF)out.ToInit(),
		(COLORREF)0u,
		TextPrintType::UseGradPal | TextPrintType::Right | TextPrintType::NoShadow | TextPrintType::Metal12 | TextPrintType::Background,
		true
	);

	point.X = rect_bound.Width - 3;
	point.Y = rect_bound.Height - value_plusone * (pFont->field_1C + 2);
	rect = pComposite->Get_Rect();
	fore->BaseColor.ToColorStruct(&out);

	TextDrawing::Simple_Text_Print_Wide(
	&_temp,
	buffer.data(),
	pComposite,
	&rect,
	&point,
	(COLORREF)out.ToInit(),
	(COLORREF)0u,
	TextPrintType::UseGradPal | TextPrintType::Right | TextPrintType::NoShadow | TextPrintType::Metal12 | TextPrintType::Background,
	true
	);
}

DEFINE_FUNCTION_JUMP(CALL, 0x6D49AA, FakeTacticalClass::__DrawTimers);
DEFINE_FUNCTION_JUMP(CALL, 0x6D4B08, FakeTacticalClass::__DrawTimers);
DEFINE_FUNCTION_JUMP(LJMP, 0x6D4B50, FakeTacticalClass::__DrawTimers);

#endif

#include <New/Entity/BannerClass.h>
#include <Ext/SWType/Body.h>

double GetSuperChargePercent(SuperClass* pSuper, bool backward = false)
{
	int rechargeTime = pSuper->GetRechargeTime();

	// Avoid division by zero
	if (rechargeTime <= 0)
		return backward ? 0.0 : 100.0;

	int timeLeft = pSuper->RechargeTimer.GetTimeLeft();

	// Already ready
	if (timeLeft <= 0)
		return backward ? 0.0 : 100.0;

	double ratio = (double)timeLeft / (double)rechargeTime;
	double percent = backward ? ratio * 100.0 : (1.0 - ratio) * 100.0;

	return std::clamp(percent, 0.0, 100.0);
}

void FakeTacticalClass::__DrawTimersSW(SuperClass* pSuper, int value, int interval)
{
	for (auto& pBanner : BannerClass::Array) {
		pBanner.Render();
	}

	BitFont* pFont = BitFont::BitFontPtr(TextPrintType::UseGradPal | TextPrintType::Right | TextPrintType::NoShadow | TextPrintType::Metal12 | TextPrintType::Background);

	auto pTypeExt = SWTypeExtContainer::Instance.Find(pSuper->Type);

	static fmt::basic_memory_buffer<wchar_t> buffer;
	buffer.clear();
	if (!pTypeExt->ChargeTimer) {
		 
		const auto percent = GetSuperChargePercent(pSuper , pTypeExt->ChargeTimer_Backwards);

		fmt::format_to(std::back_inserter(buffer), L"{:.2f} %", percent);

	} else {
		const int hour = interval / 60 / 60;
		const int minute = interval / 60 % 60;
		const int second = interval % 60;

		if (hour) {
			fmt::format_to(std::back_inserter(buffer), L"{:02}:{:02}:{:02}", hour, minute, second);
		} else {
			fmt::format_to(std::back_inserter(buffer), L"{:02}:{:02}", minute, second);
		}
	}
    buffer.push_back(L'\0');


	static fmt::basic_memory_buffer<wchar_t> labe_buffer;
	labe_buffer.clear();
	fmt::format_to(std::back_inserter(labe_buffer), L"{}  ", !pSuper->Type->UIName ? L"" : pSuper->Type->UIName);
	labe_buffer.push_back(L'\0');

	int width = 0;
	RectangleStruct rect_bound = DSurface::ViewBounds();
	pFont->GetTextDimension(buffer.data(), &width, nullptr, rect_bound.Width);
	ColorScheme* fore = ColorScheme::Array->Items[pSuper->Owner->ColorSchemeIndex];

	if (!interval)
	{
		const auto now = Game::AudioGetTime();
		if (static_cast<uint64_t>(pSuper->BlinkTimer.QuadPart) < static_cast<uint64_t>(now.QuadPart)) {
			pSuper->BlinkTimer.QuadPart = now.QuadPart + 1000;
			pSuper->BlinkState = !pSuper->BlinkState;
		}

		if (pSuper->BlinkState)
		{
			fore = ColorScheme::Array->Items[RulesExtData::Instance()->TimerBlinkColorScheme];
		}
	}

	int value_plusone = value + 1;
	Point2D point {
		rect_bound.Width - width - 3 ,
		rect_bound.Height - (value_plusone) * (pFont->field_1C + 2)
	};

	auto pComposite = DSurface::Composite();
	auto rect = pComposite->Get_Rect();
	Point2D _temp {};
	ColorStruct out {};
	ColorScheme::Array->Items[pSuper->Owner->ColorSchemeIndex]->BaseColor.ToColorStruct(&out);

	TextDrawing::Simple_Text_Print_Wide(
		&_temp,
		labe_buffer.data(),
		pComposite,
		&rect,
		&point,
		(COLORREF)out.ToInit(),
		(COLORREF)0u,
		TextPrintType::UseGradPal | TextPrintType::Right | TextPrintType::NoShadow | TextPrintType::Metal12 | TextPrintType::Background,
		true
	);

	point.X = rect_bound.Width - 3;
	point.Y = rect_bound.Height - value_plusone * (pFont->field_1C + 2);
	rect = pComposite->Get_Rect();
	fore->BaseColor.ToColorStruct(&out);

	TextDrawing::Simple_Text_Print_Wide(
	&_temp,
	buffer.data(),
	pComposite,
	&rect,
	&point,
	(COLORREF)out.ToInit(),
	(COLORREF)0u,
	TextPrintType::UseGradPal | TextPrintType::Right | TextPrintType::NoShadow | TextPrintType::Metal12 | TextPrintType::Background,
	true
	);
}

DEFINE_FUNCTION_JUMP(CALL, 0x6D4B2B, FakeTacticalClass::__DrawAllTacticalText)

bool __fastcall FakeTacticalClass::TypeSelectFilter(TechnoClass* pTechno, DynamicVectorClass<const char*>& names)
{
	const auto pTechnoType = pTechno->GetTechnoType();
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pTechnoType);
	const char* id = pTypeExt->GetSelectionGroupID(pTechnoType);

	if (std::ranges::none_of(names, [id](const char* pID) { return IS_SAME_STR_(pID, id); }))
		return false;

	if (pTechnoType->Gunner && !TacticalExtData::IFVGroups.empty() && (size_t)pTechno->CurrentWeaponNumber >= pTypeExt->WeaponGroupAs.size()) {
		auto gunnerID = &pTypeExt->WeaponGroupAs[pTechno->CurrentWeaponNumber];

		if (gunnerID->empty() || !GeneralUtils::IsValidString(gunnerID->c_str())) {
			sprintf_s(gunnerID->data(), 0x20, "%d", pTechno->CurrentWeaponNumber + 1);
		}

		if (std::ranges::none_of(TacticalExtData::IFVGroups, [gunnerID](const char* pID) {
			return IS_SAME_STR_(pID, gunnerID->c_str());
		}))
			return false;
	}

	if (pTechno->CanBeSelectedNow() || ((pTechno->WhatAmI() == BuildingClass::AbsID && (pTechnoType->UndeploysInto || RulesExtData::Instance()->BuildingTypeSelectable))))
		return true;

	return false;
}

IStream* TacticalExtData::g_pStm = nullptr;
std::unique_ptr<TacticalExtData> TacticalExtData::Data = nullptr;
std::vector<const char*> TacticalExtData::IFVGroups;

void TacticalExtData::Allocate(TacticalClass* pThis)
{
	Data = std::make_unique<TacticalExtData>();
	Data->AttachedToObject = pThis;
}

void TacticalExtData::Remove(TacticalClass* pThis)
{
	Data = nullptr;
}

void TacticalExtData::Screen_Flash_AI()
{
	static bool _is_fading_in = false;
	static bool _is_fading_out = false;
	static MSTimerClass _fading_in_timer;
	static MSTimerClass _fading_out_timer;
	static constexpr int FADE_IN_RATE = 1000 / 4;
	static constexpr int FADE_OUT_RATE = 1000 / 3;

	/**
	 *  If a screen flash was triggered, initialize the timers.
	 */

	if (IsPendingScreenFlash)
	{
		_is_fading_in = true;
		_is_fading_out = false;
		_fading_in_timer = FADE_IN_RATE;
		IsPendingScreenFlash = false;

	}

	/**
	 *  Fading from white to game.
	 */

	if (_is_fading_out)
	{
		if (_fading_out_timer.Expired())
		{
			_is_fading_out = false;
		}

		unsigned adjust = (_fading_out_timer.GetTimeLeft())*ScreenFlashTrans / FADE_OUT_RATE;
		DSurface::Composite->Fill_Rect_Trans(&TacticalClass::view_bound(), &ScreenFlashColor, adjust);
	}

	/**
	 *  Fading from game to white.
	 */

	if (_is_fading_in)
	{
		if (_fading_in_timer.Expired())
		{
			_is_fading_in = false;
		}

		unsigned adjust = (FADE_IN_RATE - _fading_in_timer.GetTimeLeft()) * ScreenFlashTrans / FADE_IN_RATE;
		DSurface::Composite->Fill_Rect_Trans(&TacticalClass::view_bound(), &ScreenFlashColor, adjust);

		/**
		 *  Flag and prepare the fade back timer.
		 */

		if (!_is_fading_in)
		{
			_is_fading_out = true;
			_fading_out_timer = FADE_OUT_RATE;
		}
	}
}

// =============================
// container hooks

ASMJIT_PATCH(0x6D1E24, TacticalClass_CTOR, 0x5)
{
	GET(TacticalClass*, pItem, ESI);

	TacticalExtData::Allocate(pItem);

	return 0;
}

ASMJIT_PATCH(0x6DC48E, TacticalClass_DTOR_A, 0xA)
{
	GET(TacticalClass*, pItem, ESI);
	TacticalExtData::Remove(pItem);
	return 0;
}

ASMJIT_PATCH(0x6D1E9B, TacticalClass_DTOR_B, 0xA)
{
	GET(TacticalClass*, pItem, ECX);
	TacticalExtData::Remove(pItem);
	return 0;
}

ASMJIT_PATCH(0x6DBE00, TacticalClass_SaveLoad_Prefix, 0x8)
{
	GET_STACK(IStream*, pStm, 0x8);

	TacticalExtData::g_pStm = pStm;

	return 0;
}ASMJIT_PATCH_AGAIN(0x6DBD20, TacticalClass_SaveLoad_Prefix, 0x7)

ASMJIT_PATCH(0x6DBDED, TacticalClass_Load_Suffix, 0x6)
{
	auto buffer = TacticalExtData::Instance();
	if (!buffer)
		Debug::FatalErrorAndExit("TacticalClassExt_Load Apparently TacticalExtData Global Pointer is missing !/n ");

	PhobosByteStream Stm(0);
	if (Stm.ReadFromStream(TacticalExtData::g_pStm))
	{
		PhobosStreamReader Reader(Stm);

		if (Reader.Expect(TacticalExtData::Canary) && Reader.RegisterChange(buffer))
			buffer->LoadFromStream(Reader);
	}

	return 0;
}

ASMJIT_PATCH(0x6DBE18, TacticalClass_Save_Suffix, 0x5)
{
	auto buffer = TacticalExtData::Instance();

	if (!buffer)
		Debug::FatalErrorAndExit("TacticalClassExt_Save Apparently TacticalExtData Global Pointer is missing !/n ");

	PhobosByteStream saver(sizeof(TacticalExtData));
	PhobosStreamWriter writer(saver);

 writer.Save(TacticalExtData::Canary);
 writer.Save(buffer);

	buffer->SaveToStream(writer);
	saver.WriteToStream(TacticalExtData::g_pStm);

	return 0;
}