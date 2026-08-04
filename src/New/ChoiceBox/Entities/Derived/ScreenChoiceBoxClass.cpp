#include "ScreenChoiceBoxClass.h"
#include "WaypointChoiceBoxClass.h"
#include "../../Types/ChoiceBoxTypeClass.h"

#include <Surface.h>
#include <Utilities/Stream.h>

// DIFF: no static array definition - MapChoiceBoxClass::Array is the sole owner.

// ========== 构造 ==========

ScreenChoiceBoxClass::ScreenChoiceBoxClass(int id, int x, int y, const char* label,
	const ChoiceBoxTypeClass* pType)
	: MapChoiceBoxClass(id, label, pType)
	, ScreenX(x)
	, ScreenY(y)
{}

// ========== 虚接口实现 ==========

bool ScreenChoiceBoxClass::GetDrawPosition(Point2D& outPos) const
{
	// 将百分比转换为实际像素坐标
	int viewW = DSurface::ViewBounds->Width;
	int viewH = DSurface::ViewBounds->Height;

	if (viewW <= 0 || viewH <= 0)
		return false;

	outPos.X = static_cast<int>(this->ScreenX / 100.0 * viewW);
	outPos.Y = static_cast<int>(this->ScreenY / 100.0 * viewH);

	return true;
}

// ========== 查找/创建 ==========

ScreenChoiceBoxClass* ScreenChoiceBoxClass::FindOrCreate(int x, int y,
	const char* label, const ChoiceBoxTypeClass* pType)
{
	return FindOrCreate(-1, x, y, label, pType);
}

ScreenChoiceBoxClass* ScreenChoiceBoxClass::FindOrCreate(int id, int x, int y,
	const char* label, const ChoiceBoxTypeClass* pType)
{
	if (!pType)
		return nullptr;

	// 如果指定了 ID，先移除已有同 ID 的实例（避免重复）
	// DIFF: one call covers both kinds - see WaypointChoiceBoxClass.cpp.
	if (id >= 0)
		MapChoiceBoxClass::RemoveByID(id);

	// NOTE: despite the name this never finds anything, it always creates.
	// Preserved verbatim - callers rely on "create, replacing any same ID".
	// SUSPECT: rename to Create() once call sites are audited.
	return MapChoiceBoxClass::Array.Create<ScreenChoiceBoxClass>(
		id, x, y, label, pType);
}

// ========== 移除 ==========

void ScreenChoiceBoxClass::RemoveByLabel(const char* label)
{
	if (!label || label[0] == '\0')
		return;

	MapChoiceBoxClass::Array.RemoveIfOf<ScreenChoiceBoxClass>(
		[label](ScreenChoiceBoxClass& box)
		{
			return box.Label == label;
		});
}

void ScreenChoiceBoxClass::ClearAll()
{
	// BUGFIX: the original cleared only the derived array, leaving the base
	// array holding the last reference - boxes stayed alive and findable by
	// FindByID() but were never drawn again.
	MapChoiceBoxClass::Array.RemoveAllOf<ScreenChoiceBoxClass>();
}

void ScreenChoiceBoxClass::Clear()
{
	ClearAll();
}

// ========== 序列化 ==========
// DIFF: derived fields only; base fields are written once by the base.

template <typename T>
bool ScreenChoiceBoxClass::Serialize(T& Stm)
{
	return Stm
		.Process(this->ScreenX)
		.Process(this->ScreenY)
		.Success();
}

bool ScreenChoiceBoxClass::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	if (!this->MapChoiceBoxClass::Load(Stm, RegisterForChange))
		return false;

	return Serialize(Stm);
}

bool ScreenChoiceBoxClass::Save(PhobosStreamWriter& Stm) const
{
	if (!this->MapChoiceBoxClass::Save(Stm))
		return false;

	return const_cast<ScreenChoiceBoxClass*>(this)->Serialize(Stm);
}