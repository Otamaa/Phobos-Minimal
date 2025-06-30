#pragma once

#include <TacticalClass.h>
#include <CoordStruct.h>
#include <ColorStruct.h>

class SuperClass;
class FakeTacticalClass final : public TacticalClass
{
public:

	using callback_type = bool(__fastcall*)(ObjectClass*);
	static OPTIONALINLINE struct TacticalSelectablesHelper
	{
		OPTIONALINLINE size_t size()
		{
			return TacticalClass::Instance->SelectableCount;
		}

		OPTIONALINLINE TacticalSelectableStruct* begin()
		{
			return &Unsorted::TacticalSelectables[0];
		}

		OPTIONALINLINE TacticalSelectableStruct* end()
		{
			return &Unsorted::TacticalSelectables[size()];
		}
	} Array {};

	// Reversed from Is_Selectable, w/o Select call
	static bool ObjectClass_IsSelectable(ObjectClass* pThis);

	bool __ClampTacticalPos(Point2D* tacticalPos);

#ifndef ___test
	static void __fastcall __DrawTimersA(int value, ColorScheme* color, int interval, const wchar_t* label, LARGE_INTEGER* _arg, bool* _arg1);
	static void __fastcall __DrawTimersB(int value, ColorScheme* color, int interval, const wchar_t* label, LARGE_INTEGER* _arg, bool* _arg1);
	static void __fastcall __DrawTimersC(int value, ColorScheme* color, int interval, const wchar_t* label, LARGE_INTEGER* _arg, bool* _arg1);
#else
	static void __fastcall __DrawTimers(int value, ColorScheme* color, int interval, const wchar_t* label, LARGE_INTEGER* _arg, bool* _arg1);
#endif

	static void __DrawTimersSW(SuperClass* pSuper , int value, int interval);
	static void __DrawRadialIndicator(bool draw_indicator, bool animate, Coordinate center_coord, ColorStruct color, float radius, bool concentric, bool round);
	static void __RenderOverlapForeignMap();

	// Reversed from Tactical::Select
	bool IsInSelectionRect(LTRBStruct* pRect, const TacticalSelectableStruct& selectable);
	bool IsHighPriorityInRect(LTRBStruct* rect);

	// Reversed from Tactical::Select
	void SelectFiltered(LTRBStruct* pRect, callback_type fpCheckCallback, bool bPriorityFiltering);

	// Reversed from Tactical::MakeSelection
	void Tactical_MakeFilteredSelection(callback_type fpCheckCallback);

	void __DrawAllTacticalText(wchar_t* text);

	//void DrawDebugOverlay();
	//bool DrawCurrentCell(); //TODO
	//bool DebugDrawAllCellInfo(); //TODO
	//bool DebugDrawBridgeInfo(); //TODO
	//void DebugDrawMouseCellMembers //TODO

};

static_assert(sizeof(FakeTacticalClass) == sizeof(TacticalClass), "MustBe Same!");
