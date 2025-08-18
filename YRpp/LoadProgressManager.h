#pragma once

#include <YRPP.h>

#include <Helpers/CompileTime.h>

struct SHPStruct;
class Surface;

struct LoadRectangle : public RectangleStruct
{
	RectangleStruct* GetLoadingBound() const
	{ JMP_THIS(0x554150); }
};

class LoadProgressManager
{
public:
	static COMPILETIMEEVAL reference<LoadProgressManager*, 0xABC9BCu> const Instance{};
	static COMPILETIMEEVAL reference<LoadProgressManager*, 0xABC9BCu> const LPMgr{};
	static COMPILETIMEEVAL reference<ConvertClass*, 0xB0FB88u> const LoadScreenPal {};
	static COMPILETIMEEVAL reference<BytePalette*, 0xB0FB84u> const LoadScreenBytePal {};

	//static void DrawText(const wchar_t *pText, int X, int Y, DWORD dwColor);

public:

	LoadProgressManager()
		{ JMP_THIS(0x552A40); }

	virtual ~LoadProgressManager()
		{ JMP_THIS(0x552AA0); }

	void Draw()
		{ JMP_THIS(0x552D60); }

	static void DrawTheText(const wchar_t* pText, int X, int Y, DWORD dwColor);

	static void __fastcall sub_554280(const wchar_t* pWString, Surface* pSurface,
		RectangleStruct& pRect, DWORD a4, char a5)
	{
		JMP_THIS(0x554280);
	}

	static void __fastcall sub_5541C0(const wchar_t* pWString, Surface* pSurface,
		Point2D position, int a5, bool a6 = false, bool a7 = false)
	{
		JMP_THIS(0x5541C0);
	}

	static void __fastcall FillRectWithColor(RectangleStruct& pRect,
		Surface* pSurface, DWORD a3 = 0, DWORD a4 = 0x9Fu)
	{
		JMP_THIS(0x621B80);
	}

	static RectangleStruct GetLoadingBound()
	{
		LoadRectangle ret {};
		return *ret.GetLoadingBound();
	}

public:

	DWORD field_4;
	DWORD field_8;
	RectangleStruct TitleBarRect;
	RectangleStruct LoadBarSHPRect;
	RectangleStruct LoadScreenSHPRect;
	wchar_t* LoadMessage;
	wchar_t* LoadBriefing;
	SHPStruct* TitleBarSHP;
	SHPStruct* LoadScreenSHP;
	SHPStruct* LoadBarSHP;
	bool TitleBarSHP_loaded;
	bool LoadScreenSHP_loaded;
	bool LoadBarSHP_loaded;
	DWORD field_54;
	DWORD field_58;
	DWORD field_5C;
	DSurface* ProgressSurface;
};

static_assert(sizeof(LoadProgressManager) == 0x64 , "Invalid Size");