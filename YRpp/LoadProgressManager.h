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
	static constexpr reference<LoadProgressManager*, 0xABC9BCu> const Instance{};
	static constexpr reference<LoadProgressManager*, 0xABC9BCu> const LPMgr{};
	
	static void DrawText(const wchar_t *pText, int X, int Y, DWORD dwColor);

	LoadProgressManager()
		{ JMP_THIS(0x552A40); }

	virtual ~LoadProgressManager()
		{ JMP_THIS(0x552AA0); }

	void Draw()
		{ JMP_THIS(0x552D60); }

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
	DWORD field_C;
	DWORD field_10;
	DWORD field_14;
	DWORD field_18;
	DWORD field_1C;
	DWORD field_20;
	DWORD field_24;
	DWORD field_28;
	DWORD field_2C;
	DWORD field_30;
	DWORD field_34;
	DWORD field_38;
	wchar_t* LoadMessage;
	wchar_t* LoadBriefing;
	SHPStruct * TitleBarSHP;
	SHPStruct * LoadBarSHP;
	SHPStruct * BackgroundSHP;
	bool TitleBarSHP_loaded;
	bool LoadBarSHP_loaded;
	bool BackgroundSHP_loaded;
	DWORD field_54;
	DWORD field_58;
	DWORD field_5C;
	Surface * ProgressSurface;
};

static_assert(sizeof(LoadProgressManager) == 0x64 , "Invalid Size");