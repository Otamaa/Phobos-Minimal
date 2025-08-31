#pragma once
#include <BuildingTypeClass.h>
#include <SuperClass.h>
#include <SuperWeaponTypeClass.h>


#include <Helpers/Macro.h>
#include <Utilities/PhobosMap.h>
#include <Utilities/TemplateDef.h>

#include <DirStruct.h>

#include <Ext/TechnoType/Body.h>

#include <New/AnonymousType/BuildSpeedBonus.h>

#include <New/Type/TunnelTypeClass.h>
#include <New/Type/CursorTypeClass.h>

#include <New/AnonymousType/PrismForwardingData.h>

#include <Misc/Defines.h>

enum class BunkerSoundMode : int
{
	Up, Down
};

class SuperClass;
class BuildingTypeExtData final : public TechnoTypeExtData
{
public:

	using base_type = BuildingTypeClass;

public:

#pragma region ClassMembers

	PrismForwardingData PrismForwarding {};
	Valueable<AffectedHouse> PowersUp_Owner { AffectedHouse::Owner };
	ValueableVector<BuildingTypeClass*> PowersUp_Buildings {};
	ValueableIdxVector<SuperWeaponTypeClass> SuperWeapons {};
	ValueableVector<BuildingTypeClass*> PowerPlantEnhancer_Buildings {};
	Valueable<int> PowerPlantEnhancer_Amount { 0 };
	Valueable<float> PowerPlantEnhancer_Factor { 1.0f };
	std::vector<Point2D> OccupierMuzzleFlashes {};
	Valueable<bool> Refinery_UseStorage { false };
	Valueable<bool> Grinding_AllowAllies { false };
	Valueable<bool> Grinding_AllowOwner { true };
	ValueableVector<TechnoTypeClass*> Grinding_AllowTypes {};
	ValueableVector<TechnoTypeClass*> Grinding_DisallowTypes {};
	NullableIdx<VocClass> Grinding_Sound {};
	Valueable<WeaponTypeClass*> Grinding_Weapon { nullptr };
	Valueable<bool> Grinding_PlayDieSound { true };
	Valueable<int> Grinding_Weapon_RequiredCredits { 0 };
	Nullable<bool> PlacementPreview_Show { };
	Nullable<Theater_SHPStruct*> PlacementPreview_Shape {};
	Nullable<int> PlacementPreview_ShapeFrame {};
	Valueable<CoordStruct> PlacementPreview_Offset { {0, -15, 1} };
	Valueable<bool> PlacementPreview_Remap { true };
	CustomPalette PlacementPreview_Palette { CustomPalette::PaletteMode::Temperate };
	Nullable<int> PlacementPreview_TranslucentLevel {};
	Nullable<AffectedHouse> RadialIndicator_Visibility {};
	Valueable<bool> SpyEffect_Custom { false };
	NullableIdx<SuperWeaponTypeClass> SpyEffect_VictimSuperWeapon {};
	NullableIdx<SuperWeaponTypeClass> SpyEffect_InfiltratorSuperWeapon {};
	Valueable<bool> SpyEffect_InfiltratorSW_JustGrant { false };
	Valueable<bool> SpyEffect_VictimSW_RealLaunch { false };
	Valueable<bool> SpyEffect_RevealProduction { false };
	Valueable<bool> SpyEffect_ResetSW { false };
	Valueable<bool> SpyEffect_ResetRadar { false };
	Valueable<bool> SpyEffect_RevealRadar { false };
	Valueable<bool> SpyEffect_RevealRadarPersist { false };
	Valueable<bool> SpyEffect_GainVeterancy { false };
	Valueable<bool> SpyEffect_UnReverseEngineer { false };
	std::bitset<MaxHouseCount> SpyEffect_StolenTechIndex_result {};
	Valueable<int> SpyEffect_StolenMoneyAmount { 0 };
	Valueable<float> SpyEffect_StolenMoneyPercentage { 0 };
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
	Valueable<int> Solid_Height { 0 };
	Valueable<int> Solid_Level { 1 };
	Nullable<bool> AIBaseNormal { };
	Nullable<bool> AIInnerBase { };
	NullableIdx<VocClass> GateDownSound {};
	NullableIdx<VocClass> GateUpSound {};
	Nullable<bool> UnitSell {};
	NullableVector<AnimTypeClass*> DamageFireTypes {};
	NullableVector<AnimTypeClass*> OnFireTypes {};
	NullableVector<int> OnFireIndex {};
	HealthOnFireData HealthOnfire {};
	Valueable<BuildingTypeClass*> RubbleIntact { nullptr };
	Valueable<BuildingTypeClass*> RubbleDestroyed { nullptr };
	Valueable<AnimTypeClass*> RubbleDestroyedAnim { nullptr };
	Valueable<AnimTypeClass*> RubbleIntactAnim { nullptr };
	Valueable<OwnerHouseKind> RubbleDestroyedOwner { OwnerHouseKind::Default };
	Valueable<OwnerHouseKind> RubbleIntactOwner { OwnerHouseKind::Default };
	Valueable<int> RubbleDestroyedStrength { 0 };
	Valueable<int> RubbleIntactStrength { -1 };
	Valueable<bool> RubbleDestroyedRemove { false };
	Valueable<bool> RubbleIntactRemove { false };
	Valueable<bool> RubbleIntactConsumeEngineer { false };
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
	std::vector<AnimTypeClass*> GarrisonAnim_idle {};
	std::vector<AnimTypeClass*> GarrisonAnim_ActiveOne {};
	std::vector<AnimTypeClass*> GarrisonAnim_ActiveTwo {};
	std::vector<AnimTypeClass*> GarrisonAnim_ActiveThree {};
	std::vector<AnimTypeClass*> GarrisonAnim_ActiveFour {};
	CustomPalette PipShapes01Palette { CustomPalette::PaletteMode::Temperate };
	Valueable<bool> PipShapes01Remap { false };
	Nullable<AnimTypeClass*> TurretAnim_LowPower {};
	Nullable<AnimTypeClass*> TurretAnim_DamagedLowPower {};
	Valueable<bool> BuildUp_UseNormalLIght {};
	Valueable<bool> Power_DegradeWithHealth { true };
	Nullable<float> AutoSellTime {};
	Valueable<bool> IsJuggernaut { false };
	Nullable<SHPStruct*> BuildingPlacementGrid_Shape {};
	BuildSpeedBonus SpeedBonus {};
	CustomPalette RubblePalette { CustomPalette::PaletteMode::Temperate };
	ValueableVector<FacingType> DockPoseDir {};
	Nullable<bool> EngineerRepairable {};
	signed int IsTrench { -1 };
	ValueableIdx<TunnelTypeClass> TunnelType { -1 };
	Valueable<double> UCPassThrough { 0.0 };
	Valueable<double> UCFatalRate { 0.0 };
	Valueable<double> UCDamageMultiplier { 1.0 };
	ValueableIdx<CursorTypeClass> Cursor_Spy { (int)MouseCursorType::Enter };
	ValueableIdx<CursorTypeClass> Cursor_Sabotage { 93 };
	Nullable<bool> ImmuneToSaboteurs {};
	Valueable<bool> ReverseEngineersVictims { false };
	Valueable<bool> ReverseEngineersVictims_Passengers { false };
	Valueable<double> LightningRod_Modifier { 1.0 };
	Nullable<bool> Returnable {};
	Nullable<double> BuildupTime {};
	Nullable<double> SellTime {};
	NullableIdx<VocClass> SlamSound {};
	Valueable<bool> Destroyed_CreateSmudge { true };
	ValueableVector<InfantryTypeClass*> AllowedOccupiers {};
	ValueableVector<InfantryTypeClass*> DisallowedOccupiers {};
	Valueable<bool> BunkerRaidable { false };
	Valueable<bool> Firestorm_Wall { false };
	NullableIdx<VocClass> AbandonedSound {};
	Valueable<bool> CloningFacility { false };
	Valueable<bool> Factory_ExplicitOnly { false };
	ValueableIdx<VoxClass> LostEvaEvent { -1 };
	Valueable<CSFText> MessageCapture {};
	Valueable<CSFText> MessageLost {};
	Nullable<PartialVector3D<int>> AIBuildCounts {};
	Nullable<PartialVector3D<int>> AIExtraCounts {};
	Nullable<FacingType> LandingDir {};
	int SellFrames { 0 };
	bool IsCustom { false };
	int CustomWidth { 0 };
	int CustomHeight { 0 };
	int OutlineLength { 0 };
	std::vector<CellStruct> CustomData {};
	std::vector<CellStruct> OutlineData {};
	DynamicVectorClass<Point2D> FoundationRadarShape {};
	NullableVector<TechnoTypeClass*> Secret_Boons {};
	Valueable<bool> Secret_RecalcOnCapture { false };
	mutable OptionalStruct<bool> Academy {};
	ValueableVector<TechnoTypeClass*> AcademyWhitelist {};
	ValueableVector<TechnoTypeClass*> AcademyBlacklist {};
	Valueable<double> AcademyInfantry { 0.0 };
	Valueable<double> AcademyAircraft { 0.0 };
	Valueable<double> AcademyVehicle { 0.0 };
	Valueable<double> AcademyBuilding { 0.0 };
	Nullable<int> DegradeAmount {};
	Nullable<double> DegradePercentage {};
	Valueable<bool> IsPassable { false };
	Valueable<bool> ProduceCashDisplay { false };
	Nullable<bool> Storage_ActiveAnimations {};
	Nullable<float> PurifierBonus {};
	Valueable<bool> PurifierBonus_RequirePower { false };
	Valueable<bool> FactoryPlant_RequirePower { false };
	Valueable<bool> SpySat_RequirePower { false };
	Valueable<bool> Cloning_RequirePower { false };
	Valueable<bool> Radar_RequirePower { true };
	Nullable<bool> DisplayIncome {};
	Nullable<AffectedHouse> DisplayIncome_Houses {};
	Valueable<Point2D> DisplayIncome_Offset {};
	Valueable<unsigned int> FreeUnit_Count { 1 };
	Valueable<bool> SpawnCrewOnlyOnce { true };
	Valueable<bool> IsDestroyableObstacle { false };
	ValueableIdx<VoxClass> EVA_Online { -1 };
	ValueableIdx<VoxClass> EVA_Offline { -1 };
	Valueable<bool> Explodes_DuringBuildup { true };
	Nullable<int> SpyEffect_SellDelay {};
	Valueable<AnimTypeClass*> SpyEffect_Anim {};
	Valueable<int> SpyEffect_Anim_Duration { -1 };
	Valueable<AffectedHouse> SpyEffect_Anim_DisplayHouses { AffectedHouse::All };
	Valueable<bool> SpyEffect_SWTargetCenter { false };
	Valueable<bool> ShowPower { true };
	Valueable<bool> EMPulseCannon_UseWeaponSelection { false };
	ValueableVector<TechnoTypeClass*> FactoryPlant_AllowTypes {};
	ValueableVector<TechnoTypeClass*> FactoryPlant_DisallowTypes {};
	Valueable<bool> ExcludeFromMultipleFactoryBonus { false };
	Valueable<bool> NoBuildAreaOnBuildup {};
	ValueableVector<BuildingTypeClass*> Adjacent_Allowed {};
	ValueableVector<BuildingTypeClass*> Adjacent_Disallowed {};
	Nullable<double> Units_RepairRate {};
	Nullable<int> Units_RepairStep {};
	Nullable<double> Units_RepairPercent {};
	Nullable<bool> Units_UseRepairCost {};
	Valueable<double> PowerPlant_DamageFactor { 1.0 };
	Valueable<BuildingTypeClass*> NextBuilding_Prev { nullptr };
	Valueable<BuildingTypeClass*> NextBuilding_Next { nullptr };
	int NextBuilding_CurrentHeapId { -1 };
	Nullable<Point2D> BarracksExitCell {};
	Nullable<bool> AutoBuilding { };
	Valueable<int> AutoBuilding_Gap { 1 };
	Valueable<bool> LimboBuild { false };
	Valueable<int> LimboBuildID { -1 };
	Valueable<BuildingTypeClass*> LaserFencePost_Fence {};
	Valueable<BuildingTypeClass*> PlaceBuilding_OnLand {};
	Valueable<BuildingTypeClass*> PlaceBuilding_OnWater {};
	Nullable<bool> Cameo_ShouldCount {};
	Valueable<bool> IsAnimDelayedBurst { true };
	Valueable<bool> AllowAlliesRepair { false };
	Valueable<bool> AllowRepairFlyMZone { false };
	Valueable<int> Overpower_KeepOnline { 2 };
	Valueable<int> Overpower_ChargeWeapon { 1 };
	Valueable<bool> NewEvaVoice {};
	Nullable<int> NewEvaVoice_Index {};
	Valueable<int> NewEvaVoice_Priority { 0 };
	Valueable<bool> NewEvaVoice_RecheckOnDeath {};
	ValueableIdx<VoxClass> NewEvaVoice_InitialMessage {};
	Valueable<bool> BattlePointsCollector {};
	Valueable<bool> BattlePointsCollector_RequirePower {};
	NullableIdx<VocClass> BuildingRepairedSound {};
	Valueable<bool> Refinery_UseNormalActiveAnim { false };
	ValueableVector<bool> HasPowerUpAnim {};
#pragma endregion

public :

	BuildingTypeExtData(BuildingTypeClass* pObj) : TechnoTypeExtData(pObj) {
		this->LostEvaEvent = VoxClass::FindIndexById(GameStrings::EVA_TechBuildingLost());
		this->PrismForwarding.Initialize(This());
		this->EVA_Online = VoxClass::FindIndexById(GameStrings::EVA_BuildingOnLine());
		this->EVA_Offline = VoxClass::FindIndexById(GameStrings::EVA_BuildingOffLine());
		this->NextBuilding_CurrentHeapId = This()->ArrayIndex;
	}
	BuildingTypeExtData(BuildingTypeClass* pObj, noinit_t& nn) : TechnoTypeExtData(pObj, nn) { }

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

	template<BunkerSoundMode UpSound>
	struct BunkerSound
	{
		COMPILETIMEEVAL void operator ()(BuildingClass* pThis)
		{

			if COMPILETIMEEVAL (UpSound == BunkerSoundMode::Up)
			{
				const auto nSound = BuildingTypeExtContainer::Instance.Find(pThis->Type)->BunkerWallsUpSound.Get(RulesClass::Instance->BunkerWallsUpSound);
				VocClass::SafeImmedietelyPlayAt(nSound, pThis->Location);
			}
			else
			{
				const auto nSound = BuildingTypeExtContainer::Instance.Find(pThis->Type)->BunkerWallsDownSound.Get(RulesClass::Instance->BunkerWallsDownSound);
				VocClass::SafeImmedietelyPlayAt(nSound, pThis->Location);
			}
		}
	};

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

	static bool LoadGlobals(PhobosStreamReader& Stm)
	{
		return Stm
			.Process(BuildingTypeExtData::trenchKinds)
			.Success()
			;
	}

	static bool SaveGlobals(PhobosStreamWriter& Stm)
	{
		return Stm
			.Process(BuildingTypeExtData::trenchKinds)
			.Success()
			;
	}

	static void Clear()
	{
		Array.clear();
		BuildingTypeExtData::trenchKinds.clear();
	}

	static void InvalidatePointer(AbstractClass* const ptr, bool bRemoved)
	{
		for (auto& ext : Array)
		{
			ext->InvalidatePointer(ptr, bRemoved);
		}
	}

	virtual bool WriteDataToTheByteStream(BuildingTypeExtData::base_type* key, IStream* pStm) { return true;  };
	virtual bool ReadDataFromTheByteStream(BuildingTypeExtData::base_type* key, IStream* pStm) {  return true; };
};

class NOVTABLE FakeBuildingTypeClass : public BuildingTypeClass
{
public:

	bool _CanUseWaypoint();

	BuildingTypeExtData* _GetExtData() {
		return *reinterpret_cast<BuildingTypeExtData**>(((DWORD)this) + 0x18);
	}
};
static_assert(sizeof(FakeBuildingTypeClass) == sizeof(BuildingTypeClass), "Invalid Size !");