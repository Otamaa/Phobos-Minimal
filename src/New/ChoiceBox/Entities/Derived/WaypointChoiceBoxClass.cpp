#include "WaypointChoiceBoxClass.h"
#include "ScreenChoiceBoxClass.h"
#include "../../Types/ChoiceBoxTypeClass.h"

#include <TacticalClass.h>
#include <ScenarioClass.h>
#include <CellClass.h>
#include <MapClass.h>
#include <Unsorted.h>

#include <Utilities/Stream.h>

// DIFF: no static array definition here any more - MapChoiceBoxClass::Array is
// the sole owner.

// ========== 构造 ==========

WaypointChoiceBoxClass::WaypointChoiceBoxClass(int id, int wpIndex, const char* label,
	const ChoiceBoxTypeClass* pType)
	: MapChoiceBoxClass(id, label, pType)
	, WaypointIndex(wpIndex)
{}

// ========== 虚接口实现 ==========

bool WaypointChoiceBoxClass::CanDraw() const
{
	if (this->WaypointIndex < 0)
		return false;

	if (!ScenarioClass::Instance())
		return false;

	if (!ScenarioClass::Instance->IsDefinedWaypoint(this->WaypointIndex))
		return false;

	return true;
}

bool WaypointChoiceBoxClass::GetDrawPosition(Point2D& outPos) const
{
	if (!TacticalClass::Instance())
		return false;

	CellStruct cell = ScenarioClass::Instance->GetWaypointCoords(this->WaypointIndex);

	if (cell.X < 0 && cell.Y < 0)
		return false;

	CellClass* pCellData = MapClass::Instance->GetCellAt(cell);
	int cellZ = pCellData ? pCellData->GetLevel() * Unsorted::LevelHeight : 0;
	CoordStruct coords = CellClass::Cell2Coord(cell, cellZ);
	const auto& result = TacticalClass::Instance->GetCoordsToClientSituation(coords);
	outPos = result.first;
	return result.second;
}

// ========== 查找/创建 ==========

WaypointChoiceBoxClass* WaypointChoiceBoxClass::FindOrCreate(int wpIndex,
	const char* label, const ChoiceBoxTypeClass* pType)
{
	return FindOrCreate(-1, wpIndex, label, pType);
}

WaypointChoiceBoxClass* WaypointChoiceBoxClass::FindOrCreate(int id, int wpIndex,
	const char* label, const ChoiceBoxTypeClass* pType)
{
	if (wpIndex < 0 || !pType)
		return nullptr;

	// 如果指定了 ID，先移除已有同 ID 的实例（避免重复）
	//
	// DIFF: was two calls - WaypointChoiceBoxClass::RemoveByID(id) and
	// ScreenChoiceBoxClass::RemoveByID(id) - each doing its own two-array
	// erase. One array, one call, both kinds covered.
	if (id >= 0)
		MapChoiceBoxClass::RemoveByID(id);

	// DIFF: was make_shared + push_back into the derived array + push_back into
	// the base array + `return Array.back().get()`. Create() returns the typed
	// pointer directly, so the fragile back()-after-move is gone.
	return MapChoiceBoxClass::Array.Create<WaypointChoiceBoxClass>(
		id, wpIndex, label, pType);
}

// ========== 移除 ==========
//
// DIFF: each of these was ~12 lines - find_if in the derived array, then a
// remove_if into the base array keyed on the raw pointer. Forgetting either
// half leaked or dangled. One array means one erase.

void WaypointChoiceBoxClass::Remove(int wpIndex)
{
	MapChoiceBoxClass::Array.RemoveIfOf<WaypointChoiceBoxClass>(
		[wpIndex](WaypointChoiceBoxClass& box)
		{
			return box.WaypointIndex == wpIndex;
		});
}

void WaypointChoiceBoxClass::RemoveByLabel(const char* label)
{
	if (!label || label[0] == '\0')
		return;

	MapChoiceBoxClass::Array.RemoveIfOf<WaypointChoiceBoxClass>(
		[label](WaypointChoiceBoxClass& box)
		{
			return box.Label == label;
		});
}

void WaypointChoiceBoxClass::ClearAll()
{
	// BUGFIX: the original did `Array.clear()` on the derived array only,
	// leaving MapChoiceBoxClass::Array holding the last shared_ptr. The boxes
	// stayed alive, invisible to every draw path but still matched by
	// FindByID(). Removing by kind cannot half-succeed.
	MapChoiceBoxClass::Array.RemoveAllOf<WaypointChoiceBoxClass>();
}

void WaypointChoiceBoxClass::Clear()
{
	ClearAll();
}

// ========== 序列化 ==========
//
// DIFF: derived fields only. The original re-wrote ID, Label, ClickedIndex,
// RemainingFrames and IsExpired here after the base had already written them -
// symmetric, so it round-tripped, but the second write overwrote the first and
// wasted ~24 bytes per box.

template <typename T>
bool WaypointChoiceBoxClass::Serialize(T& Stm)
{
	return Stm
		.Process(this->WaypointIndex)
		.Success();
}

bool WaypointChoiceBoxClass::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	if (!this->MapChoiceBoxClass::Load(Stm, RegisterForChange))
		return false;

	return Serialize(Stm);
}

bool WaypointChoiceBoxClass::Save(PhobosStreamWriter& Stm) const
{
	if (!this->MapChoiceBoxClass::Save(Stm))
		return false;

	return const_cast<WaypointChoiceBoxClass*>(this)->Serialize(Stm);
}