#pragma once

#include <ToolTipManager.h>
#include <Drawing.h>

#include <Helpers/CompileTime.h>

class NOVTABLE CCToolTip : public ToolTipManager
{
public:
	// It's also used in MoneyFormat at 6A934A, not sure what side effect it might leads
	static COMPILETIMEEVAL reference<bool, 0x884B8Cu> const HideName {};
	static COMPILETIMEEVAL reference<bool, 0x884B8Fu> const Bound{};
	static COMPILETIMEEVAL reference<CCToolTip*, 0x887368u> const Instance {};
	static COMPILETIMEEVAL reference<ColorStruct, 0xB0FA1Cu> const ToolTipTextColor{};
	static OPTIONALINLINE COMPILETIMEEVAL DWORD vtable = 0x7F74C4;

	virtual ~CCToolTip() JMP_THIS(0x7784A0);

	void DrawThis(bool bFullRedraw) { JMP_THIS(0x478E10); }
	void DrawThisWithData(ToolTipManagerData& nData) { JMP_THIS(0x478E30); }
	wchar_t* GetToolTipText(int nGadGetId) { JMP_THIS(0x479050); }

	//Constructors
	CCToolTip(HWND hWnd) noexcept
		: ToolTipManager(hWnd)
	{ VTable::Set(this, vtable); }

	//Properties
public:
	bool FullRedraw;
	int Delay;
};

static_assert(sizeof(CCToolTip) == 0x268, "Invalid Size !");
