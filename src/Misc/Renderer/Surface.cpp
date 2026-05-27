#include "Surface.h"

#include <Utilities/Debug.h>

#include <ColorStruct.h>

#include <Unsorted.h>

class DXSurfaceImpl {
public:
	DXSurfaceImpl(int width, int height);
	~DXSurfaceImpl();

	int Pitch;
	std::unique_ptr<BYTE[]> Buffer;
};

void DXSurface::CTOR(int width, int height) {
	this->Width = width;
	this->Height = height;
	this->LockLevel = 0;
	this->BytesPerPixel = 2;
	ImplRef() = new DXSurfaceImpl(width, height);
}

void DXSurface::DTOR() {
	if (ImplRef()) {
		delete ImplRef();
		ImplRef() = nullptr;
	}
}

DXSurface::DXSurface(int width, int height) : DSurface { noinit_t{} } {
	CTOR(width, height);
}

DXSurface::~DXSurface() {
	DTOR();
}

bool DXSurface::Copy_From(Surface* fromsurface, bool trans_blit, bool a3) {
	JMP_THIS(0x7BBAF0);
}

bool DXSurface::Copy_From(RectangleStruct& torect, Surface* fromsurface, RectangleStruct& fromrect, bool trans_blit, bool a5) {
	JMP_THIS(0x7BBB90);
}

bool DXSurface::Copy_From(RectangleStruct& toarea, RectangleStruct& torect, Surface* fromsurface, RectangleStruct& fromarea, RectangleStruct& fromrect, bool trans_blit, bool a7) {
	JMP_THIS(0x7BBCF0);
}

bool DXSurface::Fill_Rect(RectangleStruct& area, RectangleStruct& RectangleStruct, unsigned color) {
	JMP_THIS(0x7BB050);
}

bool DXSurface::Fill_Rect(RectangleStruct& RectangleStruct, unsigned color) {
	JMP_THIS(0x7BB020);
}

bool DXSurface::Fill(unsigned nColor) {
	JMP_THIS(0x7BBAB0);
}

bool DXSurface::Fill_Rect_Trans(RectangleStruct* pClipRect, ColorStruct* pColor, int nOpacity) {
	JMP_THIS(0x4BB830);
}

bool DXSurface::Draw_Ellipse(Point2D center, int radius_x, int radius_y, RectangleStruct clip, unsigned color) {
	JMP_THIS(0x7BB350);
}

bool DXSurface::Put_Pixel(Point2D& point, unsigned color) {
	JMP_THIS(0x7BAEB0);
}

unsigned DXSurface::Get_Pixel(Point2D& point) {
	JMP_THIS(0x7BAE60);
}

bool DXSurface::Draw_Line_Rect(RectangleStruct& area, Point2D& start, Point2D& end, unsigned color) {
	JMP_THIS(0x7BA610);
}

bool DXSurface::Draw_Line(Point2D& start, Point2D& end, unsigned color) {
	JMP_THIS(0x7BA5E0);
}

bool DXSurface::DrawLineColor_AZ(RectangleStruct& area, Point2D& start, Point2D& end, unsigned color, int a5, int a6, bool z_only) {
	JMP_THIS(0x4BFD30);
}

bool DXSurface::DrawMultiplyingLine_AZ(RectangleStruct& area, Point2D& start, Point2D& end, int a4, int a5, int a6, bool a7) {
	JMP_THIS(0x4BBCA0);
}

bool DXSurface::DrawSubtractiveLine_AZB(RectangleStruct& area, Point2D& start, Point2D& end, ColorStruct& color, int a5, int a6, bool a7, bool a8, bool a9, bool a10, float a11) {
	JMP_THIS(0x4BC750);
}

bool DXSurface::DrawRGBMultiplyingLine_AZ(RectangleStruct* pRect, Point2D* pStart, Point2D* pEnd, ColorStruct* pColor,
		float Intensity, DWORD dwUnk1, DWORD dwUnk2) {
	JMP_THIS(0x4BDF00);
}

bool DXSurface::PlotLine(RectangleStruct& area, Point2D& start, Point2D& end, bool(__fastcall* AddRedrawPoint)(Point2D&)) {
	JMP_THIS(0x7BAB90);
}

bool DXSurface::Draw_Dashed_Line(Point2D& start, Point2D& end, unsigned color, bool* pattern, int offset) {
	JMP_THIS(0x7BA8C0);
}

bool DXSurface::DrawDashedLine_(Point2D& start, Point2D& end, unsigned color, bool* pattern, int offset, bool a6) {
	JMP_THIS(0x4C0750);
}

bool DXSurface::DrawLine_(Point2D& start, Point2D& end, unsigned color, bool a4) {
	JMP_THIS(0x4C0E30);
}

bool DXSurface::Draw_Rect(RectangleStruct& area, RectangleStruct& RectangleStruct, unsigned color) {
	JMP_THIS(0x7BADC0);
}

bool DXSurface::Draw_Rect(RectangleStruct& RectangleStruct, unsigned color) {
	JMP_THIS(0x7BAD90);
}

void* DXSurface::Lock(int X, int Y) {
	if (X >= 0 && Y >= 0) {
		++LockLevel;
		return Impl()->Buffer.get() + Y * Impl()->Pitch + X * Get_Bytes_Per_Pixel();
	}
	return nullptr;
}

bool DXSurface::Unlock() {
	if (LockLevel > 0) {
		--LockLevel;
		return true;
	}

	return false;
}

bool DXSurface::Can_Lock(int x, int y) const {
	return true;
}

bool DXSurface::vt_entry_68(int x, int y) const {
	return true;
}

bool DXSurface::Is_Locked() const {
	return false;
}

int DXSurface::Get_Bytes_Per_Pixel() const {
	return this->BytesPerPixel;
}

int DXSurface::Get_Pitch() const {
	return Impl()->Pitch;
}

RectangleStruct DXSurface::Get_Rect() const {
	return { 0, 0, Get_Width(), Get_Height() };
}

int DXSurface::Get_Width() const {
	return Width;
}

int DXSurface::Get_Height() const {
	return Height;
}

bool DXSurface::IsDSurface() const {
	return true;
}

bool DXSurface::Put_Pixel_Clip(Point2D& point, unsigned color, RectangleStruct& rect) {
	JMP_THIS(0x7BAF90);
}

unsigned DXSurface::Get_Pixel_Clip(Point2D& point, RectangleStruct& rect) {
	JMP_THIS(0x7BAF10);
}

bool DXSurface::DrawGradientLine(RectangleStruct& area, Point2D& start, Point2D& end, ColorStruct& a4, ColorStruct& a5, float a6, int a7) {
	JMP_THIS(0x4BF750);
}

bool DXSurface::Can_Blit() const {
	return true;
}

void* DXSurface::GetBuffer() {
	return Impl()->Buffer.get();
}

DXSurfaceImpl::DXSurfaceImpl(int width, int height) {
	const int sourceRowBytes = width * 2;
	Pitch = (sourceRowBytes + 256 - 1) & ~(256 - 1);
	const size_t bufferSize = static_cast<size_t>(Pitch) * height;
	Buffer.reset(new BYTE[bufferSize]);
	std::memset(Buffer.get(), 0, bufferSize);  // ✅ Zero the buffer (black)
}

DXSurfaceImpl::~DXSurfaceImpl() {}
#include <Drawing.h>
#include "GlobalColorPacker.h"

DXSurface* __fastcall DXSurface::CreatePrimary() {
	DSurface::AllowHardwareBlitFills = false; // AllowHardwareBlitFills
	DSurface::AllowStretchBlits = false;	// AllowHardwareBlitStretch

	Debug::Log("[RenderDX] D3D12 surface created as primary surface.\n");

	auto surface = new DXSurface(Game::ScreenWidth(), Game::ScreenHeight());

	GlobalColorPacker::SetColorPacker();

	DSurface::HalfbrightMask = static_cast<unsigned short>(DSurface::Build_Hicolor_Pixel_RGB(127, 127, 127));
	DSurface::QuarterbrightMask = static_cast<unsigned short>(DSurface::Build_Hicolor_Pixel_RGB(63, 63, 63));
	DSurface::EighthbrightMask = static_cast<unsigned short>(DSurface::Build_Hicolor_Pixel_RGB(31, 31, 31));

	uint16_t testRed = Build_Hicolor_Pixel_RGB(255, 0, 0);
	uint16_t testYellow = Build_Hicolor_Pixel_RGB(255, 255, 0);
	uint16_t testCyan = Build_Hicolor_Pixel_RGB(0, 255, 255);

	Debug::Log("[Test] RED(255,0,0) = 0x%04X\n", testRed);
	Debug::Log("[Test] YELLOW(255,255,0) = 0x%04X\n", testYellow);
	Debug::Log("[Test] CYAN(0,255,255) = 0x%04X\n", testCyan);

	Debug::Log("[RenderDX] D3D12 surface created as primary surface done.\n");

	return surface;
}
