#pragma once
#include <SideClass.h>

#include <Helpers/Macro.h>
#include <Ext/Abstract/Body.h>
#include <Utilities/TemplateDefB.h>

class SideExt
{
public:
	class ExtData final : public Extension<SideClass>
	{
	public:
		static constexpr size_t Canary = 0x05B10501;
		using base_type = SideClass;

	public:
		Valueable<int> ArrayIndex;
		Valueable<bool> Sidebar_GDIPositions;
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

		Nullable<ColorStruct> ToolTip_Background_Color;
		Nullable<int> ToolTip_Background_Opacity;
		Nullable<float> ToolTip_Background_BlurSize;

		Nullable<SHPStruct*> GClock_Shape;
		Nullable<int> GClock_Transculency;
		//CustomPalette GClock_Palette {};

		Nullable<int> SurvivorDivisor;
		Nullable<InfantryTypeClass*> Crew;
		Nullable<InfantryTypeClass*> Engineer;
		Nullable<InfantryTypeClass*> Technician;
		ValueableIdx<AircraftTypeClass> ParaDropPlane;
		Nullable<AircraftTypeClass*> SpyPlane;
		Valueable<UnitTypeClass*> HunterSeeker;

		NullableVector<TechnoTypeClass*> ParaDropTypes{ };
		NullableVector<int> ParaDropNum { };

		ExtData(SideClass* OwnerObject) : Extension<SideClass>(OwnerObject)
			, ArrayIndex { -1 }
			, Sidebar_GDIPositions { false }
			, IngameScore_WinTheme { -2 }
			, IngameScore_LoseTheme { -2 }
			, Sidebar_HarvesterCounter_Offset { { 0,0 } }
			, Sidebar_HarvesterCounter_Yellow { Drawing::DefaultColors[(int)DefaultColorList::Yellow] }
			, Sidebar_HarvesterCounter_Red { Drawing::DefaultColors[(int)DefaultColorList::Red] }
			, Sidebar_ProducingProgress_Offset { { 0,0 } }
			, Sidebar_PowerDelta_Offset { { 0,0 } }
			, Sidebar_PowerDelta_Green { Drawing::DefaultColors[(int)DefaultColorList::Green] }
			, Sidebar_PowerDelta_Grey { Drawing::DefaultColors[(int)DefaultColorList::Grey] }
			, Sidebar_PowerDelta_Yellow { Drawing::DefaultColors[(int)DefaultColorList::Yellow] }
			, Sidebar_PowerDelta_Red { Drawing::DefaultColors[(int)DefaultColorList::Red] }
			, Sidebar_PowerDelta_Align { TextAlign::Left }

			, ToolTip_Background_Color { }
			, ToolTip_Background_Opacity { }
			, ToolTip_Background_BlurSize { }

			, GClock_Shape { }
			, GClock_Transculency { }
			//, GClock_Palette { }

			, SurvivorDivisor { }
			, Crew { }
			, Engineer { }
			, Technician{ }
			, ParaDropPlane { -1 }

			, HunterSeeker { }
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
