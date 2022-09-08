#pragma once

#include <ToolTipManager.h>
#include <Drawing.h>

#include <Helpers/CompileTime.h>

class NOVTABLE CCToolTip : public ToolTipManager
{
public:
	// It's also used in MoneyFormat at 6A934A, not sure what side effect it might leads
	static constexpr reference<bool, 0x884B8Cu> HideName {};
	static constexpr reference<CCToolTip*, 0x887368u> Instance {};
	static constexpr reference<ColorStruct, 0xB0FA1C> ToolTipTextColor{};

	virtual ~CCToolTip() JMP_THIS(0x7784A0);

	void DrawThis(bool bFullRedraw) { JMP_THIS(0x478E10); }
	void DrawThisWithData(ToolTipManagerData& nData) { JMP_THIS(0x478E30); }
	wchar_t* GetToolTipText(int nGadGetId) { JMP_THIS(0x479050); }

	//Constructors
	CCToolTip(HWND hWnd) noexcept
		: ToolTipManager(hWnd) 
	{*((unsigned long*)this) = (unsigned long)0x7F74C4; }

	//Properties
public:
	bool FullRedraw;
	int Delay;
};