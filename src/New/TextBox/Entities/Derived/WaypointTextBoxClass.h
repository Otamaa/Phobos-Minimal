#pragma once

#include "../Base/MapTextBoxClass.h"

class PhobosStreamWriter;
class PhobosStreamReader;
class TextBoxTypeClass;

class WaypointTextBoxClass final : public MapTextBoxClass
{
public:
	// DIFF: the derived shared_ptr array is gone - see MapTextBoxClass.h.
	static constexpr KindType ClassKind = KindType::Waypoint;

	int WaypointIndex { -1 };

	// DIFF: `const TextBoxTypeClass* Type` removed. It was assigned in the
	// constructor and in FindOrCreate, never read anywhere, and never
	// serialized - so it came back nullptr after a load. MapTextBoxClass::
	// TypeIndex now carries the type identity and does survive save/load.

	WaypointTextBoxClass() = default;
	WaypointTextBoxClass(int wpIndex, const char* csfLabel, const char* typeName);

	// ===== 虚接口实现 =====
	bool IsAlive() const override;
	bool CanDraw() const override;
	bool GetDrawPosition(Point2D& outPos) const override;

	KindType GetKind() const override { return ClassKind; }
	const char* GetTypeMarker() const override { return "WaypointTextBoxClass"; }

	// ===== 工具函数 =====
	static void ConvertColorEnum(int enumVal, int& r, int& g, int& b);

	// ===== 查找/创建/移除 =====
	static WaypointTextBoxClass* FindOrCreate(int wpIndex,
		const char* csfLabel, const char* typeName);
	static void Remove(int wpIndex);
	static void ClearAll();
	static void Clear();

	// ===== 序列化 =====
	bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	bool Save(PhobosStreamWriter& Stm) const override;

protected:
	template <typename T>
	bool Serialize(T& Stm);
};