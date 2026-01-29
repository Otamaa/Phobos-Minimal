#pragma once
#include <SideClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/PhobosPCXFile.h>
#include <Utilities/PhobosFixedString.h>
#include <Utilities/TemplateDefB.h>

#include <Misc/Ares/EVAVoices.h>

#include <FileFormats/SHP.h>

#include <Ext/AbstractType/Body.h>

class SideExtData final : public AbstractTypeExtData
{
public:
	using base_type = SideClass;
	static COMPILETIMEEVAL const char* ClassName = "SideExtData";
	static COMPILETIMEEVAL const char* BaseClassName = "SideClass";
	static COMPILETIMEEVAL unsigned Marker = UuidFirstPart<base_type>::value;
	static COMPILETIMEEVAL auto Marker_str = to_hex_string<Marker>();

public:
#pragma region ClassMembers
	// ============================================================
	// Large aggregates: PhobosFixedString (32 bytes each)
	// ============================================================
	PhobosFixedString<0x20> ScoreMultiplayBackground;
	PhobosFixedString<0x20> ScoreMultiplayPalette;
	PhobosFixedString<0x20> ScoreMultiplayBars;
	PhobosFixedString<0x20> ScoreCampaignBackground;
	PhobosFixedString<0x20> ScoreCampaignTransition;
	PhobosFixedString<0x20> ScoreCampaignAnimation;
	PhobosFixedString<0x20> ScoreCampaignPalette;
	PhobosFixedString<0x20> GraphicalTextImage;
	PhobosFixedString<0x20> GraphicalTextPalette;
	PhobosFixedString<0x20> DialogBackgroundImage;
	PhobosFixedString<0x20> DialogBackgroundPalette;
	PhobosFixedString<0x20> ScoreCampaignThemeUnderPar;
	PhobosFixedString<0x20> ScoreCampaignThemeOverPar;
	PhobosFixedString<0x20> ScoreMultiplayThemeWin;
	PhobosFixedString<0x20> ScoreMultiplayThemeLose;

	// ============================================================
	// Large aggregates: PhobosPCXFile
	// ============================================================
	PhobosPCXFile SuperWeaponSidebar_OnPCX;
	PhobosPCXFile SuperWeaponSidebar_OffPCX;
	PhobosPCXFile SuperWeaponSidebar_TopPCX;
	PhobosPCXFile SuperWeaponSidebar_CenterPCX;
	PhobosPCXFile SuperWeaponSidebar_BottomPCX;

	// ============================================================
	// Large aggregates: CustomPalette
	// ============================================================
	CustomPalette GClock_Palette;

	// ============================================================
	// 24-byte aligned: Vectors
	// ============================================================
	NullableVector<TechnoTypeClass*> ParaDropTypes;
	NullableVector<int> ParaDropNum;
	NullableVector<BuildingTypeClass*> BaseDefenses;
	NullableVector<int> BaseDefenseCounts;

	// ============================================================
	// 8-byte aligned: Valueable<pointer>
	// ============================================================
	Valueable<SHPStruct*> GClock_Shape;
	Valueable<AircraftTypeClass*> SpyPlane;
	Valueable<UnitTypeClass*> HunterSeeker;
	Valueable<SHPStruct*> MouseShape;

	// ============================================================
	// Nullable<pointer> (pointer + bool + padding ≈ 16 bytes)
	// ============================================================
	Nullable<InfantryTypeClass*> Crew;
	Nullable<InfantryTypeClass*> Engineer;
	Nullable<InfantryTypeClass*> Technician;
	Nullable<AnimTypeClass*> ParachuteAnim;
	Nullable<InfantryTypeClass*> Disguise;

	// ============================================================
	// Nullable<int/float> (~8 bytes each)
	// ============================================================
	Nullable<int> SurvivorDivisor;
	Nullable<int> ToolTip_Background_Opacity;
	Nullable<float> ToolTip_Background_BlurSize;

	// ============================================================
	// Nullable<ColorStruct> (3 bytes + bool + padding ≈ 4-8 bytes)
	// ============================================================
	Nullable<ColorStruct> Sidebar_WeedsCounter_Color;
	Nullable<ColorStruct> ToolTip_Background_Color;
	Nullable<ColorStruct> Sidebar_BattlePoints_Color;

	// ============================================================
	// Nullable<bool> (~2-4 bytes)
	// ============================================================
	Nullable<bool> Sidebar_GDIPositions;

	// ============================================================
	// Valueable<Point2D> (8 bytes each)
	// ============================================================
	Valueable<Point2D> Sidebar_HarvesterCounter_Offset;
	Valueable<Point2D> Sidebar_ProducingProgress_Offset;
	Valueable<Point2D> Sidebar_PowerDelta_Offset;
	Valueable<Point2D> Sidebar_WeedsCounter_Offset;
	Valueable<Point2D> Sidebar_BattlePoints_Offset;

	// ============================================================
	// Valueable<ColorStruct> (3-4 bytes each)
	// ============================================================
	Valueable<ColorStruct> Sidebar_HarvesterCounter_Yellow;
	Valueable<ColorStruct> Sidebar_HarvesterCounter_Red;
	Valueable<ColorStruct> Sidebar_PowerDelta_Green;
	Valueable<ColorStruct> Sidebar_PowerDelta_Grey;
	Valueable<ColorStruct> Sidebar_PowerDelta_Yellow;
	Valueable<ColorStruct> Sidebar_PowerDelta_Red;
	Valueable<ColorStruct> ToolTipTextColor;

	// ============================================================
	// Valueable<int> / ValueableIdx (4 bytes each)
	// ============================================================
	Valueable<int> ArrayIndex;
	Valueable<int> IngameScore_WinTheme;
	Valueable<int> IngameScore_LoseTheme;
	Valueable<int> BriefingTheme;
	Valueable<int> SidebarMixFileIndex;
	ValueableIdx<AircraftTypeClass> ParaDropPlane;
	ValueableIdx<ColorScheme> MessageTextColorIndex;
#ifndef disable_aresOverride
	ValueableIdx<EVAVoices> EVAIndex;
#else
	ValueableIdx<int> EVAIndex;
#endif

	// ============================================================
	// Valueable<enum> (4 bytes each)
	// ============================================================
	Valueable<TranslucencyLevel> GClock_Transculency;
	Valueable<TextAlign> Sidebar_PowerDelta_Align;
	Valueable<TextAlign> Sidebar_BattlePoints_Align;

	// ============================================================
	// Valueable<bool> (1 byte, at the end)
	// ============================================================
	Valueable<bool> SidebarYuriFileNames;
	// 1 byte + 3 bytes padding for alignment

#pragma endregion

public:
	SideExtData(SideClass* pObj) : AbstractTypeExtData(pObj)
		// PhobosFixedString
		, ScoreMultiplayBackground()
		, ScoreMultiplayPalette()
		, ScoreMultiplayBars()
		, ScoreCampaignBackground()
		, ScoreCampaignTransition()
		, ScoreCampaignAnimation()
		, ScoreCampaignPalette()
		, GraphicalTextImage()
		, GraphicalTextPalette()
		, DialogBackgroundImage()
		, DialogBackgroundPalette()
		, ScoreCampaignThemeUnderPar()
		, ScoreCampaignThemeOverPar()
		, ScoreMultiplayThemeWin()
		, ScoreMultiplayThemeLose()
		// PhobosPCXFile
		, SuperWeaponSidebar_OnPCX()
		, SuperWeaponSidebar_OffPCX()
		, SuperWeaponSidebar_TopPCX()
		, SuperWeaponSidebar_CenterPCX()
		, SuperWeaponSidebar_BottomPCX()
		// CustomPalette
		, GClock_Palette(CustomPalette::PaletteMode::Default)
		// Vectors
		, ParaDropTypes()
		, ParaDropNum()
		, BaseDefenses()
		, BaseDefenseCounts()
		// Valueable<pointer>
		, GClock_Shape()
		, SpyPlane(nullptr)
		, HunterSeeker(nullptr)
		, MouseShape(nullptr)
		// Nullable<pointer>
		, Crew()
		, Engineer()
		, Technician()
		, ParachuteAnim()
		, Disguise()
		// Nullable<int/float>
		, SurvivorDivisor()
		, ToolTip_Background_Opacity()
		, ToolTip_Background_BlurSize()
		// Nullable<ColorStruct>
		, Sidebar_WeedsCounter_Color()
		, ToolTip_Background_Color()
		, Sidebar_BattlePoints_Color()
		// Nullable<bool>
		, Sidebar_GDIPositions()
		// Valueable<Point2D>
		, Sidebar_HarvesterCounter_Offset({ 0, 0 })
		, Sidebar_ProducingProgress_Offset({ 0, 0 })
		, Sidebar_PowerDelta_Offset({ 0, 0 })
		, Sidebar_WeedsCounter_Offset({ 0, 0 })
		, Sidebar_BattlePoints_Offset(Point2D())
		// Valueable<ColorStruct>
		, Sidebar_HarvesterCounter_Yellow(Drawing::DefaultColors[(int)DefaultColorList::Yellow])
		, Sidebar_HarvesterCounter_Red(Drawing::DefaultColors[(int)DefaultColorList::Red])
		, Sidebar_PowerDelta_Green(Drawing::DefaultColors[(int)DefaultColorList::Green])
		, Sidebar_PowerDelta_Grey(Drawing::DefaultColors[(int)DefaultColorList::Grey])
		, Sidebar_PowerDelta_Yellow(Drawing::DefaultColors[(int)DefaultColorList::Yellow])
		, Sidebar_PowerDelta_Red(Drawing::DefaultColors[(int)DefaultColorList::Red])
		, ToolTipTextColor(ColorStruct())
		// Valueable<int> / ValueableIdx
		, ArrayIndex(-1)
		, IngameScore_WinTheme(-2)
		, IngameScore_LoseTheme(-2)
		, BriefingTheme(-1)
		, SidebarMixFileIndex(-1)
		, ParaDropPlane(-1)
		, MessageTextColorIndex(-1)
		, EVAIndex(-1)
		// Valueable<enum>
		, GClock_Transculency()
		, Sidebar_PowerDelta_Align(TextAlign::Left)
		, Sidebar_BattlePoints_Align(TextAlign::Left)
		// Valueable<bool>
		, SidebarYuriFileNames(false)
	{
		this->AbsType = SideClass::AbsID;
		this->Initialize();
	}

	SideExtData(SideClass* pObj, noinit_t nn) : AbstractTypeExtData(pObj, nn) { }

	virtual ~SideExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override
	{
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->AbstractTypeExtData::Internal_LoadFromStream(Stm);
		this->Serialize(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		const_cast<SideExtData*>(this)->AbstractTypeExtData::Internal_SaveToStream(Stm);
		const_cast<SideExtData*>(this)->Serialize(Stm);
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const { };

	SideClass* This() const { return reinterpret_cast<SideClass*>(this->AttachedToObject); }
	const SideClass* This_Const() const { return reinterpret_cast<const SideClass*>(this->AttachedToObject); }

	virtual bool LoadFromINI(CCINIClass* pINI, bool parseFailAddr);
	virtual bool WriteToINI(CCINIClass* pINI) const { return true; }

	void Initialize();

public:

	int GetSurvivorDivisor() const;
	int GetDefaultSurvivorDivisor() const;

	InfantryTypeClass* GetCrew() const;
	InfantryTypeClass* GetDefaultCrew() const;
	InfantryTypeClass* GetEngineer() const;
	InfantryTypeClass* GetTechnician() const;

	UnitTypeClass* GetHunterSeeker() const;

	Iterator<TechnoTypeClass*> GetParaDropTypes() const;
	Iterator<InfantryTypeClass*> GetDefaultParaDropTypes() const;

	Iterator<int> GetParaDropNum() const;
	Iterator<int> GetDefaultParaDropNum() const;

	Iterator<int> GetBaseDefenseCounts() const;
	Iterator<int> GetDefaultBaseDefenseCounts() const;

	Iterator<BuildingTypeClass*> GetBaseDefenses() const;
	Iterator<BuildingTypeClass*> GetDefaultBaseDefenses() const;

	InfantryTypeClass* GetDisguise() const;
	InfantryTypeClass* GetDefaultDisguise() const;

	const char* GetMultiplayerScoreBarFilename(unsigned int index) const;
public:

	static bool isNODSidebar();
	static void UpdateGlobalFiles();

	static int CurrentLoadTextColor;

	static SHPStruct* s_GraphicalTextImage;
	static CustomPalette s_GraphicalTextConvert;

	static SHPStruct* s_DialogBackgroundImage;
	static CustomPalette s_DialogBackgroundConvert;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static COMPILETIMEEVAL SHPStruct* GetGraphicalTextImage() {
		return SideExtData::s_GraphicalTextImage ?
		 SideExtData::s_GraphicalTextImage : FileSystem::GRFXTXT_SHP();
	}

	static COMPILETIMEEVAL ConvertClass* GetGraphicalTextConvert() {
		return SideExtData::s_GraphicalTextConvert.GetConvert() ?
		 SideExtData::s_GraphicalTextConvert.GetConvert() : FileSystem::GRFXTXT_Convert();
	}

private:
	template <typename T>
	void Serialize(T& Stm);
};

class SideExtContainer final : public Container<SideExtData>
	, public ReadWriteContainerInterfaces<SideExtData>
{
public:
	static COMPILETIMEEVAL const char* ClassName = "SideExtContainer";
	using ext_t = SideExtData;

public:
	static SideExtContainer Instance;

	virtual bool LoadAll(const json& root);
	virtual bool SaveAll(json& root);

	virtual void LoadFromINI(ext_t::base_type* key, CCINIClass* pINI, bool parseFailAddr);
	virtual void WriteToINI(ext_t::base_type* key, CCINIClass* pINI);
};

class FakeSideClass final : public SideClass
{
public:
	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, BOOL clearDirty);

};

static_assert(sizeof(FakeSideClass) == sizeof(SideClass), "Invalid Size !");
