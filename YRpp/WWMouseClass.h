#pragma once

#include <GeneralDefinitions.h>
#include <Helpers/CompileTime.h>
#include <RectangleStruct.h>
#include <Surface.h>

struct MouseThreadParameter
{
	static constexpr reference<bool, 0xB7816Cu> const ThreadNotActive {};
	static constexpr reference<MouseThreadParameter, 0xB78138u> const Thread {};
	static constexpr reference<HANDLE, 0xB78168u> const Mutex {};
	static constexpr reference<HANDLE, 0xB78154u> const Thread_Handle {};
	static constexpr reference<DWORD, 0xB78158u> const ThreadID {};

	MouseThreadParameter() :
		  SkipProcessing { 0 }
		, RefCount { 0 }
		, SkipSleep { 0 }
		, field_C { 0 }
		, SleepTime { 16 }
		, dword14 { 0 }
		, SomeState18 { 0 }

	{ }

	DWORD SkipProcessing;
	DWORD RefCount;
	DWORD SkipSleep;
	DWORD field_C;
	DWORD SleepTime;
	DWORD dword14;
	DWORD SomeState18;
};

//static_assert(sizeof(MouseThreadParameter) == 0x1C);

struct SHPStruct;
struct CoordStruct;
class WWMouseClass
{
public:
	static constexpr reference<WWMouseClass*, 0x887640u> const Instance{};
	static constexpr reference<WWMouseClass*, 0xB78164u> const Thread_Instance {};

	virtual ~WWMouseClass()
		{ JMP_THIS(0x0); }

	virtual void Draw(CellStruct const& coords, SHPStruct const* pImage, int idxFrame)
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
	DWORD       MouseBuffer;
	RectangleStruct MouseBuffRect;
	DWORD       TacticalBuffer;
	RectangleStruct TacticalBuffRect;
	DWORD       SidebarBuffer;
	RectangleStruct SidebarBuffRect;
	RectangleStruct rect_80;
	DWORD       field_90;
	DWORD       TimerHandle;

};
