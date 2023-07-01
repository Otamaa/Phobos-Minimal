#pragma once
#include <BuildingTypeClass.h>
#include <SuperClass.h>
#include <SuperWeaponTypeClass.h>


#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>
#include <DirStruct.h>

#include <Ext/TechnoType/Body.h>

#include <New/Type/TunnelTypeClass.h>
#include <New/Type/CursorTypeClass.h>

enum class BunkerSoundMode : int
{
	Up, Down
};

struct BuildSpeedBonus
{
	bool Enabled { false };
	double SpeedBonus_Aircraft { 0.000 };
	double SpeedBonus_Building { 0.000 };
	double SpeedBonus_Infantry { 0.000 };
	double SpeedBonus_Unit { 0.000 };
	ValueableVector<TechnoTypeClass*> AffectedType { };

	void Read(INI_EX& parser, const char* pSection)
	{
		Nullable<double> nBuff {};
		nBuff.Read(parser, pSection, "BuildSpeedBonus.Aircraft");

		if (nBuff.isset() && nBuff.Get() != 0.000)
		{
			Enabled = true;
			SpeedBonus_Aircraft = nBuff.Get();
		}

		nBuff.Reset();
		nBuff.Read(parser, pSection, "BuildSpeedBonus.Building");

		if (nBuff.isset() && nBuff.Get() != 0.000)
		{
			Enabled = true;
			SpeedBonus_Building = nBuff.Get();
		}

		nBuff.Reset();
		nBuff.Read(parser, pSection, "BuildSpeedBonus.Infantry");

		if (nBuff.isset() && nBuff.Get() != 0.000)
		{
			Enabled = true;
			SpeedBonus_Infantry = nBuff.Get();
		}

		nBuff.Reset();
		nBuff.Read(parser, pSection, "BuildSpeedBonus.Unit");

		if (nBuff.isset() && nBuff.Get() != 0.000)
		{
			Enabled = true;
			SpeedBonus_Unit = nBuff.Get();
		}

		if (Enabled)
			AffectedType.Read(parser, pSection, "BuildSpeedBonus.AffectedTypes");
	}

	bool Load(PhobosStreamReader& stm, bool registerForChange)
	{
		return Serialize(stm);
	}
	bool Save(PhobosStreamWriter& stm) const
	{
		return const_cast<BuildSpeedBonus*>(this)->Serialize(stm);
	}

private:
	template <typename T>
	bool Serialize(T& stm)
	{
		return stm
			.Process(Enabled)
			.Process(SpeedBonus_Aircraft)
			.Process(SpeedBonus_Building)
			.Process(SpeedBonus_Infantry)
			.Process(SpeedBonus_Unit)
			.Process(AffectedType)
			.Success();
	}
};

class BuildingTypeExt
{ 
public:

	static std::vector<std::string> trenchKinds; //!< Vector of strings associating known trench names with IsTrench IDs. \sa IsTrench
	static const DirStruct DefaultJuggerFacing;

	class ExtData final : public Extension<BuildingTypeClass>
	{
	public:
		static constexpr size_t Canary = 0x11111111;
		using base_type = BuildingTypeClass;
		static constexpr size_t ExtOffset = 0x1794;

	public:

		TechnoTypeExt::ExtData* Type { nullptr };

		Valueable<AffectedHouse> PowersUp_Owner { AffectedHouse::Owner };
		ValueableVector<BuildingTypeClass*> PowersUp_Buildings {};
		ValueableIdxVector<SuperWeaponTypeClass> SuperWeapons {};

		ValueableVector<BuildingTypeClass*> PowerPlantEnhancer_Buildings {};
		Nullable<int> PowerPlantEnhancer_Amount {};
		Nullable<float> PowerPlantEnhancer_Factor {};

		std::vector<Point2D> OccupierMuzzleFlashes {};

		Valueable<bool> Refinery_UseStorage { false };

		Nullable<bool> AllowAirstrike {};

		Valueable<bool> Grinding_AllowAllies { false };
		Valueable<bool> Grinding_AllowOwner { true };
		ValueableVector<TechnoTypeClass*> Grinding_AllowTypes {};
		ValueableVector<TechnoTypeClass*> Grinding_DisallowTypes {};
		NullableIdx<VocClass> Grinding_Sound {};
		Nullable<WeaponTypeClass*> Grinding_Weapon {};
		Valueable<bool> Grinding_DisplayRefund { false };
		Valueable<AffectedHouse> Grinding_DisplayRefund_Houses { AffectedHouse::All };
		Valueable<Point2D> Grinding_DisplayRefund_Offset { {0, 0} };
		Valueable<bool> Grinding_PlayDieSound { false };

		Valueable<bool> Refinery_DisplayDumpedMoneyAmount { false };
		Valueable<Point2D> Refinery_DisplayRefund_Offset { {0, 0} };

		Nullable<bool> PlacementPreview_Show {};
		Nullable<Theater_SHPStruct*> PlacementPreview_Shape {};
		Nullable<int> PlacementPreview_ShapeFrame {};
		Valueable<CoordStruct> PlacementPreview_Offset { {0, -15, 1} };
		Valueable<bool> PlacementPreview_Remap { true };
		Valueable<PaletteManager*> PlacementPreview_Palette {}; //CustomPalette::PaletteMode::Default
		Nullable<int> PlacementPreview_TranslucentLevel {};

		Nullable<AffectedHouse> RadialIndicator_Visibility {};

		Valueable<bool> SpyEffect_Custom { false };
		NullableIdx<SuperWeaponTypeClass> SpyEffect_VictimSuperWeapon {};
		NullableIdx<SuperWeaponTypeClass> SpyEffect_InfiltratorSuperWeapon {};

		//#814
		Valueable<bool> SpyEffect_InfiltratorSW_JustGrant { false };
		Valueable<bool> SpyEffect_VictimSW_RealLaunch { false };

		Valueable<bool> SpyEffect_RevealProduction { false };
		Valueable<bool> SpyEffect_ResetSW { false };
		Valueable<bool> SpyEffect_ResetRadar { false };
		Valueable<bool> SpyEffect_RevealRadar { false };
		Valueable<bool> SpyEffect_RevealRadarPersist { false };
		Valueable<bool> SpyEffect_GainVeterancy { false };
		Valueable<bool> SpyEffect_UnReverseEngineer { false };
		Valueable<int> SpyEffect_StolenTechIndex { -1 };
		Valueable<int> SpyEffect_StolenMoneyAmount { 0 };
		Valueable<int> SpyEffect_StolenMoneyPercentage { 0 };
		Valueable<int> SpyEffect_PowerOutageDuration { 0 };
		Valueable<int> SpyEffect_SabotageDelay { 0 };
		Valueable<SuperWeaponTypeClass*> SpyEffect_SuperWeapon { nullptr };
		Valueable<bool> SpyEffect_SuperWeaponPermanent { false };

		Valueable<bool> SpyEffect_InfantryVeterancy { false };
		Valueable<bool> SpyEffect_VehicleVeterancy { false };
		Valueable<bool> SpyEffect_NavalVeterancy { false };
		Valueable<bool> SpyEffect_AircraftVeterancy { false };
		Valueable<bool> SpyEffect_BuildingVeterancy { false };

		Valueable<bool> ZShapePointMove_OnBuildup { false };
		Valueable<int> SellBuildupLength { 23 };

		Valueable<bool> CanC4_AllowZeroDamage { false };
		Valueable<double> C4_Modifier { 1.0 };

		Valueable<CellStruct> DockUnload_Cell { { 3, 1 } };
		Nullable<DirType32> DockUnload_Facing { };

		// solid
		Valueable<int> Solid_Height { 0 };
		Valueable<int> Solid_Level { 1 };

		Nullable<bool> AIBaseNormal { };
		Nullable<bool> AIInnerBase { };
		Valueable<bool> Cloning_Facility { false };

		// gates
		NullableIdx<VocClass> GateDownSound {};
		NullableIdx<VocClass> GateUpSound {};
		Nullable<bool> UnitSell {};

		NullableVector<AnimTypeClass*> DamageFireTypes {};
		NullableVector<AnimTypeClass*> OnFireTypes {};
		NullableVector<int> OnFireIndex {};
		HealthOnFireData HealthOnfire {};

		Valueable<BuildingTypeClass*> RubbleIntact { nullptr }; 		//!< What BuildingType to turn into when reconstructed. (This is the normal building, set on rubble.)
		Valueable<BuildingTypeClass*> RubbleDestroyed { nullptr };	//!< What BuildingType to turn into when destroyed. (This is the rubble, set on normal buildings.)
		Valueable<AnimTypeClass*> RubbleDestroyedAnim { nullptr };
		Valueable<AnimTypeClass*> RubbleIntactAnim { nullptr };
		Valueable<OwnerHouseKind> RubbleDestroyedOwner { OwnerHouseKind::Default };
		Valueable<OwnerHouseKind> RubbleIntactOwner { OwnerHouseKind::Default };
		Valueable<int> RubbleDestroyedStrength { 0 };
		Valueable<int> RubbleIntactStrength { -1 };
		Valueable<bool> RubbleDestroyedRemove { false };
		Valueable<bool> RubbleIntactRemove { false };

		ValueableVector<Point2D> DamageFire_Offs {};

		Nullable<double> RepairRate {};
		Nullable<int> RepairStep {};

		Nullable<bool> PlayerReturnFire {};

		Valueable<bool> PackupSound_PlayGlobal { false };
		Valueable<bool> DisableDamageSound { false };

		Nullable<float> BuildingOccupyDamageMult {};
		Nullable<float> BuildingOccupyROFMult {};

		Nullable<float> BuildingBunkerDamageMult {};
		Nullable<float> BuildingBunkerROFMult {};


		NullableIdx<VocClass> BunkerWallsUpSound {};
		NullableIdx<VocClass> BunkerWallsDownSound {};

		ValueableIdxVector<BuildingTypeClass> AIBuildInsteadPerDiff {};

		PhobosMap<int, AnimTypeClass*> GarrisonAnim_idle {};
		PhobosMap<int, AnimTypeClass*> GarrisonAnim_ActiveOne {};
		PhobosMap<int, AnimTypeClass*> GarrisonAnim_ActiveTwo {};
		PhobosMap<int, AnimTypeClass*> GarrisonAnim_ActiveThree {};
		PhobosMap<int, AnimTypeClass*> GarrisonAnim_ActiveFour {};

		Valueable<PaletteManager*> PipShapes01Palette {}; //CustomPalette::PaletteMode::Temperate
		Valueable<bool> PipShapes01Remap { false };

		Nullable<AnimTypeClass*> TurretAnim_LowPower {};
		Nullable<AnimTypeClass*> TurretAnim_DamagedLowPower {};

		Valueable<bool> BuildUp_UseNormalLIght {};

		Valueable<bool> Power_DegradeWithHealth { true };

		Nullable<float> AutoSellTime {};

		Valueable<bool> IsJuggernaut { false };

		Nullable<SHPStruct*> BuildingPlacementGrid_Shape {};
		BuildSpeedBonus SpeedBonus {};

		Valueable<PaletteManager*> RubblePalette {}; //CustomPalette::PaletteMode::Temperates

		NullableIdx<VocClass> EnterBioReactorSound {};
		NullableIdx<VocClass> LeaveBioReactorSound {};
		std::vector<int> DockPoseDir {};

		Nullable<bool> EngineerRepairable {};

		signed int IsTrench { -1 };					//!< Enables moving between segments - saves ID of a kind of trench. \sa trenchKinds

		ValueableIdx<TunnelTypeClass> TunnelType { -1 };

		Valueable<double> UCPassThrough { 0.0 }; 					//!< How many percent of the shots pass through the building to the occupants? 0.0 = 0%, 1.0 = 100%; Defaults to 0.0.
		Valueable<double> UCFatalRate { 0.0 }; 					//!< Chance of an occupant getting killed instantly when a bullet passes through. 0.0 = 0%, 1.0 = 100%; Defaults to 0.0.
		Valueable<double> UCDamageMultiplier { 1.0 }; 				//!< How many percent of normal damage are applied if an occupant is hit when a bullet passes through. 0.0 = 0%, 1.0 = 100%; Defaults to 1.0.

		ValueableIdx<CursorTypeClass> Cursor_Spy { (int)MouseCursorType::Enter };
		ValueableIdx<CursorTypeClass> Cursor_Sabotage { 93 };
		Nullable<bool> ImmuneToSaboteurs {};
		Valueable<bool> ReverseEngineersVictims { false };

		// lightning rod
		Valueable<double> LightningRod_Modifier { 1.0 };
		Nullable<bool> Returnable {};
		Nullable<double> BuildupTime {};
		NullableIdx<VocClass> SlamSound {};

		Valueable<bool> Destroyed_CreateSmudge { true } ;

		Valueable<BuildingTypeClass*> LaserFenceType { nullptr };
		Nullable<BuildingTypeClass*> LaserFenceWEType { };
		ValueableVector<BuildingTypeClass*> LaserFencePostLinks { };
		Valueable<short> LaserFenceDirection {};

		ExtData(BuildingTypeClass* OwnerObject) : Extension<BuildingTypeClass>(OwnerObject) { }
		virtual ~ExtData() override = default;

		void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);
		void Initialize();
		void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
		void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

		void CompleteInitialization();
		int GetSuperWeaponCount() const;
		int GetSuperWeaponIndex(int index, HouseClass* pHouse) const;
		int GetSuperWeaponIndex(int index) const;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<BuildingTypeExt::ExtData>
	{
	public:
		ExtContainer();
		~ExtContainer();

		virtual bool Load(BuildingTypeClass* pThis, IStream* pStm) override;

		static bool LoadGlobals(PhobosStreamReader& Stm)
		{
			return Stm
				.Process(BuildingTypeExt::trenchKinds)
				.Success()
				;
		}

		static bool SaveGlobals(PhobosStreamWriter& Stm)
		{
			return Stm
				.Process(BuildingTypeExt::trenchKinds)
				.Success()
				;
		}
	};

	static ExtContainer ExtMap;

	// Short check: Is the building of a linkable kind at all?
	static bool IsLinkable(BuildingTypeClass* pThis);
	static int GetEnhancedPower(BuildingClass* pBuilding, HouseClass* pHouse);
	static double GetExternalFactorySpeedBonus(TechnoClass* pWhat);
	static double GetExternalFactorySpeedBonus(TechnoClass* pWhat, HouseClass* pOwner);
	static double GetExternalFactorySpeedBonus(TechnoTypeClass* pWhat, HouseClass* pOwner);
	static bool CanUpgrade(BuildingClass* pBuilding, BuildingTypeClass* pUpgradeType, HouseClass* pUpgradeOwner);
	static int GetUpgradesAmount(BuildingTypeClass* pBuilding, HouseClass* pHouse);

	template<BunkerSoundMode UpSound>
	struct BunkerSound
	{
		constexpr void operator ()(BuildingClass* pThis)
		{

			if constexpr (UpSound == BunkerSoundMode::Up)
			{
				const auto nSound = BuildingTypeExt::ExtMap.Find(pThis->Type)->BunkerWallsUpSound.Get(RulesClass::Instance->BunkerWallsUpSound);
				VocClass::PlayIndexAtPos(nSound, pThis->Location);
			}
			else
			{
				const auto nSound = BuildingTypeExt::ExtMap.Find(pThis->Type)->BunkerWallsDownSound.Get(RulesClass::Instance->BunkerWallsDownSound);
				VocClass::PlayIndexAtPos(nSound, pThis->Location);
			}
		}
	};

	static void DisplayPlacementPreview();
	static Point2D* GetOccupyMuzzleFlash(BuildingClass* pThis, int nOccupyIdx);
	static int CheckBuildLimit(HouseClass const* pHouse, BuildingTypeClass const* pItem, bool includeQueued);
	static int BuildLimitRemaining(HouseClass const* pHouse, BuildingTypeClass const* pItem);
	static int GetBuildingAnimTypeIndex(BuildingClass* pThis, const BuildingAnimSlot& nSlot, const char* pDefault);

	static bool __fastcall IsFactory(BuildingClass* pThis, void* _);
	static void __fastcall DrawPlacementGrid(Surface* Surface, ConvertClass* Pal, SHPStruct* SHP, int FrameIndex, const Point2D* const Position, const RectangleStruct* const Bounds, BlitterFlags Flags, int Remap, int ZAdjust, ZGradient ZGradientDescIndex, int Brightness, int TintColor, SHPStruct* ZShape, int ZShapeFrame, int XOffset, int YOffset);
};