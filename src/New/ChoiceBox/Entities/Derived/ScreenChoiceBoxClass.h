#pragma once

#include "../Base/MapChoiceBoxClass.h"

class PhobosStreamWriter;
class PhobosStreamReader;
class ChoiceBoxTypeClass;

class ScreenChoiceBoxClass final : public MapChoiceBoxClass
{
public:
	// DIFF: derived shared_ptr array removed - see WaypointChoiceBoxClass.h.
	static constexpr KindType ClassKind = KindType::Screen;

	int ScreenX { 50 }; // 屏幕 X 坐标（百分比 0~100）
	int ScreenY { 50 }; // 屏幕 Y 坐标（百分比 0~100）

	ScreenChoiceBoxClass() = default;
	ScreenChoiceBoxClass(int id, int x, int y, const char* label,
		const ChoiceBoxTypeClass* pType);

	// ===== 虚接口实现 =====
	bool GetDrawPosition(Point2D& outPos) const override;
	bool ClampToScreen() const override { return true; }

	KindType GetKind() const override { return ClassKind; }
	const char* GetTypeMarker() const override { return "ScreenChoiceBoxClass"; }

	// ===== 查找/创建/移除 =====
	static ScreenChoiceBoxClass* FindOrCreate(int x, int y,
		const char* label, const ChoiceBoxTypeClass* pType);
	static ScreenChoiceBoxClass* FindOrCreate(int id, int x, int y,
		const char* label, const ChoiceBoxTypeClass* pType);

	static void RemoveByLabel(const char* label);

	// DIFF: RemoveByID removed - use MapChoiceBoxClass::RemoveByID(id), which
	// covers both kinds. Every call site already paired the two.

	static void ClearAll();
	static void Clear();

	// ===== 序列化 =====
	bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	bool Save(PhobosStreamWriter& Stm) const override;

protected:
	template <typename T>
	bool Serialize(T& Stm);
};