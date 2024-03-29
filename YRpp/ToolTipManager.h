#pragma once

#include <YRPPCore.h>
#include <GeneralStructures.h>
#include <ArrayClasses.h>
#include <IndexClass.h>
#include <RectangleStruct.h>

struct ToolTip
{
	unsigned int GadgetID {0};
	RectangleStruct Bounds { };
	const char* Text { nullptr };
	bool field_18{false};
};
static_assert(sizeof(ToolTip) == 0x1C, "Invalid Size !");

struct ToolTipManagerData
{
	RectangleStruct Dimension;
	wchar_t HelpText[0x100];
};
static_assert(sizeof(ToolTipManagerData) == 0x210, "Invalid Size !");

class NOVTABLE ToolTipManager
{
public:
	using ToolTipIndexData = IndexClass<int, ToolTip*>;

	//Destructor
	virtual ~ToolTipManager() RX;

	// ToolTipManager
	virtual bool AI(ToolTipManagerData& from) R0; // update in ToolTipManager, Process in CCToolTip, so I called it AI
	virtual void MarkToRedraw(ToolTipManagerData& from) RX; // parameter seems to be useless, also clear
	virtual void Draw(bool bOnSidebar) RX;
	virtual void DrawText(ToolTipManagerData& from) RX;
	virtual wchar_t* GetToolTipText(unsigned int ID) R0; // seems abandoned, nobody calls it

	//Non virtual
	void SetState(bool bState) { JMP_THIS(0x7241A0); }
	void ProcessMessage(MSG* pMSG) { JMP_THIS(0x724200); }
	int GetTimerDelay() { JMP_THIS(0x724510); }
	void SetTimerDelay(int value) { JMP_THIS(0x724520); }
	void SaveTimerDelay() { JMP_THIS(0x724530); }
	void RestoreTimeDelay() { JMP_THIS(0x724540); }
	int GetLifeTime() { JMP_THIS(0x724550); }
	void SetLifeTime(int value) { JMP_THIS(0x724560); }
	int GetToolTipCount() { JMP_THIS(0x724570); }
	bool Add(ToolTip& tooltip) { JMP_THIS(0x724580); }
	void Remove(unsigned int ID) { JMP_THIS(0x724730); }
	bool Find(unsigned int ID, ToolTip& tooltip) { JMP_THIS(0x7248C0); }
	ToolTip* FindFromPosition(Point2D& point) { JMP_THIS(0x724A30); }
	bool Process() { JMP_THIS(0x724AD0); }
	void Hide() { JMP_THIS(0x724BE0); }
	bool IsToolTipShowing() { JMP_THIS(0x724C00); }

	//Statics

	//Constructors
	ToolTipManager(HWND hWnd) noexcept
		: ToolTipManager(noinit_t()) { JMP_THIS(0x724000); }

protected:
	explicit __forceinline ToolTipManager(noinit_t)  noexcept
	{
	}

	//Properties
public:
	ToolTip* CurrentToolTip;
	HWND hWnd;
	bool IsActive;
	Point2D CurrentMousePosition;
	ToolTipManagerData CurrentToolTipData;
	int ToolTipDelay;
	int LastToolTipDelay;
	int ToolTipLifeTime;
	DynamicVectorClass<ToolTip*> ToolTips;
	IndexClass<int, ToolTip*> ToolTipIndex;
};

static_assert(sizeof(ToolTipManager) == 0x260, "Invalid Size !");