#pragma once
// Westwood owner draw and dialog hell

#include <Helpers/CompileTime.h>

#include <Dictionary.h>
#include <Unsorted.h>

enum class WWControlType : int
{
	Button = 0,
	Edit = 1,
	Static = 2,
	ComboBox = 3,
	ListBox = 4,
	Unknown5 = 5,
	Progress = 6,
	TrackBar = 7,
	ScrollBar = 8,
	Hotkey = 9,
	SysTab = 10,
	ColorTextInput = 11
};

struct WWControlData
{
	char data[0x200];
};
static_assert(sizeof(WWControlData) == 0x200, "WWControlData size mismatch");

enum WWControlMessage
{
	WW_RECALCLAYOUT = 0x497,
	WW_SETTEXTCOLOR = 0x498,
	WW_SETIMAGE = 0x49C,
	WW_SETHASIMAGE = 0x49D,
	WW_SETGDIPROPS = 0x49E,
	WW_GETGDIPROPS = 0x49F,
	WW_GETHWND = 0x4A0,
	WW_BRINGTOTOP = 0x4A9,
	WW_SETACTIVEIMAGE = 0x4AA,
	WW_SETFILLCOLOR = 0x4B1,
	WW_SETTEXTW = 0x4B2,
	WW_GETTEXTW = 0x4B3,
	WW_SETTEXTA = 0x4B4,
	WW_GETTEXTA = 0x4B5,
	WW_SETHASTEXT = 0x4CE,
	WW_SETUNKNOWNPROP30 = 0x4D1,
	WW_RESETANIMTIMER = 0x4D3,
	WW_SETUNKNOWNPROP50 = 0x4EB,
	WW_STATIC_STOPANIM = 0x4D4,
	WW_STATIC_SETANIMFRAME = 0x4D5,
	WW_STATIC_GETANIMFRAME = 0x4D6,
	WW_STATIC_SETANIMFRAMENOTIFYHWND = 0x4D7,
	WW_STATIC_SETCURRENTMOVIEBYINDEX = 0x4DF,
	WW_STATIC_PAUSEMOVIE = 0x4E0,
	WW_STATIC_CONTINUEMOVIE = 0x4E1,
	WW_STATIC_DETACHMOVIE = 0x4E2,
	WW_STATIC_SETLOOPMOVIE = 0x4E3,
	WW_STATIC_SETCURRENTMOVIEBYNAME = 0x4E4,
	WW_STATIC_REVEALTEXTS = 0x4EE,
	WW_STATIC_BLITMOVIE = 0x4F0,
};

class OwnerDraw
{
public:
	using HwndProcDict = Dictionary<HWND, WNDPROC>;
	struct MsgInProcessGuard
	{
		HWND Hwnd;
		bool InProcess;
	};
	using MsgInProcessDict = Dictionary<UINT, MsgInProcessGuard>;
	using HwndControlDataDict = Dictionary<HWND, WWControlData>;

	static constexpr reference<HwndProcDict, 0xAC1B48> const DialogProcs {};
	static constexpr reference<HwndProcDict, 0xAC18C0> const SubclassProcs {}; // Custom subclass procedures for owner-draw controls, 
	static constexpr reference<MsgInProcessDict, 0xAC18C0> const MessageProcessedGuard {}; // generic OwnerDraw::WindowProc preventing a message being processed multiple times
	static constexpr reference<HwndControlDataDict, 0xAC1B00> const ControlData {}; // Control data

	// WWControlType::Button
	static constexpr reference<WNDPROC, 0x6163A0> const CheckBoxButtonHandler {};
	static constexpr reference<WNDPROC, 0x61E700> const GroupBoxButtonHandler {};
	static constexpr reference<WNDPROC, 0x616980> const AutoRadioButtonHandler {};
	static constexpr reference<WNDPROC, 0x612B70> const OwnerDrawButtonHandler {};
	// WWControlType::Edit
	static constexpr reference<WNDPROC, 0x614190> const EditHandler {};
	static constexpr reference<WNDPROC, 0x614B30> const NewEditHandler {};
	// WWControlType::Static
	static constexpr reference<WNDPROC, 0x6153E0> const StaticHandler {};
	// WWControlType::ComboBox
	static constexpr reference<WNDPROC, 0x617250> const ComboBoxHandler {};
	// WWControlType::ListBox
	static constexpr reference<WNDPROC, 0x618D40> const ListBoxHandler {};
	// WWControlType::Progress
	static constexpr reference<WNDPROC, 0x61D6D0> const ProgressHandler {};
	// WWControlType::TrackBar
	static constexpr reference<WNDPROC, 0x61D950> const TrackBarHandler {};
	// WWControlType::ScrollBar
	static constexpr reference<WNDPROC, 0x61C690> const ScrollBarHandler {};
	// WWControlType::Hotkey
	static constexpr reference<WNDPROC, 0x61ECA0> const HotkeyHandler {};
	// WWControlType::SysTab
	static constexpr reference<WNDPROC, 0x6137D0> const SysTabHandler {};
	// WWControlType::ColorTextInput
	static constexpr reference<WNDPROC, 0x612A60> const ColorTextInputHandler {};

	static constexpr reference<WNDPROC, 0x610CA0> const DefaultHandler {}; // Generic handler which call the handles above

	};