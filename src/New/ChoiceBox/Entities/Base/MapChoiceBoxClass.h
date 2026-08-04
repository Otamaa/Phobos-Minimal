#pragma once

#include <Utilities/PhobosOwnedArray.h>

#include <string>
#include <vector>
#include <Point2D.h>
#include <RectangleStruct.h>

class PhobosStreamWriter;
class PhobosStreamReader;
class ChoiceBoxTypeClass;

class MapChoiceBoxClass
{
public:
	// ========================================================================
	// Kind dispatch
	//
	// DIFF: replaces the strcmp(GetTypeMarker(), "...") dispatch used by
	// LoadGlobals and the removal paths with an int compare. GetTypeMarker()
	// is kept, but is now used ONLY for the savegame stream, so the on-disk
	// type marker format is unchanged.
	// ========================================================================
	enum class KindType : int
	{
		Waypoint = 0,
		Screen = 1,
	};

	// ===== 公开成员 =====
	int ID { -1 };
	std::string Label;
	const ChoiceBoxTypeClass* Type { nullptr };
	int TypeIndex { -1 };

	// ------------------------------------------------------------------------
	// Trigger contract - DO NOT repurpose these. Read by TEvent 557/558/559.
	//
	//   ClickedIndex     -1 = unanswered, -2 = timed out, >=0 = button index
	//   ClickedConsumed  one-fire-per-click latch (TEvent 557/558)
	//   IsExpired        "timed out UNANSWERED" - polled by TEvent 559, which
	//                    has NO consumption latch and re-fires every evaluation
	//   RemainingFrames  visual countdown / phase-three driver
	//   ClickExpireCounter  hidden period so 557/558 can observe ClickedIndex
	//
	// IsExpired is not a general "this box is done" flag. Setting it on a
	// successful answer would fire the timeout trigger for an answered box.
	// ------------------------------------------------------------------------
	int ClickedIndex { -1 };
	bool ClickedConsumed { false };
	int RemainingFrames { -1 };
	int ClickExpireCounter { -1 };
	bool IsExpired { false };

	// ------------------------------------------------------------------------
	// EXTENSION: network answer arbitration
	// ------------------------------------------------------------------------

	// Absolute frame after which answers are no longer accepted. -1 = no
	// timeout. Set at creation to CurrentFrame + Type->Duration; deterministic
	// because creation runs from a lockstep TAction.
	//
	// Purely additive: RemainingFrames keeps its existing role. Only the
	// ChoiceBoxClick event reads this.
	int DeadlineFrame { -1 };

	// House index that supplied the current answer, -1 = unanswered. Needed
	// for first-wins arbitration because ClickedIndex cannot distinguish
	// "nobody answered" from "answered with button 0".
	//
	// Reset to -1 on a Bounce-mode rearm, so each round arbitrates separately.
	int AnsweredBy { -1 };

	// LOCAL ONLY. Stops this machine's UI queueing a second event while one is
	// in flight. Never read by a TEvent, never influences a branch, never
	// serialized - it is presentation state, not gamestate.
	bool IsAwaitingLocalResponse { false };

	// Absolute frame at which IsExpired was set, -1 while still live. Only used
	// by the optional post-expiry sweep; harmless when that is disabled.
	int ExpiredAtFrame { -1 };

	// ------------------------------------------------------------------------
	// Timing constants
	// ------------------------------------------------------------------------

	// Minimum frames an answered box stays alive (hidden) so TEvent 557/558 can
	// observe ClickedIndex. NOT a timeout: phase two keeps waiting past this
	// until ClickedConsumed is set.
	//
	// DIFF: was a file-static in the .cpp; promoted so the event handler can
	// reference it. Value unchanged.
	static constexpr int CLICK_EXPIRE_FRAMES = 5;

	// EXTENSION: bounded lifetime for timed-out boxes, in frames past the
	// IsExpired flip.
	//
	// -1 (default) reproduces existing behaviour exactly: timed-out boxes are
	// never swept and live until Clear(). That is a leak, but TEvent 559 polls
	// IsExpired *on the box*, so removing it early makes the timeout trigger
	// impossible to fire. Enabling this changes mission-visible behaviour and
	// is therefore opt-in.
	//
	// VERIFY: before setting a real value, establish the worst-case gap between
	// the expiry frame and trigger evaluation in your mission set.
	static constexpr int POST_EXPIRY_LIFETIME = -1;

	// 禁止拷贝 / 移动
	MapChoiceBoxClass(const MapChoiceBoxClass&) = delete;
	MapChoiceBoxClass& operator=(const MapChoiceBoxClass&) = delete;
	MapChoiceBoxClass(MapChoiceBoxClass&&) = delete;
	MapChoiceBoxClass& operator=(MapChoiceBoxClass&&) = delete;

	virtual ~MapChoiceBoxClass();

	// ===== 构造 =====
	MapChoiceBoxClass(int id, const char* label, const ChoiceBoxTypeClass* pType);

	// ===== 绘制 =====
	void DrawAt(Point2D centerPos);

	// ===== 交互 =====
	//
	// DIFF: CheckMouseClick() is gone. It wrote ClickedIndex directly from
	// local input, which made the answer local state rather than replicated
	// gamestate - the desync. It is replaced by:
	//
	//   HitTestButtons()   pure local hit-test, writes nothing
	//   PollLocalInput()   hit-tests and raises a ChoiceBoxClick event
	//
	// ClickedIndex is now written in exactly two places: the ChoiceBoxClick
	// event handler, and the timeout path in the phase-three sweep (-2).
	bool HitTestButtons(int& outIndex) const;
	void PollLocalInput();
	void ResetChoice();

	// ===== 虚接口（派生类实现） =====
	virtual bool CanDraw() const;
	virtual bool GetDrawPosition(Point2D& outPos) const = 0;
	virtual bool ClampToScreen() const;

	// Runtime dispatch. Cheap; used by every filtered iteration.
	virtual KindType GetKind() const = 0;

	// Savegame stream only. Kept as a string so the on-disk format is stable.
	virtual const char* GetTypeMarker() const = 0;

	// ========================================================================
	// 全局管理
	//
	// DIFF: single owning array. WaypointChoiceBoxClass::Array and
	// ScreenChoiceBoxClass::Array are deleted outright. Every access through
	// them was a linear scan, so ForEachOf<T> / FindOf<T> / RemoveIfOf<T>
	// replace them at identical cost - and there is no second container left to
	// fall out of sync with.
	//
	// This removes, structurally:
	//   - ClearAll() clearing only one of the two arrays
	//   - the hand-written two-array erase in every Remove* function
	//   - FindOrCreate's Array.back().get() dance
	// ========================================================================
	static PhobosOwnedArray<MapChoiceBoxClass> Array;

	static void DrawAll();
	static void DrawWaypoint();
	static void DrawScreen();

	// DIFF: ClearAll() delegated to Clear() already; with one array there is no
	// distinction left to get wrong. Both kept for call-site compatibility.
	static void Clear();
	static void ClearAll();

	// Removes any box with this ID regardless of kind.
	// DIFF: replaces the paired WaypointChoiceBoxClass::RemoveByID(id) +
	// ScreenChoiceBoxClass::RemoveByID(id) calls at every call site.
	static void RemoveByID(int id);

	static MapChoiceBoxClass* FindByID(int id);

	// 全局存档/读档
	static bool SaveGlobals(PhobosStreamWriter& Stm);
	static bool LoadGlobals(PhobosStreamReader& Stm);

	// ===== 序列化 =====
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	virtual bool Save(PhobosStreamWriter& Stm) const;

protected:
	MapChoiceBoxClass() = default;

	template <typename T>
	bool Serialize(T& Stm);

private:
	struct BtnLayoutItem;
	struct ButtonRect
	{
		int Index;
		RectangleStruct Rect;
	};
	std::vector<ButtonRect> m_buttonRects;

	void UpdateButtonRects(Point2D topLeft, int bgWidth, int buttonsStartY,
		const std::vector<BtnLayoutItem>& btnItems);
};