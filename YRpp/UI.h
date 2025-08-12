#pragma once

#include <windows.h>
#include <GeneralDefinitions.h>
#include <Unsorted.h>

class UI {
public:
	typedef BOOL (CALLBACK *Callback)(HWND, UINT, WPARAM, LPARAM);

	static HGLOBAL __fastcall GetResource(LPCTSTR lpName, LPCTSTR lpType)
		{ JMP_FAST(0x4A3B40); }

	static BOOL __fastcall StandardWndProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
		{ JMP_FAST(0x622B50); }

	static HWND __fastcall BeginDialog(LPCTSTR lpName, Callback windProc, DWORD dwUnk)
		{ JMP_FAST(0x622650); }

	static void __fastcall EndDialog(HWND hDlg)
		{ JMP_FAST(0x622720); }

	static HWND __fastcall ShowMessageWithCancelOnly(LPARAM lParam, LPARAM a2, LONG dwNewLong)
		{ JMP_FAST(0x623230); }

	static bool __cdecl Updated()
		{ JMP_STD(0x623120); }

	static void __fastcall FocusOnWindow(HWND hWnd)
		{ JMP_FAST(0x622800); }

	static void __fastcall RegisterWindow(HWND hWnd, LPARAM msg)
		{ JMP_FAST(0x622820); }
};
