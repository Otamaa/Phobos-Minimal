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
	static constexpr unsigned Marker = UuidFirstPart<base_type>::value;

public:

#pragma region ClassMembers
	PrismForwardingData PrismForwarding;
	Valueable<AffectedHouse> PowersUp_Owner;
	ValueableVector<BuildingTypeClass*> PowersUp_Buildings;
	ValueableIdxVector<SuperWeaponTypeClass> SuperWeapons;
	ValueableVector<BuildingTypeClass*> PowerPlantEnhancer_Buildings;
	Valueable<int> PowerPlantEnhancer_Amount;
	Valueable<float> PowerPlantEnhancer_Factor;
	std::vector<Point2D> OccupierMuzzleFlashes;
	Valueable<bool> Refinery_UseStorage;
	Valueable<bool> Grinding_AllowAllies;
	Valueable<bool> Grinding_AllowOwner;
	ValueableVector<TechnoTypeClass*> Grinding_AllowTypes;
	ValueableVector<TechnoTypeClass*> Grinding_DisallowTypes;
	NullableIdx<VocClass> Grinding_Sound;
	Valueable<WeaponTypeClass*> Grinding_Weapon;
	Valueable<bool> Grinding_PlayDieSound;
	Valueable<int> Grinding_Weapon_RequiredCredits;
	Nullable<bool> PlacementPreview_Show;
	TheaterSpecificSHP PlacementPreview_Shape;
	Nullable<int> PlacementPreview_ShapeFrame;
	Valueable<CoordStruct> PlacementPreview_Offset;
	Valueable<bool> PlacementPreview_Remap;
	CustomPalette PlacementPreview_Palette;
	Nullable<int> PlacementPreview_TranslucentLevel;
	Nullable<AffectedHouse> RadialIndicator_Visibility;
	Valueable<bool> SpyEffect_Custom;
	NullableIdx<SuperWeaponTypeClass> SpyEffect_VictimSuperWeapon;
	NullableIdx<SuperWeaponTypeClass> SpyEffect_InfiltratorSuperWeapon;
	Valueable<bool> SpyEffect_InfiltratorSW_JustGrant;
	Valueable<bool> SpyEffect_VictimSW_RealLaunch;
	Valueable<bool> SpyEffect_RevealProduction;
	Valueable<bool> SpyEffect_ResetSW;
	Valueable<bool> SpyEffect_ResetRadar;
	Valueable<bool> SpyEffect_RevealRadar;
	Valueable<bool> SpyEffect_RevealRadarPersist;
	Valueable<bool> SpyEffect_GainVeterancy;
	Valueable<bool> SpyEffect_UnReverseEngineer;
	std::bitset<MaxHouseCount> SpyEffect_StolenTechIndex_result;
	Valueable<int> SpyEffect_StolenMoneyAmount;
	Valueable<float> SpyEffect_StolenMoneyPercentage;
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
	Valueable<int> Solid_Height;
	Valueable<int> Solid_Level;
	Nullable<bool> AIBaseNormal;
	Nullable<bool> AIInnerBase;
	NullableIdx<VocClass> GateDownSound;
	NullableIdx<VocClass> GateUpSound;
	Nullable<bool> UnitSell;
	NullableVector<AnimTypeClass*> DamageFireTypes;
	NullableVector<AnimTypeClass*> OnFireTypes;
	NullableVector<int> OnFireIndex;
	HealthOnFireData HealthOnfire;
	Valueable<BuildingTypeClass*> RubbleIntact;
	Valueable<BuildingTypeClass*> RubbleDestroyed;
	Valueable<AnimTypeClass*> RubbleDestroyedAnim;
	Valueable<AnimTypeClass*> RubbleIntactAnim;
	Valueable<OwnerHouseKind> RubbleDestroyedOwner;
	Valueable<OwnerHouseKind> RubbleIntactOwner;
	Valueable<int> RubbleDestroyedStrength;
	Valueable<int> RubbleIntactStrength;
	Valueable<bool> RubbleDestroyedRemove;
	Valueable<bool> RubbleIntactRemove;
	Valueable<bool> RubbleIntactConsumeEngineer;
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
	std::vector<AnimTypeClass*> GarrisonAnim_idle;
	std::vector<AnimTypeClass*> GarrisonAnim_ActiveOne;
	std::vector<AnimTypeClass*> GarrisonAnim_ActiveTwo;
	std::vector<AnimTypeClass*> GarrisonAnim_ActiveThree;
	std::vector<AnimTypeClass*> GarrisonAnim_ActiveFour;
	CustomPalette PipShapes01Palette;
	Valueable<bool> PipShapes01Remap;
	Nullable<AnimTypeClass*> TurretAnim_LowPower;
	Nullable<AnimTypeClass*> TurretAnim_DamagedLowPower;
	Valueable<bool> BuildUp_UseNormalLIght;
	Valueable<bool> Power_DegradeWithHealth;
	Nullable<float> AutoSellTime;
	Valueable<bool> IsJuggernaut;
	Nullable<SHPStruct*> BuildingPlacementGrid_Shape;
	BuildSpeedBonus SpeedBonus;
	CustomPalette RubblePalette;
	ValueableVector<FacingType> DockPoseDir;
	Nullable<bool> EngineerRepairable;
	signed int IsTrench;
	ValueableIdx<TunnelTypeClass> TunnelType;
	Valueable<double> UCPassThrough;
	Valueable<double> UCFatalRate;
	Valueable<double> UCDamageMultiplier;
	ValueableIdx<CursorTypeClass> Cursor_Spy;
	ValueableIdx<CursorTypeClass> Cursor_Sabotage;
	Nullable<bool> ImmuneToSaboteurs;
	Valueable<bool> ReverseEngineersVictims;
	Valueable<bool> ReverseEngineersVictims_Passengers;
	Valueable<double> LightningRod_Modifier;
	Nullable<bool> Returnable;
	Nullable<double> BuildupTime;
	Nullable<double> SellTime;
	NullableIdx<VocClass> SlamSound;
	Valueable<bool> Destroyed_CreateSmudge;
	ValueableVector<InfantryTypeClass*> AllowedOccupiers;
	ValueableVector<InfantryTypeClass*> DisallowedOccupiers;
	Valueable<bool> BunkerRaidable;
	Valueable<bool> Firestorm_Wall;
	NullableIdx<VocClass> AbandonedSound;
	Valueable<bool> CloningFacility;
	Valueable<bool> Factory_ExplicitOnly;
	ValueableIdx<VoxClass> LostEvaEvent;
	Valueable<CSFText> MessageCapture;
	Valueable<CSFText> MessageLost;
	Nullable<PartialVector3D<int>> AIBuildCounts;
	Nullable<PartialVector3D<int>> AIExtraCounts;
	Nullable<FacingType> LandingDir;
	int SellFrames;
	bool IsCustom;
	int CustomWidth;
	int CustomHeight;
	int OutlineLength;
	std::vector<CellStruct> CustomData;
	std::vector<CellStruct> OutlineData;
	DynamicVectorClass<Point2D> FoundationRadarShape;
	NullableVector<TechnoTypeClass*> Secret_Boons;
	Valueable<bool> Secret_RecalcOnCapture;
	mutable OptionalStruct<bool> Academy;
	ValueableVector<TechnoTypeClass*> AcademyWhitelist;
	ValueableVector<TechnoTypeClass*> AcademyBlacklist;
	Valueable<double> AcademyInfantry;
	Valueable<double> AcademyAircraft;
	Valueable<double> AcademyVehicle;
	Valueable<double> AcademyBuilding;
	Nullable<int> DegradeAmount;
	Nullable<double> DegradePercentage;
	Valueable<bool> IsPassable;
	Valueable<bool> ProduceCashDisplay;
	Nullable<bool> Storage_ActiveAnimations;
	Nullable<float> PurifierBonus;
	Valueable<bool> PurifierBonus_RequirePower;
	Valueable<bool> FactoryPlant_RequirePower;
	Valueable<bool> SpySat_RequirePower;
	Valueable<bool> Cloning_RequirePower;
	Valueable<bool> Radar_RequirePower;
	Nullable<bool> DisplayIncome;
	Nullable<AffectedHouse> DisplayIncome_Houses;
	Valueable<Point2D> DisplayIncome_Offset;
	Valueable<unsigned int> FreeUnit_Count;
	Valueable<bool> SpawnCrewOnlyOnce;
	Valueable<bool> IsDestroyableObstacle;
	ValueableIdx<VoxClass> EVA_Online;
	ValueableIdx<VoxClass> EVA_Offline;
	Valueable<bool> Explodes_DuringBuildup;
	Nullable<int> SpyEffect_SellDelay;
	Valueable<AnimTypeClass*> SpyEffect_Anim;
	Valueable<int> SpyEffect_Anim_Duration;
	Valueable<AffectedHouse> SpyEffect_Anim_DisplayHouses;
	Valueable<bool> SpyEffect_SWTargetCenter;
	Valueable<bool> ShowPower;
	Valueable<bool> EMPulseCannon_UseWeaponSelection;
	ValueableVector<TechnoTypeClass*> FactoryPlant_AllowTypes;
	ValueableVector<TechnoTypeClass*> FactoryPlant_DisallowTypes;
	Valueable<bool> ExcludeFromMultipleFactoryBonus;
	Valueable<bool> NoBuildAreaOnBuildup;
	ValueableVector<BuildingTypeClass*> Adjacent_Allowed;
	ValueableVector<BuildingTypeClass*> Adjacent_Disallowed;
	Nullable<double> Units_RepairRate;
	Nullable<int> Units_RepairStep;
	Nullable<double> Units_RepairPercent;
	Nullable<bool> Units_UseRepairCost;
	Valueable<double> PowerPlant_DamageFactor;
	Valueable<BuildingTypeClass*> NextBuilding_Prev;
	Valueable<BuildingTypeClass*> NextBuilding_Next;
	int NextBuilding_CurrentHeapId;
	Nullable<Point2D> BarracksExitCell;
	Nullable<bool> AutoBuilding;
	Valueable<int> AutoBuilding_Gap;
	Valueable<bool> LimboBuild;
	Valueable<int> LimboBuildID;
	Valueable<BuildingTypeClass*> LaserFencePost_Fence;
	Valueable<BuildingTypeClass*> PlaceBuilding_OnLand;
	Valueable<BuildingTypeClass*> PlaceBuilding_OnWater;
	Nullable<bool> Cameo_ShouldCount;
	Valueable<bool> IsAnimDelayedBurst;
	Valueable<bool> AllowAlliesRepair;
	Valueable<bool> AllowRepairFlyMZone;
	Valueable<int> Overpower_KeepOnline;
	Valueable<int> Overpower_ChargeWeapon;
	Valueable<bool> NewEvaVoice;
	Nullable<int> NewEvaVoice_Index;
	Valueable<int> NewEvaVoice_Priority;
	Valueable<bool> NewEvaVoice_RecheckOnDeath;
	ValueableIdx<VoxClass> NewEvaVoice_InitialMessage;
	Valueable<bool> BattlePointsCollector;
	Valueable<bool> BattlePointsCollector_RequirePower;
	NullableIdx<VocClass> BuildingRepairedSound;
	Valueable<bool> Refinery_UseNormalActiveAnim;
	ValueableVector<bool> HasPowerUpAnim;
	Valueable<bool> AggressiveModeExempt;
	Valueable<bool> IsBarGate;
	Valueable<bool> IsHideDuringSpecialAnim;

	Nullable<bool> AISellCapturedBuilding;

#pragma endregion

public:

	BuildingTypeExtData(BuildingTypeClass* pObj)
		: TechnoTypeExtData(pObj),
		PrismForwarding(),
		PowersUp_Owner(AffectedHouse::Owner),
		PowersUp_Buildings(),
		SuperWeapons(),
		PowerPlantEnhancer_Buildings(),
		PowerPlantEnhancer_Amount(0),
		PowerPlantEnhancer_Factor(1.0f),
		OccupierMuzzleFlashes(),
		Refinery_UseStorage(false),
		Grinding_AllowAllies(false),
		Grinding_AllowOwner(true),
		Grinding_AllowTypes(),
		Grinding_DisallowTypes(),
		Grinding_Sound(),
		Grinding_Weapon(nullptr),
		Grinding_PlayDieSound(true),
		Grinding_Weapon_RequiredCredits(0),
		PlacementPreview_Show(),
		PlacementPreview_Shape(),
		PlacementPreview_ShapeFrame(),
		PlacementPreview_Offset({ 0, -15, 1 }),
		PlacementPreview_Remap(true),
		PlacementPreview_Palette(CustomPalette::PaletteMode::Temperate),
		PlacementPreview_TranslucentLevel(),
		RadialIndicator_Visibility(),
		SpyEffect_Custom(false),
		SpyEffect_VictimSuperWeapon(),
		SpyEffect_InfiltratorSuperWeapon(),
		SpyEffect_InfiltratorSW_JustGrant(false),
		SpyEffect_VictimSW_RealLaunch(false),
		SpyEffect_RevealProduction(false),
		SpyEffect_ResetSW(false),
		SpyEffect_ResetRadar(false),
		SpyEffect_RevealRadar(false),
		SpyEffect_RevealRadarPersist(false),
		SpyEffect_GainVeterancy(false),
		SpyEffect_UnReverseEngineer(false),
		SpyEffect_StolenTechIndex_result(),
		SpyEffect_StolenMoneyAmount(0),
		SpyEffect_StolenMoneyPercentage(0),
		SpyEffect_PowerOutageDuration(0),
		SpyEffect_SabotageDelay(0),
		SpyEffect_SuperWeapon(nullptr),
		SpyEffect_SuperWeaponPermanent(false),
		SpyEffect_InfantryVeterancy(false),
		SpyEffect_VehicleVeterancy(false),
		SpyEffect_NavalVeterancy(false),
		SpyEffect_AircraftVeterancy(false),
		SpyEffect_BuildingVeterancy(false),
		ZShapePointMove_OnBuildup(false),
		SellBuildupLength(23),
		CanC4_AllowZeroDamage(false),
		C4_Modifier(1.0),
		DockUnload_Cell({ 3, 1 }),
		DockUnload_Facing(),
		Solid_Height(0),
		Solid_Level(1),
		AIBaseNormal(),
		AIInnerBase(),
		GateDownSound(),
		GateUpSound(),
		UnitSell(),
		DamageFireTypes(),
		OnFireTypes(),
		OnFireIndex(),
		HealthOnfire(),
		RubbleIntact(nullptr),
		RubbleDestroyed(nullptr),
		RubbleDestroyedAnim(nullptr),
		RubbleIntactAnim(nullptr),
		RubbleDestroyedOwner(OwnerHouseKind::Default),
		RubbleIntactOwner(OwnerHouseKind::Default),
		RubbleDestroyedStrength(0),
		RubbleIntactStrength(-1),
		RubbleDestroyedRemove(false),
		RubbleIntactRemove(false),
		RubbleIntactConsumeEngineer(false),
		DamageFire_Offs(),
		RepairRate(),
		RepairStep(),
		PlayerReturnFire(),
		PackupSound_PlayGlobal(false),
		DisableDamageSound(false),
		BuildingOccupyDamageMult(),
		BuildingOccupyROFMult(),
		BuildingBunkerDamageMult(),
		BuildingBunkerROFMult(),
		BunkerWallsUpSound(),
		BunkerWallsDownSound(),
		AIBuildInsteadPerDiff(),
		GarrisonAnim_idle(),
		GarrisonAnim_ActiveOne(),
		GarrisonAnim_ActiveTwo(),
		GarrisonAnim_ActiveThree(),
		GarrisonAnim_ActiveFour(),
		PipShapes01Palette(CustomPalette::PaletteMode::Temperate),
		PipShapes01Remap(false),
		TurretAnim_LowPower(),
		TurretAnim_DamagedLowPower(),
		BuildUp_UseNormalLIght(false),
		Power_DegradeWithHealth(true),
		AutoSellTime(),
		IsJuggernaut(false),
		BuildingPlacementGrid_Shape(),
		SpeedBonus(),
		RubblePalette(CustomPalette::PaletteMode::Temperate),
		DockPoseDir(),
		EngineerRepairable(),
		IsTrench(-1),
		TunnelType(-1),
		UCPassThrough(0.0),
		UCFatalRate(0.0),
		UCDamageMultiplier(1.0),
		Cursor_Spy((int)MouseCursorType::Enter),
		Cursor_Sabotage(93),
		ImmuneToSaboteurs(),
		ReverseEngineersVictims(false),
		ReverseEngineersVictims_Passengers(false),
		LightningRod_Modifier(1.0),
		Returnable(),
		BuildupTime(),
		SellTime(),
		SlamSound(),
		Destroyed_CreateSmudge(true),
		AllowedOccupiers(),
		DisallowedOccupiers(),
		BunkerRaidable(false),
		Firestorm_Wall(false),
		AbandonedSound(),
		CloningFacility(false),
		Factory_ExplicitOnly(false),
		LostEvaEvent(-1),
		MessageCapture(),
		MessageLost(),
		AIBuildCounts(),
		AIExtraCounts(),
		LandingDir(),
		SellFrames(0),
		IsCustom(false),
		CustomWidth(0),
		CustomHeight(0),
		OutlineLength(0),
		CustomData(),
		OutlineData(),
		FoundationRadarShape(),
		Secret_Boons(),
		Secret_RecalcOnCapture(false),
		Academy(),
		AcademyWhitelist(),
		AcademyBlacklist(),
		AcademyInfantry(0.0),
		AcademyAircraft(0.0),
		AcademyVehicle(0.0),
		AcademyBuilding(0.0),
		DegradeAmount(),
		DegradePercentage(),
		IsPassable(false),
		ProduceCashDisplay(false),
		Storage_ActiveAnimations(),
		PurifierBonus(),
		PurifierBonus_RequirePower(false),
		FactoryPlant_RequirePower(false),
		SpySat_RequirePower(false),
		Cloning_RequirePower(false),
		Radar_RequirePower(true),
		DisplayIncome(),
		DisplayIncome_Houses(),
		DisplayIncome_Offset({ 0, 0 }),
		FreeUnit_Count(1),
		SpawnCrewOnlyOnce(true),
		IsDestroyableObstacle(false),
		EVA_Online(-1),
		EVA_Offline(-1),
		Explodes_DuringBuildup(true),
		SpyEffect_SellDelay(),
		SpyEffect_Anim(nullptr),
		SpyEffect_Anim_Duration(-1),
		SpyEffect_Anim_DisplayHouses(AffectedHouse::All),
		SpyEffect_SWTargetCenter(false),
		ShowPower(true),
		EMPulseCannon_UseWeaponSelection(false),
		FactoryPlant_AllowTypes(),
		FactoryPlant_DisallowTypes(),
		ExcludeFromMultipleFactoryBonus(false),
		NoBuildAreaOnBuildup(false),
		Adjacent_Allowed(),
		Adjacent_Disallowed(),
		Units_RepairRate(),
		Units_RepairStep(),
		Units_RepairPercent(),
		Units_UseRepairCost(),
		PowerPlant_DamageFactor(1.0),
		NextBuilding_Prev(nullptr),
		NextBuilding_Next(nullptr),
		NextBuilding_CurrentHeapId(-1),
		BarracksExitCell(),
		AutoBuilding(),
		AutoBuilding_Gap(1),
		LimboBuild(false),
		LimboBuildID(-1),
		LaserFencePost_Fence(nullptr),
		PlaceBuilding_OnLand(nullptr),
		PlaceBuilding_OnWater(nullptr),
		Cameo_ShouldCount(),
		IsAnimDelayedBurst(true),
		AllowAlliesRepair(false),
		AllowRepairFlyMZone(false),
		Overpower_KeepOnline(2),
		Overpower_ChargeWeapon(1),
		NewEvaVoice(false),
		NewEvaVoice_Index(),
		NewEvaVoice_Priority(0),
		NewEvaVoice_RecheckOnDeath(false),
		NewEvaVoice_InitialMessage(-1),
		BattlePointsCollector(false),
		BattlePointsCollector_RequirePower(false),
		BuildingRepairedSound(),
		Refinery_UseNormalActiveAnim(false),
		HasPowerUpAnim(),
		AggressiveModeExempt(false),
		IsBarGate(false),
		IsHideDuringSpecialAnim(false),
		AISellCapturedBuilding()
	{
		this->Initialize();
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

	virtual BuildingTypeClass* This() const override { return reinterpret_cast<BuildingTypeClass*>(this->TechnoTypeExtData::This()); }
	virtual const BuildingTypeClass* This_Const() const override { return reinterpret_cast<const BuildingTypeClass*>(this->TechnoTypeExtData::This_Const()); }

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

	static std::vector<std::string> trenchKinds; //!< Vector of strings associating known trench names with IsTrench IDs. \sa IsTrench
	static const DirStruct DefaultJuggerFacing;
	static const Foundation CustomFoundation = static_cast<Foundation>(0x7F);
	static const CellStruct FoundationEndMarker;
};

class BuildingTypeExtContainer final : public Container<BuildingTypeExtData>
{
public:
	static BuildingTypeExtContainer Instance;

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

class NOVTABLE FakeBuildingTypeClass : public BuildingTypeClass
{
public:

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, BOOL clearDirty);

	bool _CanUseWaypoint();

	BuildingTypeExtData* _GetExtData() {
		return *reinterpret_cast<BuildingTypeExtData**>(((DWORD)this) + AbstractExtOffset);
	}

	bool _ReadFromINI(CCINIClass* pINI);
};
static_assert(sizeof(FakeBuildingTypeClass) == sizeof(BuildingTypeClass), "Invalid Size !");