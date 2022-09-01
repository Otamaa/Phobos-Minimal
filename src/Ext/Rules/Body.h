#pragma once

#include <CCINIClass.h>
#include <RulesClass.h>

#include <Utilities/Container.h>
#include <Utilities/Constructs.h>
#include <Utilities/Template.h>

#include <Utilities/Debug.h>

#include <ScriptTypeClass.h>

#ifdef COMPILE_PORTED_DP_FEATURES
#include <Misc/DynamicPatcher/Techno/AircraftPut/AircraftPutDataRules.h>

#endif

class AnimTypeClass;
class MouseCursor;
class TechnoTypeClass;
class VocClass;
class WarheadTypeClass;

class RulesExt
{
public:
	static constexpr size_t Canary = 0x12341234;
	using base_type = RulesClass;

	class ExtData final : public Extension<RulesClass>
	{
	public:
		Valueable<Point3D> Pips_Shield;
		Valueable<Point3D> Pips_Shield_Buildings;
		Valueable<int> RadApplicationDelay_Building;
		PhobosFixedString<32u> MissingCameo;

		DynamicVectorClass<DynamicVectorClass<TechnoTypeClass*>> AITargetTypesLists;
		DynamicVectorClass<DynamicVectorClass<ScriptTypeClass*>> AIScriptsLists;
		DynamicVectorClass<DynamicVectorClass<std::string>> AIConditionsLists;

		Valueable<double> JumpjetCrash;
		Valueable<bool> JumpjetNoWobbles;
		Valueable<bool> JumpjetAllowLayerDeviation;
		Valueable<bool> JumpjetTurnToTarget;
		Valueable<int> Storage_TiberiumIndex;
		Valueable<int> PlacementGrid_TranslucentLevel;
		Valueable<int> BuildingPlacementPreview_TranslucentLevel;

		Nullable<SHPStruct*> Pips_Shield_Background_SHP;
		Valueable<Point3D> Pips_Shield_Building;
		Nullable<int> Pips_Shield_Building_Empty;

		Valueable<Point2D> Pips_SelfHeal_Infantry;
		Valueable<Point2D> Pips_SelfHeal_Units;
		Valueable<Point2D> Pips_SelfHeal_Buildings;
		Valueable<Point2D> Pips_SelfHeal_Infantry_Offset;
		Valueable<Point2D> Pips_SelfHeal_Units_Offset;
		Valueable<Point2D> Pips_SelfHeal_Buildings_Offset;

		Nullable<int> InfantryGainSelfHealCap;
		Nullable<int> UnitsGainSelfHealCap;
		Valueable<bool> EnemyInsignia;
		Valueable<bool> ShowAllyDisguiseBlinking;

		Valueable<SHPStruct*> SHP_SelectBrdSHP_INF;
		CustomPalette SHP_SelectBrdPAL_INF;
		Valueable<SHPStruct*> SHP_SelectBrdSHP_UNIT;
		CustomPalette SHP_SelectBrdPAL_UNIT;

		Nullable<bool> UseSelectBrd;
		Valueable<Point3D> SelectBrd_Frame_Infantry;
		Valueable<Point2D> SelectBrd_DrawOffset_Infantry;
		Valueable<Point3D> SelectBrd_Frame_Unit;
		Valueable<Point2D> SelectBrd_DrawOffset_Unit;
		Valueable<int> SelectBrd_DefaultTranslucentLevel;
		Valueable<bool> SelectBrd_DefaultShowEnemy;

		Valueable<bool> AIRepairBaseNodes;

		Valueable<bool> RadWarhead_Detonate;
		Valueable<bool> RadHasOwner;
		Valueable<bool> RadHasInvoker;
		Valueable<bool> IronCurtain_SyncDeploysInto;

		Valueable<ColorStruct> ToolTip_Background_Color;
		Valueable<int> ToolTip_Background_Opacity;
		Valueable<float> ToolTip_Background_BlurSize;

	#pragma region Otamaa
		NullableIdx<ParticleTypeClass> VeinholeParticle;
		PhobosFixedString<0x19> NukeWarheadName;
		Nullable<bool> Building_PlacementPreview;
		NullableVector<float> AI_AutoSellHealthRatio;

		Valueable<AnimTypeClass*> CarryAll_LandAnim;
		Valueable<AnimTypeClass*> DropShip_LandAnim;
		Valueable<AnimTypeClass*> Aircraft_LandAnim;
		Valueable<AnimTypeClass*> Aircraft_TakeOffAnim;

		Valueable<bool> DisablePathfindFailureLog;
		Valueable<bool> CreateSound_PlayerOnly;

		int CivilianSideIndex;
		int SpecialCountryIndex;
		int NeutralCountryIndex;

		ValueableVector<BuildingTypeClass*> WallTowers;
#ifdef COMPILE_PORTED_DP_FEATURES
		AircraftPutDataRules MyPutData;
#endif
	#pragma endregion

		ExtData(RulesClass* OwnerObject) : Extension<RulesClass>(OwnerObject)
			, Pips_Shield { { -1,-1,-1 } }
			, Pips_Shield_Buildings { { -1,-1,-1 } }
			, RadApplicationDelay_Building { 0 }
			, MissingCameo { "xxicon.shp" }
			, AITargetTypesLists { }
			, AIScriptsLists { }
			, AIConditionsLists { }
			, JumpjetCrash { 5.0 }
			, JumpjetNoWobbles { false }
			, JumpjetAllowLayerDeviation { true }
			, JumpjetTurnToTarget { false }
			, Storage_TiberiumIndex { -1 }
			, PlacementGrid_TranslucentLevel{ 0 }
			, BuildingPlacementPreview_TranslucentLevel { 3 }
			, Pips_Shield_Background_SHP {}
			, Pips_Shield_Building { { -1,-1,-1 } }
			, Pips_Shield_Building_Empty {}

			, Pips_SelfHeal_Infantry { { 13, 20 } }
			, Pips_SelfHeal_Units { { 13, 20 } }
			, Pips_SelfHeal_Buildings { { 13, 20 } }
			, Pips_SelfHeal_Infantry_Offset { { 25, -35 } }
			, Pips_SelfHeal_Units_Offset { { 33, -32 } }
			, Pips_SelfHeal_Buildings_Offset { { 15, 10 } }

			, InfantryGainSelfHealCap {}
			, UnitsGainSelfHealCap {}
			, EnemyInsignia{ true }
			, ShowAllyDisguiseBlinking{ false }

			, SHP_SelectBrdSHP_INF { nullptr }
			, SHP_SelectBrdPAL_INF { CustomPalette::PaletteMode::Temperate }
			, SHP_SelectBrdSHP_UNIT { nullptr }
			, SHP_SelectBrdPAL_UNIT { CustomPalette::PaletteMode::Temperate }
			, UseSelectBrd{}

			, SelectBrd_Frame_Infantry { {0,0,0} }
			, SelectBrd_DrawOffset_Infantry { {0,0} }

			, SelectBrd_Frame_Unit { {3,3,3} }
			, SelectBrd_DrawOffset_Unit{ {0,0} }

			, SelectBrd_DefaultTranslucentLevel{ 0 }
			, SelectBrd_DefaultShowEnemy{ true }

			, AIRepairBaseNodes { false }

			, RadWarhead_Detonate { false }
			, RadHasOwner { false }
			, RadHasInvoker { false }
			, IronCurtain_SyncDeploysInto { false }

			, ToolTip_Background_Color { {0, 0, 0} }
			, ToolTip_Background_Opacity { 100 }
			, ToolTip_Background_BlurSize { 0.f }

			, VeinholeParticle { }
			, NukeWarheadName { }
			, Building_PlacementPreview { }
			, AI_AutoSellHealthRatio { }
			, CarryAll_LandAnim { nullptr }
			, DropShip_LandAnim { nullptr }
			, Aircraft_LandAnim { nullptr }
			, Aircraft_TakeOffAnim { nullptr }
			, DisablePathfindFailureLog { false }
			, CreateSound_PlayerOnly { false }

			, CivilianSideIndex { -1 }
			, SpecialCountryIndex { -1 }
			, NeutralCountryIndex { -1 }

			, WallTowers { }
#ifdef COMPILE_PORTED_DP_FEATURES
			, MyPutData { }
#endif
		{ }

		virtual ~ExtData() = default;
		void Uninitialize() { }
		void LoadFromINIFile(CCINIClass* pINI);
		virtual void LoadBeforeTypeData(RulesClass* pThis, CCINIClass* pINI);
		virtual void LoadAfterTypeData(RulesClass* pThis, CCINIClass* pINI);
		void InitializeConstants();
		void InitializeAfterTypeData(RulesClass* pThis);

		void InvalidatePointer(void* ptr, bool bRemoved) { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

private:
	static std::unique_ptr<ExtData> Data;

public:
	static IStream* g_pStm;

	static void Allocate(RulesClass* pThis);
	static void Remove(RulesClass* pThis);

	static void LoadFromINIFile(RulesClass* pThis, CCINIClass* pINI);
	static void LoadBeforeTypeData(RulesClass* pThis, CCINIClass* pINI);
	static void LoadAfterTypeData(RulesClass* pThis, CCINIClass* pINI);
	static void LoadBeforeGeneralData(RulesClass* pThis, CCINIClass* pINI);
	static void LoadAfterAllLogicData(RulesClass* pThis, CCINIClass* pINI);
	static void LoadEarlyBeforeColor(RulesClass* pThis, CCINIClass* pINI);

	static ExtData* Global()
	{
		return Data.get();
	}

	static void Clear()
	{
		Allocate(RulesClass::Instance);
	}

	static void PointerGotInvalid(void* ptr, bool removed)
	{
		Global()->InvalidatePointer(ptr, removed);
	}

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static bool DetailsCurrentlyEnabled();
	static bool DetailsCurrentlyEnabled(int minDetailLevel);
};
