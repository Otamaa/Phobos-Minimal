#pragma once
#include <BuildingTypeClass.h>
#include <SuperClass.h>
#include <SuperWeaponTypeClass.h>

#include <Helpers/Macro.h>

#include <Utilities/PhobosMap.h>
#include <Utilities/TemplateDef.h>

#include <Ext/TechnoType/Body.h>

#include <New/Entity/BuildSpeedBonus.h>
#include <New/Entity/TheaterSpecificSHP.h>
#include <New/Entity/PrismForwardingData.h>

#include <New/Type/TunnelTypeClass.h>
#include <New/Type/CursorTypeClass.h>

#include <Misc/Defines.h>

#include <DirStruct.h>

class SuperClass;
class BuildingTypeExtData final : public TechnoTypeExtData
{
public:
	using base_type = BuildingTypeClass;
	static COMPILETIMEEVAL const char* ClassName = "BuildingTypeExtData";
	static COMPILETIMEEVAL const char* BaseClassName = "BuildingTypeClass";

public:

#pragma region ClassMembers
	// ============================================================
	// Large aggregates (unknown internal alignment, place first)
	// ============================================================
	PrismForwardingData PrismForwarding {};
	TheaterSpecificSHP PlacementPreview_Shape {};
	CustomPalette PlacementPreview_Palette { CustomPalette::PaletteMode::Temperate };
	CustomPalette PipShapes01Palette { CustomPalette::PaletteMode::Temperate };
	CustomPalette RubblePalette { CustomPalette::PaletteMode::Temperate };
	HealthOnFireData HealthOnfire {};
	BuildSpeedBonus SpeedBonus {};
	std::bitset<MaxHouseCount> SpyEffect_StolenTechIndex_result {};
	DynamicVectorClass<Point2D> FoundationRadarShape {};
	Valueable<CSFText> MessageCapture {};
	Valueable<CSFText> MessageLost {};

	// ============================================================
	// 24-byte aligned: Vectors (grouped together)
	// ============================================================
	ValueableVector<BuildingTypeClass*> PowersUp_Buildings {};
	ValueableIdxVector<SuperWeaponTypeClass> SuperWeapons {};
	ValueableVector<BuildingTypeClass*> PowerPlantEnhancer_Buildings {};
	std::vector<Point2D> OccupierMuzzleFlashes {};
	ValueableVector<TechnoTypeClass*> Grinding_AllowTypes {};
	ValueableVector<TechnoTypeClass*> Grinding_DisallowTypes {};
	NullableVector<AnimTypeClass*> DamageFireTypes {};
	NullableVector<AnimTypeClass*> OnFireTypes {};
	NullableVector<int> OnFireIndex {};
	ValueableVector<Point2D> DamageFire_Offs {};
	ValueableIdxVector<BuildingTypeClass> AIBuildInsteadPerDiff {};
	std::vector<AnimTypeClass*> GarrisonAnim_idle {};
	std::vector<AnimTypeClass*> GarrisonAnim_ActiveOne {};
	std::vector<AnimTypeClass*> GarrisonAnim_ActiveTwo {};
	std::vector<AnimTypeClass*> GarrisonAnim_ActiveThree {};
	std::vector<AnimTypeClass*> GarrisonAnim_ActiveFour {};
	ValueableVector<FacingType> DockPoseDir {};
	ValueableVector<InfantryTypeClass*> AllowedOccupiers {};
	ValueableVector<InfantryTypeClass*> DisallowedOccupiers {};
	std::vector<CellStruct> CustomData {};
	std::vector<CellStruct> OutlineData {};
	NullableVector<TechnoTypeClass*> Secret_Boons {};
	ValueableVector<TechnoTypeClass*> AcademyWhitelist {};
	ValueableVector<TechnoTypeClass*> AcademyBlacklist {};
	ValueableVector<TechnoTypeClass*> FactoryPlant_AllowTypes {};
	ValueableVector<TechnoTypeClass*> FactoryPlant_DisallowTypes {};
	ValueableVector<BuildingTypeClass*> Adjacent_Allowed {};
	ValueableVector<BuildingTypeClass*> Adjacent_Disallowed {};
	ValueableVector<bool> HasPowerUpAnim {};

	// ============================================================
	// Valueable<pointer> (8 bytes each)
	// ============================================================
	Valueable<WeaponTypeClass*> Grinding_Weapon { nullptr };
	Valueable<SuperWeaponTypeClass*> SpyEffect_SuperWeapon { nullptr };
	Valueable<BuildingTypeClass*> RubbleIntact { nullptr };
	Valueable<BuildingTypeClass*> RubbleDestroyed { nullptr };
	Valueable<AnimTypeClass*> RubbleDestroyedAnim { nullptr };
	Valueable<AnimTypeClass*> RubbleIntactAnim { nullptr };
	Valueable<AnimTypeClass*> SpyEffect_Anim { nullptr };
	Valueable<BuildingTypeClass*> NextBuilding_Prev { nullptr };
	Valueable<BuildingTypeClass*> NextBuilding_Next { nullptr };
	Valueable<BuildingTypeClass*> LaserFencePost_Fence { nullptr };
	Valueable<BuildingTypeClass*> PlaceBuilding_OnLand { nullptr };
	Valueable<BuildingTypeClass*> PlaceBuilding_OnWater { nullptr };

	// ============================================================
	// Nullable<pointer> (pointer + bool + padding ≈ 16 bytes)
	// ============================================================
	Nullable<SHPStruct*> BuildingPlacementGrid_Shape {};
	Nullable<AnimTypeClass*> TurretAnim_LowPower {};
	Nullable<AnimTypeClass*> TurretAnim_DamagedLowPower {};

	// ============================================================
	// Valueable<double> (8 bytes each)
	// ============================================================
	Valueable<double> C4_Modifier { 1.0 };
	Valueable<double> UCPassThrough { 0.0 };
	Valueable<double> UCFatalRate { 0.0 };
	Valueable<double> UCDamageMultiplier { 1.0 };
	Valueable<double> LightningRod_Modifier { 1.0 };
	Valueable<double> AcademyInfantry { 0.0 };
	Valueable<double> AcademyAircraft { 0.0 };
	Valueable<double> AcademyVehicle { 0.0 };
	Valueable<double> AcademyBuilding { 0.0 };
	Valueable<double> PowerPlant_DamageFactor { 1.0 };

	// ============================================================
	// Nullable<double> (double + bool + padding ≈ 16 bytes)
	// ============================================================
	Nullable<double> RepairRate {};
	Nullable<double> BuildupTime {};
	Nullable<double> SellTime {};
	Nullable<double> DegradePercentage {};
	Nullable<double> Units_RepairRate {};
	Nullable<double> Units_RepairPercent {};

	// ============================================================
	// Nullable<PartialVector3D<int>> (12+ bytes)
	// ============================================================
	Nullable<PartialVector3D<int>> AIBuildCounts {};
	Nullable<PartialVector3D<int>> AIExtraCounts {};

	// ============================================================
	// Valueable<CoordStruct> (12 bytes)
	// ============================================================
	Valueable<CoordStruct> PlacementPreview_Offset { { 0, -15, 1 } };

	// ============================================================
	// Valueable<Point2D> (8 bytes)
	// ============================================================
	Valueable<Point2D> DisplayIncome_Offset { { 0, 0 } };

	// ============================================================
	// Nullable<Point2D> (8 bytes + bool + padding)
	// ============================================================
	Nullable<Point2D> BarracksExitCell {};

	// ============================================================
	// OptionalStruct
	// ============================================================
	mutable OptionalStruct<bool> Academy {};

	// ============================================================
	// Nullable<float> (float + bool + padding ≈ 8 bytes)
	// ============================================================
	Nullable<float> AutoSellTime {};
	Nullable<float> BuildingOccupyDamageMult {};
	Nullable<float> BuildingOccupyROFMult {};
	Nullable<float> BuildingBunkerDamageMult {};
	Nullable<float> BuildingBunkerROFMult {};
	Nullable<float> PurifierBonus {};

	// ============================================================
	// Nullable<int/enum> (int + bool + padding ≈ 8 bytes)
	// ============================================================
	Nullable<int> PlacementPreview_ShapeFrame {};
	Nullable<int> PlacementPreview_TranslucentLevel {};
	Nullable<int> RepairStep {};
	Nullable<int> DegradeAmount {};
	Nullable<int> SpyEffect_SellDelay {};
	Nullable<int> Units_RepairStep {};
	Nullable<int> NewEvaVoice_Index {};
	Nullable<int> RevealToAll_Radius {};
	Nullable<AffectedHouse> RadialIndicator_Visibility {};
	Nullable<DirType32> DockUnload_Facing {};
	Nullable<AffectedHouse> DisplayIncome_Houses {};
	Nullable<FacingType> LandingDir {};

	// ============================================================
	// NullableIdx (int + bool ≈ 8 bytes)
	// ============================================================
	NullableIdx<VocClass> Grinding_Sound {};
	NullableIdx<SuperWeaponTypeClass> SpyEffect_VictimSuperWeapon {};
	NullableIdx<SuperWeaponTypeClass> SpyEffect_InfiltratorSuperWeapon {};
	NullableIdx<VocClass> GateDownSound {};
	NullableIdx<VocClass> GateUpSound {};
	NullableIdx<VocClass> BunkerWallsUpSound {};
	NullableIdx<VocClass> BunkerWallsDownSound {};
	NullableIdx<VocClass> SlamSound {};
	NullableIdx<VocClass> AbandonedSound {};
	NullableIdx<VocClass> BuildingRepairedSound {};

	// ============================================================
	// Nullable<bool> (bool + bool ≈ 2-4 bytes)
	// ============================================================
	Nullable<bool> PlacementPreview_Show {};
	Nullable<bool> AIBaseNormal {};
	Nullable<bool> AIInnerBase {};
	Nullable<bool> UnitSell {};
	Nullable<bool> PlayerReturnFire {};
	Nullable<bool> EngineerRepairable {};
	Nullable<bool> ImmuneToSaboteurs {};
	Nullable<bool> Returnable {};
	Nullable<bool> Storage_ActiveAnimations {};
	Nullable<bool> DisplayIncome {};
	Nullable<bool> Cameo_ShouldCount {};
	Nullable<bool> AutoBuilding {};
	Nullable<bool> AISellCapturedBuilding {};
	Nullable<bool> BuildingRadioLink_SyncOwner {};
	Nullable<bool> Units_UseRepairCost {};

	// ============================================================
	// Valueable<int> (4 bytes each)
	// ============================================================
	Valueable<int> PowerPlantEnhancer_Amount { 0 };
	Valueable<int> Grinding_Weapon_RequiredCredits { 0 };
	Valueable<int> SpyEffect_StolenMoneyAmount { 0 };
	Valueable<int> SpyEffect_PowerOutageDuration { 0 };
	Valueable<int> SpyEffect_SabotageDelay { 0 };
	Valueable<int> SellBuildupLength { 23 };
	Valueable<int> Solid_Height { 0 };
	Valueable<int> Solid_Level { 1 };
	Valueable<int> RubbleDestroyedStrength { 0 };
	Valueable<int> RubbleIntactStrength { -1 };
	Valueable<int> SpyEffect_Anim_Duration { -1 };
	Valueable<int> FactoryPlant_MaxCount { -1 };
	Valueable<int> Adjacent_Disallowed_ProhibitDistance { 0 };
	Valueable<int> AutoBuilding_Gap { 1 };
	Valueable<int> LimboBuildID { -1 };
	Valueable<int> Overpower_KeepOnline { 2 };
	Valueable<int> Overpower_ChargeWeapon { 1 };
	Valueable<int> NewEvaVoice_Priority { 0 };
	Valueable<int> PowerPlantEnhancer_MaxCount { -1 };

	// ============================================================
	// Valueable<unsigned int> (4 bytes)
	// ============================================================
	Valueable<unsigned int> FreeUnit_Count { 1 };

	// ============================================================
	// Valueable<float> (4 bytes each)
	// ============================================================
	Valueable<float> PowerPlantEnhancer_Factor { 1.0f };
	Valueable<float> SpyEffect_StolenMoneyPercentage { 0.0f };

	// ============================================================
	// ValueableIdx (4 bytes each)
	// ============================================================
	ValueableIdx<TunnelTypeClass> TunnelType { -1 };
	ValueableIdx<CursorTypeClass> Cursor_Spy { (int)MouseCursorType::Enter };
	ValueableIdx<CursorTypeClass> Cursor_Sabotage { 93 };
	ValueableIdx<VoxClass> LostEvaEvent { -1 };
	ValueableIdx<VoxClass> EVA_Online { -1 };
	ValueableIdx<VoxClass> EVA_Offline { -1 };
	ValueableIdx<VoxClass> NewEvaVoice_InitialMessage { -1 };

	// ============================================================
	// Valueable<enum> (4 bytes each)
	// ============================================================
	Valueable<AffectedHouse> PowersUp_Owner { AffectedHouse::Owner };
	Valueable<OwnerHouseKind> RubbleDestroyedOwner { OwnerHouseKind::Default };
	Valueable<OwnerHouseKind> RubbleIntactOwner { OwnerHouseKind::Default };
	Valueable<AffectedHouse> SpyEffect_Anim_DisplayHouses { AffectedHouse::All };

	// ============================================================
	// Valueable<CellStruct> (4 bytes)
	// ============================================================
	Valueable<CellStruct> DockUnload_Cell { { 3, 1 } };

	// ============================================================
	// Plain int (4 bytes each)
	// ============================================================
	signed int IsTrench { -1 };
	int SellFrames { 0 };
	int CustomWidth { 0 };
	int CustomHeight { 0 };
	int OutlineLength { 0 };
	int NextBuilding_CurrentHeapId { -1 };

	// ============================================================
	// Valueable<bool> (1 byte each, packed together)
	// ============================================================
	Valueable<bool> Refinery_UseStorage { false };
	Valueable<bool> Grinding_AllowAllies { false };
	Valueable<bool> Grinding_AllowOwner { true };
	Valueable<bool> Grinding_PlayDieSound { true };
	Valueable<bool> PlacementPreview_Remap { true };
	Valueable<bool> SpyEffect_Custom { false };
	Valueable<bool> SpyEffect_InfiltratorSW_JustGrant { false };
	Valueable<bool> SpyEffect_VictimSW_RealLaunch { false };
	Valueable<bool> SpyEffect_RevealProduction { false };
	Valueable<bool> SpyEffect_ResetSW { false };
	Valueable<bool> SpyEffect_ResetRadar { false };
	Valueable<bool> SpyEffect_RevealRadar { false };
	Valueable<bool> SpyEffect_RevealRadarPersist { false };
	Valueable<bool> SpyEffect_GainVeterancy { false };
	Valueable<bool> SpyEffect_UnReverseEngineer { false };
	Valueable<bool> SpyEffect_SuperWeaponPermanent { false };
	Valueable<bool> SpyEffect_InfantryVeterancy { false };
	Valueable<bool> SpyEffect_VehicleVeterancy { false };
	Valueable<bool> SpyEffect_NavalVeterancy { false };
	Valueable<bool> SpyEffect_AircraftVeterancy { false };
	Valueable<bool> SpyEffect_BuildingVeterancy { false };
	Valueable<bool> ZShapePointMove_OnBuildup { false };
	Valueable<bool> CanC4_AllowZeroDamage { false };
	Valueable<bool> RubbleDestroyedRemove { false };
	Valueable<bool> RubbleIntactRemove { false };
	Valueable<bool> RubbleIntactConsumeEngineer { false };
	Valueable<bool> PackupSound_PlayGlobal { false };
	Valueable<bool> DisableDamageSound { false };
	Valueable<bool> PipShapes01Remap { false };
	Valueable<bool> BuildUp_UseNormalLIght { false };
	Valueable<bool> Power_DegradeWithHealth { true };
	Valueable<bool> IsJuggernaut { false };
	Valueable<bool> ReverseEngineersVictims { false };
	Valueable<bool> ReverseEngineersVictims_Passengers { false };
	Valueable<bool> Destroyed_CreateSmudge { true };
	Valueable<bool> BunkerRaidable { false };
	Valueable<bool> Firestorm_Wall { false };
	Valueable<bool> CloningFacility { false };
	Valueable<bool> Factory_ExplicitOnly { false };
	Valueable<bool> Secret_RecalcOnCapture { false };
	Valueable<bool> IsPassable { false };
	Valueable<bool> ProduceCashDisplay { false };
	Valueable<bool> PurifierBonus_RequirePower { false };
	Valueable<bool> FactoryPlant_RequirePower { false };
	Valueable<bool> SpySat_RequirePower { false };
	Valueable<bool> Cloning_RequirePower { false };
	Valueable<bool> Radar_RequirePower { true };
	Valueable<bool> SpawnCrewOnlyOnce { true };
	Valueable<bool> IsDestroyableObstacle { false };
	Valueable<bool> Explodes_DuringBuildup { true };
	Valueable<bool> SpyEffect_SWTargetCenter { false };
	Valueable<bool> ShowPower { true };
	Valueable<bool> EMPulseCannon_UseWeaponSelection { false };
	Valueable<bool> ExcludeFromMultipleFactoryBonus { false };
	Valueable<bool> NoBuildAreaOnBuildup { false };
	Valueable<bool> LimboBuild { false };
	Valueable<bool> IsAnimDelayedBurst { true };
	Valueable<bool> AllowAlliesRepair { false };
	Valueable<bool> AllowRepairFlyMZone { false };
	Valueable<bool> NewEvaVoice { false };
	Valueable<bool> NewEvaVoice_RecheckOnDeath { false };
	Valueable<bool> BattlePointsCollector { false };
	Valueable<bool> BattlePointsCollector_RequirePower { false };
	Valueable<bool> Refinery_UseNormalActiveAnim { false };
	Valueable<bool> AggressiveModeExempt { false };
	Valueable<bool> IsBarGate { false };
	Valueable<bool> IsHideDuringSpecialAnim { false };
	Valueable<bool> ApplyPerTargetEffectsOnDetonate { true };
	Valueable<bool> Adjacent_Disallowed_Prohibit { false };

	// ============================================================
	// Plain bool (1 byte, at the very end)
	// ============================================================
	bool IsCustom { false };
	// Total bools: 68 Valueable<bool> + 1 plain bool = 69 bytes
	// Pads to 72 for 4-byte alignment

	bool FoundationPowerTextShowLong {};
	bool FoundationPrimaryFactoryTextShowLong {};
#pragma endregion

public:
	BuildingTypeExtData(BuildingTypeClass* pObj) : TechnoTypeExtData(pObj)
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
	
	static std::pair<int, int> GetEnhancedPowerPair(BuildingTypeClass* pBuilding, int output, HouseClass* pHouse);
	static int GetEnhancedPower(BuildingClass* pBuilding, HouseClass* pHouse);
	static int GetEnhancedPower(BuildingTypeClass* pBuilding, int output, HouseClass* pHouse);

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

	static DWORD FoundationLength(CellStruct const* const pFoundation);
	static const std::vector<CellStruct>* GetCoveredCells(BuildingClass* const pThis, CellStruct const mainCoords, int const shadowHeight);
	static void GetDisplayRect(RectangleStruct* out, CellStruct* cells);

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

	virtual bool LoadAll(const PhobosStreamReader& stm) { return true; }
	virtual bool SaveAll(PhobosStreamWriter& stm){ return true; }

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