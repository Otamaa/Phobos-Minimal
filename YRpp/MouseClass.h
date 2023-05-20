#pragma once

#include <SidebarClass.h>
#include <FileFormats/SHP.h>

class MouseCursor
{
public:

	static inline constexpr size_t DefaultCursorsCount = (size_t)MouseCursorType::count;
	static constexpr reference<MouseCursor, 0x82D028u, (size_t)MouseCursorType::count> const DefaultCursors{};
	static constexpr reference<MouseCursor, 0x82D028u, (size_t)MouseCursorType::count> const DefaultCursorsB {};

	static MouseCursor& GetDefaultCursor(MouseCursorType cursor)
		{ return DefaultCursors[static_cast<int>(cursor)]; }

	static MouseCursor& GetCursor(MouseCursorType cursor)
		{ return DefaultCursors[static_cast<int>(cursor)]; }

	static MouseCursor& GetCursor(size_t cursor)
		{ return DefaultCursors[cursor]; }

	FORCEINLINE int GetFrameRate() const
	{
		return FrameRate;
	}

	FORCEINLINE int GetMouseFrame(bool wsmall) const
	{
		return !wsmall || SmallFrame == -1 ?
			StartFrame : SmallFrame;
	}

	FORCEINLINE int GetMouseFrameCount(bool wsmall) const
	{
		return !wsmall ? FrameCount : SmallFrameCount;
	}

	FORCEINLINE Point2D GetMouseHotSpot(const ShapeFileStruct* pShape) const
	{
		Point2D nResult { 0 , 0 };

		if (X == MouseHotSpotX::Center)
			nResult.X = pShape->GetWidth() / 2;

		if (X == MouseHotSpotX::Right)
			nResult.X = pShape->GetWidth();

		if (Y == MouseHotSpotY::Middle)
			nResult.Y = pShape->GetHeight() / 2;

		if (Y == MouseHotSpotY::Bottom)
			nResult.Y = pShape->GetHeight();

		return nResult;
	}

	MouseCursor() = default;

	MouseCursor(
		int frame, int count, int interval, int miniFrame, int miniCount,
		MouseHotSpotX hotX, MouseHotSpotY hotY)
		: StartFrame(frame), FrameCount(count), FrameRate(interval), SmallFrame(miniFrame),
		SmallFrameCount(miniCount), X(hotX), Y(hotY)
	{ }

	~MouseCursor() noexcept = default;

public:

	int StartFrame { 0 };
	int FrameCount { 1 };
	int FrameRate { 1 };
	int	SmallFrame { -1 };
	int	SmallFrameCount { 0 };
	MouseHotSpotX X { MouseHotSpotX::Center };
	MouseHotSpotY Y { MouseHotSpotY::Middle };
};

struct MouseCursorDataStruct 
{
	MouseCursorDataStruct() = default;

	MouseCursorDataStruct(
		int frame, int count, int interval,
		int miniFrame, int miniCount,  int miniInterval ,
		MouseHotSpotX hotX, MouseHotSpotY hotY
	) : OriginalData { frame , count, interval, miniFrame, miniCount, hotX, hotY },
		SmallFrameRate(miniInterval)
	{ }

	MouseCursorDataStruct(MouseCursor const& nData , int miniInterval) :
		OriginalData { nData },
		SmallFrameRate { miniInterval }
	{ }

	~MouseCursorDataStruct() noexcept = default;

public:
	MouseCursor OriginalData {};
	int SmallFrameRate { -1 };
};

static_assert(sizeof(MouseCursor) == 0x1C);

struct TabDataClass
{
	int TargetValue;
	int LastValue;
	bool NeedsRedraw;
	bool ValueIncreased;
	bool ValueChanged;
	PROTECTED_PROPERTY(BYTE, align_B);
	int ValueDelta;
};

class TabClass : public SidebarClass, public INoticeSink
{
public:
	//Static
	static constexpr constant_ptr<TabClass, 0x87F7E8u> const Instance{};

	void Activate(int control = 1)
		{ JMP_THIS(0x6D04F0); }

public:
	TabDataClass TabData;
	CDTimerClass unknown_timer_552C;
	CDTimerClass InsufficientFundsBlinkTimer;
	BYTE unknown_byte_5544;
	bool MissionTimerPinged;
	BYTE unknown_byte_5546;
	PROTECTED_PROPERTY(BYTE, padding_5547);
};

class ScrollClass : public TabClass
{
public:
	//Static
	static constexpr constant_ptr<ScrollClass, 0x87F7E8u> const Instance{};

	virtual void Update(const int& keyCode, const Point2D& mouseCoords) override JMP_THIS(0x6922E0);

	void Update_(const int* keyCode, const Point2D* mouseCoords) JMP_THIS(0x6922E0);
public:
	DWORD unknown_int_5548;
	BYTE unknown_byte_554C;
	PROTECTED_PROPERTY(BYTE, align_554D[3]);
	DWORD RightclickPressPoint; //5550
	DWORD unknown_int_5554;
	BYTE unknown_byte_5548;
	BYTE unknown_byte_5549;
	BYTE unknown_byte_554A;
	PROTECTED_PROPERTY(BYTE, padding_554B);
};

class NOVTABLE MouseClass : public ScrollClass
{
public:
	//Static
	static constexpr constant_ptr<MouseClass, 0x87F7E8u> const Instance{};
	static constexpr constant_ptr<MouseClass, 0x87F7E8u> const Global{};
	static constexpr reference<ShapeFileStruct*, 0xABF294u> const ShapeData{};
	static constexpr reference<bool, 0xABF2DDu> const ShapeOverride{};
	static constexpr reference<SystemTimerClass, 0xABF2A0u> const Timer {};
	//Destructor
	virtual ~MouseClass() RX;

	//GScreenClass
	virtual bool SetCursor(MouseCursorType idxCursor, bool miniMap) override R0;
	virtual bool UpdateCursor(MouseCursorType idxCursor, bool miniMap) override R0;
	virtual bool RestoreCursor() override R0;
	virtual void UpdateCursorMinimapState(bool miniMap) override RX;

	//DisplayClass
	virtual MouseCursorType GetLastMouseCursor() override RT(MouseCursorType);

public:
	bool MouseCursorIsMini;
	PROTECTED_PROPERTY(BYTE, unknown_byte_5559[3]);
	MouseCursorType MouseCursorIndex;
	MouseCursorType MouseCursorLastIndex;
	int MouseCursorCurrentFrame;
};
