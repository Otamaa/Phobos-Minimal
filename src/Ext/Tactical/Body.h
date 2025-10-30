#pragma once

#include <TacticalClass.h>
#include <CoordStruct.h>
#include <ColorStruct.h>

#include <Utilities/SavegameDef.h>

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

class TacticalExtData final
{
private:
	static std::unique_ptr<TacticalExtData> Data;

public:

	static COMPILETIMEEVAL size_t Canary = 0x51DEBA12;
	using base_type = TacticalClass;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };

	bool IsPendingScreenFlash {};
	ColorStruct ScreenFlashColor { 255,255,255 };
	unsigned ScreenFlashTrans { 100 };

public:

	bool LoadFromStream(PhobosStreamReader& Stm) { return this->Serialize(Stm); }
	bool SaveToStream(PhobosStreamWriter& Stm) { return this->Serialize(Stm); }

	void Screen_Flash_AI();

private:
	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(this->Initialized)
			.Process(this->IsPendingScreenFlash)
			.Process(this->ScreenFlashColor)
			.Process(this->ScreenFlashTrans)
			;
	}
public:
	static IStream* g_pStm;

	static void Allocate(TacticalClass* pThis);
	static void Remove(TacticalClass* pThis);

	static TacticalExtData* Instance()
	{
		return Data.get();
	}

	static void Clear()
	{
		Allocate(TacticalClass::Instance());
	}

	static void DrawProducingProgress();
};

