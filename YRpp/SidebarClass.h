#pragma once

#include <PowerClass.h>
#include <ProgressTimer.h>
#include <ControlClass.h>
#include <RectangleStruct.h>

class ColorScheme;
class FactoryClass;
struct SHPStruct;
// SidebarClass::StripClass::BuildType
struct BuildType
{
	int               ItemIndex;
	AbstractType      ItemType;
	BuildCat		  Cat; // set on buildings that go on tab 2
	FactoryClass* CurrentFactory;
	DWORD             Status;
	ProgressTimer     Progress; // 0 to 54, how much of this object is constructed (gclock anim level)
	int               FlashEndFrame;

public:

	BuildType() noexcept :
		ItemIndex { -1 },
		ItemType { AbstractType::None },
		Cat { BuildCat::DontCare },
		CurrentFactory { nullptr },
		Status { 0 },
		Progress { },
		FlashEndFrame { 0 }
	{
	}

	BuildType(int itemIndex, AbstractType itemType) noexcept :
		ItemIndex { itemIndex },
		ItemType { itemType },
		Cat { BuildCat::DontCare },
		CurrentFactory { nullptr },
		Status { 0 },
		Progress { },
		FlashEndFrame { 0 }
	{
	}

	BuildType&operator =(const BuildType&) = default;
	BuildType(const BuildType&) = default;
	BuildType&operator=(BuildType&&) = default;

	bool operator == (const BuildType& rhs) const
	{
		return ItemIndex == rhs.ItemIndex && ItemType == rhs.ItemType;
	}

	bool operator != (const BuildType& rhs) const
	{
		return ItemIndex != rhs.ItemIndex || ItemType != rhs.ItemType;
	}

	static bool __stdcall SortsBefore(AbstractType leftType, int leftIndex, AbstractType rightType, int rightIndex) {
		JMP_STD(0x6A8420);
	}

	bool operator < (const BuildType& rhs) const
	{
		return SortsBefore(this->ItemType, this->ItemIndex, rhs.ItemType, rhs.ItemIndex);
	}

};

typedef BuildType CameoDataStruct;
// SidebarClass::StripClass
struct StripClass
{
	static void __stdcall LoadDarkenShapeOnce(int nArgs) { JMP_STD(0x6A8160); }

	SHPStruct* GetSuperWeaponTypeCameo(int nIndex) const { JMP_THIS(0x6A8180); }
	void Clear() const { JMP_THIS(0x6A81B0); }
	void InitIndex(int nIndex) const { JMP_THIS(0x6A8220); }
	bool Activate() const { JMP_THIS(0x6A8330); }
	void Deactivate() const { JMP_THIS(0x6A83E0); }
	void Add(AbstractType AbsType, int nIndex) const { JMP_THIS(0x6A87F0); }
	void Add_(AbstractType AbsType, int nIndex) const { JMP_THIS(0x6A8710); }
	bool ScrollBy(bool bUpward, int nBy) const { JMP_THIS(0x6A8860); }
	bool Func_6A8920(bool bUpward) const { JMP_THIS(0x6A8920); }
	bool Func_6A89E0(bool bUpward) const { JMP_THIS(0x6A89E0); }
	void Redraw() const { JMP_THIS(0x6A8B10); }
	void AI(int* nKeyNumType, Point2D& nPos) const { JMP_THIS(0x6A8B30); }
	wchar_t* HelpText(int nArgs2) const { JMP_THIS(0x6A92E0); }
	bool Func_6A93F0_GScreenAddButton() const { JMP_THIS(0x6A93F0); }
	bool Func_6A94B0_GScreenRemoveButton() const { JMP_THIS(0x6A94B0); }
	int Inlined_6A94F0(int nArgs2) const { JMP_THIS(0x6A94F0); }
	void DrawIt(int nArgs1) const { JMP_THIS(0x6A9540); }
	bool Recalc() const { JMP_THIS(0x6AA600); }
	bool Factory(FactoryClass* pFactory, AbstractType AbsType, int nIndex) const { JMP_THIS(0x6ABA60); }
	bool HasCameos() const { JMP_THIS(0x6ABAC0); }
	bool Func_6ABB60(FactoryClass* pFactory) const { JMP_THIS(0x6ABB60); }
	void Func_6AC740(int nIndex, int nDuration) const { JMP_THIS(0x6AC740); }

public:
	StageClass        Progress;
	bool              AllowedToDraw; // prevents redrawing when layouting the list
	PROTECTED_PROPERTY(BYTE, align_1D[3]);
	Point2D           Location;
	RectangleStruct   Bounds;
	int               Index; // the index of this tab
	bool              NeedsRedraw;
	BYTE              unknown_3D;
	BYTE              unknown_3E;
	BYTE              unknown_3F;
	DWORD             unknown_40;
	int               TopRowIndex; // scroll position, which row is topmost visible
	DWORD             unknown_48;
	DWORD             unknown_4C;
	DWORD             unknown_50;
	int               CameoCount; // filled cameos
	BuildType         Cameos[75];
};

typedef StripClass TabDataStruct;
static_assert(sizeof(StripClass) == 0xF94);

class NOVTABLE SidebarClass : public PowerClass
{
public:
	//Static
	static inline constexpr size_t TooltipLength = 0x42;
	static constexpr constant_ptr<SidebarClass, 0x87F7E8u> const Instance {};
	static constexpr reference<DWORD, 0xB0B500u> const ObjectHeight {};
	static constexpr reference<wchar_t, 0xB07BC4u, TooltipLength> const TooltipBuffer {};
	static constexpr reference<SHPFrame*, 0xB0B478u> const Shape_B0B478 {};
	static constexpr reference<int, 0x884B7C> const something_884B7C {};
	static constexpr reference<int, 0x884B80> const something_884B80 {};
	static constexpr reference<int, 0x884B84> const something_884B84 {};

	void SidebarNeedsRepaint(int mode = 0)
	{
		this->SidebarNeedsRedraw = true;
		this->SidebarBackgroundNeedsRedraw = true;
		this->Tabs[this->ActiveTabIndex].AllowedToDraw = true;
		this->Tabs[this->ActiveTabIndex].NeedsRedraw = true;
		this->RedrawSidebar(mode);
		SidebarClass::Draw(1);
	}

	void RepaintSidebar(int tab = 0)
	{ JMP_THIS(0x6A60A0); }

	bool AddCameo(AbstractType absType, int idxType)
	{ JMP_THIS(0x6A6300); }

	//virtual void Draw(DWORD dwUnk) override
	//	{ JMP_THIS(0x6A6C30); }

	//Destructor
	virtual ~SidebarClass() override JMP_THIS(0x6AC7F0);

	//GScreenClass
	virtual void One_Time() override JMP_THIS(0x6A5000);
	virtual void Init_Clear() override JMP_THIS(0x6A5030);
	virtual void Init_IO() override JMP_THIS(0x6A5310);
	virtual void Update(const int& keyCode, const Point2D& mouseCoords) override JMP_THIS(0x6A7780);
	virtual void Draw(DWORD dwUnk) override JMP_THIS(0x6A6C30);

	//DisplayClass
	virtual HRESULT Load(IStream* pStm) override JMP_THIS(0x6AC5D0);
	virtual HRESULT Save(IStream* pStm) override JMP_THIS(0x6AC5E0);
	virtual const wchar_t* GetToolTip(UINT nDlgID) override JMP_THIS(0x6AC210);
	virtual void CloseWindow() override JMP_THIS(0x6ABD30); //prolly wrong naming

	//RadarClass
	virtual void DisposeOfArt() override JMP_THIS(0x6A5BF0);
	virtual void Init_For_House() override JMP_THIS(0x6A5840);

	//SidebarClass
	virtual bool Activate(int control = 1) JMP_THIS(0x6A7D70);

	//Non-virtual
	void ToggleStuffs() const { JMP_THIS(0x6A6610); }

	// which tab does the 'th object of that type belong in?
	static int __fastcall GetObjectTabIdx(AbstractType abs, int idxType, int unused)
	{ JMP_STD(0x6ABC60); }

	// which tab does the 'th object of that type belong in?
	static int __fastcall GetObjectTabIdx(AbstractType abs, BuildCat buildCat, bool isNaval)
	{ JMP_STD(0x6ABCD0); }

	void ChangeTab(int nStrip) const { JMP_THIS(0x6A7590); }

	void Recalc() const { JMP_THIS(0x6A7D20); }

	int Func_6AC430() const { JMP_THIS(0x6AC430); }
protected:
	//Constructor
	SidebarClass() noexcept	//don't need this
	{ JMP_THIS(0x6A4E60); }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	StripClass Tabs[0x4];
	DWORD unknown_5394;
	DWORD unknown_5398;
	int ActiveTabIndex;
	DWORD unknown_53A0;
	bool HideObjectNameInTooltip; // see 0x6A9343
	bool IsSidebarActive;
	bool SidebarNeedsRedraw;
	bool SidebarBackgroundNeedsRedraw;
	bool unknown_bool_53A8;

	//Information for the Diplomacy menu, I believe
	HouseClass* DiplomacyHouses[0x8];		//8 players max!
	int DiplomacyKills[0x8];		//total amount of kills per house
	int DiplomacyOwned[0x8];		//total amount of currently owned unit/buildings per house
	int DiplomacyPowerDrain[0x8];	//current power drain per house
	ColorScheme* DiplomacyColors[0x8];		//color scheme per house
	DWORD unknown_544C[0x8];			//??? per house - unused
	DWORD unknown_546C[0x8];			//??? per house - unused
	DWORD unknown_548C[0x8];			//??? per house - unused
	DWORD unknown_54AC[0x8];			//??? per house - unused
	DWORD unknown_54CC[0x8];			//??? per house - unused
	DWORD unknown_54EC[0x8];			//??? per house - unused
	BYTE unknown_550C;
	int DiplomacyNumHouses;			//possibly?

	bool unknown_bool_5514;
	bool unknown_bool_5515;
	PROTECTED_PROPERTY(BYTE, padding_5516[2]);
};

class NOVTABLE SelectClass : public ControlClass
{

public:

	static constexpr reference<SelectClass*, 0xB07E80u , 14u> const Buttons {};
	static constexpr constant_ptr<SelectClass, 0xB07E80> const ButtonsPtr {};
	static constexpr constant_ptr<SelectClass, 0xB07E8C> const Buttons_beginPtr {};
	static constexpr constant_ptr<SelectClass, 0xB0B300> const Buttons_endPtr {};

	//Destructor
	virtual ~SelectClass() RX;

	//GadgetClass

	//ControlClass

	void SetOwner(StripClass* pStrip, int nIdx) const { JMP_THIS(0x6AACE0); }
	void OnMouseEnter() const { JMP_THIS(0x6AB990); }
	void OnMouseLeave() const { JMP_THIS(0x6AB9E0); }
	int Action(int flags, int* key, DWORD nKeyModifierFlag) const { JMP_THIS(0x6AAD00); }

	//Constructors
	SelectClass() noexcept
		: ControlClass(noinit_t())
	{
		JMP_THIS(0x6AACB0);
	}

protected:
	explicit __forceinline SelectClass(noinit_t)  noexcept
		: ControlClass(noinit_t())
	{
	}

public:
	StripClass* Strip;
	int Index;
	DWORD __MouseOver;
};

static_assert(sizeof(SelectClass) == 0x38);
