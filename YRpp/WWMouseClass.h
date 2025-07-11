#pragma once

#include <GeneralDefinitions.h>
#include <Helpers/CompileTime.h>
#include <RectangleStruct.h>
#include <Surface.h>
#include <GScreenClass.h>
#include <MouseClass.h>

#pragma pack(push, 4)
struct MouseThreadParameter
{
	static COMPILETIMEEVAL reference<bool, 0xB7816Cu> const ThreadNotActive {};
	static COMPILETIMEEVAL reference<MouseThreadParameter, 0xB78138u> const Thread {};
	static COMPILETIMEEVAL reference<HANDLE, 0xB78168u> const Mutex {};
	static COMPILETIMEEVAL reference<HANDLE, 0xB78154u> const Thread_Handle {};
	static COMPILETIMEEVAL reference<DWORD, 0xB78158u> const StaticThreadID {};

public:

	bool SkipProcessing { 0 };
	int RefCount { 0 };
	bool SkipSleep { 0 };
	int field_C { 0 };
	unsigned long long SleepTime { 16 };
	bool dword14 { 0 };
	HANDLE SomeState18 { 0 };
	DWORD ThreadID { 0 };
};
#pragma pack(pop)
static_assert(sizeof(MouseThreadParameter) == 0x24);

typedef MouseThreadParameter MouseThreadClass;

struct SHPStruct;
struct CoordStruct;
struct ShapeFileStruct;
class WWMouseClass
{
public:
	static COMPILETIMEEVAL reference<WWMouseClass*, 0x887640u> const Instance{};
	static COMPILETIMEEVAL reference<WWMouseClass*, 0xB78164u> const Thread_Instance {};
	static COMPILETIMEEVAL reference<bool, 0xB04470u> const NeedToRelease {};

	virtual ~WWMouseClass()
		{ JMP_THIS(0x0); }

	virtual void Draw(Point2D const& coords, SHPStruct const* pImage, int idxFrame)
		{ JMP_THIS(0x7B8A00); }

	void DrawSHPFrame(Point2D const& coords, ShapeFileStruct const* pImage, int idxFrame)
		{ JMP_THIS(0x7B8A00); }

	virtual bool IsRefCountNegative()
		{ JMP_THIS(0x7BA320); }

	virtual void HideCursor()
		{ JMP_THIS(0x7B9930); }

	virtual void ShowCursor()
		{ JMP_THIS(0x7B9750); }

	virtual void ReleaseMouse()
		{ JMP_THIS(0x7B9C30); }

	virtual void CaptureMouse()
		{ JMP_THIS(0x7B9A60); }

	virtual byte GetField10()
		{ JMP_THIS(0x7BA330); }

	virtual void func_20(RectangleStruct Useless)
		{ JMP_THIS(0x7B9D70); }

	virtual void CallFunc10()
		{ JMP_THIS(0x7B9D80); }

	virtual DWORD GetRefCount()
		{ JMP_THIS(0x7B89F0); }

	virtual int GetX()
		{ JMP_THIS(0x7BA340); }

	virtual int GetY()
		{ JMP_THIS(0x7BA350); }

	virtual Point2D* GetCoords(Point2D *buffer)
		{ JMP_THIS(0x7BA360); }

	virtual void SetCoords(Point2D buffer)
		{ JMP_THIS(0x7BA380); }

	virtual void func_3C(DSurface* pSurface, bool bUnk)
		{ JMP_THIS(0x7B90C0); }

	virtual void func_40(DSurface* pSurface, bool bUnk)
		{ JMP_THIS(0x7B92D0); }

	virtual void func_44(int *arg1, int *arg2)
		{ JMP_THIS(0x7B9D90); }

	static CoordStruct GetCoordsUnderCursor();
	static CellStruct GetCellUnderCursor();

	Point2D* GetCoords_(Point2D& buffer)
	{ JMP_THIS(0x7BA360); }

	Point2D GetCoords_()
	{
		Point2D nbuffer;
		GetCoords_(nbuffer);
		return nbuffer;
	}

	void Process() const
	{ JMP_THIS(0x7BA090); }

	static void PrepareScreen()
	{
		WWMouseClass::Instance->HideCursor();

		DSurface::Hidden->Fill(0);
		GScreenClass::DoBlit(true, DSurface::Hidden);
		DSurface::Temp = DSurface::Hidden;

		WWMouseClass::Instance->ShowCursor();

		MouseClass::Instance->SetCursor(MouseCursorType::NoMove, false);
		MouseClass::Instance->RestoreCursor();

		TabClass::Instance->Activate();
		MouseClass::Instance->RedrawSidebar(0);
	}

public:

	SHPStruct * Image;
	int         ImageFrameIndex;
	DWORD       RefCount;
	byte        Captured_10;
	byte        field_11;
	byte        field_12;
	byte        field_13;
	Point2D     XY_Old;
	Point2D     XY1;
	DSurface *  Surface;
	HWND        hWnd;
	RectangleStruct __ConfiningRect;
	Point2D     MouseHot;
	BSurface*   MouseBuffer;
	RectangleStruct MouseBuffRect;
	BSurface*   TacticalBuffer;
	RectangleStruct TacticalBuffRect;
	BSurface*   SidebarBuffer;
	RectangleStruct SidebarBuffRect;
	RectangleStruct rect_80;
	DWORD       field_90;
	DWORD       TimerHandle;

};
