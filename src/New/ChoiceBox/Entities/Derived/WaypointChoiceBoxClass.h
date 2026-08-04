#pragma once

#include "../Base/MapChoiceBoxClass.h"

class PhobosStreamWriter;
class PhobosStreamReader;
class ChoiceBoxTypeClass;

class WaypointChoiceBoxClass final : public MapChoiceBoxClass
{
public:
	// DIFF: `static std::vector<std::shared_ptr<WaypointChoiceBoxClass>> Array`
	// is gone. Every use of it was a linear scan, so the filtered helpers on
	// MapChoiceBoxClass::Array replace it at identical cost - and there is no
	// longer a second container that Clear()/Remove* can forget to update.
	static constexpr KindType ClassKind = KindType::Waypoint;

	int WaypointIndex { -1 }; // 路径点索引

	WaypointChoiceBoxClass() = default;                              // 供反序列化
	WaypointChoiceBoxClass(int id, int wpIndex, const char* label,
		const ChoiceBoxTypeClass* pType);

	// ===== 虚接口实现 =====
	bool CanDraw() const override;
	bool GetDrawPosition(Point2D& outPos) const override;

	KindType GetKind() const override { return ClassKind; }
	const char* GetTypeMarker() const override { return "WaypointChoiceBoxClass"; }

	// ===== 查找/创建/移除 =====
	static WaypointChoiceBoxClass* FindOrCreate(int wpIndex,
		const char* label, const ChoiceBoxTypeClass* pType);
	static WaypointChoiceBoxClass* FindOrCreate(int id, int wpIndex,
		const char* label, const ChoiceBoxTypeClass* pType);

	static void Remove(int wpIndex);
	static void RemoveByLabel(const char* label);

	// DIFF: RemoveByID is gone. ID is unique across kinds - every call site
	// paired WaypointChoiceBoxClass::RemoveByID(id) with
	// ScreenChoiceBoxClass::RemoveByID(id) anyway. Use
	// MapChoiceBoxClass::RemoveByID(id), which handles both.

	static void ClearAll();
	static void Clear();

	// ===== 序列化 =====
	bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	bool Save(PhobosStreamWriter& Stm) const override;

protected:
	template <typename T>
	bool Serialize(T& Stm);
};