#include "WaypointTextBoxClass.h"
#include "../../Types/TextBoxTypeClass.h"

#include <StringTable.h>
#include <TacticalClass.h>
#include <ScenarioClass.h>
#include <CellClass.h>
#include <MapClass.h>
#include <Unsorted.h>

#include <Ext/Rules/Body.h>

#include <Utilities/Debug.h>
#include <Utilities/Stream.h>

#include <algorithm>

// DIFF: no static array definition - MapTextBoxClass::Array is the sole owner.

// ========== 构造 ==========

WaypointTextBoxClass::WaypointTextBoxClass(int wpIndex, const char* csfLabel,
	const char* typeName)
	: WaypointIndex(wpIndex)
{
	const TextBoxTypeClass* pType = TextBoxTypeClass::Find(typeName);

	if (!pType)
	{
		Debug::Log("[WaypointTextBoxClass] Warning: type \"%s\" not found!\n", typeName);
		this->CurrentLabel = csfLabel ? csfLabel : "";
		return;
	}

	// 从类型复制样式参数
	this->CurrentLabel = csfLabel ? csfLabel : "";
	this->MaxLineWidth = pType->MaxWidth;
	this->BackgroundOpacity = pType->BackgroundOpacity;
	this->ColorR = pType->ColorR;
	this->ColorG = pType->ColorG;
	this->ColorB = pType->ColorB;
	this->RemainingFrames = pType->Duration;

	// DIFF: was `this->Type = pType`, which never survived a save. The index
	// does, and it is what RemoveByType matches on.
	for (size_t i = 0; i < TextBoxTypeClass::Array.size(); ++i)
	{
		if (TextBoxTypeClass::Array[i].get() == pType)
		{
			this->TypeIndex = static_cast<int>(i);
			break;
		}
	}
}

// ============================================================================
// 生存判定 - GAMESTATE ONLY
//
// A waypoint is scenario data, not a live object, so there is nothing here that
// can be destroyed mid-mission. The shroud test stays in CanDraw().
// ============================================================================

bool WaypointTextBoxClass::IsAlive() const
{
	if (this->WaypointIndex < 0)
		return false;

	if (!ScenarioClass::Instance())
		return false;

	// IsDefinedWaypoint reads scenario data - identical on every machine.
	if (!ScenarioClass::Instance->IsDefinedWaypoint(this->WaypointIndex))
		return false;

	return true;
}

// ============================================================================
// 绘制判定 - may read local state freely
// ============================================================================

bool WaypointTextBoxClass::CanDraw() const
{
	// 黑幕遮挡检测 - LOCAL, per-player. Never let this reach IsAlive().
	if (FakeRulesClass::Instance->ShowTextBoxInShroud_Waypoint)
	{
		if (!TacticalClass::Instance())
			return false;

		CellStruct cell = ScenarioClass::Instance->GetWaypointCoords(this->WaypointIndex);

		// BUGFIX: was `char isShrouded` compared to -2, relying on char being
		// signed. Correct under MSVC's default but silently wrong under /J.
		const int occlusion = static_cast<int>(
			TacticalClass::Instance->GetOcclusion(cell, false));

		if (occlusion == -2) // -2 表示完全在黑幕中
			return false;
	}

	return true;
}

bool WaypointTextBoxClass::GetDrawPosition(Point2D& outPos) const
{
	if (!TacticalClass::Instance())
		return false;

	CellStruct cell = ScenarioClass::Instance->GetWaypointCoords(this->WaypointIndex);

	if (cell.X < 0 && cell.Y < 0)
		return false;

	// 考虑地形高度：坐标 Z = 地形层级 × 每层高度
	CellClass* pCellData = MapClass::Instance->GetCellAt(cell);
	int cellZ = pCellData ? pCellData->GetLevel() * Unsorted::LevelHeight : 0;
	CoordStruct coords = CellClass::Cell2Coord(cell, cellZ);
	const auto& _result = TacticalClass::Instance->GetCoordsToClientSituation(coords);
	outPos = _result.first;
	return _result.second;
}

// ========== 查找/创建 ==========

WaypointTextBoxClass* WaypointTextBoxClass::FindOrCreate(int wpIndex,
	const char* csfLabel, const char* typeName)
{
	if (wpIndex < 0)
		return nullptr;

	const TextBoxTypeClass* pType = TextBoxTypeClass::Find(typeName);

	if (!pType)
	{
		Debug::Log("[WaypointTextBoxClass] Warning: type \"%s\" not found!\n", typeName);
		return nullptr;
	}

	int typeIndex = -1;

	for (size_t i = 0; i < TextBoxTypeClass::Array.size(); ++i)
	{
		if (TextBoxTypeClass::Array[i].get() == pType)
		{
			typeIndex = static_cast<int>(i);
			break;
		}
	}

	auto* pExisting = MapTextBoxClass::Array.FindOf<WaypointTextBoxClass>(
		[wpIndex](WaypointTextBoxClass& label)
		{
			return label.WaypointIndex == wpIndex;
		});

	if (pExisting)
	{
		// 更新已有实例的样式和内容
		pExisting->CurrentLabel = csfLabel ? csfLabel : "";
		pExisting->MaxLineWidth = pType->MaxWidth;
		pExisting->BackgroundOpacity = pType->BackgroundOpacity;
		pExisting->ColorR = pType->ColorR;
		pExisting->ColorG = pType->ColorG;
		pExisting->ColorB = pType->ColorB;
		pExisting->RemainingFrames = pType->Duration;
		pExisting->TypeIndex = typeIndex;
		pExisting->UpdateLayout();
		return pExisting;
	}

	// DIFF: was make_shared + push_back into both arrays + Array.back().get().
	auto* pLabel = MapTextBoxClass::Array.Create<WaypointTextBoxClass>(
		wpIndex, csfLabel, typeName);

	pLabel->UpdateLayout();

	return pLabel;
}

// ========== 移除 ==========

void WaypointTextBoxClass::Remove(int wpIndex)
{
	// DIFF: was ~20 lines - find_if in the derived array, record the raw
	// pointer, erase, then find_if into the base array and erase again.
	MapTextBoxClass::Array.RemoveIfOf<WaypointTextBoxClass>(
		[wpIndex](WaypointTextBoxClass& label)
		{
			return label.WaypointIndex == wpIndex;
		});
}

// ========== 全局清理 ==========

void WaypointTextBoxClass::ClearAll()
{
	// DIFF: was a manual loop over the base array with an inner find_if into
	// the derived array for every element, then clearing the derived array.
	MapTextBoxClass::Array.RemoveAllOf<WaypointTextBoxClass>();
}

void WaypointTextBoxClass::Clear()
{
	ClearAll();
}

// ========== 工具函数 ==========

void WaypointTextBoxClass::ConvertColorEnum(int enumVal, int& r, int& g, int& b)
{
	switch (enumVal)
	{
	case 0:  r = 255; g = 215; b = 0;   break;  // gold（金色）
	case 1:  r = 255; g = 255; b = 255; break;  // white（白色）
	case 2:  r = 255; g = 0;   b = 0;   break;  // red（红色）
	case 3:  r = 0;   g = 0;   b = 255; break;  // blue（蓝色）
	case 4:  r = 0;   g = 128; b = 0;   break;  // green（绿色）
	case 5:  r = 255; g = 255; b = 0;   break;  // yellow（黄色）
	case 6:  r = 128; g = 0;   b = 128; break;  // purple（紫色）
	case 7:  r = 255; g = 192; b = 203; break;  // pink（粉色）
	case 8:  r = 173; g = 216; b = 230; break;  // lightblue（浅蓝）
	default: r = 255; g = 215; b = 0;   break;  // 默认金色
	}
}

// ============================================================================
// 序列化
// DIFF: derived fields only; the base writes the shared ones. The original
// duplicated every base field here and never chained to the base.
// ============================================================================

template <typename T>
bool WaypointTextBoxClass::Serialize(T& Stm)
{
	return Stm
		.Process(this->WaypointIndex)
		.Success();
}

bool WaypointTextBoxClass::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	if (!this->MapTextBoxClass::Load(Stm, RegisterForChange))
		return false;

	return this->Serialize(Stm);
}

bool WaypointTextBoxClass::Save(PhobosStreamWriter& Stm) const
{
	if (!this->MapTextBoxClass::Save(Stm))
		return false;

	return const_cast<WaypointTextBoxClass*>(this)->Serialize(Stm);
}