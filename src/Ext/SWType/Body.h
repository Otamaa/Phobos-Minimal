#pragma once
#include <SuperWeaponTypeClass.h>

#include <Utilities/Container.h>
#include <Utilities/PhobosPCXFile.h>
#include <Utilities/PhobosFixedString.h>
#include <Utilities/TemplateDefB.h>

#include <New/Type/CursorTypeClass.h>
#include <New/Type/DroppodProperties.h>

#include <BitFont.h>
#include <Misc/Defines.h>

enum class SWTargetFlags
{
	DisallowEmpty,
	AllowEmpty,
	CheckHousePower
};

enum class CloakHandling
{
	RandomizeCloaked = 0,
	AgnosticToCloak = 1,
	IgnoreCloaked = 2,
	RequireCloaked = 3
};

struct LightingColor
{
	int Red, Green, Blue, Ambient;
	bool HasValue;
};

class ParadropData
{
public:
	Valueable<AircraftTypeClass*> Aircraft;
	ValueableVector<TechnoTypeClass*> Types;
	ValueableVector<int> Num;
};

template <>
struct Savegame::PhobosStreamObject<ParadropData>
{
	bool ReadFromStream(PhobosStreamReader& Stm, ParadropData& Value, bool RegisterForChange) const
	{
		return Stm
			.Process(Value.Aircraft, RegisterForChange)
			.Process(Value.Types, RegisterForChange)
			.Process(Value.Num, RegisterForChange)
			.Success();
	};

	bool WriteToStream(PhobosStreamWriter& Stm, const ParadropData& Value) const
	{
		return Stm
			.Process(Value.Aircraft)
			.Process(Value.Types)
			.Process(Value.Num)
			.Success();
	};
};

struct AITargetingModeInfo
{
	SuperWeaponAITargetingMode Mode;
	SuperWeaponTarget Target;
	AffectedHouse House;
	TargetingConstraints Constraints;
	TargetingPreference Preference;
};

struct TargetResult
{
	CellStruct Target;
	SWTargetFlags Flags;
};

struct TargetingData;
class SuperClass;
class NewSWType;
class ColorScheme;
class SWTypeExtData final
{
public:
	static COMPILETIMEEVAL size_t Canary = 0x11111111;
	using base_type = SuperWeaponTypeClass;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
public:

#pragma region EVAs
	ValueableIdx<VoxClass> EVA_Activated { -1 };
	ValueableIdx<VoxClass> EVA_Ready { -1 };
	ValueableIdx<VoxClass> EVA_Detected { -1 };
	ValueableIdx<VoxClass> EVA_InsufficientFunds { -1 };
	ValueableIdx<VoxClass> EVA_InsufficientBattlePoints { -1 };
	ValueableIdx<VoxClass> EVA_SelectTarget { -1 };
#pragma endregion

#pragma region Messages
	Valueable<CSFText> Message_Launch {};
	Valueable<CSFText> Message_CannotFire {};
	Valueable<CSFText> Message_Activate {};
	Valueable<CSFText> Message_Abort {};
	Valueable<CSFText> Message_InsufficientFunds {};
	Valueable<CSFText> Message_InsufficientBattlePoints {};
	Valueable<CSFText> Message_Impatient {};
	Valueable<CSFText> Message_Detected {};
	Valueable<CSFText> Message_Ready {};

	Valueable<bool> Message_FirerColor { false };
	ValueableIdx<ColorScheme> Message_ColorScheme { -1 };
#pragma endregion

#pragma region Texts
	Valueable<CSFText> Text_Preparing {};
	Valueable<CSFText> Text_Hold {};
	Valueable<CSFText> Text_Ready {};
	Valueable<CSFText> Text_Charging {};
	Valueable<CSFText> Text_Active {};
#pragma endregion

	Valueable<bool> SW_RadarEvent { true };

	Valueable<CSFText> UIDescription {};
	Valueable<int> CameoPriority { 0 };

#pragma region LimboDeliver
	ValueableVector<BuildingTypeClass*> LimboDelivery_Types {};
	ValueableVector<int> LimboDelivery_IDs {};
	ValueableVector<float> LimboDelivery_RollChances {};
	Valueable<AffectedHouse> LimboKill_Affected { AffectedHouse::Owner };
	ValueableVector<int> LimboKill_IDs {};
	std::vector<std::vector<int>> LimboDelivery_RandomWeightsData {};
#pragma endregion

	Valueable<double> RandomBuffer { 0.0 };

#pragma region SWNext
	ValueableIdxVector<SuperWeaponTypeClass*> SW_Next {};
	Valueable<bool> SW_Next_RealLaunch { true };
	Valueable<bool> SW_Next_IgnoreInhibitors { false };
	Valueable<bool> SW_Next_IgnoreDesignators { true };
	ValueableVector<float> SW_Next_RollChances {};
	std::vector<std::vector<int>> SW_Next_RandomWeightsData {};
	std::vector<std::vector<int>> SW_GrantOneTime_RandomWeightsData {};
#pragma endregion

	ValueableVector<TechnoTypeClass*> SW_Inhibitors {};
	Valueable<bool> SW_AnyInhibitor { false };

	ValueableVector<TechnoTypeClass*> SW_Designators {};
	Valueable<bool> SW_AnyDesignator { false };
	Valueable<bool> ShowDesignatorRange { true };

	//Enemy Inhibitors
	ValueableVector<TechnoTypeClass*> SW_Suppressors {};
	Valueable<bool> SW_AnySuppressor {};

	//Enemy Designator
	ValueableVector<TechnoTypeClass*> SW_Attractors {};
	Valueable<bool> SW_AnyAttractor {};

	Valueable<double> SW_RangeMinimum { -1.0 };
	Valueable<double> SW_RangeMaximum { -1.0 };

	IndexBitfield<HouseTypeClass*> SW_RequiredHouses { 0xFFFFFFFFu };
	IndexBitfield<HouseTypeClass*> SW_ForbiddenHouses { 0u };

	ValueableVector<BuildingTypeClass*> SW_AuxBuildings {};
	ValueableVector<BuildingTypeClass*> SW_NegBuildings {};

	Valueable<bool> SW_InitialReady { false };
	Valueable<bool> SW_AlwaysGranted { false };
#pragma region Detonate
	Valueable<WarheadTypeClass*> Detonate_Warhead {};
	Valueable<WeaponTypeClass*> Detonate_Weapon {};
	Nullable<int> Detonate_Damage {};
	Valueable<bool> Detonate_AtFirer { false };
#pragma endregion

#pragma region Otamaa
	Nullable<SHPStruct*> GClock_Shape {};
	Nullable<int> GClock_Transculency {};
	CustomPalette GClock_Palette {}; //CustomPalette::PaletteMode::Default
	Valueable<bool> ChargeTimer { false };
	Valueable<bool> ChargeTimer_Backwards { false };
#pragma endregion

	Valueable<int> SW_Priority { 0 };
	Nullable<int> SW_Damage {};
	ValueableIdx<CursorTypeClass*> CursorType { (int)MouseCursorType::Attack };
	ValueableIdx<CursorTypeClass*> NoCursorType { (int)MouseCursorType::NoMove };
	Valueable<SWRange> SW_Range {};
	Valueable<SuperWeaponAITargetingMode> SW_AITargetingMode { SuperWeaponAITargetingMode::None };
	Valueable<int> SW_Group { 0 };
	Valueable<int> SW_Shots { -1 };
	Valueable<bool> SW_AutoFire { false };
	Valueable<bool> SW_AutoFire_CheckAvail { false };
	Valueable<bool> SW_AllowPlayer { true };
	Valueable<bool> SW_AllowAI { true };
	Nullable<double> SW_ChargeToDrainRatio {};
	Valueable<AffectedHouse> SW_AffectsHouse { AffectedHouse::All };
	Valueable<AffectedHouse> SW_AnimVisibility { AffectedHouse::All };
	Valueable<int> SW_AnimHeight { 0 };
	SuperWeaponType HandledType { SuperWeaponType::Invalid };
	Action LastAction { Action::None };
	Nullable<TargetingConstraints> SW_AITargetingConstrain {};
	Nullable<SuperWeaponTarget> SW_AIRequiresTarget {};
	Nullable<AffectedHouse> SW_AIRequiresHouse {};
	Nullable<TargetingPreference> SW_AITargetingPreference {};
	Valueable<bool> SW_FireToShroud { true };
	Valueable<bool> SW_UseAITargeting { false };
	Valueable<SuperWeaponTarget> SW_RequiresTarget { SuperWeaponTarget::None };
	Valueable<AffectedHouse> SW_RequiresHouse { AffectedHouse::None };
	Nullable<WarheadTypeClass*> SW_Warhead {};
	Nullable<AnimTypeClass*> SW_Anim {};
	NullableIdx<VocClass> SW_Sound {};
	NullableIdx<VocClass> SW_ActivationSound {};
	Nullable<int> SW_Power {};
	Valueable<SuperWeaponTarget> SW_AffectsTarget { SuperWeaponTarget::All };
	PhobosFixedString<0x19> SW_PostDependent {};
	Nullable<int> SW_Deferment {};
	Valueable<int> SW_MaxCount { -1 };
	Valueable<bool> SW_ShowCameo { true };
	Valueable<bool> SW_VirtualCharge { false };
	Valueable<AffectedHouse> SW_TimerVisibility { AffectedHouse::All };
	Valueable<bool> SW_ManualFire { true };
	Valueable<bool> SW_Unstoppable { false };

#pragma region converts
	Valueable<bool> Converts_UseSWRange { false };
	std::vector<TechnoTypeConvertData> ConvertsPair {};
	Valueable<AnimTypeClass*> Convert_SucceededAnim { nullptr };
#pragma endregion

#pragma region Nuke
	Valueable<WeaponTypeClass*> Nuke_Payload { nullptr };
	Valueable<AnimTypeClass*> Nuke_PsiWarning { nullptr };
	Nullable<AnimTypeClass*> Nuke_TakeOff {};
	Valueable<bool> Nuke_SiloLaunch { true };
#pragma endregion

#pragma region SpyPlane
	ValueableVector<AircraftTypeClass*> SpyPlanes_TypeIndex {};
	ValueableVector<int> SpyPlanes_Count {};
	ValueableVector<Mission> SpyPlanes_Mission {};
	ValueableVector<Rank> SpyPlanes_Rank {};
#pragma endregion

#pragma region Battery
	ValueableVector<BuildingTypeClass*> Battery_Overpower {};
	ValueableVector<BuildingTypeClass*> Battery_KeepOnline {};
#pragma endregion

#pragma region Chronosphere
	Nullable<AnimTypeClass*> Chronosphere_BlastSrc {};
	Nullable<AnimTypeClass*> Chronosphere_BlastDest {};
	Valueable<bool> Chronosphere_KillOrganic { true };
	Valueable<bool> Chronosphere_KillTeleporters { true };
	Valueable<bool> Chronosphere_AffectUndeployable { false };
	Valueable<bool> Chronosphere_AffectBuildings { false };
	Valueable<bool> Chronosphere_AffectUnwarpable { false };
	Valueable<bool> Chronosphere_AffectIronCurtain { false };
	Valueable<bool> Chronosphere_BlowUnplaceable { true };
	Valueable<bool> Chronosphere_ReconsiderBuildings { false };
	Nullable<int> Chronosphere_Delay {};
	Valueable<bool> Chronosphere_KillCargo { false };
#pragma endregion

#pragma region Psychic Dominator
	Valueable<bool> Dominator_Capture { true };
	Nullable<int> Dominator_FireAtPercentage {};
	Valueable<int> Dominator_FirstAnimHeight { 1 };
	Valueable<int> Dominator_SecondAnimHeight { 1 };
	Nullable<AnimTypeClass*> Dominator_FirstAnim {};
	Nullable<AnimTypeClass*> Dominator_SecondAnim {};
	Nullable<AnimTypeClass*> Dominator_ControlAnim {};
	Valueable<bool> Dominator_Ripple { true };
	Valueable<bool> Dominator_CaptureMindControlled { true };
	Valueable<bool> Dominator_CapturePermaMindControlled { true };
	Valueable<bool> Dominator_CaptureImmuneToPsionics { false };
	Valueable<bool> Dominator_PermanentCapture { true };
#pragma endregion

#pragma region Drop Pod
	Nullable<int> DropPod_Minimum {};
	Nullable<int> DropPod_Maximum {};
	Valueable<double> DropPod_Veterancy { 2.0 };
	ValueableVector<TechnoTypeClass*> DropPod_Types {};
	Valueable<int> Droppod_RetryCount { 3 };

	DroppodProperties DroppodProp {};
#pragma endregion

#pragma region EMPField
	Valueable<int> EMPField_Duration { 100 };
#pragma endregion

#pragma region EMPulse / Fire
	Valueable<bool> EMPulse_Linked { false };
	Valueable<bool> EMPulse_TargetSelf { false };
	Valueable<int> EMPulse_PulseDelay { 32 };
	Nullable<AnimTypeClass*> EMPulse_PulseBall {};
	ValueableVector<BuildingTypeClass*> EMPulse_Cannons {};
	Valueable<bool> EMPulse_SuspendOthers {};
	Valueable<int> EMPulse_WeaponIndex { 0 };

#pragma endregion

#pragma region Genetic Mutator
	Nullable<bool> Mutate_Explosion {};
	Valueable<bool> Mutate_IgnoreCyborg { false };
	Valueable<bool> Mutate_IgnoreNotHuman { true };
	Valueable<bool> Mutate_KillNatural { true };
#pragma endregion

#pragma region Hunter Seeker
	Nullable<UnitTypeClass*> HunterSeeker_Type {};
	Valueable<bool> HunterSeeker_RandomOnly { false };
	ValueableVector<BuildingTypeClass*> HunterSeeker_Buildings {};
	Valueable<bool> HunterSeeker_AllowAttachedBuildingAsFallback { false };
#pragma endregion

#pragma region Lightning Storm
	Nullable<int> Weather_Duration {};
	Nullable<int> Weather_HitDelay {};
	Nullable<int> Weather_ScatterDelay {};
	Valueable<int> Weather_ScatterCount { 1 };
	Nullable<int> Weather_Separation {};
	Valueable<int> Weather_CloudHeight { -1 };
	Nullable<int> Weather_RadarOutage {};
	Valueable<int> Weather_DebrisMin { 0 };
	Valueable<int> Weather_DebrisMax { 1 };
	Nullable<bool> Weather_PrintText {};
	Valueable<bool> Weather_IgnoreLightningRod {};
	Nullable<AnimTypeClass*> Weather_BoltExplosion {};
	NullableVector<AnimTypeClass*> Weather_Clouds {};
	NullableVector<AnimTypeClass*> Weather_Bolts {};
	NullableVector<AnimTypeClass*> Weather_Debris {};
	NullableIdxVector<VocClass> Weather_Sounds {};
	Valueable<AffectedHouse> Weather_RadarOutageAffects { AffectedHouse::All };

	Valueable<bool> Weather_UseSeparateState { false };
	ValueableVector<BuildingTypeClass*> Weather_LightningRodTypes {};
#pragma endregion

#pragma region  Generic Paradrop
	PhobosMap<AbstractTypeClass*, std::vector<ParadropData>> ParaDropDatas {};
#pragma endregion

#pragma region Generic Protection
	Nullable<int> Protect_Duration {};
	Nullable<int> Protect_PlayFadeSoundTime {};
	Nullable<int> Protect_PowerOutageDuration {};
	Valueable<bool> Protect_IsForceShield {};
#pragma endregion

#pragma region Sonar
	Valueable<int> Sonar_Delay { 0 };
#pragma endregion

#pragma region Unit Delivery
	ValueableVector<TechnoTypeClass*> SW_Deliverables {};
	ValueableVector<int> SW_DeliverableCounts {};
	ValueableVector<FacingType> SW_Deliverables_Facing {};
	Valueable<bool> SW_DeliverBuildups { true };
	Valueable<bool> SW_BaseNormal { true };
	Valueable<OwnerHouseKind> SW_OwnerHouse { OwnerHouseKind::Default };
	Nullable<bool> SW_DeliverableScatter { };
#pragma endregion

#pragma region Lighting
	Nullable<bool> Lighting_Enabled {};
	Nullable<int> Lighting_Ambient {};
	Nullable<int> Lighting_Green {};
	Nullable<int> Lighting_Blue {};
	Nullable<int> Lighting_Red {};
#pragma endregion

#pragma region Money
	Valueable<int> Money_Amount { 0 };
	Valueable<int> Money_DrainAmount { 0 };
	Valueable<int> Money_DrainDelay { 0 };
#pragma endregion

	CustomPalette SidebarPalette {}; //PaletteManager::Mode::Default
	PhobosPCXFile SidebarPCX {};

	ValueableIdxVector<SuperWeaponTypeClass> SW_ResetType {};
	ValueableVector<int> SW_Require {};
	ValueableVector<TechnoTypeClass*> Aux_Techno {};
	ValueableVector<BuildingTypeClass*> SW_Lauchsites {};

#pragma region MeteorShower
	Valueable<int> MeteorCounts { 15 };
	Valueable<int> MeteorImactCounts { 5 };
	Valueable<int> MeteorAddImpactChance { 10 };
	Valueable<int> MeteorKindChance { 30 };
	Valueable<int> MeteorImpactKindChance { 50 };

	Valueable<AnimTypeClass*> MeteorSmall { nullptr };
	Valueable<AnimTypeClass*> MeteorLarge { nullptr };

	Valueable<VoxelAnimTypeClass*> MeteorImpactSmall { nullptr };
	Valueable<VoxelAnimTypeClass*> MeteorImpactLarge { nullptr };
#pragma endregion

#pragma region IonCannon
	Valueable<bool> IonCannon_Ripple { true };
	Valueable<AnimTypeClass*> IonCannon_Blast {};
	Valueable<AnimTypeClass*> IonCannon_Beam {};
	Valueable<int> IonCannon_BlastHeight { 0 };
	Valueable<int> IonCannon_BeamHeight { 0 };
	Valueable<int> IonCannon_FireAtPercentage { 0 };
#pragma endregion

#pragma region LaserStrike
	Valueable<int> LaserStrikeDuration { 1000 };
	Valueable<int> LaserStrikeRadius { 4096 };
	Valueable<int> LaserStrikeMax { 2 };
	Valueable<int> LaserStrikeMin { 1 };
	Valueable<int> LaserStrikeMaxRadius { -1 };
	Valueable<int> LaserStrikeMinRadius { -1 };
	Valueable<int> LaserStrikeRadiusReduce { 20 };
	Valueable<int> LaserStrikeRadiusReduceAcceleration { 0 };
	Valueable<int> LaserStrikeRadiusReduceMax { 0 };
	Valueable<int> LaserStrikeRadiusReduceMin { 0 };
	Valueable<int> LaserStrikeROF { 0 };
	Valueable<int> LaserStrikeScatter_Max { 0 };
	Valueable<int> LaserStrikeScatter_Min { 0 };
	Valueable<int> LaserStrikeScatter_Max_IncreaseMax { 0 };
	Valueable<int> LaserStrikeScatter_Max_IncreaseMin { 0 };
	Valueable<int> LaserStrikeScatter_Max_Increase { 0 };
	Valueable<int> LaserStrikeScatter_Min_IncreaseMax { 0 };
	Valueable<int> LaserStrikeScatter_Min_IncreaseMin { 0 };
	Valueable<int> LaserStrikeScatter_Min_Increase { 0 };
	Valueable<int> LaserStrikeLines { 8 };
	Valueable<int> LaserStrikeAngle { 2 };
	Valueable<int> LaserStrikeAngleAcceleration { 0 };
	Valueable<int> LaserStrikeAngleMax { 0 };
	Valueable<int> LaserStrikeAngleMin { 0 };
	Valueable<WeaponTypeClass*> LaserStrikeZeroRadius_Weapon { nullptr };
	Valueable<ColorStruct> LaserStrikeInnerColor { {255, 0, 0} };
	Valueable<ColorStruct> LaserStrikeOuterColor { {255, 0, 0} };
	Valueable<ColorStruct> LaserStrikeOuterSpread { {255, 0, 0} };
	Valueable<int> LaserStrikeLaserDuration { 3 };
	Valueable<int> LaserStrikeLaserHeight { 20000 };
	Valueable<int> LaserStrikeThickness { 10 };
	Valueable<int> LaserStrikeRate { 0 };
#pragma endregion

#pragma region GenericWarheadSW
	Valueable<bool> Generic_Warhead_Detonate { false };
#pragma endregion

	Valueable<bool> UseWeeds { false };
	Valueable<int> UseWeeds_Amount { RulesClass::Instance->WeedCapacity };
	Valueable<bool> UseWeeds_StorageTimer { false };
	Valueable<int> UseWeeds_ReadinessAnimationPercentage { 90 };

	ValueableIdxVector<SuperWeaponTypeClass> SW_GrantOneTime {};
	Nullable<bool> SW_GrantOneTime_InitialReady {};
	ValueableVector<float> SW_GrantOneTime_RollChances {};
	Valueable<CSFText> Message_GrantOneTimeLaunched {};
	NullableIdx<VoxClass> EVA_GrantOneTimeLaunched {};

	Valueable<bool> CrateGoodies { false };
	Nullable<bool> SuperWeaponSidebar_Allow { };
	DWORD SuperWeaponSidebar_PriorityHouses { 0u };
	DWORD SuperWeaponSidebar_RequiredHouses { 0xFFFFFFFFu };

	Valueable<int> TabIndex { 1 };

	Valueable<int> BattlePoints_Amount {};
	Valueable<int> BattlePoints_DrainAmount { 0 };
	Valueable<int> BattlePoints_DrainDelay { 0 };

	Valueable<int> SuperWeaponSidebar_Significance {};

	~SWTypeExtData() noexcept;

	void FireSuperWeapon(SuperClass* pSW, HouseClass* pHouse, const CellStruct* const pCell, bool IsCurrentPlayer);

	bool IsInhibitor(HouseClass* pOwner, TechnoClass* pTechno) const;
	bool HasInhibitor(HouseClass* pOwner, const CellStruct& Coords) const;
	bool IsInhibitorEligible(HouseClass* pOwner, const CellStruct& Coords, TechnoClass* pTechno) const;
	bool IsDesignator(HouseClass* pOwner, TechnoClass* pTechno) const;
	bool HasDesignator(HouseClass* pOwner, const CellStruct& coords) const;
	bool IsDesignatorEligible(HouseClass* pOwner, const CellStruct& coords, TechnoClass* pTechno) const;
	bool IsLaunchSiteEligible(const CellStruct& Coords, BuildingClass* pBuilding, bool ignoreRange);
	bool IsLaunchSite(BuildingClass* pBuilding) const;
	std::pair<double, double> GetLaunchSiteRange(BuildingClass* pBuilding = nullptr) const;

	void ApplyDetonation(SuperClass* pSW, HouseClass* pHouse, const CellStruct& cell);
	void ApplySWNext(SuperClass* pSW, const CellStruct& cell, bool IsPlayer);

	void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);
	void LoadFromRulesFile(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }
	void Initialize();

	COMPILETIMEEVAL OPTIONALINLINE const char* get_ID(){
		return this->AttachedToObject->ID;
	}

	//with arg(s)

	// check shootamount
	bool CanFire(HouseClass* pOwner) const;
	bool CanFireAt(HouseClass* pOwner, const CellStruct& coords, bool manual);

	bool IsAnimVisible(HouseClass* pFirer) const;
	bool IsHouseAffected(HouseClass* pFirer, HouseClass* pHouse);
	bool IsHouseAffected(HouseClass* pFirer, HouseClass* pHouse, AffectedHouse value);
	bool Launch(NewSWType* pNewType, SuperClass* pSuper, CellStruct const cell, bool const isPlayer);
	void PrintMessage(const CSFText& message, HouseClass* pFirer);
	Iterator<TechnoClass*> GetPotentialAITargets(HouseClass* pTarget , std::vector<TechnoClass*>& outVec) const;
	bool IsCellEligible(CellClass* pCell, SuperWeaponTarget allowed);
	bool IsTechnoEligible(TechnoClass* pTechno, SuperWeaponTarget allowed);
	bool IsTechnoAffected(TechnoClass* pTechno);
	bool IsAvailable(HouseClass* pHouse);

	void UneableToTransactMoney(HouseClass* pHouse);
	void UneableToTransactBattlePoints(HouseClass* pHouse);
	void UneableToFireAtTheMoment(HouseClass* pHouse);

	bool ApplyDrainMoney(int timeLeft, HouseClass* pHouse);
	bool ApplyDrainBattlePoint(int timeLeft, HouseClass* pHouse);

	//no arg(s)
	COMPILETIMEEVAL OPTIONALINLINE double GetChargeToDrainRatio() const {
		return this->SW_ChargeToDrainRatio.Get(RulesClass::Instance->ChargeToDrainRatio);
	}
	SuperWeaponTarget GetAIRequiredTarget() const;
	AffectedHouse GetAIRequiredHouse() const;
	std::pair<TargetingConstraints, bool> GetAITargetingConstraints() const;
	TargetingPreference GetAITargetingPreference() const;
	bool UpdateLightingColor(LightingColor& Lighting) const;
	// is this an original type handled by a NewSWType?

	bool IsTypeRedirected() const;
	bool IsOriginalType() const;
	NewSWType* GetNewSWType() const;

	void GrantOneTimeFromList(SuperClass* pSW);

	//statics
	static bool Deactivate(SuperClass* pSuper, CellStruct const cell, bool const isPlayer);
	static bool Activate(SuperClass* pSuper, CellStruct const cell, bool const isPlayer);
	static AffectedHouse GetRelation(HouseClass* pFirer, HouseClass* pHouse);
	static Action GetAction(SuperWeaponTypeClass* pSuper, CellStruct* pTarget);
	static bool TryFire(SuperClass* pThis, bool IsPlayer);
	static bool IsTargetConstraintsEligible(SuperClass* pThis, bool IsPlayer);
	static TargetResult PickSuperWeaponTarget(NewSWType* pNewType, const TargetingData* pTargeting, const SuperClass* pSuper);
	static void ApplyBattlePoints(SuperClass* pSW);
	static bool IsResourceAvailable(SuperClass* pSuper);
	static bool LauchSuper(SuperClass* pSuper);
	static bool DrawDarken(SuperClass* pSuper);

	COMPILETIMEEVAL FORCEDINLINE static size_t size_Of()
	{
		return sizeof(SWTypeExtData) -
			(4u //AttachedToObject
			 );
	}
private:

	std::vector<int> WeightedRollsHandler(std::vector<float>* chances, std::vector<std::vector<int>>* weights, size_t size);

	void ApplyLimboDelivery(HouseClass* pHouse);
	void ApplyLimboKill(HouseClass* pHouse);

	template <typename T>
	void Serialize(T& Stm);

public:
	static bool Handled;
	static SuperClass* TempSuper;
	static SuperClass* LauchData;
	static std::array<const AITargetingModeInfo, (size_t)SuperWeaponAITargetingMode::count> AITargetingModes;
	static SuperWeaponTypeClass* CurrentSWType;

	static void LimboDeliver(BuildingTypeClass* pType, HouseClass* pOwner, int ID);
	static void WeightedRollsHandler(std::vector<int>& nResult, Valueable<double>& RandomBuffer, const ValueableVector<float>& rolls, const ValueableVector<ValueableVector<int>>& weights, size_t size);
	static void Launch(SuperClass* pFired, HouseClass* pHouse, SWTypeExtData* pLauncherTypeExt, int pLaunchedType, const CellStruct& cell, bool IsPlayer);
	static void ClearChronoAnim(SuperClass* pThis);
	static void CreateChronoAnim(SuperClass* pThis, const CoordStruct& Coords, AnimTypeClass* pAnimType);
	static bool ChangeLighting(SuperWeaponTypeClass* pCustom = nullptr);
	static LightingColor GetLightingColor(SuperWeaponTypeClass* pCustom = nullptr);
};

class SWTypeExtContainer final : public Container<SWTypeExtData>
{
public:
	static SWTypeExtContainer Instance;

public:

	static void InvalidatePointer(AbstractClass* ptr, bool bRemoved);
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
	static void Clear();
};

class NOVTABLE FakeSuperWeaponTypeClass : public SuperWeaponTypeClass
{
public:
	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);

	SWTypeExtData* _GetExtData() {
		return *reinterpret_cast<SWTypeExtData**>((DWORD)this + AbstractExtOffset);
	}

};
static_assert(sizeof(FakeSuperWeaponTypeClass) == sizeof(SuperWeaponTypeClass), "Invalid Size !");
