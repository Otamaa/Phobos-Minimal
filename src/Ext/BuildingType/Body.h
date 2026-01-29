#pragma once
#include <BuildingTypeClass.h>
#include <SuperClass.h>
#include <SuperWeaponTypeClass.h>

#include <Helpers/Macro.h>
#include <Utilities/PhobosMap.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/TheaterSpecificSHP.h>

#include <DirStruct.h>

#include <Ext/TechnoType/Body.h>

#include <New/AnonymousType/BuildSpeedBonus.h>

#include <New/Type/TunnelTypeClass.h>
#include <New/Type/CursorTypeClass.h>

#include <New/AnonymousType/PrismForwardingData.h>

#include <Misc/Defines.h>

//enum class BunkerSoundMode : int
//{
//	Up, Down
//};

class SuperClass;
class BuildingTypeExtData final : public TechnoTypeExtData
{
public:
	using base_type = BuildingTypeClass;
	static COMPILETIMEEVAL const char* ClassName = "BuildingTypeExtData";
	static COMPILETIMEEVAL const char* BaseClassName = "BuildingTypeClass";
	static COMPILETIMEEVAL unsigned Marker = UuidFirstPart<base_type>::value;
	static COMPILETIMEEVAL auto Marker_str = to_hex_string<Marker>();

public:

#pragma region ClassMembers
	// ============================================================
	// Large aggregates (unknown internal alignment, place first)
	// ============================================================
	PrismForwardingData PrismForwarding;
	TheaterSpecificSHP PlacementPreview_Shape;
	CustomPalette PlacementPreview_Palette;
	CustomPalette PipShapes01Palette;
	CustomPalette RubblePalette;
	HealthOnFireData HealthOnfire;
	BuildSpeedBonus SpeedBonus;
	std::bitset<MaxHouseCount> SpyEffect_StolenTechIndex_result;
	DynamicVectorClass<Point2D> FoundationRadarShape;
	Valueable<CSFText> MessageCapture;
	Valueable<CSFText> MessageLost;

	// ============================================================
	// 24-byte aligned: Vectors (grouped together)
	// ============================================================
	ValueableVector<BuildingTypeClass*> PowersUp_Buildings;
	ValueableIdxVector<SuperWeaponTypeClass> SuperWeapons;
	ValueableVector<BuildingTypeClass*> PowerPlantEnhancer_Buildings;
	std::vector<Point2D> OccupierMuzzleFlashes;
	ValueableVector<TechnoTypeClass*> Grinding_AllowTypes;
	ValueableVector<TechnoTypeClass*> Grinding_DisallowTypes;
	NullableVector<AnimTypeClass*> DamageFireTypes;
	NullableVector<AnimTypeClass*> OnFireTypes;
	NullableVector<int> OnFireIndex;
	ValueableVector<Point2D> DamageFire_Offs;
	ValueableIdxVector<BuildingTypeClass> AIBuildInsteadPerDiff;
	std::vector<AnimTypeClass*> GarrisonAnim_idle;
	std::vector<AnimTypeClass*> GarrisonAnim_ActiveOne;
	std::vector<AnimTypeClass*> GarrisonAnim_ActiveTwo;
	std::vector<AnimTypeClass*> GarrisonAnim_ActiveThree;
	std::vector<AnimTypeClass*> GarrisonAnim_ActiveFour;
	ValueableVector<FacingType> DockPoseDir;
	ValueableVector<InfantryTypeClass*> AllowedOccupiers;
	ValueableVector<InfantryTypeClass*> DisallowedOccupiers;
	std::vector<CellStruct> CustomData;
	std::vector<CellStruct> OutlineData;
	NullableVector<TechnoTypeClass*> Secret_Boons;
	ValueableVector<TechnoTypeClass*> AcademyWhitelist;
	ValueableVector<TechnoTypeClass*> AcademyBlacklist;
	ValueableVector<TechnoTypeClass*> FactoryPlant_AllowTypes;
	ValueableVector<TechnoTypeClass*> FactoryPlant_DisallowTypes;
	ValueableVector<BuildingTypeClass*> Adjacent_Allowed;
	ValueableVector<BuildingTypeClass*> Adjacent_Disallowed;
	ValueableVector<bool> HasPowerUpAnim;

	// ============================================================
	// Valueable<pointer> (8 bytes each)
	// ============================================================
	Valueable<WeaponTypeClass*> Grinding_Weapon;
	Valueable<SuperWeaponTypeClass*> SpyEffect_SuperWeapon;
	Valueable<BuildingTypeClass*> RubbleIntact;
	Valueable<BuildingTypeClass*> RubbleDestroyed;
	Valueable<AnimTypeClass*> RubbleDestroyedAnim;
	Valueable<AnimTypeClass*> RubbleIntactAnim;
	Valueable<AnimTypeClass*> SpyEffect_Anim;
	Valueable<BuildingTypeClass*> NextBuilding_Prev;
	Valueable<BuildingTypeClass*> NextBuilding_Next;
	Valueable<BuildingTypeClass*> LaserFencePost_Fence;
	Valueable<BuildingTypeClass*> PlaceBuilding_OnLand;
	Valueable<BuildingTypeClass*> PlaceBuilding_OnWater;

	// ============================================================
	// Nullable<pointer> (pointer + bool + padding ≈ 16 bytes)
	// ============================================================
	Nullable<SHPStruct*> BuildingPlacementGrid_Shape;
	Nullable<AnimTypeClass*> TurretAnim_LowPower;
	Nullable<AnimTypeClass*> TurretAnim_DamagedLowPower;

	// ============================================================
	// Valueable<double> (8 bytes each)
	// ============================================================
	Valueable<double> C4_Modifier;
	Valueable<double> UCPassThrough;
	Valueable<double> UCFatalRate;
	Valueable<double> UCDamageMultiplier;
	Valueable<double> LightningRod_Modifier;
	Valueable<double> AcademyInfantry;
	Valueable<double> AcademyAircraft;
	Valueable<double> AcademyVehicle;
	Valueable<double> AcademyBuilding;
	Valueable<double> PowerPlant_DamageFactor;

	// ============================================================
	// Nullable<double> (double + bool + padding ≈ 16 bytes)
	// ============================================================
	Nullable<double> RepairRate;
	Nullable<double> BuildupTime;
	Nullable<double> SellTime;
	Nullable<double> DegradePercentage;
	Nullable<double> Units_RepairRate;
	Nullable<double> Units_RepairPercent;

	// ============================================================
	// Nullable<PartialVector3D<int>> (12+ bytes)
	// ============================================================
	Nullable<PartialVector3D<int>> AIBuildCounts;
	Nullable<PartialVector3D<int>> AIExtraCounts;

	// ============================================================
	// Valueable<CoordStruct> (12 bytes)
	// ============================================================
	Valueable<CoordStruct> PlacementPreview_Offset;

	// ============================================================
	// Valueable<Point2D> (8 bytes)
	// ============================================================
	Valueable<Point2D> DisplayIncome_Offset;

	// ============================================================
	// Nullable<Point2D> (8 bytes + bool + padding)
	// ============================================================
	Nullable<Point2D> BarracksExitCell;

	// ============================================================
	// OptionalStruct
	// ============================================================
	mutable OptionalStruct<bool> Academy;

	// ============================================================
	// Nullable<float> (float + bool + padding ≈ 8 bytes)
	// ============================================================
	Nullable<float> AutoSellTime;
	Nullable<float> BuildingOccupyDamageMult;
	Nullable<float> BuildingOccupyROFMult;
	Nullable<float> BuildingBunkerDamageMult;
	Nullable<float> BuildingBunkerROFMult;
	Nullable<float> PurifierBonus;

	// ============================================================
	// Nullable<int/enum> (int + bool + padding ≈ 8 bytes)
	// ============================================================
	Nullable<int> PlacementPreview_ShapeFrame;
	Nullable<int> PlacementPreview_TranslucentLevel;
	Nullable<int> RepairStep;
	Nullable<int> DegradeAmount;
	Nullable<int> SpyEffect_SellDelay;
	Nullable<int> Units_RepairStep;
	Nullable<int> NewEvaVoice_Index;
	Nullable<int> RevealToAll_Radius;
	Nullable<AffectedHouse> RadialIndicator_Visibility;
	Nullable<DirType32> DockUnload_Facing;
	Nullable<AffectedHouse> DisplayIncome_Houses;
	Nullable<FacingType> LandingDir;

	// ============================================================
	// NullableIdx (int + bool ≈ 8 bytes)
	// ============================================================
	NullableIdx<VocClass> Grinding_Sound;
	NullableIdx<SuperWeaponTypeClass> SpyEffect_VictimSuperWeapon;
	NullableIdx<SuperWeaponTypeClass> SpyEffect_InfiltratorSuperWeapon;
	NullableIdx<VocClass> GateDownSound;
	NullableIdx<VocClass> GateUpSound;
	NullableIdx<VocClass> BunkerWallsUpSound;
	NullableIdx<VocClass> BunkerWallsDownSound;
	NullableIdx<VocClass> SlamSound;
	NullableIdx<VocClass> AbandonedSound;
	NullableIdx<VocClass> BuildingRepairedSound;

	// ============================================================
	// Nullable<bool> (bool + bool ≈ 2-4 bytes)
	// ============================================================
	Nullable<bool> PlacementPreview_Show;
	Nullable<bool> AIBaseNormal;
	Nullable<bool> AIInnerBase;
	Nullable<bool> UnitSell;
	Nullable<bool> PlayerReturnFire;
	Nullable<bool> EngineerRepairable;
	Nullable<bool> ImmuneToSaboteurs;
	Nullable<bool> Returnable;
	Nullable<bool> Storage_ActiveAnimations;
	Nullable<bool> DisplayIncome;
	Nullable<bool> Cameo_ShouldCount;
	Nullable<bool> AutoBuilding;
	Nullable<bool> AISellCapturedBuilding;
	Nullable<bool> BuildingRadioLink_SyncOwner;
	Nullable<bool> Units_UseRepairCost;

	// ============================================================
	// Valueable<int> (4 bytes each)
	// ============================================================
	Valueable<int> PowerPlantEnhancer_Amount;
	Valueable<int> Grinding_Weapon_RequiredCredits;
	Valueable<int> SpyEffect_StolenMoneyAmount;
	Valueable<int> SpyEffect_PowerOutageDuration;
	Valueable<int> SpyEffect_SabotageDelay;
	Valueable<int> SellBuildupLength;
	Valueable<int> Solid_Height;
	Valueable<int> Solid_Level;
	Valueable<int> RubbleDestroyedStrength;
	Valueable<int> RubbleIntactStrength;
	Valueable<int> SpyEffect_Anim_Duration;
	Valueable<int> FactoryPlant_MaxCount;
	Valueable<int> Adjacent_Disallowed_ExtraDistance;
	Valueable<int> AutoBuilding_Gap;
	Valueable<int> LimboBuildID;
	Valueable<int> Overpower_KeepOnline;
	Valueable<int> Overpower_ChargeWeapon;
	Valueable<int> NewEvaVoice_Priority;
	Valueable<int> PowerPlantEnhancer_MaxCount;

	// ============================================================
	// Valueable<unsigned int> (4 bytes)
	// ============================================================
	Valueable<unsigned int> FreeUnit_Count;

	// ============================================================
	// Valueable<float> (4 bytes each)
	// ============================================================
	Valueable<float> PowerPlantEnhancer_Factor;
	Valueable<float> SpyEffect_StolenMoneyPercentage;

	// ============================================================
	// ValueableIdx (4 bytes each)
	// ============================================================
	ValueableIdx<TunnelTypeClass> TunnelType;
	ValueableIdx<CursorTypeClass> Cursor_Spy;
	ValueableIdx<CursorTypeClass> Cursor_Sabotage;
	ValueableIdx<VoxClass> LostEvaEvent;
	ValueableIdx<VoxClass> EVA_Online;
	ValueableIdx<VoxClass> EVA_Offline;
	ValueableIdx<VoxClass> NewEvaVoice_InitialMessage;

	// ============================================================
	// Valueable<enum> (4 bytes each)
	// ============================================================
	Valueable<AffectedHouse> PowersUp_Owner;
	Valueable<OwnerHouseKind> RubbleDestroyedOwner;
	Valueable<OwnerHouseKind> RubbleIntactOwner;
	Valueable<AffectedHouse> SpyEffect_Anim_DisplayHouses;

	// ============================================================
	// Valueable<CellStruct> (4 bytes)
	// ============================================================
	Valueable<CellStruct> DockUnload_Cell;

	// ============================================================
	// Plain int (4 bytes each)
	// ============================================================
	signed int IsTrench;
	int SellFrames;
	int CustomWidth;
	int CustomHeight;
	int OutlineLength;
	int NextBuilding_CurrentHeapId;

	// ============================================================
	// Valueable<bool> (1 byte each, packed together)
	// ============================================================
	Valueable<bool> Refinery_UseStorage;
	Valueable<bool> Grinding_AllowAllies;
	Valueable<bool> Grinding_AllowOwner;
	Valueable<bool> Grinding_PlayDieSound;
	Valueable<bool> PlacementPreview_Remap;
	Valueable<bool> SpyEffect_Custom;
	Valueable<bool> SpyEffect_InfiltratorSW_JustGrant;
	Valueable<bool> SpyEffect_VictimSW_RealLaunch;
	Valueable<bool> SpyEffect_RevealProduction;
	Valueable<bool> SpyEffect_ResetSW;
	Valueable<bool> SpyEffect_ResetRadar;
	Valueable<bool> SpyEffect_RevealRadar;
	Valueable<bool> SpyEffect_RevealRadarPersist;
	Valueable<bool> SpyEffect_GainVeterancy;
	Valueable<bool> SpyEffect_UnReverseEngineer;
	Valueable<bool> SpyEffect_SuperWeaponPermanent;
	Valueable<bool> SpyEffect_InfantryVeterancy;
	Valueable<bool> SpyEffect_VehicleVeterancy;
	Valueable<bool> SpyEffect_NavalVeterancy;
	Valueable<bool> SpyEffect_AircraftVeterancy;
	Valueable<bool> SpyEffect_BuildingVeterancy;
	Valueable<bool> ZShapePointMove_OnBuildup;
	Valueable<bool> CanC4_AllowZeroDamage;
	Valueable<bool> RubbleDestroyedRemove;
	Valueable<bool> RubbleIntactRemove;
	Valueable<bool> RubbleIntactConsumeEngineer;
	Valueable<bool> PackupSound_PlayGlobal;
	Valueable<bool> DisableDamageSound;
	Valueable<bool> PipShapes01Remap;
	Valueable<bool> BuildUp_UseNormalLIght;
	Valueable<bool> Power_DegradeWithHealth;
	Valueable<bool> IsJuggernaut;
	Valueable<bool> ReverseEngineersVictims;
	Valueable<bool> ReverseEngineersVictims_Passengers;
	Valueable<bool> Destroyed_CreateSmudge;
	Valueable<bool> BunkerRaidable;
	Valueable<bool> Firestorm_Wall;
	Valueable<bool> CloningFacility;
	Valueable<bool> Factory_ExplicitOnly;
	Valueable<bool> Secret_RecalcOnCapture;
	Valueable<bool> IsPassable;
	Valueable<bool> ProduceCashDisplay;
	Valueable<bool> PurifierBonus_RequirePower;
	Valueable<bool> FactoryPlant_RequirePower;
	Valueable<bool> SpySat_RequirePower;
	Valueable<bool> Cloning_RequirePower;
	Valueable<bool> Radar_RequirePower;
	Valueable<bool> SpawnCrewOnlyOnce;
	Valueable<bool> IsDestroyableObstacle;
	Valueable<bool> Explodes_DuringBuildup;
	Valueable<bool> SpyEffect_SWTargetCenter;
	Valueable<bool> ShowPower;
	Valueable<bool> EMPulseCannon_UseWeaponSelection;
	Valueable<bool> ExcludeFromMultipleFactoryBonus;
	Valueable<bool> NoBuildAreaOnBuildup;
	Valueable<bool> LimboBuild;
	Valueable<bool> IsAnimDelayedBurst;
	Valueable<bool> AllowAlliesRepair;
	Valueable<bool> AllowRepairFlyMZone;
	Valueable<bool> NewEvaVoice;
	Valueable<bool> NewEvaVoice_RecheckOnDeath;
	Valueable<bool> BattlePointsCollector;
	Valueable<bool> BattlePointsCollector_RequirePower;
	Valueable<bool> Refinery_UseNormalActiveAnim;
	Valueable<bool> AggressiveModeExempt;
	Valueable<bool> IsBarGate;
	Valueable<bool> IsHideDuringSpecialAnim;
	Valueable<bool> ApplyPerTargetEffectsOnDetonate;

	// ============================================================
	// Plain bool (1 byte, at the very end)
	// ============================================================
	bool IsCustom;
	// Total bools: 68 Valueable<bool> + 1 plain bool = 69 bytes
	// Pads to 72 for 4-byte alignment

#pragma endregion

public:
	BuildingTypeExtData(BuildingTypeClass* pObj)
		: TechnoTypeExtData(pObj)
		// Large aggregates
		, PrismForwarding()
		, PlacementPreview_Shape()
		, PlacementPreview_Palette(CustomPalette::PaletteMode::Temperate)
		, PipShapes01Palette(CustomPalette::PaletteMode::Temperate)
		, RubblePalette(CustomPalette::PaletteMode::Temperate)
		, HealthOnfire()
		, SpeedBonus()
		, SpyEffect_StolenTechIndex_result()
		, FoundationRadarShape()
		, MessageCapture()
		, MessageLost()
		// Vectors
		, PowersUp_Buildings()
		, SuperWeapons()
		, PowerPlantEnhancer_Buildings()
		, OccupierMuzzleFlashes()
		, Grinding_AllowTypes()
		, Grinding_DisallowTypes()
		, DamageFireTypes()
		, OnFireTypes()
		, OnFireIndex()
		, DamageFire_Offs()
		, AIBuildInsteadPerDiff()
		, GarrisonAnim_idle()
		, GarrisonAnim_ActiveOne()
		, GarrisonAnim_ActiveTwo()
		, GarrisonAnim_ActiveThree()
		, GarrisonAnim_ActiveFour()
		, DockPoseDir()
		, AllowedOccupiers()
		, DisallowedOccupiers()
		, CustomData()
		, OutlineData()
		, Secret_Boons()
		, AcademyWhitelist()
		, AcademyBlacklist()
		, FactoryPlant_AllowTypes()
		, FactoryPlant_DisallowTypes()
		, Adjacent_Allowed()
		, Adjacent_Disallowed()
		, HasPowerUpAnim()
		// Valueable<pointer>
		, Grinding_Weapon(nullptr)
		, SpyEffect_SuperWeapon(nullptr)
		, RubbleIntact(nullptr)
		, RubbleDestroyed(nullptr)
		, RubbleDestroyedAnim(nullptr)
		, RubbleIntactAnim(nullptr)
		, SpyEffect_Anim(nullptr)
		, NextBuilding_Prev(nullptr)
		, NextBuilding_Next(nullptr)
		, LaserFencePost_Fence(nullptr)
		, PlaceBuilding_OnLand(nullptr)
		, PlaceBuilding_OnWater(nullptr)
		// Nullable<pointer>
		, BuildingPlacementGrid_Shape()
		, TurretAnim_LowPower()
		, TurretAnim_DamagedLowPower()
		// Valueable<double>
		, C4_Modifier(1.0)
		, UCPassThrough(0.0)
		, UCFatalRate(0.0)
		, UCDamageMultiplier(1.0)
		, LightningRod_Modifier(1.0)
		, AcademyInfantry(0.0)
		, AcademyAircraft(0.0)
		, AcademyVehicle(0.0)
		, AcademyBuilding(0.0)
		, PowerPlant_DamageFactor(1.0)
		// Nullable<double>
		, RepairRate()
		, BuildupTime()
		, SellTime()
		, DegradePercentage()
		, Units_RepairRate()
		, Units_RepairPercent()
		// Nullable<PartialVector3D<int>>
		, AIBuildCounts()
		, AIExtraCounts()
		// Valueable<CoordStruct>
		, PlacementPreview_Offset({ 0, -15, 1 })
		// Valueable<Point2D>
		, DisplayIncome_Offset({ 0, 0 })
		// Nullable<Point2D>
		, BarracksExitCell()
		// OptionalStruct
		, Academy()
		// Nullable<float>
		, AutoSellTime()
		, BuildingOccupyDamageMult()
		, BuildingOccupyROFMult()
		, BuildingBunkerDamageMult()
		, BuildingBunkerROFMult()
		, PurifierBonus()
		// Nullable<int/enum>
		, PlacementPreview_ShapeFrame()
		, PlacementPreview_TranslucentLevel()
		, RepairStep()
		, DegradeAmount()
		, SpyEffect_SellDelay()
		, Units_RepairStep()
		, NewEvaVoice_Index()
		, RevealToAll_Radius()
		, RadialIndicator_Visibility()
		, DockUnload_Facing()
		, DisplayIncome_Houses()
		, LandingDir()
		// NullableIdx
		, Grinding_Sound()
		, SpyEffect_VictimSuperWeapon()
		, SpyEffect_InfiltratorSuperWeapon()
		, GateDownSound()
		, GateUpSound()
		, BunkerWallsUpSound()
		, BunkerWallsDownSound()
		, SlamSound()
		, AbandonedSound()
		, BuildingRepairedSound()
		// Nullable<bool>
		, PlacementPreview_Show()
		, AIBaseNormal()
		, AIInnerBase()
		, UnitSell()
		, PlayerReturnFire()
		, EngineerRepairable()
		, ImmuneToSaboteurs()
		, Returnable()
		, Storage_ActiveAnimations()
		, DisplayIncome()
		, Cameo_ShouldCount()
		, AutoBuilding()
		, AISellCapturedBuilding()
		, BuildingRadioLink_SyncOwner()
		, Units_UseRepairCost()
		// Valueable<int>
		, PowerPlantEnhancer_Amount(0)
		, Grinding_Weapon_RequiredCredits(0)
		, SpyEffect_StolenMoneyAmount(0)
		, SpyEffect_PowerOutageDuration(0)
		, SpyEffect_SabotageDelay(0)
		, SellBuildupLength(23)
		, Solid_Height(0)
		, Solid_Level(1)
		, RubbleDestroyedStrength(0)
		, RubbleIntactStrength(-1)
		, SpyEffect_Anim_Duration(-1)
		, FactoryPlant_MaxCount(-1)
		, Adjacent_Disallowed_ExtraDistance(0)
		, AutoBuilding_Gap(1)
		, LimboBuildID(-1)
		, Overpower_KeepOnline(2)
		, Overpower_ChargeWeapon(1)
		, NewEvaVoice_Priority(0)
		, PowerPlantEnhancer_MaxCount(-1)
		// Valueable<unsigned int>
		, FreeUnit_Count(1)
		// Valueable<float>
		, PowerPlantEnhancer_Factor(1.0f)
		, SpyEffect_StolenMoneyPercentage(0)
		// ValueableIdx
		, TunnelType(-1)
		, Cursor_Spy((int)MouseCursorType::Enter)
		, Cursor_Sabotage(93)
		, LostEvaEvent(-1)
		, EVA_Online(-1)
		, EVA_Offline(-1)
		, NewEvaVoice_InitialMessage(-1)
		// Valueable<enum>
		, PowersUp_Owner(AffectedHouse::Owner)
		, RubbleDestroyedOwner(OwnerHouseKind::Default)
		, RubbleIntactOwner(OwnerHouseKind::Default)
		, SpyEffect_Anim_DisplayHouses(AffectedHouse::All)
		// Valueable<CellStruct>
		, DockUnload_Cell({ 3, 1 })
		// Plain int
		, IsTrench(-1)
		, SellFrames(0)
		, CustomWidth(0)
		, CustomHeight(0)
		, OutlineLength(0)
		, NextBuilding_CurrentHeapId(-1)
		// Valueable<bool>
		, Refinery_UseStorage(false)
		, Grinding_AllowAllies(false)
		, Grinding_AllowOwner(true)
		, Grinding_PlayDieSound(true)
		, PlacementPreview_Remap(true)
		, SpyEffect_Custom(false)
		, SpyEffect_InfiltratorSW_JustGrant(false)
		, SpyEffect_VictimSW_RealLaunch(false)
		, SpyEffect_RevealProduction(false)
		, SpyEffect_ResetSW(false)
		, SpyEffect_ResetRadar(false)
		, SpyEffect_RevealRadar(false)
		, SpyEffect_RevealRadarPersist(false)
		, SpyEffect_GainVeterancy(false)
		, SpyEffect_UnReverseEngineer(false)
		, SpyEffect_SuperWeaponPermanent(false)
		, SpyEffect_InfantryVeterancy(false)
		, SpyEffect_VehicleVeterancy(false)
		, SpyEffect_NavalVeterancy(false)
		, SpyEffect_AircraftVeterancy(false)
		, SpyEffect_BuildingVeterancy(false)
		, ZShapePointMove_OnBuildup(false)
		, CanC4_AllowZeroDamage(false)
		, RubbleDestroyedRemove(false)
		, RubbleIntactRemove(false)
		, RubbleIntactConsumeEngineer(false)
		, PackupSound_PlayGlobal(false)
		, DisableDamageSound(false)
		, PipShapes01Remap(false)
		, BuildUp_UseNormalLIght(false)
		, Power_DegradeWithHealth(true)
		, IsJuggernaut(false)
		, ReverseEngineersVictims(false)
		, ReverseEngineersVictims_Passengers(false)
		, Destroyed_CreateSmudge(true)
		, BunkerRaidable(false)
		, Firestorm_Wall(false)
		, CloningFacility(false)
		, Factory_ExplicitOnly(false)
		, Secret_RecalcOnCapture(false)
		, IsPassable(false)
		, ProduceCashDisplay(false)
		, PurifierBonus_RequirePower(false)
		, FactoryPlant_RequirePower(false)
		, SpySat_RequirePower(false)
		, Cloning_RequirePower(false)
		, Radar_RequirePower(true)
		, SpawnCrewOnlyOnce(true)
		, IsDestroyableObstacle(false)
		, Explodes_DuringBuildup(true)
		, SpyEffect_SWTargetCenter(false)
		, ShowPower(true)
		, EMPulseCannon_UseWeaponSelection(false)
		, ExcludeFromMultipleFactoryBonus(false)
		, NoBuildAreaOnBuildup(false)
		, LimboBuild(false)
		, IsAnimDelayedBurst(true)
		, AllowAlliesRepair(false)
		, AllowRepairFlyMZone(false)
		, NewEvaVoice(false)
		, NewEvaVoice_RecheckOnDeath(false)
		, BattlePointsCollector(false)
		, BattlePointsCollector_RequirePower(false)
		, Refinery_UseNormalActiveAnim(false)
		, AggressiveModeExempt(false)
		, IsBarGate(false)
		, IsHideDuringSpecialAnim(false)
		, ApplyPerTargetEffectsOnDetonate(true)
		// Plain bool
		, IsCustom(false)
	{
		this->InitializeConstant();
		this->NextBuilding_CurrentHeapId = pObj->ArrayIndex;
		this->AbsType = BuildingTypeClass::AbsID;
	}

	BuildingTypeExtData(BuildingTypeClass* pObj, noinit_t nn) : TechnoTypeExtData(pObj, nn) { }

	virtual ~BuildingTypeExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override
	{
		this->TechnoTypeExtData::InvalidatePointer(ptr, bRemoved);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->TechnoTypeExtData::LoadFromStream(Stm);
		this->Serialize(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		this->TechnoTypeExtData::SaveToStream(Stm);
		const_cast<BuildingTypeExtData*>(this)->Serialize(Stm);
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const
	{
		this->TechnoTypeExtData::CalculateCRC(crc);
	}

	BuildingTypeClass* This() const { return reinterpret_cast<BuildingTypeClass*>(this->AttachedToObject); }
	const BuildingTypeClass* This_Const() const { return reinterpret_cast<const BuildingTypeClass*>(this->AttachedToObject); }

	virtual bool LoadFromINI(CCINIClass* pINI, bool parseFailAddr);
	virtual bool WriteToINI(CCINIClass* pINI) const {  return true; }

public:

	void CompleteInitialization();

	// Assuming SuperWeapon & SuperWeapon2 are used (for the moment)
	COMPILETIMEEVAL int FORCEDINLINE GetSuperWeaponCount() const
	{
		// The user should only use SuperWeapon and SuperWeapon2 if the attached sw count isn't bigger than 2
		return 2 + this->SuperWeapons.size();
	}

	int GetSuperWeaponIndex(int index, HouseClass* pHouse) const;
	int GetSuperWeaponIndex(int index) const;
	SuperClass* GetSuperWeaponByIndex(int index, HouseClass* pHouse) const;

	bool CanBeOccupiedBy(InfantryClass* whom) const;

	bool IsAcademy() const;

	void UpdateFoundationRadarShape();

public:

	static bool IsFoundationEqual(BuildingTypeClass* pType1, BuildingTypeClass* pType2);
	// Short check: Is the building of a linkable kind at all?
	static bool IsLinkable(BuildingTypeClass* pThis);
	static int GetEnhancedPower(BuildingClass* pBuilding, HouseClass* pHouse);
	static float GetPurifierBonusses(HouseClass* pHouse);
	static double GetExternalFactorySpeedBonus(TechnoClass* pWhat);
	static double GetExternalFactorySpeedBonus(TechnoClass* pWhat, HouseClass* pOwner);
	static double GetExternalFactorySpeedBonus(TechnoTypeClass* pWhat, HouseClass* pOwner);
	static bool CanUpgrade(BuildingClass* pBuilding, BuildingTypeClass* pUpgradeType, HouseClass* pUpgradeOwner);
	static int GetUpgradesAmount(BuildingTypeClass* pBuilding, HouseClass* pHouse);

	//template<BunkerSoundMode UpSound>
	//struct BunkerSound
	//{
	//	COMPILETIMEEVAL void operator ()(BuildingClass* pThis)
	//	{

	//		if COMPILETIMEEVAL (UpSound == BunkerSoundMode::Up)
	//		{
	//			const auto nSound = BuildingTypeExtContainer::Instance.Find(pThis->Type)->BunkerWallsUpSound.Get(RulesClass::Instance->BunkerWallsUpSound);
	//			VocClass::SafeImmedietelyPlayAt(nSound, pThis->Location);
	//		}
	//		else
	//		{
	//			const auto nSound = BuildingTypeExtContainer::Instance.Find(pThis->Type)->BunkerWallsDownSound.Get(RulesClass::Instance->BunkerWallsDownSound);
	//			VocClass::SafeImmedietelyPlayAt(nSound, pThis->Location);
	//		}
	//	}
	//};

	static void DisplayPlacementPreview();
	static int GetBuildingAnimTypeIndex(BuildingClass* pThis, const BuildingAnimSlot& nSlot, const char* pDefault);

	static void UpdateBuildupFrames(BuildingTypeClass* pThis);

	static void __fastcall DrawPlacementGrid(Surface* Surface, ConvertClass* Pal, SHPStruct* SHP, int FrameIndex, const Point2D* const Position, const RectangleStruct* const Bounds, BlitterFlags Flags, int Remap, int ZAdjust, ZGradient ZGradientDescIndex, int Brightness, int TintColor, SHPStruct* ZShape, int ZShapeFrame, int XOffset, int YOffset);

	static bool ShouldExistGreyCameo(TechnoTypeClass* pType);
	static CanBuildResult CheckAlwaysExistCameo(TechnoTypeClass* pType, CanBuildResult canBuild);

	static bool CheckOccupierCanLeave(HouseClass* pBuildingHouse, HouseClass* pOccupierHouse);
	static bool CleanUpBuildingSpace(BuildingTypeClass* pBuildingType, CellStruct topLeftCell, HouseClass* pHouse, TechnoClass* pExceptTechno = nullptr);
	static bool AutoPlaceBuilding(BuildingClass* pBuilding);
	static bool BuildLimboBuilding(BuildingClass* pBuilding);
	static void CreateLimboBuilding(BuildingClass* pBuilding, BuildingTypeClass* pType, HouseClass* pOwner, int ID);
	static int CountOwnedNowWithDeployOrUpgrade(BuildingTypeClass* pBuilding, HouseClass* pHouse);

	static bool IsSameBuildingType(BuildingTypeClass* pType1, BuildingTypeClass* pType2);

private:

	template <typename T>
	void Serialize(T& Stm);

public:

	static const DirStruct DefaultJuggerFacing;
	static const Foundation CustomFoundation = static_cast<Foundation>(0x7F);
	static const CellStruct FoundationEndMarker;
};

class BuildingTypeExtContainer final : public Container<BuildingTypeExtData>
	, public ReadWriteContainerInterfaces<BuildingTypeExtData>
{
public:
	static COMPILETIMEEVAL const char* ClassName = "BuildingTypeExtContainer";
	using base_container_t = Container<BuildingTypeExtData>;

public:

	std::vector<std::string> trenchKinds;

public:
	static BuildingTypeExtContainer Instance;

	virtual bool LoadAll(const json& root);
	virtual bool SaveAll(json& root);
	virtual void Clear();

	virtual void LoadFromINI(BuildingTypeClass* key, CCINIClass* pINI, bool parseFailAddr);
	virtual void WriteToINI(BuildingTypeClass* key, CCINIClass* pINI);
};

class NOVTABLE FakeBuildingTypeClass : public BuildingTypeClass
{
public:

	bool _CanUseWaypoint();

	BuildingTypeExtData* _GetExtData() {
		return *reinterpret_cast<BuildingTypeExtData**>(((DWORD)this) + AbstractExtOffset);
	}

	bool _ReadFromINI(CCINIClass* pINI);
};
static_assert(sizeof(FakeBuildingTypeClass) == sizeof(BuildingTypeClass), "Invalid Size !");