#pragma once

#include <SidebarClass.h>
#include <FileFormats/SHP.h>
#include "CreditClass.h"

class MouseCursor
{
public:

	static inline constexpr size_t DefaultCursorsCount = (size_t)MouseCursorType::count;
	static constexpr reference<MouseCursor, 0x82D028u, (size_t)MouseCursorType::count> const DefaultCursors{};

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

	MouseCursor(const MouseCursor& other) : StartFrame(other.StartFrame)
		, FrameCount(other.FrameCount)
		, FrameRate(other.FrameRate)
		, SmallFrame(other.SmallFrame)
		, SmallFrameCount(other.SmallFrameCount)
		, X(other.X)
		, Y(other.Y)
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
static inline constexpr size_t MouseCursorClassSize = sizeof(MouseCursor);

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

	MouseCursorDataStruct& operator=(const MouseCursorDataStruct& other) {
		std::memcpy(this, &other, sizeof(*this));
		return *this;
	}

public:
	MouseCursor OriginalData {};
	int SmallFrameRate { -1 };
};

static_assert(sizeof(MouseCursor) == 0x1C);

class TabClass : public SidebarClass, public INoticeSink
{
public:

	//Static
	static constexpr constant_ptr<TabClass, 0x87F7E8u> const Instance {};

	//Destructor
	virtual ~TabClass() override JMP_THIS(0x5BE9E0)

	//GScreenClass
	virtual void One_Time() override JMP_THIS(0x6D0260);
	virtual void Init_IO() override JMP_THIS(0x6D03A0);
	virtual void Update(const int& keyCode, const Point2D& mouseCoords) override JMP_THIS(0x6D0680);
	virtual void Draw(DWORD dwUnk) override JMP_THIS(0x6D0A20);

	//DisplayClass
	virtual const wchar_t* GetToolTip(UINT nDlgID) override JMP_THIS(0x6D1800);

	//RadarClass
	virtual void DisposeOfArt() override JMP_THIS(0x6D0270);
	virtual void Init_For_House() override JMP_THIS(0x6D02B0);

	//SidebarClass
	virtual bool Activate(int control = 1) override JMP_THIS(0x6D04F0);

	TabClass() JMP_THIS(0x6CFE20);

public:
	CreditClass TabData;
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

	//Destructor
	virtual ~ScrollClass() override JMP_THIS(0x6938F0);

	//GScreenClass
	virtual void Init_IO() override JMP_THIS(0x40D270);
	virtual void Update(const int& keyCode, const Point2D& mouseCoords) override JMP_THIS(0x6922E0);

	//MapClass
	virtual bool DraggingInProgress() override JMP_THIS(0x693060);

	//DisplayClass
	virtual void vt_entry_8C() override JMP_THIS(0x6938C0);
	virtual void vt_entry_B0(DWORD dwUnk) override JMP_THIS(0x693880);
	virtual void RightMouseButtonUp(DWORD dwUnk) override JMP_THIS(0x693840);

	ScrollClass() JMP_THIS(0x692290);

	void Update_(const int* keyCode, const Point2D* mouseCoords) JMP_THIS(0x6922E0);
public:
	DWORD unknown_int_5548;
	BYTE unknown_byte_554C;
	PROTECTED_PROPERTY(BYTE, align_554D[3]);
	DWORD RightclickPressPoint; //5550
	DWORD unknown_int_5554;
	BYTE unknown_byte_5558;
	BYTE unknown_byte_5559;
	BYTE unknown_byte_555A;
	PROTECTED_PROPERTY(BYTE, padding_555B);
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
	virtual ~MouseClass() JMP_THIS(0x40D290);

	//GScreenClass
	virtual void One_Time() override JMP_THIS(0x5BDF30);
	virtual void Init_Clear() override JMP_THIS(0x5BDF50);
	virtual void Update(const int& keyCode, const Point2D& mouseCoords) override JMP_THIS(0x5BDDC0);
	virtual bool SetCursor(MouseCursorType idxCursor, bool miniMap) override JMP_THIS(0x5BDA80);
	virtual bool UpdateCursor(MouseCursorType idxCursor, bool miniMap) override JMP_THIS(0x5BDC80);
	virtual bool RestoreCursor() override JMP_THIS(0x5BDAA0);
	virtual void UpdateCursorMinimapState(bool miniMap) override JMP_THIS(0x5BDAB0);

	//DisplayClass
	virtual HRESULT Load(IStream* pStm) override JMP_THIS(0x5BDF70);
	virtual HRESULT Save(IStream* pStm) override JMP_THIS(0x5BE6D0);
	virtual MouseCursorType GetLastMouseCursor() override JMP_THIS(0x40D280);

	MouseClass() JMP_THIS(0x5BDA40);

public:
	bool MouseCursorIsMini;
	PROTECTED_PROPERTY(BYTE, unknown_byte_5559[3]);
	MouseCursorType MouseCursorIndex;
	MouseCursorType MouseCursorLastIndex;
	int MouseCursorCurrentFrame;
};
