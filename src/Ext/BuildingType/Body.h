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
	bool Enabled;
	double SpeedBonus_Aircraft;
	double SpeedBonus_Building;
	double SpeedBonus_Infantry;
	double SpeedBonus_Unit;
	ValueableVector<TechnoTypeClass*> AffectedType;

	BuildSpeedBonus() : Enabled { false }
		, SpeedBonus_Aircraft { 0.000 }
		, SpeedBonus_Building { 0.000 }
		, SpeedBonus_Infantry { 0.000 }
		, SpeedBonus_Unit { 0.000 }
		, AffectedType { }
	{
	}

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
		//Debug::Log("Processing Items From BuildSpeedBonus ! \n");

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
		TechnoTypeExt::ExtData* Type;
		Valueable<AffectedHouse> PowersUp_Owner;
		ValueableVector<BuildingTypeClass*> PowersUp_Buildings;
		ValueableIdxVector<SuperWeaponTypeClass> SuperWeapons;

		ValueableVector<BuildingTypeClass*> PowerPlantEnhancer_Buildings;
		Nullable<int> PowerPlantEnhancer_Amount;
		Nullable<float> PowerPlantEnhancer_Factor;

		std::vector<Point2D> OccupierMuzzleFlashes;

		Valueable<bool> Refinery_UseStorage;
		Nullable<bool> AllowAirstrike;
		Valueable<bool> Grinding_AllowAllies;
		Valueable<bool> Grinding_AllowOwner;
		ValueableVector<TechnoTypeClass*> Grinding_AllowTypes;
		ValueableVector<TechnoTypeClass*> Grinding_DisallowTypes;
		NullableIdx<VocClass> Grinding_Sound;
		Nullable<WeaponTypeClass*> Grinding_Weapon;
		Valueable<bool> Grinding_DisplayRefund;
		Valueable<AffectedHouse> Grinding_DisplayRefund_Houses;
		Valueable<Point2D> Grinding_DisplayRefund_Offset;
		Valueable<bool> Grinding_PlayDieSound;

		Valueable<bool> Refinery_DisplayDumpedMoneyAmount;
		Valueable<Point2D> Refinery_DisplayRefund_Offset;

		Nullable<bool> PlacementPreview_Show;
		Nullable<Theater_SHPStruct*> PlacementPreview_Shape;
		Nullable<int> PlacementPreview_ShapeFrame;
		Valueable<CoordStruct> PlacementPreview_Offset;
		Valueable<bool> PlacementPreview_Remap;
		Valueable<PaletteManager*> PlacementPreview_Palette; //CustomPalette::PaletteMode::Default
		Nullable<int> PlacementPreview_TranslucentLevel;

		Nullable<AffectedHouse> RadialIndicator_Visibility;

		Valueable<bool> SpyEffect_Custom;
		NullableIdx<SuperWeaponTypeClass> SpyEffect_VictimSuperWeapon;
		NullableIdx<SuperWeaponTypeClass> SpyEffect_InfiltratorSuperWeapon;

		//#814
		Valueable<bool> SpyEffect_InfiltratorSW_JustGrant;
		Valueable<bool> SpyEffect_VictimSW_RealLaunch;

		Valueable<bool> SpyEffect_RevealProduction;
		Valueable<bool> SpyEffect_ResetSW;
		Valueable<bool> SpyEffect_ResetRadar;
		Valueable<bool> SpyEffect_RevealRadar;
		Valueable<bool> SpyEffect_RevealRadarPersist;
		Valueable<bool> SpyEffect_GainVeterancy;
		Valueable<bool> SpyEffect_UnReverseEngineer;
		Valueable<int> SpyEffect_StolenTechIndex;
		Valueable<int> SpyEffect_StolenMoneyAmount;
		Valueable<int> SpyEffect_StolenMoneyPercentage;
		Valueable<int> SpyEffect_PowerOutageDuration;
		Valueable<int> SpyEffect_SabotageDelay;
		Valueable<SuperWeaponTypeClass*> SpyEffect_SuperWeapon;
		Valueable<bool> SpyEffect_SuperWeaponPermanent;

		Valueable<bool> SpyEffect_InfantryVeterancy;
		Valueable<bool> SpyEffect_VehicleVeterancy;
		Valueable<bool> SpyEffect_NavalVeterancy;
		Valueable<bool> SpyEffect_AircraftVeterancy;
		Valueable<bool> SpyEffect_BuildingVeterancy;

		Valueable<bool> ZShapePointMove_OnBuildup;
		Valueable<int> SellBuildupLength;

		Valueable<bool> CanC4_AllowZeroDamage;
		Valueable<double> C4_Modifier;

		Valueable<CellStruct> DockUnload_Cell;
		Nullable<DirType32> DockUnload_Facing;

		// solid
		Valueable<int> Solid_Height;
		Valueable<int> Solid_Level;
#pragma region Otamaa
		NullableVector<AnimTypeClass*> DamageFireTypes;
		NullableVector<AnimTypeClass*> OnFireTypes;
		NullableVector<int> OnFireIndex;
		HealthOnFireData HealthOnfire;

		Valueable<BuildingTypeClass*> RubbleIntact; 		//!< What BuildingType to turn into when reconstructed. (This is the normal building, set on rubble.)
		Valueable<BuildingTypeClass*> RubbleDestroyed;	//!< What BuildingType to turn into when destroyed. (This is the rubble, set on normal buildings.)
		Valueable<AnimTypeClass*> RubbleDestroyedAnim;
		Valueable<AnimTypeClass*> RubbleIntactAnim;
		Valueable<OwnerHouseKind> RubbleDestroyedOwner;
		Valueable<OwnerHouseKind> RubbleIntactOwner;
		Valueable<int> RubbleDestroyedStrength;
		Valueable<int> RubbleIntactStrength;
		Valueable<bool> RubbleDestroyedRemove;
		Valueable<bool> RubbleIntactRemove;

		ValueableVector<Point2D> DamageFire_Offs;

		Nullable<double> RepairRate;
		Nullable<int> RepairStep;

		Nullable<bool> PlayerReturnFire;

		Valueable<bool> PackupSound_PlayGlobal;
		Valueable<bool> DisableDamageSound;

		Nullable<float> BuildingOccupyDamageMult;
		Nullable<float> BuildingOccupyROFMult;

		Nullable<float> BuildingBunkerDamageMult;
		Nullable<float> BuildingBunkerROFMult;


		NullableIdx<VocClass> BunkerWallsUpSound;
		NullableIdx<VocClass> BunkerWallsDownSound;

		ValueableIdxVector<BuildingTypeClass> AIBuildInsteadPerDiff;

		PhobosMap<int, AnimTypeClass*> GarrisonAnim_idle;
		PhobosMap<int, AnimTypeClass*> GarrisonAnim_ActiveOne;
		PhobosMap<int, AnimTypeClass*> GarrisonAnim_ActiveTwo;
		PhobosMap<int, AnimTypeClass*> GarrisonAnim_ActiveThree;
		PhobosMap<int, AnimTypeClass*> GarrisonAnim_ActiveFour;

		Valueable<PaletteManager*> PipShapes01Palette; //CustomPalette::PaletteMode::Temperate
		Valueable<bool> PipShapes01Remap;

		Nullable<AnimTypeClass*> TurretAnim_LowPower;
		Nullable<AnimTypeClass*> TurretAnim_DamagedLowPower;

		Valueable<bool> BuildUp_UseNormalLIght;

		Valueable<bool> Power_DegradeWithHealth;

		Nullable<float> AutoSellTime;

		Valueable<bool> IsJuggernaut;

		Nullable<SHPStruct*> BuildingPlacementGrid_Shape;
		BuildSpeedBonus SpeedBonus;

		Valueable<PaletteManager*> RubblePalette; //CustomPalette::PaletteMode::Temperates

		NullableIdx<VocClass> EnterBioReactorSound;
		NullableIdx<VocClass> LeaveBioReactorSound;
		std::vector<int> DockPoseDir;

		Nullable<bool> EngineerRepairable;
#pragma endregion

		signed int IsTrench; 					//!< Enables moving between segments - saves ID of a kind of trench. \sa trenchKinds

		ValueableIdx<TunnelTypeClass> TunnelType;

		Valueable<double> UCPassThrough; 					//!< How many percent of the shots pass through the building to the occupants? 0.0 = 0%, 1.0 = 100%; Defaults to 0.0.
		Valueable<double> UCFatalRate; 					//!< Chance of an occupant getting killed instantly when a bullet passes through. 0.0 = 0%, 1.0 = 100%; Defaults to 0.0.
		Valueable<double> UCDamageMultiplier; 				//!< How many percent of normal damage are applied if an occupant is hit when a bullet passes through. 0.0 = 0%, 1.0 = 100%; Defaults to 1.0.

		ValueableIdx<CursorTypeClass> Cursor_Spy;
		ValueableIdx<CursorTypeClass> Cursor_Sabotage;
		Nullable<bool> ImmuneToSaboteurs;
		Valueable<bool> ReverseEngineersVictims;

		ExtData(BuildingTypeClass* OwnerObject) : Extension<BuildingTypeClass>(OwnerObject)
			, Type { nullptr }
			, PowersUp_Owner { AffectedHouse::Owner }
			, PowersUp_Buildings {}
			, PowerPlantEnhancer_Buildings {}
			, PowerPlantEnhancer_Amount {}
			, PowerPlantEnhancer_Factor {}
			, OccupierMuzzleFlashes { }
			, Refinery_UseStorage { false }
			, AllowAirstrike {}
			, Grinding_AllowAllies { false }
			, Grinding_AllowOwner { true }
			, Grinding_AllowTypes {}
			, Grinding_DisallowTypes {}
			, Grinding_Sound {}
			, Grinding_Weapon {}
			, Grinding_DisplayRefund { false }
			, Grinding_DisplayRefund_Houses { AffectedHouse::All }
			, Grinding_DisplayRefund_Offset { { 0,0 } }
			, Grinding_PlayDieSound { false }

			, Refinery_DisplayDumpedMoneyAmount { false }
			, Refinery_DisplayRefund_Offset { {0,0} }
			, PlacementPreview_Show {}
			, PlacementPreview_Shape {}
			, PlacementPreview_ShapeFrame {}
			, PlacementPreview_Offset { {0,-15,1} }
			, PlacementPreview_Remap { true }
			, PlacementPreview_Palette {}
			, PlacementPreview_TranslucentLevel {}
			, RadialIndicator_Visibility {}
			, SpyEffect_Custom { false }
			, SpyEffect_VictimSuperWeapon {}
			, SpyEffect_InfiltratorSuperWeapon {}
			, SpyEffect_InfiltratorSW_JustGrant { false }
			, SpyEffect_VictimSW_RealLaunch { false }

			, SpyEffect_RevealProduction {}
			, SpyEffect_ResetSW {}
			, SpyEffect_ResetRadar {}
			, SpyEffect_RevealRadar {}
			, SpyEffect_RevealRadarPersist {}
			, SpyEffect_GainVeterancy {}
			, SpyEffect_UnReverseEngineer {}
			, SpyEffect_StolenTechIndex { -1 }
			, SpyEffect_StolenMoneyAmount { 0 }
			, SpyEffect_StolenMoneyPercentage { 0 }
			, SpyEffect_PowerOutageDuration { 0 }
			, SpyEffect_SabotageDelay { 0 }
			, SpyEffect_SuperWeapon {}
			, SpyEffect_SuperWeaponPermanent {}
			, SpyEffect_InfantryVeterancy {}
			, SpyEffect_VehicleVeterancy {}
			, SpyEffect_NavalVeterancy {}
			, SpyEffect_AircraftVeterancy {}
			, SpyEffect_BuildingVeterancy {}

			, ZShapePointMove_OnBuildup { false }
			, SellBuildupLength { 23 }

			, CanC4_AllowZeroDamage { false }
			, C4_Modifier { 1.0 }
			, DockUnload_Cell { { 3, 1 } }
			, DockUnload_Facing {}
			, Solid_Height { 0 }
			, Solid_Level { 1 }
			, DamageFireTypes {}
			, OnFireTypes {}
			, OnFireIndex {}
			, HealthOnfire {}
			, RubbleIntact(nullptr)
			, RubbleDestroyed(nullptr)
			, RubbleDestroyedAnim(nullptr)
			, RubbleIntactAnim(nullptr)
			, RubbleDestroyedOwner(OwnerHouseKind::Default)
			, RubbleIntactOwner(OwnerHouseKind::Default)
			, RubbleDestroyedStrength(0)
			, RubbleIntactStrength(-1)
			, RubbleDestroyedRemove(false)
			, RubbleIntactRemove(false)
			, RepairRate {}
			, RepairStep {}

			, PlayerReturnFire {}

			, PackupSound_PlayGlobal { false }
			, DisableDamageSound { false }

			, BuildingOccupyDamageMult {}
			, BuildingOccupyROFMult {}

			, BuildingBunkerDamageMult {}
			, BuildingBunkerROFMult {}

			, BunkerWallsUpSound {}
			, BunkerWallsDownSound {}

			, AIBuildInsteadPerDiff {}

			, GarrisonAnim_idle {}
			, GarrisonAnim_ActiveOne {}
			, GarrisonAnim_ActiveTwo {}
			, GarrisonAnim_ActiveThree {}
			, GarrisonAnim_ActiveFour {}

			, PipShapes01Palette { }
			, PipShapes01Remap { false }

			, TurretAnim_LowPower {}
			, TurretAnim_DamagedLowPower {}
			, BuildUp_UseNormalLIght { false }
			, Power_DegradeWithHealth { true }
			, IsJuggernaut { false }
			, BuildingPlacementGrid_Shape {}
			, SpeedBonus {}
			, RubblePalette {}
			, EnterBioReactorSound {}
			, LeaveBioReactorSound {}
			, DockPoseDir {}
			, EngineerRepairable {}
			, IsTrench { -1 }
			, TunnelType { -1 }
			, UCPassThrough { 0.0 }
			, UCFatalRate { 0.0 }
			, UCDamageMultiplier { 1.0 }
			, Cursor_Spy { (int)MouseCursorType::Enter }
			, Cursor_Sabotage { 93 }
			, ImmuneToSaboteurs { }
			, ReverseEngineersVictims { false }
		{ }

		virtual ~ExtData() override = default;

		void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);
		void Initialize() {
			this->OccupierMuzzleFlashes.reserve(((BuildingTypeClass*)this->Type->Get())->MaxNumberOccupants);
			this->DockPoseDir.reserve(((BuildingTypeClass*)this->Type->Get())->NumberOfDocks);
		}

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

	static bool FC IsFactory(BuildingClass* pThis, void* _);
	static void FC DrawPlacementGrid(Surface* Surface, ConvertClass* Pal, SHPStruct* SHP, int FrameIndex, const Point2D* const Position, const RectangleStruct* const Bounds, BlitterFlags Flags, int Remap, int ZAdjust, ZGradient ZGradientDescIndex, int Brightness, int TintColor, SHPStruct* ZShape, int ZShapeFrame, int XOffset, int YOffset);
};