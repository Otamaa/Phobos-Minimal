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
	static constexpr unsigned Marker = UuidFirstPart<base_type>::value;

public:
#pragma region ClassMembers
	Valueable<int> ArrayIndex;
	Nullable<bool> Sidebar_GDIPositions;
	Valueable<int> IngameScore_WinTheme;
	Valueable<int> IngameScore_LoseTheme;
	Valueable<Point2D> Sidebar_HarvesterCounter_Offset;
	Valueable<ColorStruct> Sidebar_HarvesterCounter_Yellow;
	Valueable<ColorStruct> Sidebar_HarvesterCounter_Red;
	Valueable<Point2D> Sidebar_ProducingProgress_Offset;
	Valueable<Point2D> Sidebar_PowerDelta_Offset;
	Valueable<ColorStruct> Sidebar_PowerDelta_Green;
	Valueable<ColorStruct> Sidebar_PowerDelta_Grey;
	Valueable<ColorStruct> Sidebar_PowerDelta_Yellow;
	Valueable<ColorStruct> Sidebar_PowerDelta_Red;
	Valueable<TextAlign> Sidebar_PowerDelta_Align;
	Valueable<Point2D> Sidebar_WeedsCounter_Offset;
	Nullable<ColorStruct> Sidebar_WeedsCounter_Color;
	Nullable<ColorStruct> ToolTip_Background_Color;
	Nullable<int> ToolTip_Background_Opacity;
	Nullable<float> ToolTip_Background_BlurSize;
	Nullable<SHPStruct*> GClock_Shape;
	Nullable<int> GClock_Transculency;
	//CustomPalette GClock_Palette;
	Nullable<int> SurvivorDivisor;
	Nullable<InfantryTypeClass*> Crew;
	Nullable<InfantryTypeClass*> Engineer;
	Nullable<InfantryTypeClass*> Technician;
	ValueableIdx<AircraftTypeClass> ParaDropPlane;
	Valueable<AircraftTypeClass*> SpyPlane;
	Valueable<UnitTypeClass*> HunterSeeker;
	NullableVector<TechnoTypeClass*> ParaDropTypes;
	NullableVector<int> ParaDropNum;
	ValueableIdx<ColorScheme> MessageTextColorIndex;
	Nullable<AnimTypeClass*> ParachuteAnim;
#ifndef disable_aresOverride
	ValueableIdx<EVAVoices> EVAIndex;
#else
	ValueableIdx<int> EVAIndex;
#endif
	Valueable<int> BriefingTheme;
	NullableVector<BuildingTypeClass*> BaseDefenses;
	NullableVector<int> BaseDefenseCounts;
	Nullable<InfantryTypeClass*> Disguise;
	Valueable<bool> SidebarYuriFileNames;
	Valueable<ColorStruct> ToolTipTextColor;
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
	Valueable<int> SidebarMixFileIndex;
	Valueable<SHPStruct*> MouseShape;
	PhobosPCXFile SuperWeaponSidebar_OnPCX;
	PhobosPCXFile SuperWeaponSidebar_OffPCX;
	PhobosPCXFile SuperWeaponSidebar_TopPCX;
	PhobosPCXFile SuperWeaponSidebar_CenterPCX;
	PhobosPCXFile SuperWeaponSidebar_BottomPCX;
	Valueable<Point2D> Sidebar_BattlePoints_Offset;
	Nullable<ColorStruct> Sidebar_BattlePoints_Color;
	Valueable<TextAlign> Sidebar_BattlePoints_Align;
#pragma endregion

public:
	SideExtData(SideClass* pObj) : AbstractTypeExtData(pObj),
		ArrayIndex(-1),
		IngameScore_WinTheme(-2),
		IngameScore_LoseTheme(-2),
		Sidebar_HarvesterCounter_Offset({ 0, 0 }),
		Sidebar_HarvesterCounter_Yellow(Drawing::DefaultColors[(int)DefaultColorList::Yellow]),
		Sidebar_HarvesterCounter_Red(Drawing::DefaultColors[(int)DefaultColorList::Red]),
		Sidebar_ProducingProgress_Offset({ 0, 0 }),
		Sidebar_PowerDelta_Offset({ 0, 0 }),
		Sidebar_PowerDelta_Green(Drawing::DefaultColors[(int)DefaultColorList::Green]),
		Sidebar_PowerDelta_Grey(Drawing::DefaultColors[(int)DefaultColorList::Grey]),
		Sidebar_PowerDelta_Yellow(Drawing::DefaultColors[(int)DefaultColorList::Yellow]),
		Sidebar_PowerDelta_Red(Drawing::DefaultColors[(int)DefaultColorList::Red]),
		Sidebar_PowerDelta_Align(TextAlign::Left),
		Sidebar_WeedsCounter_Offset({ 0, 0 }),
		ParaDropPlane(-1),
		SpyPlane(nullptr),
		HunterSeeker(nullptr),
		MessageTextColorIndex(-1),
		EVAIndex(-1),
		BriefingTheme(-1),
		SidebarYuriFileNames(false),
		ToolTipTextColor(ColorStruct()),
		SidebarMixFileIndex(-1),
		MouseShape(nullptr),
		Sidebar_BattlePoints_Offset(Point2D()),
		Sidebar_BattlePoints_Align(TextAlign::Left)
	{
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
	virtual SideClass* This() const override { return reinterpret_cast<SideClass*>(this->AbstractTypeExtData::This()); }
	virtual const SideClass* This_Const() const override { return reinterpret_cast<const SideClass*>(this->AbstractTypeExtData::This_Const()); }

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
{
public:
	static SideExtContainer Instance;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static void InvalidatePointer(AbstractClass* const ptr, bool bRemoved)
	{
		for (auto& ext : Array)
		{
			ext->InvalidatePointer(ptr, bRemoved);
		}
	}

};

class FakeSideClass final : public SideClass
{
public:
	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, BOOL clearDirty);

};

static_assert(sizeof(FakeSideClass) == sizeof(SideClass), "Invalid Size !");
