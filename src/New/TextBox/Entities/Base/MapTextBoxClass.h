#pragma once

#include <Utilities/PhobosOwnedArray.h>

#include <string>
#include <vector>
#include <Point2D.h>

class PhobosStreamWriter;
class PhobosStreamReader;

class MapTextBoxClass
{
public:
	// ========================================================================
	// Kind dispatch
	//
	// DIFF: replaces the strcmp(GetTypeMarker(), "...") dispatch that DrawAll
	// and LoadGlobals used. GetTypeMarker() survives for the savegame stream,
	// so the on-disk format is unchanged.
	// ========================================================================
	enum class KindType : int
	{
		Waypoint = 0,
		Techno = 1,
	};

	// ===== 公开成员 =====
	std::string CurrentLabel;       // CSF 标签名
	int MaxLineWidth { 250 };       // 单行最大像素宽度
	int BackgroundOpacity { 75 };   // 背景不透明度 (0-100)
	int ColorR { 255 };
	int ColorG { 215 };
	int ColorB { 0 };

	// ADD: identity of the style type, mirroring MapChoiceBoxClass::TypeIndex.
	//
	// BUGFIX: TechnoTextBoxClass::RemoveByType used to match on style VALUES -
	// comparing MaxWidth, opacity and all three colour components - so any two
	// TextBoxTypes with identical appearance were indistinguishable and
	// removing one removed the other's labels. Matching on index is exact.
	//
	// Also fixes WaypointTextBoxClass::Type, which was assigned but never
	// serialized and so came back nullptr after a load.
	int TypeIndex { -1 };

	// 禁止拷贝 / 移动
	MapTextBoxClass(const MapTextBoxClass&) = delete;
	MapTextBoxClass& operator=(const MapTextBoxClass&) = delete;
	MapTextBoxClass(MapTextBoxClass&&) = delete;
	MapTextBoxClass& operator=(MapTextBoxClass&&) = delete;

	virtual ~MapTextBoxClass();

	// ===== 绘制 =====
	void DrawAt(Point2D centerPos);

	// ===== 布局缓存 =====
	void UpdateLayout();
	void ResetCache();

	// ========================================================================
	// 虚接口
	//
	// CRITICAL: IsAlive() and CanDraw() must not be conflated.
	//
	//   IsAlive()  MAY READ GAMESTATE ONLY. Drives REMOVAL, so it must return
	//              the same answer on every machine. Never touch TacticalClass,
	//              WWMouseClass, HouseClass::CurrentPlayer, or anything
	//              shroud/visibility related from here.
	//
	//   CanDraw()  May read local state freely. Drives DRAWING only; machines
	//              are allowed to disagree about what is on screen.
	//
	// The original fused both into CanDraw(), which was harmless while the
	// result only skipped drawing. Now that IsAlive() removes objects, putting
	// the occlusion check on the wrong side would make Array contents differ
	// per machine - which breaks MP save/load and would become a live desync
	// the moment anything reads textbox state.
	// ========================================================================
	virtual bool IsAlive() const;
	virtual bool CanDraw() const;
	virtual bool GetDrawPosition(Point2D& outPos) const = 0;

	virtual KindType GetKind() const = 0;
	virtual const char* GetTypeMarker() const = 0;

	// ========================================================================
	// 全局管理
	//
	// DIFF: single owning array. WaypointTextBoxClass::Array and
	// TechnoTextBoxClass::Array are deleted. Every access through them was a
	// linear scan, so the filtered helpers cost the same - and there is no
	// second container left to fall out of sync with.
	//
	// This removes ~120 lines of hand-written two-array bookkeeping in
	// TechnoTextBoxClass alone, plus the ClearAll() bug where the base array
	// was emptied without touching the derived ones.
	// ========================================================================
	static PhobosOwnedArray<MapTextBoxClass> Array;

	static void DrawAll();
	static void Clear();
	static void ClearAll();

	// 全局存档/读档
	static bool SaveGlobals(PhobosStreamWriter& Stm);
	static bool LoadGlobals(PhobosStreamReader& Stm);

	// ===== 序列化 =====
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	virtual bool Save(PhobosStreamWriter& Stm) const;

protected:
	MapTextBoxClass() = default;
	MapTextBoxClass(const char* csfLabel,
		int maxWidth = 250, int opacityPercent = 75,
		int colorR = 255, int colorG = 215, int colorB = 0);

	int VerticalOffset { 0 };

	// 剩余显示帧数：-1 = 无限，>=0 时每帧递减
	int RemainingFrames { -1 };

	template <typename T>
	bool Serialize(T& Stm);

private:
	struct Cache
	{
		bool IsLayoutDirty { true };
		std::vector<std::wstring> CachedLines;
		int CachedBgWidth { 0 };
		int CachedBgHeight { 0 };
	};

	// DIFF: was std::unique_ptr<Cache>. Cache is a four-member aggregate; the
	// heap allocation bought nothing and forced null checks in UpdateLayout(),
	// DrawAt() and ResetCache(). As a value member the NSDMIs apply even
	// through the defaulted deserialization constructor, so ResetCache() is
	// just `m_cache = {}` and every `if (!m_cache)` guard is gone.
	Cache m_cache {};
};