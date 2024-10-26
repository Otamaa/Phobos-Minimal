#pragma once

#include <LinkClass.h>
#include <RectangleStruct.h>

enum class KeyModifier : int
{
	None = 0,
	Shift = 1,
	Ctrl = 2,
	Alt = 4
};

enum class GadgetFlag : int
{
	LeftPress = 0x1,
	LeftHeld = 0x2,
	LeftRelease = 0x4,
	LeftUp = 0x8,
	RightPress = 0x10,
	RightHeld = 0x20,
	RightRelease = 0x40,
	RightUp = 0x80,
	Keyboard = 0x100
};

class NOVTABLE GadgetClass : public LinkClass
{
public:
	static constexpr reference<GadgetClass*, 0x8B3E90> Focused {};

	//Destructor
	virtual ~GadgetClass() RX;

	//LinkClass
	virtual GadgetClass* GetNext() override JMP_THIS(0x4E14A0);
	virtual GadgetClass* GetPrev() override JMP_THIS(0x4E14B0);
	virtual GadgetClass* Remove() override JMP_THIS(0x4E1480);

	//GadgetClass
	virtual DWORD Input() JMP_THIS(0x4E1640);
	virtual void DrawAll(bool forced) JMP_THIS(0x4E1570);
	virtual void DeleteList() JMP_THIS(0x4E14C0);
	virtual GadgetClass* ExtractGadget(unsigned int id) JMP_THIS(0x4E1920);
	virtual void MarkListToRedraw() JMP_THIS(0x488690);
	virtual void Disable() JMP_THIS(0x4E1460);
	virtual void Enable() JMP_THIS(0x4E1450);
	virtual unsigned int const GetID() JMP_THIS(0x4AEBA0); //return 0;
	virtual void MarkRedraw() JMP_THIS(0x4E1960);
	virtual void PeerToPeer(unsigned int flags, DWORD* pKey, GadgetClass* pSendTo) JMP_THIS(0x48E650);
	virtual void SetArchiveTarget() JMP_THIS(0x4E19A0);
	virtual void KillFocus() JMP_THIS(0x4E19D0);
	virtual bool IsFocused() JMP_THIS(0x4E19FA);
	virtual bool IsListToRedraw() JMP_THIS(0x4E1A00);
	virtual bool IsToRedraw() JMP_THIS(0x4886A0);
	virtual void SetPosition(int x, int y) JMP_THIS(0x4E1A20);
	virtual void SetDimension(int width, int height) JMP_THIS(0x4E1A40);
	virtual bool Draw(bool forced) JMP_THIS(0x4E1550);
	virtual void OnMouseEnter() JMP_THIS(0x4E1510);
	virtual void OnMouseLeave() JMP_THIS(0x4E1520);
	virtual void StickyProcess(GadgetFlag flags) JMP_THIS(0x4E1970);
	virtual bool Action(GadgetFlag flags, DWORD* pKey, KeyModifier modifier) JMP_THIS(0x4E1530);
	virtual bool Clicked(DWORD* pKey, GadgetFlag flags, int x, int y, KeyModifier modifier) JMP_THIS(0x4E13F0);

	//Non virtual
	GadgetClass& operator=(GadgetClass& another) { JMP_THIS(0x4B5780); }
	GadgetClass* ExtractGadgetAt(int nX, int nY) { JMP_THIS(0x4E15A0); }

	//Statics
	static int __fastcall GetColorScheme() { JMP_STD(0x4E12D0); }

	//Constructors
	GadgetClass(int nX,int nY,int nWidth,int nHeight,GadgetFlag eFlag, bool bSticky) noexcept
		: GadgetClass(noinit_t()) { JMP_THIS(0x4E12F0); }

	GadgetClass(RectangleStruct rect , GadgetFlag eFlag, bool bSticky) noexcept
		: GadgetClass(noinit_t()) { JMP_THIS(0x4E12F0); }

	GadgetClass(GadgetClass& another) noexcept
		: GadgetClass(noinit_t()) { JMP_THIS(0x4E1340); }

protected:
	explicit __forceinline GadgetClass(noinit_t)  noexcept
		: LinkClass(noinit_t())
	{
	}

	//Properties
public:
	RectangleStruct Rect;
	bool NeedsRedraw;
	bool IsSticky;
	bool Disabled;
	GadgetFlag Flags;
};
static_assert(sizeof(GadgetClass) == 0x24, "Invalid Size ! ");

class NOVTABLE TextLabelClass : public GadgetClass
{
	//Destructor
	virtual ~TextLabelClass() JMP_THIS(0x72A670);

	virtual bool Draw(bool bForced) override JMP_THIS(0x72A4A0);

	//TextLabelClass
	virtual void SetText(wchar_t* pText) JMP_THIS(0x72A660);

	void Set_Text(wchar_t *a2) {
	  this->Text = a2;
	}

	TextLabelClass(wchar_t* txt, int x, int y, int ColorSchemeIndex, TextPrintType style) noexcept
		: TextLabelClass(noinit_t()) { JMP_THIS(0x72A440); }

	TextLabelClass(wchar_t* txt, Point2D point, int ColorSchemeIndex, TextPrintType style) noexcept
		: TextLabelClass(noinit_t()) { JMP_THIS(0x72A440); }
protected:
	explicit __forceinline TextLabelClass(noinit_t)  noexcept
		: GadgetClass(noinit_t())
	{ }

	//Properties
public:
 void* UserData1;
 void* UserData2;
  DWORD Style;
  wchar_t *Text;
  int ColorSchemeIndex;
  DWORD PixWidth;
  DWORD anim_dword3C;
  bool SkipDraw;
  bool Animate;
  DWORD anim_pos44;
  DWORD anim_timing48;
};

static_assert(sizeof(TextLabelClass) == 0x4C , "Invalid Size ! ");