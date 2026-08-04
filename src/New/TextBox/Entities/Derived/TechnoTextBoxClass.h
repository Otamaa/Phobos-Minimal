#pragma once

#include "../Base/MapTextBoxClass.h"
#include <TechnoClass.h>
 
class PhobosStreamWriter;
class PhobosStreamReader;
class TriggerClass;

class TechnoTextBoxClass final : public MapTextBoxClass
{
public:
	// DIFF: the derived shared_ptr array is gone - see MapTextBoxClass.h.
	static constexpr KindType ClassKind = KindType::Techno;

	TechnoClass* Target { nullptr };  // 当前绑定的目标指针
	DWORD SavedTargetUID { 0 };       // 存档时保存的 UID，用于读档重建指针

	TechnoTextBoxClass() = default;
	TechnoTextBoxClass(TechnoClass* pTarget, const char* csfLabel,
		const char* typeName);

	// ===== 虚接口实现 =====
	bool IsAlive() const override;
	bool CanDraw() const override;
	bool GetDrawPosition(Point2D& outPos) const override;

	KindType GetKind() const override { return ClassKind; }
	const char* GetTypeMarker() const override { return "TechnoTextBoxClass"; }

	// ========================================================================
	// BUGFIX: dangling Target.
	//
	// Target was a raw TechnoClass* with no invalidation. The only protection
	// was a Health <= 0 test in CanDraw() plus a once-per-frame sweep in
	// CleanupDeadLabels(), which leaves a window where the TechnoClass has been
	// freed but Target still points at it - and CanDraw() dereferenced
	// Target->InLimbo before any check that would catch reuse. YR pools and
	// reuses those allocations, so a stale read can return a plausible Health
	// belonging to a completely different unit.
	//
	// PhobosTypeRegistry already dispatches T::PointerGotInvalid(void*, bool)
	// for every registered type, and TechnoTextBoxClass is registered in
	// Phobos.Ext.cpp - so simply declaring this hooks it up. No new DEFINE_HOOK
	// is needed; AnnounceInvalidPointer at 0x7258D0 already feeds the registry.
	// ========================================================================
	static void PointerGotInvalid(void* ptr, bool removed);

	// ===== 查找/创建 =====
	static TechnoTextBoxClass* FindOrCreate(TechnoClass* pTarget,
		const char* csfLabel, const char* typeName);
	static TechnoTextBoxClass* Find(TechnoClass* pTarget);

	// ===== 批量移除 =====
	static void Remove(TechnoClass* pTarget);
	static void RemoveByType(int typeIndex);
	static void RemoveByTrigger(TriggerClass* pTrigger);
	static void RemoveByTeam(int teamIndex);

	// ===== 全局清理 =====
	static void ClearAll();
	static void Clear();

	// DIFF: CleanupDeadLabels() removed. It was a second full pass per frame
	// doing what IsAlive() now does inside the single DrawAll() pass. With
	// PointerGotInvalid wired up, labels are invalidated at the moment of
	// death rather than swept a frame later.

	// ===== 序列化 =====
	bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	bool Save(PhobosStreamWriter& Stm) const override;

private:
	template <typename T>
	bool Serialize(T& Stm);
};