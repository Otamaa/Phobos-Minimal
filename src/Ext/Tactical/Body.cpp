#include "Body.h"

#include <Ext/TechnoType/Body.h>
#include <Ext/Super/Body.h>

#include <Utilities/Macro.h>
#include <Utilities/Cast.h>

#include <TacticalClass.h>
#include <HouseClass.h>
#include <Unsorted.h>

#include <FPSCounter.h>
#include <BitFont.h>

#include <Phobos.h>

// Reversed from Is_Selectable, w/o Select call
bool FakeTacticalClass::ObjectClass_IsSelectable(ObjectClass* pThis)
{
	const auto pOwner = pThis->GetOwningHouse();
	return pOwner
		&& pOwner->ControlledByCurrentPlayer()
		&& pThis->CanBeSelected()
		&& pThis->CanBeSelectedNow();
}

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
	if (selectable.Techno
		&& selectable.Techno->IsAlive
		&& !selectable.Techno->InLimbo
		&& selectable.Techno->AbstractFlags & AbstractFlags::Techno
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
		if (this->IsInSelectionRect(rect, selected) && ObjectClass_IsSelectable(selected.Techno))
		{
			return !TechnoTypeExtContainer::Instance.Find(selected.Techno->GetTechnoType())->LowSelectionPriority;
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

	for (const auto& selected : Array)
	{
		if (this->IsInSelectionRect(pRect, selected))
		{
			const auto pTechno = selected.Techno;
			const auto pTechnoType = pTechno->GetTechnoType();
			const auto TypeExt = TechnoTypeExtContainer::Instance.Find(pTechnoType);

			if (bPriorityFiltering && TypeExt->LowSelectionPriority)
				continue;

			if (TypeExt && Game::IsTypeSelecting())
				Game::UICommands_TypeSelect_7327D0(TypeExt->GetSelectionGroupID());
			else if (fpCheckCallback)
				(*fpCheckCallback)(pTechno);
			else
			{
				const auto pBldType = type_cast<BuildingTypeClass*>(pTechnoType);
				const auto pOwner = pTechno->GetOwningHouse();

				if (pOwner
					&& pOwner->ControlledByCurrentPlayer()
					&& pTechno->CanBeSelected()
					&& (!pBldType || pBldType->IsUndeployable())
					)
				{
					Unsorted::MoveFeedback = !pTechno->Select();
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
COMPILETIMEEVAL int8 __CFADD__(T x, U y)
{
	if COMPILETIMEEVAL((sizeof(T) > sizeof(U) ? sizeof(T) : sizeof(U)) == 1)
		return uint8(x) > uint8(x + y);
	else if COMPILETIMEEVAL((sizeof(T) > sizeof(U) ? sizeof(T) : sizeof(U)) == 2)
		return uint16(x) > uint16(x + y);
	else if COMPILETIMEEVAL((sizeof(T) > sizeof(U) ? sizeof(T) : sizeof(U)) == 4)
		return uint32(x) > uint32(x + y);
	else
		return unsigned __int64(x) > unsigned __int64(x + y);
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
		size = (int)((radius + 0.5) / Math::sqrt(2.0) * double(Unsorted::CellWidthInPixels)); // should be cell size global?
	}

	Point2D center_pixel = TacticalClass::Instance->CoordsToClient(center_coord);

	center_pixel.X += DSurface::ViewBounds().X;
	center_pixel.Y += DSurface::ViewBounds().Y;

	RectangleStruct draw_area(
		center_pixel.Y - size / 2,
		center_pixel.X - size,
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
	static const double _line_alpha[] = {
		//0.05, 0.20, 0.40, 1.0                     // original values.
		0.05, 0.10, 0.20, 0.40, 0.60, 0.80, 1.0     // new values.
	};

	static const int _rate = 50;

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

		if (std::fabs(angle - Math::DEG_TO_RADF(90)) < 0.001)
		{

			line_start = center_pixel;
			line_end = Point2D(center_pixel.X, int(center_pixel.Y + (-size_half)));

		}
		else if (std::fabs(angle - Math::DEG_TO_RADF(270)) < 0.001)
		{

			line_start = center_pixel;
			line_end = Point2D(center_pixel.X, int(center_pixel.Y + size_half));

		}
		else
		{

			double angle_tan = Math::tan(angle);
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

		bool enable_red_channel = false;
		bool enable_green_channel = true;
		bool enable_blue_channel = false;

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

#ifndef ___test

void __fastcall FakeTacticalClass::__DrawTimersA(int value, ColorScheme* color, int interval, const wchar_t* label, LARGE_INTEGER* _arg, bool* _arg1)
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
	fmt::format_to(std::back_inserter(labe_buffer), L"{}  ", !label ? L"" : label);
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

	Simple_Text_Print_Wide(
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

	Simple_Text_Print_Wide(
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
	fmt::format_to(std::back_inserter(labe_buffer), L"{}  ", !label ? L"" : label);
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

	Simple_Text_Print_Wide(
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

	Simple_Text_Print_Wide(
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

	Simple_Text_Print_Wide(
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

	Simple_Text_Print_Wide(
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

	Simple_Text_Print_Wide(
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

	Simple_Text_Print_Wide(
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

void FakeTacticalClass::__DrawTimersSW(SuperClass* pSuper, int value, int interval)
{
	for (auto& pBanner : BannerClass::Array) {
		pBanner.Render();
	}

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
	fmt::format_to(std::back_inserter(labe_buffer), L"{}  ", !pSuper->Type->UIName ? L"" : pSuper->Type->UIName);
	labe_buffer.push_back(L'\0');

	int width = 0;
	RectangleStruct rect_bound = DSurface::ViewBounds();
	pFont->GetTextDimension(buffer.data(), &width, nullptr, rect_bound.Width);
	ColorScheme* fore = ColorScheme::Array->Items[pSuper->Owner->ColorSchemeIndex];

	if (!interval)
	{
		if ((unsigned __int64)pSuper->BlinkTimer.QuadPart < (unsigned __int64)Game::AudioGetTime().QuadPart)
		{
			auto large = Game::AudioGetTime();
			pSuper->BlinkTimer.LowPart = large.LowPart + 1000;
			pSuper->BlinkTimer.HighPart = __CFADD__(large.LowPart, 1000) + large.HighPart;
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

	Simple_Text_Print_Wide(
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

	Simple_Text_Print_Wide(
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

IStream* TacticalExtData::g_pStm = nullptr;
std::unique_ptr<TacticalExtData> TacticalExtData::Data = nullptr;

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
	static const int FADE_IN_RATE = 1000 / 4;
	static const int FADE_OUT_RATE = 1000 / 3;

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
// load / save

template <typename T>
void TacticalExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Initialized)
		;
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
	if (Stm.ReadBlockFromStream(TacticalExtData::g_pStm))
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
	saver.WriteBlockToStream(TacticalExtData::g_pStm);

	return 0;
}