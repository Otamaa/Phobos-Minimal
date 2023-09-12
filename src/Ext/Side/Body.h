#pragma once
#include <SideClass.h>

#include <Helpers/Macro.h>
#include <Ext/Abstract/Body.h>
#include <Utilities/TemplateDefB.h>

#ifndef disable_aresOverride
#include <Misc/Ares/EVAVoices.h>
#endif
class SideExt
{
public:
	class ExtData final : public Extension<SideClass>
	{
	public:
		static constexpr size_t Canary = 0x05B10501;
		using base_type = SideClass;

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
		Nullable<AircraftTypeClass*> SpyPlane { };
		Valueable<UnitTypeClass*> HunterSeeker;

		NullableVector<TechnoTypeClass*> ParaDropTypes{ };
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

		ExtData(SideClass* OwnerObject) : Extension<SideClass>(OwnerObject)
		{ }

		virtual ~ExtData() override = default;

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


	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<SideExt::ExtData>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
	static bool isNODSidebar();
};
