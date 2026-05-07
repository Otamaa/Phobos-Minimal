#pragma once
#include <Base/Always.h>
#include <Unsorted.h>
#include <Utilities/Patch.h>
#include <Utilities/Macro.h>

class Surface;
struct GameDefinitions {
	static COMPILETIMEEVAL reference<int, 0x008A00A4> const VideoWidth {};
	static COMPILETIMEEVAL reference<int, 0x008A00A8> const VideoHeight {};
	static COMPILETIMEEVAL reference<DWORD, 0x8A00AC> const VideoBitsPerPixel {};
	static COMPILETIMEEVAL reference<int, 0xB72F50> const WSDialogCount {};
	static COMPILETIMEEVAL reference<SpecialDialogType, 0xA8EDA0> const SpecialDialogTypeSet {};
	static COMPILETIMEEVAL reference<HICON, 0xB7354C> const IconPointer {};
	static COMPILETIMEEVAL reference<HCURSOR, 0xB7354C> const CursorPointer {};
	static COMPILETIMEEVAL reference<int, 0xB73544> const CommandShow {};
	static COMPILETIMEEVAL reference<HIMC, 0xB7355C> const IMEContext {};
	static COMPILETIMEEVAL reference<bool, 0xB73561> const _MouseWheel{};
	static COMPILETIMEEVAL reference<int, 0xA8EDD8> const ReadyToQuitP{};

	static void __fastcall Update_Visible_Surface(bool, Surface*, RectangleStruct*) {
		JMP_FAST(0x4F4780);
	}
};

