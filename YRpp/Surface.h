#pragma once

#include <GeneralStructures.h>
#include <YRAllocator.h>
#include <RectangleStruct.h>
#include <CoordStruct.h>
#include <ColorStruct.h>

#include <Helpers/CompileTime.h>
#include <Helpers/VTable.h>

#include <ddraw.h>

class Blitter;
class ConvertClass;
struct SHPStruct;
class ColorScheme;
class BitFont;
class NOVTABLE Surface
{
public:
	static COMPILETIMEEVAL OPTIONALINLINE DWORD vtable = 0x7E2198;
	static COMPILETIMEEVAL reference<bool*, 0x84310Cu> const Target_Laser_Draw_Pattern{};

	Surface() : Width(0), Height(0) { VTable::Set(this, vtable); }
	Surface(int width, int height) JMP_THIS(0x4AEC60);
	virtual ~Surface() { JMP_THIS(0x4115D0); }

	virtual bool Copy_From(RectangleStruct& toarea, RectangleStruct& toRectangleStruct, Surface* fromsurface, RectangleStruct& fromarea, RectangleStruct& fromRectangleStruct, bool trans_blit = false, bool a7 = true) PURE;
	virtual bool Copy_From(RectangleStruct& toRectangleStruct, Surface* fromsurface, RectangleStruct& fromRectangleStruct, bool trans_blit = false, bool a5 = true) PURE;
	virtual bool Copy_From(Surface* fromsurface, bool trans_blit = false, bool a3 = true)PURE;
	virtual bool Fill_Rect(RectangleStruct& RectangleStruct, unsigned color)PURE;
	virtual bool Fill_Rect(RectangleStruct& area, RectangleStruct& RectangleStruct, unsigned color) PURE;
	virtual bool Fill(unsigned color) PURE;
	//virtual bool Fill_Rect_Trans(RectangleStruct& RectangleStruct, const ColorStruct& color, unsigned opacity) PURE;
	virtual bool Fill_Rect_Trans(RectangleStruct* pClipRect, ColorStruct* pColor, int nOpacity) PURE;

	virtual bool Draw_Ellipse(Point2D center, int radius_x, int radius_y, RectangleStruct clip, unsigned color) PURE;
	virtual bool Put_Pixel(Point2D& point, unsigned color) PURE;
	virtual unsigned Get_Pixel(Point2D& point) PURE;
	virtual bool Draw_Line(Point2D& start, Point2D& end, unsigned color) PURE;
	virtual bool Draw_Line_Rect(RectangleStruct& area, Point2D& start, Point2D& end, unsigned color) PURE;

	virtual bool DrawLineColor_AZ(RectangleStruct& area, Point2D& start, Point2D& end, unsigned color, int a5, int a6, bool z_only = false) PURE;
	virtual bool DrawMultiplyingLine_AZ(RectangleStruct& area, Point2D& start, Point2D& end, int a4, int a5, int a6, bool a7 = false) PURE;
	virtual bool DrawSubtractiveLine_AZ(RectangleStruct& area, Point2D& start, Point2D& end, ColorStruct& color, int a5, int a6, bool a7, bool a8, bool a9, bool a10, float a11) PURE;

	virtual bool DrawRGBMultiplyingLine_AZ(
		RectangleStruct* pRect, Point2D* pStart, Point2D* pEnd, ColorStruct* pColor,
		float Intensity, DWORD dwUnk1, DWORD dwUnk2) PURE;

	virtual bool PlotLine(RectangleStruct& area, Point2D& start, Point2D& end, void(*drawer_callback)(Point2D&)) PURE;

	virtual int Draw_Dashed_Line(Point2D& start, Point2D& end, unsigned color, bool* pattern, int offset) PURE;
	virtual int DrawDashedLine_(Point2D& start, Point2D& end, unsigned color, bool* pattern, int offset, bool a6) PURE;
	virtual bool DrawLine_(Point2D& start, Point2D& end, unsigned color, bool a4 = false) PURE;

	virtual bool Draw_Rect(RectangleStruct& RectangleStruct, unsigned color) PURE;
	virtual bool Draw_Rect(RectangleStruct& area, RectangleStruct& RectangleStruct, unsigned color) PURE;

	virtual void* Lock(int x = 0, int y = 0) PURE;
	virtual bool Unlock() PURE;
	virtual bool Can_Lock(int x = 0, int y = 0) const JMP_THIS(0x4114F0);

	virtual bool vt_entry_68(int x = 0, int y = 0) const JMP_THIS(0x411500);

	virtual bool Is_Locked() const PURE;
	virtual int Get_Bytes_Per_Pixel() const PURE;
	virtual int Get_Pitch() const PURE;
	virtual RectangleStruct Get_Rect() const JMP_THIS(0x411510);
	virtual int Get_Width() const JMP_THIS(0x411540);
	virtual int Get_Height() const JMP_THIS(0x411550);

	virtual bool IsDSurface() const JMP_THIS(0x4115C0); // guessed - secsome

	RectangleStruct Get_Rect_WithoutBottomBar() {
		auto nRect = Get_Rect();
		nRect.Height -= 32;
		return nRect;
	}

	bool Clear() { return Fill(0u); }
	bool Clear(RectangleStruct& area) { return Fill_Rect(area, 0u); }


	bool Blit(
		RectangleStruct pClipRect,
		RectangleStruct pClipRect2,	//again? hmm
		Surface* pSrc,
		RectangleStruct pDestRect,	//desired dest rect of pSrc ? (stretched? clipped?)
		RectangleStruct pSrcRect,	//desired source rect of pSrc ?
		bool bUnk1,
		bool bUnk2)
	{
		return Copy_From(pClipRect, pClipRect2, pSrc, pDestRect, pSrcRect, bUnk1, bUnk2);
	}

	bool Blit_Alter(
		RectangleStruct* pClipRect,
		RectangleStruct* pClipRect2,	//again? hmm
		Surface* pSrc,
		RectangleStruct* pDestRect,	//desired dest rect of pSrc ? (stretched? clipped?)
		RectangleStruct* pSrcRect,	//desired source rect of pSrc ?
		bool bUnk1,
		bool bUnk2)
	{
		return Copy_From(*pClipRect, *pClipRect2, pSrc, *pDestRect, *pSrcRect, bUnk1, bUnk2);
	}
	void DrawText_Old(const wchar_t* pText, RectangleStruct* pBounds, Point2D* pLocation, DWORD dwColor, DWORD unknown5, DWORD flags)
	{
		Point2D tmp = { 0, 0 };

		PUSH_VAR32(flags);
		PUSH_VAR32(unknown5);		//???
		PUSH_VAR32(dwColor);
		PUSH_VAR32(pLocation);
		PUSH_VAR32(pBounds);
		PUSH_VAR32(this);
		PUSH_VAR32(pText);
		PUSH_PTR(tmp);
		ASM_CALL(0x4A60E0);

		ADD_ESP(0x20);
	}

	void DrawText_Old(const wchar_t* pText, Point2D* pLoction, DWORD dwColor)
	{
		RectangleStruct rect = { 0, 0, 0, 0 };
		PUSH_PTR(rect);
		THISCALL(0x411510);

		DrawText_Old(pText, &rect, pLoction, dwColor, 0, 0x16);
	}

	void DrawText_Old(const wchar_t* pText, int X, int Y, DWORD dwColor)
	{
		Point2D P = { X ,Y };
		DrawText_Old(pText, &P, dwColor);
	}

	bool Blit(
		RectangleStruct& pClipRect,
		RectangleStruct& pClipRect2,	//again? hmm
		Surface* pSrc,
		RectangleStruct& pDestRect,	//desired dest rect of pSrc ? (stretched? clipped?)
		RectangleStruct& pSrcRect,	//desired source rect of pSrc ?
		bool bUnk1,
		bool bUnk2)
	{
		return Copy_From(pClipRect, pClipRect2, pSrc, pDestRect,pSrcRect, bUnk1, bUnk2);
	}

	bool DrawDashedLine(Point2D& pStart, Point2D& pEnd, int nColor, int nOffset) {
		return Draw_Dashed_Line(pStart, pEnd, nColor, Target_Laser_Draw_Pattern.get(), nOffset);
	}

public:
	int Width;
	int Height;
};
static_assert(sizeof(Surface) == 0xC, "Invalid Size !");

class NOVTABLE XSurface : public Surface
{
public:

	static OPTIONALINLINE COMPILETIMEEVAL DWORD vtable = 0x7E2104;

	XSurface() : Surface(), LockLevel(0), BytesPerPixel(0) { VTable::Set(this , vtable); }
	XSurface(int width, int height) JMP_THIS(0x5FE020);
	XSurface(int width, int height, int bpp) : Surface(width, height), LockLevel(0), BytesPerPixel(bpp) { VTable::Set(this, vtable); }
	virtual ~XSurface() { JMP_THIS(0x4115A0); }

	virtual bool Copy_From(RectangleStruct& toarea, RectangleStruct& torect, Surface* fromsurface, RectangleStruct& fromarea, RectangleStruct& fromrect, bool trans_blit = false, bool a7 = true) override JMP_THIS(0x7BBCF0);
	virtual bool Copy_From(RectangleStruct& torect, Surface* fromsurface, RectangleStruct& fromrect, bool trans_blit = false, bool a5 = true) override JMP_THIS(0x7BBB90);
	virtual bool Copy_From(Surface* fromsurface, bool trans_blit = false, bool a3 = true) override JMP_THIS(0x7BBAF0);
	virtual bool Fill_Rect(RectangleStruct& rect, unsigned color) override JMP_THIS(0x7BB020);
	virtual bool Fill_Rect(RectangleStruct& area, RectangleStruct& rect, unsigned color) override JMP_THIS(0x7BB050);
	virtual bool Fill(unsigned color) override JMP_THIS(0x7BBAB0);
	//virtual bool Fill_Rect_Trans(RectangleStruct& rect, const ColorStruct& color, unsigned opacity) override
	virtual bool Fill_Rect_Trans(RectangleStruct* pClipRect, ColorStruct* pColor, int nOpacity) JMP_THIS(0x7BB340);
	virtual bool Draw_Ellipse(Point2D point, int radius_x, int radius_y, RectangleStruct clip, unsigned color) override JMP_THIS(0x7BB350);
	virtual bool Put_Pixel(Point2D& point, unsigned color) override JMP_THIS(0x7BAEB0);
	virtual unsigned Get_Pixel(Point2D& point) override JMP_THIS(0x7BAE60);
	virtual bool Draw_Line(Point2D& start, Point2D& end, unsigned color) override JMP_THIS(0x7BA5E0);
	virtual bool Draw_Line_Rect(RectangleStruct& area, Point2D& start, Point2D& end, unsigned color) override JMP_THIS(0x7BA610);

	virtual bool DrawLineColor_AZ(RectangleStruct& area, Point2D& start, Point2D& end, unsigned color, int a5, int a6, bool z_only = false) override R0;
	virtual bool DrawMultiplyingLine_AZ(RectangleStruct& area, Point2D& start, Point2D& end, int a4, int a5, int a6, bool a7 = false) override R0;
	virtual bool DrawSubtractiveLine_AZ(RectangleStruct& area, Point2D& start, Point2D& end, ColorStruct& color, int a5, int a6, bool a7, bool a8, bool a9, bool a10, float a11) override R0;

	 virtual bool DrawRGBMultiplyingLine_AZ(
		 RectangleStruct* pRect, Point2D* pStart, Point2D* pEnd, ColorStruct* pColor,
		 float Intensity, DWORD dwUnk1, DWORD dwUnk2) override  R0;
	// {
	//	 return false;
	// }

	virtual bool PlotLine(RectangleStruct& area, Point2D& start, Point2D& end, void(*drawer_callback)(Point2D&)) override JMP_THIS(0x7BAB90);

	virtual int Draw_Dashed_Line(Point2D& start, Point2D& end, unsigned color, bool* pattern, int offset) override JMP_THIS(0x7BA8C0);
	virtual int DrawDashedLine_(Point2D& start, Point2D& end, unsigned color, bool* pattern, int offset, bool a6) override R0;
	virtual bool DrawLine_(Point2D& start, Point2D& end, unsigned color, bool a4 = false) override  R0;

	virtual bool Draw_Rect(RectangleStruct& rect, unsigned color) override JMP_THIS(0x7BAD90);
	virtual bool Draw_Rect(RectangleStruct& area, RectangleStruct& rect, unsigned color) override JMP_THIS(0x7BADC0);

	virtual void* Lock(int x = 0, int y = 0) override JMP_THIS(0x411560);
	virtual bool Unlock() override JMP_THIS(0x411570);
	virtual bool Is_Locked() const override JMP_THIS(0x411580);
	virtual int Get_Bytes_Per_Pixel() const override R0;
	virtual int Get_Pitch() const override R0;
	virtual bool IsDSurface() const override R0;

	virtual bool Put_Pixel_Clip(Point2D& point, unsigned color, RectangleStruct& rect) JMP_THIS(0x7BAF90);
	virtual unsigned Get_Pixel_Clip(Point2D& point, RectangleStruct& rect) JMP_THIS(0x7BAF10);

	void Fill_Circle(const Point2D center, unsigned radius, RectangleStruct rect, unsigned color) JMP_THIS(0x7BB920);

	void DrawEllipse(int CenterX, int CenterY, double CellSpread, COLORREF nColor)
	{
		RectangleStruct rect = this->Get_Rect_WithoutBottomBar();
		DrawEllipse(CenterX, CenterY, CellSpread, rect, nColor);
	}

	void DrawEllipse(int CenterX, int CenterY, double CellSpread, RectangleStruct Rect, COLORREF nColor)
	{
		double factor = (CellSpread * 2 + 1) / Math::SQRT_EIGHT;

		int semiMajor = static_cast<int>(factor * Unsorted::CellWidthInPixels);
		int semiMinor = static_cast<int>(factor * Unsorted::CellHeightInPixels);
		const Point2D nPoints { CenterX, CenterY };
		Draw_Ellipse(nPoints, semiMajor, semiMinor, Rect, nColor);
	}

public:
	int LockLevel;
	int BytesPerPixel;
};
static_assert(sizeof(XSurface) == 0x14, "Invalid Size !");

class NOVTABLE BSurface : public XSurface
{
public:
	static OPTIONALINLINE COMPILETIMEEVAL DWORD vtable = 0x7E2070;
	static COMPILETIMEEVAL reference<BSurface, 0xB2D928> const VoxelSurface {};

	BSurface() : XSurface(), BufferPtr() { VTable::Set(this, vtable); }
	BSurface(int width, int height, int bpp, void* buffer) : XSurface(width, height, bpp), BufferPtr((void*)buffer, int((height* width)* bpp)) { VTable::Set(this, vtable); }
	virtual ~BSurface() { JMP_THIS(0x411650); }

	virtual void* Lock(int x = 0, int y = 0) override JMP_THIS(0x4115F0);
	virtual int Get_Bytes_Per_Pixel() const override JMP_THIS(0x411630);
	virtual int Get_Pitch() const override JMP_THIS(0x411640);

	void DestroyBuffer() {
		BufferPtr.~MemoryBuffer();
	}

	static BSurface* __fastcall GetVoxelSurface() {
		JMP_FAST(0x753C70);
	}

protected:
	void* Get_Buffer_Ptr() const { return BufferPtr.Get_Buffer(); }
	void* Get_Buffer_Ptr(int x, int y) { return (unsigned char*)BufferPtr.Get_Buffer() + (x * Get_Bytes_Per_Pixel()) + (y * Get_Pitch()); }

public:
	DECLARE_PROPERTY(MemoryBuffer , BufferPtr);
};
static_assert(sizeof(BSurface) == 0x20, "Invalid Size !");

#pragma warning(push)
#pragma warning(disable : 4505)
#pragma region CommonFunction
//static long Surface_Size_Of_Region(Surface& surface, int w, int h);

static bool __fastcall  Animated_Text_Print_623880(Surface *surface ,RectangleStruct* rect, const wchar_t* text, size_t coint , BitFont* font , uint32_t color , size_t* anim, bool hasfocus, bool blankout, bool fillbg, size_t animsize) {
	JMP_FAST(0x623880);
}

static bool __fastcall Buffer_To_RLE_Surface_With_Z_Shape(Surface *surface1, Point2D *point1, RectangleStruct *rect1, Surface *surface2, Point2D *point2, RectangleStruct *rect2, void* blitter, int height_offset, int somearrayindex, int a10, int arg_20, Surface *z_shape_surface, int shape_x_offset, int shape_y_offset, int useotherblitterset) {
	JMP_FAST(0x437A10);
}

static bool __fastcall Buffer_To_Surface_with_LastArg(Surface *tosurface, RectangleStruct *rect1, RectangleStruct *torect, Surface *fromsurface, RectangleStruct *a5, RectangleStruct *fromrect, void* blitter, int z_val, int somearrayindex, int alpha_val, int use_new_blitter, int wrap_value) {
	JMP_FAST(0x4373B0);
}

static bool __fastcall Surface_To_Buffer(Surface& surface, RectangleStruct& rect, MemoryBuffer& buffer) {
	JMP_FAST(0x4371D0);
}

static bool __fastcall Buffer_To_Surface(Surface& surface, RectangleStruct& rect, MemoryBuffer& buffer) {
	JMP_FAST(0x437290);
}

//static bool Copy_To_Surface(Surface& tosurface, RectangleStruct& torect, Surface& fromsurface, RectangleStruct& fromrect, BlitterCore& blitter, int z_val = 0, ZGradient z_gradient = ZGradient::Ground, int alpha_level = 1000, int warp_val = 0);
//static bool Copy_To_Surface(Surface& tosurface, RectangleStruct& toarea, RectangleStruct& torect, Surface& fromsurface, RectangleStruct fromarea, RectangleStruct& fromrect, BlitterCore& blitter, int z_val = 0, ZGradient z_gradient = ZGradient::Ground, int alpha_level = 1000, int warp_val = 0);

//static bool Copy_To_Surface_RLE(Surface& tosurface, RectangleStruct& torect, Surface& fromsurface, RectangleStruct& fromrect, RLEBlitterCore& blitter, int z_val = 0, ZGradient z_gradient = ZGradient::Ground, int alpha_level = 1000, int warp_val = 0);
//static bool Copy_To_Surface_RLE(Surface& tosurface, RectangleStruct& toarea, RectangleStruct& torect, Surface& fromsurface, RectangleStruct& fromarea, RectangleStruct& fromrect, RLEBlitterCore& blitter, int z_val = 0, ZGradient z_gradient = ZGradient::Ground, int alpha_level = 1000, int warp_val = 0, Surface* z_shape_surface = nullptr, int z_xoff = 0, int z_yoff = 0);

// Used for linetrail
static bool __fastcall Surface_4BEAC0_Blit(Surface* Surface, RectangleStruct& nRect, Point2D& nPoint1, Point2D& nPoint2, ColorStruct& nSomeval, unsigned nSomeval2, int nAdjust_1, int nAdjust2)
{
	JMP_FAST(0x4BEAC0);
}

static RectangleStruct* __fastcall GetTextBox(RectangleStruct* pRet, char* text, int xpos, int ypos, TextPrintType flag, int x_offset, int y_offset)
{
	JMP_FAST(0x4A59E0);
}

// Comments from thomassneddon
static void __fastcall CC_Draw_Shape(Surface* Surface, ConvertClass* Palette, SHPStruct* SHP, int FrameIndex,
	const Point2D* const Position, const RectangleStruct* const Bounds, BlitterFlags Flags,
	int Remap,
	int ZAdjust, // + 1 = sqrt(3.0) pixels away from screen
	ZGradient ZGradientDescIndex,
	int Brightness, // 0~2000. Final color = saturate(OriginalColor * Brightness / 1000.0f)
	int TintColor, SHPStruct* ZShape, int ZShapeFrame, int XOffset, int YOffset)
{
	JMP_FAST(0x4AED70);
}

static void __fastcall CC_Draw_Shape(Surface* Surface, ConvertClass* Palette, SHPStruct* SHP, int FrameIndex,
	const Point2D* const Position, const RectangleStruct* const Bounds, DWORD Flags,
	int Remap,
	int ZAdjust, // + 1 = sqrt(3.0) pixels away from screen
	DWORD ZGradientDescIndex, //
	int Brightness, // 0~2000. Final color = saturate(OriginalColor * Brightness / 1000.0f)
	int TintColor, SHPStruct* ZShape, int ZShapeFrame, int XOffset, int YOffset)
{
	JMP_FAST(0x4AED70);
}

static Point2D *__fastcall Plain_Text_Print_Wide(
	Point2D* retstr,
	wchar_t* text,
	Surface* surface,
	RectangleStruct* rect,
	Point2D* xy,
	int fore,
	int back,
	TextPrintType flag,
	int scheme,
	int a9){
		JMP_FAST(0x4A66D0);
	}

// this text drawing can accept vargs , so just put the thing here , dont need to do it twice
static Point2D* __cdecl Fancy_Text_Print_Wide(Point2D* RetVal, const wchar_t* Text, Surface* Surface, RectangleStruct* Bounds,
	Point2D* Location, unsigned int ForeColor, unsigned int BackColor, TextPrintType Flag, ...)
{
	JMP_STD(0x4A60E0);
}

// this text drawing can accept vargs , so just put the thing here , dont need to do it twice
static Point2D* __cdecl Fancy_Text_Print_Wide_REF(Point2D* RetVal, const wchar_t* Text, Surface* Surface, RectangleStruct* Bounds,
	Point2D* Location, COLORREF ForeColor, COLORREF BackColor, TextPrintType Flag, ...)
{
	JMP_STD(0x4A60E0);
}

// this text drawing can accept vargs , so just put the thing here , dont need to do it twice
static Point2D* __cdecl Fancy_Text_Print_Wide(const Point2D& retBuffer, const wchar_t* Text, Surface* Surface, const RectangleStruct& Bounds,
	const Point2D& Location, ColorScheme* ForeScheme, ColorScheme* BackScheme, TextPrintType Flag, ...)
{
	JMP_STD(0x4A61C0);
}

// this text drawing can accept vargs , so just put the thing here , dont need to do it twice
static Point2D* __cdecl Fancy_Text_Print_Wide(Point2D* RetVal, const wchar_t* Text, Surface* Surface, RectangleStruct* Bounds,
	Point2D* Location, ColorScheme* fore, unsigned int BackColor, TextPrintType Flag, ...)
{
	JMP_STD(0x4A61C0);
}

//
static Point2D* __fastcall Simple_Text_Print_Wide(Point2D* RetVal, const wchar_t* Text, Surface* Surface, RectangleStruct* Bounds,
	Point2D* Location, COLORREF ForeColor, COLORREF BackColor, TextPrintType Flag, bool bUkn)
{
	JMP_FAST(0x4A5EB0);
}

static void __fastcall Draw_Radial_Indicator(bool draw_line, bool adjust_color, CoordStruct coord, ColorStruct rgb, float line_mult, bool a8, bool a9)
{
	JMP_FAST(0x456980);
}

static bool __fastcall Buffer_To_Surface_wrapper(Surface *tosurface, RectangleStruct *torect, Surface *fromsurface, RectangleStruct *fromrect) {
	JMP_FAST(0x7BC1F0);
}
static bool __fastcall Buffer_To_Surface_wrapper(Surface *tosurface, RectangleStruct *torect, Surface *fromsurface, RectangleStruct *fromrect, void* blitter, int z_val, int somearrayindex, int alpha_val, int Blit_Move_2_arg) {
	JMP_FAST(0x437350);
}

static bool __fastcall Blit_helper_lockregion(Surface* dst_surf, RectangleStruct* rect1, RectangleStruct* rect2, Surface* src_surf, RectangleStruct* rect3, RectangleStruct* rect4, bool* checkme, __int16* dst_buffer, __int16* src_buffer)
{ JMP_FAST(0x7BC040); }

static bool __fastcall Allocate_Surfaces(RectangleStruct *common_rect, RectangleStruct *composite_rect, RectangleStruct *tile_rect, RectangleStruct *sidebar_rect, char alloc_hidden_surf)
{ JMP_FAST(0x533FD0); }

#pragma warning(pop)
#pragma endregion CommonFunction

class NOVTABLE DSurface : public XSurface
{
public:
	static COMPILETIMEEVAL reference<DSurface*, 0x8872FCu> const Tile{};
	static COMPILETIMEEVAL reference<DSurface*, 0x887300u> const Sidebar{};
	static COMPILETIMEEVAL reference<DSurface*, 0x887308u> const Primary{};
	static COMPILETIMEEVAL reference<DSurface*, 0x88730Cu> const Hidden{};
	static COMPILETIMEEVAL reference<DSurface*, 0x887310u> const Alternate{};
	static COMPILETIMEEVAL reference<DSurface*, 0x887314u> const Temp{};
	static COMPILETIMEEVAL reference<DSurface*, 0x887314u> const Hidden_2{};
	static COMPILETIMEEVAL reference<DSurface*, 0x88731Cu> const Composite{};
	static COMPILETIMEEVAL reference<int, 0x8205D0u> const RGBMode{};

	static COMPILETIMEEVAL reference<RectangleStruct, 0x886F90u> const SidebarBounds{};
	static COMPILETIMEEVAL reference<RectangleStruct, 0x886FA0u> const ViewBounds{};
	static COMPILETIMEEVAL reference<RectangleStruct, 0x886FB0u> const WindowBounds{};

	static COMPILETIMEEVAL reference<unsigned, 0x8A0DD0u> const RedLeft{};
	static COMPILETIMEEVAL reference<unsigned, 0x8A0DD4u> const RedRight{};
	static COMPILETIMEEVAL reference<unsigned, 0x8A0DE0u> const GreenLeft{};
	static COMPILETIMEEVAL reference<unsigned, 0x8A0DE4u> const GreenRight{};
	static COMPILETIMEEVAL reference<unsigned, 0x8A0DD8u> const BlueLeft{};
	static COMPILETIMEEVAL reference<unsigned, 0x8A0DDCu> const BlueRight{};

	static COMPILETIMEEVAL reference<int, 0x8205D0u> const RGBPixelFormat {};
	static COMPILETIMEEVAL reference<bool, 0x8A0DEEu> const AllowStretchBlits {};
	static COMPILETIMEEVAL reference<bool, 0x8205D4u> const AllowHardwareBlitFills {};

	static COMPILETIMEEVAL reference<bool*, 0x84310C> const PatternData {};

	static int __fastcall ColorMode() { JMP_STD(0x4BBC90); }

	// Comments from thomassneddon
	void DrawSHP(ConvertClass* Palette, SHPStruct* SHP, int FrameIndex,
		const Point2D* const Position, const RectangleStruct* const Bounds, BlitterFlags Flags, int Remap,
		int ZAdjust, // + 1 = sqrt(3.0) pixels away from screen
		ZGradient ZGradientDescIndex,
		int Brightness, // 0~2000. Final color = saturate(OriginalColor * Brightness / 1000.0f)
		int TintColor, SHPStruct* ZShape, int ZShapeFrame, int XOffset, int YOffset)
	{
		CC_Draw_Shape(this, Palette, SHP, FrameIndex, Position, Bounds, Flags, Remap, ZAdjust,
			ZGradientDescIndex, Brightness, TintColor, ZShape, ZShapeFrame, XOffset, YOffset);
	}

	void DrawSHP(ConvertClass* Palette, SHPStruct* SHP, int FrameIndex,
		const Point2D* const Position, const RectangleStruct* const Bounds, BlitterFlags Flags, int Remap,
		int ZAdjust, // + 1 = sqrt(3.0) pixels away from screen
		DWORD ZGradientDescIndex,
		int Brightness, // 0~2000. Final color = saturate(OriginalColor * Brightness / 1000.0f)
		int TintColor, SHPStruct* ZShape, int ZShapeFrame, int XOffset, int YOffset)
	{
		CC_Draw_Shape(this, Palette, SHP, FrameIndex, Position, Bounds, Flags, Remap, ZAdjust,
			ZGradient(ZGradientDescIndex), Brightness, TintColor, ZShape, ZShapeFrame, XOffset, YOffset);
	}


	void DSurfaceDrawText(const wchar_t* pText, RectangleStruct* pBounds, Point2D* pLocation,
		COLORREF ForeColor, COLORREF BackColor, TextPrintType Flag)
	{
		Point2D tmp = { 0, 0 };

		Fancy_Text_Print_Wide_REF(&tmp, pText, this, pBounds, pLocation, ForeColor, BackColor, Flag);
	}

	void DrawColorSchemeText(const wchar_t* pText, RectangleStruct& pBounds, Point2D& pLocation,
	ColorScheme* ForeColor, ColorScheme* BackColor, TextPrintType Flag)
	{
		Point2D tmp = { 0, 0 };
		Fancy_Text_Print_Wide(tmp, pText, this, pBounds, pLocation, ForeColor, BackColor, Flag);
	}

	void DSurfaceDrawText(const wchar_t* pText, Point2D* pLoction, COLORREF Color)
	{
		RectangleStruct rect = this->Get_Rect();
		Point2D tmp{ 0,0 };
		Fancy_Text_Print_Wide_REF(&tmp, pText, this, &rect, pLoction, Color, 0, TextPrintType::NoShadow);
	}

	void DSurfaceDrawText(const wchar_t* pText, int X, int Y, COLORREF Color)
	{
		Point2D P = { X ,Y };
		DSurfaceDrawText(pText, &P, Color);
	}

	DSurface() JMP_THIS(0x4BA720);
	DSurface(int width, int height, bool system_mem = false) JMP_THIS(0x4BA5A0);
	DSurface(LPDIRECTDRAWSURFACE surface) JMP_THIS(0x4BAC60);
	virtual ~DSurface() JMP_THIS(0x4BA6B0);

	virtual bool Copy_From(RectangleStruct& toarea, RectangleStruct& torect, Surface* fromsurface, RectangleStruct& fromarea, RectangleStruct& fromrect, bool trans_blit = false, bool a7 = true) override JMP_THIS(0x4BB0D0);
	virtual bool Copy_From(RectangleStruct& torect, Surface* fromsurface, RectangleStruct& fromrect, bool trans_blit = false, bool a5 = true) override JMP_THIS(0x4BB080);
	virtual bool Copy_From(Surface* fromsurface, bool trans_blit = false, bool a3 = true) override JMP_THIS(0x4C1A900);
	virtual bool Fill_Rect(RectangleStruct& rect, unsigned color) override JMP_THIS(0x4BB5F0);
	virtual bool Fill_Rect(RectangleStruct& area, RectangleStruct& rect, unsigned color) override JMP_THIS(0x4BB620);
	//virtual bool Fill_Rect_Trans(RectangleStruct& rect, const ColorStruct& color, unsigned opacity) override
	virtual bool Fill_Rect_Trans(RectangleStruct* pClipRect, ColorStruct* pColor, int nOpacity) JMP_THIS(0x4BB830);

	virtual bool DrawLineColor_AZ(RectangleStruct& area, Point2D& start, Point2D& end, unsigned color, int a5, int a6, bool z_only = false) override JMP_THIS(0x4BFD30);
	virtual bool DrawMultiplyingLine_AZ(RectangleStruct& area, Point2D& start, Point2D& end, int a4, int a5, int a6, bool a7 = false) override JMP_THIS(0x4BBCA0);
	virtual bool DrawSubtractiveLine_AZ(RectangleStruct& area, Point2D& start, Point2D& end, ColorStruct& color, int a5, int a6, bool a7, bool a8, bool a9, bool a10, float a11) override JMP_THIS(0x4BC750);
	virtual bool DrawRGBMultiplyingLine_AZ(
		RectangleStruct* pRect, Point2D* pStart, Point2D* pEnd, ColorStruct* pColor,
		float Intensity, DWORD dwUnk1, DWORD dwUnk2) override JMP_THIS(0x4BDF00);


	virtual int DrawDashedLine_(Point2D& start, Point2D& end, unsigned color, bool pattern[], int offset, bool a6) override JMP_THIS(0x4C0750);
	virtual bool DrawLine_(Point2D& start, Point2D& end, unsigned color, bool a4 = false) override JMP_THIS(0x4C0E30);

	virtual void* Lock(int x = 0, int y = 0) override JMP_THIS(0x4BAD80);
	virtual bool Unlock() override JMP_THIS(0x4BAF40);
	virtual bool Can_Lock(int x = 0, int y = 0) const override JMP_THIS(0x4BAEC0);
	virtual int Get_Bytes_Per_Pixel() const override JMP_THIS(0x4BAD60);
	virtual int Get_Pitch() const override JMP_THIS(0x4BAD70);

	virtual bool IsDSurface() const override { return true; }

	virtual bool DrawGradientLine(RectangleStruct& area, Point2D& start, Point2D& end, ColorStruct& a4, ColorStruct& a5, float& a6, float& a7) JMP_THIS(0x4BF750);
	virtual bool Can_Blit() const { JMP_THIS(0x4BAF20); }

	static __int16 __fastcall _4BF650_adjust(unsigned __int16 a1, int a2, int a3)
		{ JMP_THIS(0x4BF650); }

	bool _4BEAC0_Blit(int a2, int a3, int a4, int a5, signed int a6, int a7, int a8)
		{ JMP_THIS(0x4BEAC0); }

	void DrawLineBlit(RectangleStruct* pRect, Point2D* pStart, Point2D* pEnd, ColorStruct* pStartColor, int mult, int start_z, int end_z)
		{ JMP_THIS(0x4BEAC0); }

	bool Restore_Check() JMP_THIS(0x4BB000);
	bool IsSurface_Lost() JMP_THIS(0x4BAFE0);

	int Release_DC(int hdc) JMP_THIS(0x4BAD30);
	int Get_DC() JMP_THIS(0x4BACF0);

	bool Draw_Triangle(RectangleStruct& rect, Point2D& point1, Point2D& point2, Point2D& point3, unsigned color);
	bool Fill_Triangle(RectangleStruct& rect, Point2D& point1, Point2D& point2, Point2D& point3, unsigned color);
	bool Fill_Triangle_Trans(RectangleStruct& rect, Point2D& point1, Point2D& point2, Point2D& point3, ColorStruct& rgb, unsigned opacity);

	bool Draw_Quad(RectangleStruct& rect, Point2D& point1, Point2D& point2, Point2D& point3, Point2D& point4, unsigned color);
	bool Fill_Quad(RectangleStruct& rect, Point2D& point1, Point2D& point2, Point2D& point3, Point2D& point4, unsigned color);
	bool Fill_Quad_Trans(RectangleStruct& rect, Point2D& point1, Point2D& point2, Point2D& point3, Point2D& point4, ColorStruct& rgb, unsigned opacity);

	void Draw_Circle(const Point2D center, unsigned radius, RectangleStruct rect, unsigned color);
	void Fill_Circle(const Point2D center, unsigned radius, RectangleStruct rect, unsigned color);
	void Fill_Circle_Trans(const Point2D center, unsigned radius, RectangleStruct rect, ColorStruct& rgb, unsigned opacity);

	bool Fill_Ellipse(Point2D point, int radius_x, int radius_y, RectangleStruct clip, unsigned color);
	bool Fill_Ellipse_Trans(Point2D point, int radius_x, int radius_y, RectangleStruct clip, ColorStruct& rgb, unsigned opacity);
	bool Put_Pixel_Trans(Point2D& point, ColorStruct& rgb, unsigned opacity);

	void DrawSquare(const Point2D& center, double range, COLORREF nColor)
	{
		int semiWidth = static_cast<int>(range * Unsorted::CellWidthInPixels);
		int semiHeight = static_cast<int>(range * Unsorted::CellHeightInPixels);

		Point2D points[4] = {
			center + Point2D{ semiWidth, 0 },
			center + Point2D{ 0, semiHeight },
			center + Point2D{ -semiWidth, 0 },
			center + Point2D{ 0, -semiHeight }
		};

		for (int i = 0; i < 4; i++)
		{
			this->Draw_Line_Rect(DSurface::ViewBounds(), points[i], points[(i + 1) % 4], nColor);
		}
	}

	void* Get_Buffer_Ptr(int x = 0, int y = 0) { return (unsigned char*)(BufferPtr)+(x * Get_Bytes_Per_Pixel()) + (y * Get_Pitch()); }

	bool In_Video_Ram() const { return InVideoRam; }

	LPDIRECTDRAWSURFACE Get_DD_Surface() { return VideoSurfacePtr; }

	static DSurface* Create_Primary(DSurface** backbuffer_surface = nullptr) JMP_THIS(0x4BA770);

	static unsigned RGBA_To_Pixel(unsigned r, unsigned g, unsigned b)
	{
		return (unsigned((b >> BlueRight) << BlueLeft)
			| unsigned((r >> RedRight) << RedLeft)
			| unsigned((g >> GreenRight) << GreenLeft));
	}

	static void Pixel_To_RGBA(unsigned pixel, unsigned* red, unsigned* green, unsigned* blue)
	{
		*red = ((pixel >> RedLeft) << RedRight);
		*green = ((pixel >> GreenLeft) << GreenRight);
		*blue = ((pixel >> BlueLeft) << BlueRight);
	}

	static unsigned RGB_To_Pixel(unsigned r, unsigned g, unsigned b)
	{
		return (unsigned((b >> BlueRight) << BlueLeft)
			| unsigned((r >> RedRight) << RedLeft)
			| unsigned((g >> GreenRight) << GreenLeft));
	}

	static unsigned RGB_To_Pixel(ColorStruct& rgb)
	{
		return (unsigned((rgb.R >> BlueRight) << BlueLeft)
			| unsigned((rgb.G >> RedRight) << RedLeft)
			| unsigned((rgb.B >> GreenRight) << GreenLeft));
	}

	static int Get_RGB_Pixel_Format() { return RGBPixelFormat; }

	static unsigned Get_Red_Left() { return RedLeft; }
	static unsigned Get_Red_Right() { return RedRight; }

	static unsigned Get_Green_Left() { return GreenLeft; }
	static unsigned Get_Green_Right() { return GreenRight; }

	static unsigned Get_Blue_Left() { return BlueLeft; }
	static unsigned Get_Blue_Right() { return BlueRight; }

public:
	void* BufferPtr;
	bool IsAllocated;
	bool InVideoRam;
	LPDIRECTDRAWSURFACE VideoSurfacePtr;
	LPDDSURFACEDESC VideoSurfaceDescription;
};
static_assert(sizeof(DSurface) == 0x24, "Invalid Size !");

struct RGB32
{
	char R;
	char G;
	char B;
	char A;
};

struct GDIBitmapInfo
{
	BITMAPINFO BMInfo;
	RGB32 Palette[256];
};

class NOVTABLE DCSurface
{
	bool Failed;
	GDIBitmapInfo* __BitmapInfo;
	int __GDIBitmap;
	char* __Buffer;
	int __Width;
	int __Height;
	int __GDIBitmapBits2;
	int __Width2;
	BSurface* __SurfacePtr;
};

static_assert(sizeof(DCSurface) == 0x24, "Invalid Size !");