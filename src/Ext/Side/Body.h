#pragma once
#include <SideClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/PhobosPCXFile.h>
#include <Utilities/PhobosFixedString.h>
#include <Utilities/TemplateDefB.h>

#include <Misc/Ares/EVAVoices.h>

class SideExtData final
{
public:
	static constexpr size_t Canary = 0x05B10501;
	using base_type = SideClass;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
public:

	Valueable<int> ArrayIndex { -1 };
	Nullable<bool> Sidebar_GDIPositions { };
	Valueable<int> IngameScore_WinTheme { -2 };
	Valueable<int> IngameScore_LoseTheme { -2 };
	Valueable<Point2D> Sidebar_HarvesterCounter_Offset { { 0, 0 } };
	Valueable<ColorStruct> Sidebar_HarvesterCounter_Yellow { Drawing::DefaultColors[(int)DefaultColorList::Yellow] };
	Valueable<ColorStruct> Sidebar_HarvesterCounter_Red { Drawing::DefaultColors[(int)DefaultColorList::Red] };
	Valueable<Point2D> Sidebar_ProducingProgress_Offset { { 0, 0 } };
	Valueable<Point2D> Sidebar_PowerDelta_Offset { { 0, 0 } };
	Valueable<ColorStruct> Sidebar_PowerDelta_Green { Drawing::DefaultColors[(int)DefaultColorList::Green] };
	Valueable<ColorStruct> Sidebar_PowerDelta_Grey { Drawing::DefaultColors[(int)DefaultColorList::Grey] };
	Valueable<ColorStruct> Sidebar_PowerDelta_Yellow { Drawing::DefaultColors[(int)DefaultColorList::Yellow] };
	Valueable<ColorStruct> Sidebar_PowerDelta_Red { Drawing::DefaultColors[(int)DefaultColorList::Red] };
	Valueable<TextAlign> Sidebar_PowerDelta_Align { TextAlign::Left };

	Valueable<Point2D> Sidebar_WeedsCounter_Offset { { 0, 0 } };
	Nullable<ColorStruct> Sidebar_WeedsCounter_Color {};

	Nullable<ColorStruct> ToolTip_Background_Color { };
	Nullable<int> ToolTip_Background_Opacity { };
	Nullable<float> ToolTip_Background_BlurSize { };

	Nullable<SHPStruct*> GClock_Shape { };
	Nullable<int> GClock_Transculency { };
	//CustomPalette GClock_Palette {};

	Nullable<int> SurvivorDivisor { };
	Nullable<InfantryTypeClass*> Crew { };
	Nullable<InfantryTypeClass*> Engineer { };
	Nullable<InfantryTypeClass*> Technician { };
	ValueableIdx<AircraftTypeClass> ParaDropPlane { -1 };
	Valueable<AircraftTypeClass*> SpyPlane { };
	Valueable<UnitTypeClass*> HunterSeeker;

	NullableVector<TechnoTypeClass*> ParaDropTypes { };
	NullableVector<int> ParaDropNum { };

	ValueableIdx<ColorScheme> MessageTextColorIndex { -1 };

	Nullable<AnimTypeClass*> ParachuteAnim {};

#ifndef disable_aresOverride
	ValueableIdx<EVAVoices> EVAIndex { -1 };
#else
	ValueableIdx<int> EVAIndex { -1 };
#endif
	Valueable<int> BriefingTheme { -1 };

	NullableVector<BuildingTypeClass*> BaseDefenses {};
	NullableVector<int> BaseDefenseCounts {};

	Nullable<InfantryTypeClass*> Disguise {};

	Valueable<bool> SidebarYuriFileNames { false };
	Valueable<ColorStruct> ToolTipTextColor {};

	PhobosFixedString<0x20> ScoreMultiplayBackground {};
	PhobosFixedString<0x20> ScoreMultiplayPalette {};
	PhobosFixedString<0x20> ScoreMultiplayBars {};

	PhobosFixedString<0x20> ScoreCampaignBackground {};
	PhobosFixedString<0x20> ScoreCampaignTransition {};
	PhobosFixedString<0x20> ScoreCampaignAnimation {};
	PhobosFixedString<0x20> ScoreCampaignPalette {};

	PhobosFixedString<0x20> GraphicalTextImage {};
	PhobosFixedString<0x20> GraphicalTextPalette {};

	PhobosFixedString<0x20> DialogBackgroundImage {};
	PhobosFixedString<0x20> DialogBackgroundPalette {};

	PhobosFixedString<0x20> ScoreCampaignThemeUnderPar {};
	PhobosFixedString<0x20> ScoreCampaignThemeOverPar {};

	PhobosFixedString<0x20> ScoreMultiplayThemeWin {};
	PhobosFixedString<0x20> ScoreMultiplayThemeLose {};

	Valueable<int> SidebarMixFileIndex { -1 };

	Valueable<SHPStruct*> MouseShape { nullptr };

	PhobosPCXFile ExclusiveSWSidebar_TopPCX { };
	PhobosPCXFile ExclusiveSWSidebar_CenterPCX { };
	PhobosPCXFile ExclusiveSWSidebar_BottomPCX { };
	Valueable<SHPStruct*> ExclusiveSWSidebar_ToggleShape { };

	void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);
	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }
	void Initialize();

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

	static bool isNODSidebar();
	static void UpdateGlobalFiles();

	static int CurrentLoadTextColor;

	static UniqueGamePtrB<SHPStruct> s_GraphicalTextImage;
	static ConvertClass* s_GraphicalTextConvert;

	static UniqueGamePtrB<SHPStruct> s_DialogBackgroundImage;
	static ConvertClass* s_DialogBackgroundConvert;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static constexpr SHPStruct* GetGraphicalTextImage() {
		return SideExtData::s_GraphicalTextImage ?
		 SideExtData::s_GraphicalTextImage.get() : FileSystem::GRFXTXT_SHP;
	}

	static constexpr ConvertClass* GetGraphicalTextConvert() {
		return SideExtData::s_GraphicalTextConvert ?
		 SideExtData::s_GraphicalTextConvert : FileSystem::GRFXTXT_Convert;
	}

	constexpr FORCEINLINE static size_t size_Of()
	{
		return sizeof(SideExtData) -
			(4u //AttachedToObject
			 );
	}

	const char* GetMultiplayerScoreBarFilename(unsigned int index) const;
private:
	template <typename T>
	void Serialize(T& Stm);
};

class SideExtContainer final : public Container<SideExtData>
{
public:
	static SideExtContainer Instance;
	//CONSTEXPR_NOCOPY_CLASSB(SideExtContainer, SideExtData, "SideClass");
};