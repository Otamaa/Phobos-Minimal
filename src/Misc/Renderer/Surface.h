#pragma once

#include <Surface.h>

#define DX12_SURFACE_IMPLEMENTATION

// CPU render
class DXSurfaceImpl;
class DXSurface : public DSurface {
private:
	DXSurfaceImpl* Impl() const {
		return reinterpret_cast<DXSurfaceImpl*>(BufferPtr);
	}

	DXSurfaceImpl*& ImplRef() {
		return reinterpret_cast<DXSurfaceImpl*&>(BufferPtr);
	}

public:
	static DXSurface* __fastcall CreatePrimary();

	void CTOR(int width, int height);
	void DTOR();

	DXSurface(int width, int height);

	virtual ~DXSurface() override;

	//Surface
	virtual bool Copy_From(Surface* pSrc, bool trans, bool same_copy_cpu) override;

	virtual bool Copy_From(
		RectangleStruct& pClipRect, //ignored and retrieved again...
		Surface* pSrc,
		RectangleStruct& pSrcRect,	//desired source rect of pSrc ?
		bool trans,
		bool same_copy_cpu) override;

	virtual bool Copy_From(
		RectangleStruct& pClipRect,
		RectangleStruct& pClipRect2,	//again? hmm
		Surface* pSrc,
		RectangleStruct& pDestRect,	//desired dest rect of pSrc ? (stretched? clipped?)
		RectangleStruct& pSrcRect,	//desired source rect of pSrc ?
		bool trans,
		bool same_copy_cpu) override;

	virtual bool Fill_Rect(RectangleStruct& pClipRect, RectangleStruct& pFillRect, unsigned nColor) override;
	virtual bool Fill_Rect(RectangleStruct& pFillRect, unsigned nColor) override;
	virtual bool Fill(unsigned nColor) override;
	virtual bool Fill_Rect_Trans(RectangleStruct* pClipRect, ColorStruct* pColor, int nUnknown) override;
	virtual bool Draw_Ellipse(Point2D center, int radius_x, int radius_y, RectangleStruct Rect, unsigned nColor) override;
	virtual bool Put_Pixel(Point2D& pPoint, unsigned nColor) override;
	virtual unsigned Get_Pixel(Point2D& pPoint) override;
	virtual bool Draw_Line_Rect(RectangleStruct& pClipRect, Point2D& pStart, Point2D& pEnd, unsigned nColor) override;
	virtual bool Draw_Line(Point2D& pStart, Point2D& pEnd, unsigned nColor) override;
	virtual bool DrawLineColor_AZ(
		RectangleStruct& pRect, Point2D& pStart, Point2D& pEnd, unsigned nColor,
		int startZ, int endZ, bool bUnk) override;

	virtual bool DrawMultiplyingLine_AZ(
		RectangleStruct& pRect, Point2D& pStart, Point2D& pEnd, int a4, int a5, int a6, bool a7 = false) override;

	virtual bool DrawSubtractiveLine_AZB(RectangleStruct& area, Point2D& start, Point2D& end, ColorStruct& color,
		int a5, int a6, bool a7, bool a8, bool a9, bool a10, float a11) override;

	virtual bool DrawRGBMultiplyingLine_AZ(
		 RectangleStruct* pRect, Point2D* pStart, Point2D* pEnd, ColorStruct* pColor,
		 float Intensity, DWORD dwUnk1, DWORD dwUnk2) override;

	virtual bool PlotLine(RectangleStruct& pRect, Point2D& pStart, Point2D& pEnd, bool(__fastcall* AddRedrawPoint)(Point2D&)) override;
	virtual bool Draw_Dashed_Line(Point2D& start, Point2D& end, unsigned color, bool* pattern, int offset) override;
	virtual bool DrawDashedLine_(Point2D& start, Point2D& end, unsigned color, bool* pattern, int offset, bool a6) override;
	virtual bool DrawLine_(Point2D& start, Point2D& end, unsigned color, bool a4 = false) override;
	virtual bool Draw_Rect(RectangleStruct& rect, unsigned color) override;
	virtual bool Draw_Rect(RectangleStruct& area, RectangleStruct& rect, unsigned color) override;
	virtual void* Lock(int x = 0, int y = 0) override;
	virtual bool Unlock() override;
	virtual bool Can_Lock(int x = 0, int y = 0) const override;
	virtual bool vt_entry_68(int x = 0, int y = 0) const override;
	virtual bool Is_Locked() const override;
	virtual int Get_Bytes_Per_Pixel() const override;
	virtual int Get_Pitch() const override;
	virtual RectangleStruct Get_Rect() const override;
	virtual int Get_Width() const override;
	virtual int Get_Height() const override;
	virtual bool IsDSurface() const override;
	virtual bool Put_Pixel_Clip(Point2D& point, unsigned color, RectangleStruct& rect) override;
	virtual unsigned Get_Pixel_Clip(Point2D& point, RectangleStruct& rect) override;
	virtual bool DrawGradientLine(RectangleStruct& pRect, Point2D& pStart, Point2D& pEnd,
		ColorStruct& pStartColor, ColorStruct& pEndColor, float fStep, int nColor) override;
	virtual bool Can_Blit() const override;

	void* GetBuffer();
};
static_assert(sizeof(DXSurface) == sizeof(DSurface), "Size Missmatch !");