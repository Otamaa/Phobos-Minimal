#pragma once

#include <MouseClass.h>
#include <Utilities/Enum.h>
#include <Utilities/SaveGame.h>

#include  <array>

#include <SidebarClass.h>

class MouseClassExt final : public MouseClass
{
public:

	static const MouseCursor* GetCursorData(MouseCursorType nMouse);
	static const MouseCursor* GetCursorDataFromRawAction(Action nAction);

#pragma region MappedAction
	struct MappedActions
	{
		size_t Idx;
		bool AllowShourd;
	};

	static std::array<MappedActions, (size_t)Action::count + 2> CursorIdx;
	static std::vector<BuildType> TabCameos[4u];

	static bool SaveGlobals(PhobosStreamWriter& Stm)
	{
		Stm.Process(CursorIdx);
		Stm.Process(TabCameos);
		return Stm.Success();
	}

	static bool LoadGlobals(PhobosStreamReader& Stm)
	{
		Stm.Process(CursorIdx);
		Stm.Process(TabCameos);
		return Stm.Success();
	}

	static void ClearCameos()
	{
		for (auto& cameos : TabCameos)
		{
			cameos.clear();
			cameos.reserve(100);
		}
	}

	static void ClearMappedAction();
	static void NOINLINE InsertMappedAction(MouseCursorType nCursorIdx, Action nAction, bool Shrouded);
	static void NOINLINE InsertSWMappedAction(MouseCursorType nCursorIdx, Action nAction, bool Shrouded);
	static int NOINLINE ByWeapon(TechnoClass* pThis, int nWeaponIdx, bool OutOfRange);
#pragma endregion

public:

	//7E198C - vtable
	void _Update(const int* keyCode, const Point2D* mouseCoords);

	//7E19B0 - vtable
	bool _Override_Mouse_Shape(MouseCursorType mouse, bool wsmall = false);

	//7E19B8 - vtable
	void _Mouse_Small(bool wsmall = true);

	//5BDBC0 - Not a vtable
	int _Get_Mouse_Current_Frame(MouseCursorType mouse, bool wsmall = false) const;

	//5BDB90 - Not a vtable
	int _Get_Mouse_Frame(MouseCursorType mouse, bool wsmall = false) const;

	//5BDC00 - Not a vtable
	Point2D _Get_Mouse_Hotspot(MouseCursorType mouse) const;

	//5BE970 - Not a vtable
	int _Get_Mouse_Start_Frame(MouseCursorType mouse) const;

	//5BE990 - Not a vtable
	int _Get_Mouse_Frame_Count(MouseCursorType mouse) const;

	static size_t GetActionIndex(Action nAction);
	static MouseCursorType ValidateCursorType(Action nAction);
	static Action ValidateShroudedAction(Action nAction);
};
static_assert(sizeof(MouseClassExt) == sizeof(MouseClass), "Invalid Size !");

struct MouseCursorFuncs
{
	FORCEDINLINE static void SetMouseCursorAction(size_t CursorIdx, Action nAction, bool bShrouded)
	{
		MouseClassExt::InsertMappedAction((MouseCursorType)CursorIdx, nAction, bShrouded);
	}
	FORCEDINLINE static void SetSuperWeaponCursorAction(size_t CursorIdx, Action nAction, bool bShrouded)
	{
		MouseClassExt::InsertSWMappedAction((MouseCursorType)CursorIdx, nAction, bShrouded);
	}

};
