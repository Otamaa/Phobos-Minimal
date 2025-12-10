#pragma once
#include <SuperWeaponTypeClass.h>

#include <Ext/AbstractType/Body.h>
#include <Utilities/PhobosPCXFile.h>
#include <Utilities/PhobosFixedString.h>

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

	bool load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		return Stm
			.Process(Aircraft, RegisterForChange)
			.Process(Types, RegisterForChange)
			.Process(Num, RegisterForChange)
			.Success();
	}

	bool save(PhobosStreamWriter& Stm) const
	{
		return Stm
			.Process(Aircraft)
			.Process(Types)
			.Process(Num)
			.Success();
	}
};

struct AITargetingModeInfo
{
	SuperWeaponAITargetingMode Mode;
	SuperWeaponTarget Target;
	AffectedHouse House;
	TargetingConstraints Constraints;
	TargetingPreference Preference;

	bool load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		return Stm
			.Process(Mode, RegisterForChange)
			.Process(Target, RegisterForChange)
			.Process(House, RegisterForChange)
			.Process(Constraints, RegisterForChange)
			.Process(Preference, RegisterForChange)
			.Success();
	}

	bool save(PhobosStreamWriter& Stm) const
	{
		return Stm
			.Process(Mode)
			.Process(Target)
			.Process(House)
			.Process(Constraints)
			.Process(Preference)
			.Success();
	}
};

struct TargetResult
{
	CellStruct Target;
	SWTargetFlags Flags;

	bool load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		return Stm
			.Process(Target, RegisterForChange)
			.Process(Flags, RegisterForChange)
			.Success();
	}

	bool save(PhobosStreamWriter& Stm) const
	{
		return Stm
			.Process(Target)
			.Process(Flags)
			.Success();
	}
};

struct TargetingData;
class SuperClass;
class SWTypeHandler;
class ColorScheme;
class SWTypeExtData final :public AbstractTypeExtData
{
public:

	using base_type = SuperWeaponTypeClass;
	static constexpr unsigned Marker = UuidFirstPart<base_type>::value;

public:
#pragma region ClassMembers

#pragma region EVAs
	ValueableIdx<VoxClass> EVA_Activated;
	ValueableIdx<VoxClass> EVA_Ready;
	ValueableIdx<VoxClass> EVA_Detected;
	Valueable<bool> EVA_Detected_Simple;
	ValueableIdx<VoxClass> EVA_InsufficientFunds;
	ValueableIdx<VoxClass> EVA_InsufficientBattlePoints;
	ValueableIdx<VoxClass> EVA_SelectTarget;
#pragma endregion

#pragma region Messages
	Valueable<CSFText> Message_Launch;
	Valueable<CSFText> Message_CannotFire;
	Valueable<CSFText> Message_Activate;
	Valueable<CSFText> Message_Abort;
	Valueable<CSFText> Message_InsufficientFunds;
	Valueable<CSFText> Message_InsufficientBattlePoints;
	Valueable<CSFText> Message_Impatient;
	Valueable<CSFText> Message_Detected;
	Valueable<CSFText> Message_Ready;

	Valueable<bool> Message_FirerColor;
	ValueableIdx<ColorScheme> Message_ColorScheme;
#pragma endregion

#pragma region Texts
	Valueable<CSFText> Text_Preparing;
	Valueable<CSFText> Text_Hold;
	Valueable<CSFText> Text_Ready;
	Valueable<CSFText> Text_Charging;
	Valueable<CSFText> Text_Active;
#pragma endregion

	Valueable<bool> SW_RadarEvent;

	Valueable<CSFText> UIDescription;
	Valueable<int> CameoPriority;

#pragma region LimboDeliver
	ValueableVector<BuildingTypeClass*> LimboDelivery_Types;
	ValueableVector<int> LimboDelivery_IDs;
	ValueableVector<float> LimboDelivery_RollChances;
	Valueable<AffectedHouse> LimboKill_Affected;
	ValueableVector<int> LimboKill_IDs;
	std::vector<std::vector<int>> LimboDelivery_RandomWeightsData;
#pragma endregion

	Valueable<double> RandomBuffer;

#pragma region SWNext
	ValueableIdxVector<SuperWeaponTypeClass*> SW_Next;
	Valueable<bool> SW_Next_RealLaunch;
	Valueable<bool> SW_Next_IgnoreInhibitors;
	Valueable<bool> SW_Next_IgnoreDesignators;
	ValueableVector<float> SW_Next_RollChances;
	std::vector<std::vector<int>> SW_Next_RandomWeightsData;
	std::vector<std::vector<int>> SW_Link_RandomWeightsData;
#pragma endregion

	Valueable<AffectedHouse> SW_Inhibitors_Houses;
	ValueableVector<TechnoTypeClass*> SW_Inhibitors;
	Valueable<bool> SW_AnyInhibitor;

	Valueable<AffectedHouse> SW_Designators_Houses;
	ValueableVector<TechnoTypeClass*> SW_Designators;
	Valueable<bool> SW_AnyDesignator;
	Valueable<bool> ShowDesignatorRange;

	//Enemy Inhibitors
	ValueableVector<TechnoTypeClass*> SW_Suppressors;
	Valueable<bool> SW_AnySuppressor;

	//Enemy Designator
	ValueableVector<TechnoTypeClass*> SW_Attractors;
	Valueable<bool> SW_AnyAttractor;

	Valueable<double> SW_RangeMinimum;
	Valueable<double> SW_RangeMaximum;

	IndexBitfield<HouseTypeClass*> SW_RequiredHouses;
	IndexBitfield<HouseTypeClass*> SW_ForbiddenHouses;

	ValueableVector<BuildingTypeClass*> SW_AuxBuildings;
	ValueableVector<BuildingTypeClass*> SW_NegBuildings;

	Valueable<bool> SW_InitialReady;
	Valueable<bool> SW_AlwaysGranted;
#pragma region Detonate
	Valueable<WarheadTypeClass*> Detonate_Warhead;
	Valueable<WeaponTypeClass*> Detonate_Weapon;
	Nullable<int> Detonate_Damage;
	Valueable<bool> Detonate_AtFirer;
#pragma endregion

#pragma region Otamaa
	Valueable<SHPStruct*> GClock_Shape;
	Valueable<TranslucencyLevel> GClock_Transculency;
	CustomPalette GClock_Palette; //CustomPalette::PaletteMode::Default
	Valueable<bool> ChargeTimer;
	Valueable<bool> ChargeTimer_Backwards;
#pragma endregion

	Valueable<int> SW_Priority;
	Nullable<int> SW_Damage;
	ValueableIdx<CursorTypeClass*> CursorType;
	ValueableIdx<CursorTypeClass*> NoCursorType;
	Valueable<SWRange> SW_Range;
	Valueable<SuperWeaponAITargetingMode> SW_AITargetingMode;
	Valueable<int> SW_Group;
	Valueable<int> SW_Shots;
	Valueable<bool> SW_AutoFire;
	Valueable<bool> SW_AutoFire_CheckAvail;
	Valueable<bool> SW_AllowPlayer;
	Valueable<bool> SW_AllowAI;
	Nullable<double> SW_ChargeToDrainRatio;
	Valueable<AffectedHouse> SW_AffectsHouse;
	Valueable<AffectedHouse> SW_AnimVisibility;
	Valueable<int> SW_AnimHeight;
	NewSuperType HandledType;
	Action LastAction;
	Nullable<TargetingConstraints> SW_AITargetingConstrain;
	Nullable<SuperWeaponTarget> SW_AIRequiresTarget;
	Nullable<AffectedHouse> SW_AIRequiresHouse;
	Nullable<TargetingPreference> SW_AITargetingPreference;
	Valueable<bool> SW_FireToShroud;
	Valueable<bool> SW_UseAITargeting;
	Valueable<SuperWeaponTarget> SW_RequiresTarget;
	Valueable<AffectedHouse> SW_RequiresHouse;
	Nullable<WarheadTypeClass*> SW_Warhead;
	Nullable<AnimTypeClass*> SW_Anim;
	NullableIdx<VocClass> SW_Sound;
	NullableIdx<VocClass> SW_ActivationSound;
	Nullable<int> SW_Power;
	Valueable<SuperWeaponTarget> SW_AffectsTarget;
	PhobosFixedString<0x19> SW_PostDependent;
	Nullable<int> SW_Deferment;
	Valueable<int> SW_MaxCount;
	Valueable<bool> SW_ShowCameo;
	Valueable<bool> SW_VirtualCharge;
	Valueable<AffectedHouse> SW_TimerVisibility;
	Valueable<bool> SW_ManualFire;
	Valueable<bool> SW_Unstoppable;

#pragma region converts
	Valueable<bool> Converts_UseSWRange;
	std::vector<TechnoTypeConvertData> ConvertsPair;
	Valueable<AnimTypeClass*> Convert_SucceededAnim;
#pragma endregion

#pragma region Nuke
	Valueable<WeaponTypeClass*> Nuke_Payload;
	Valueable<AnimTypeClass*> Nuke_PsiWarning;
	Nullable<AnimTypeClass*> Nuke_TakeOff;
	Valueable<bool> Nuke_SiloLaunch;
#pragma endregion

#pragma region SpyPlane
	ValueableVector<AircraftTypeClass*> SpyPlanes_TypeIndex;
	ValueableVector<int> SpyPlanes_Count;
	ValueableVector<Mission> SpyPlanes_Mission;
	ValueableVector<Rank> SpyPlanes_Rank;
#pragma endregion

#pragma region Battery
	ValueableVector<BuildingTypeClass*> Battery_Overpower;
	ValueableVector<BuildingTypeClass*> Battery_KeepOnline;
#pragma endregion

#pragma region Chronosphere
	Nullable<AnimTypeClass*> Chronosphere_BlastSrc;
	Nullable<AnimTypeClass*> Chronosphere_BlastDest;
	Valueable<bool> Chronosphere_KillOrganic;
	Valueable<bool> Chronosphere_KillTeleporters;
	Valueable<bool> Chronosphere_AffectUndeployable;
	Valueable<bool> Chronosphere_AffectBuildings;
	Valueable<bool> Chronosphere_AffectUnwarpable;
	Valueable<bool> Chronosphere_AffectIronCurtain;
	Valueable<bool> Chronosphere_BlowUnplaceable;
	Valueable<bool> Chronosphere_ReconsiderBuildings;
	Nullable<int> Chronosphere_Delay;
	Valueable<bool> Chronosphere_KillCargo;
#pragma endregion

#pragma region Psychic Dominator
	Valueable<bool> Dominator_Capture;
	Nullable<int> Dominator_FireAtPercentage;
	Valueable<int> Dominator_FirstAnimHeight;
	Valueable<int> Dominator_SecondAnimHeight;
	Nullable<AnimTypeClass*> Dominator_FirstAnim;
	Nullable<AnimTypeClass*> Dominator_SecondAnim;
	Nullable<AnimTypeClass*> Dominator_ControlAnim;
	Valueable<bool> Dominator_Ripple;
	Valueable<bool> Dominator_CaptureMindControlled;
	Valueable<bool> Dominator_CapturePermaMindControlled;
	Valueable<bool> Dominator_CaptureImmuneToPsionics;
	Valueable<bool> Dominator_PermanentCapture;
#pragma endregion

#pragma region Drop Pod
	Nullable<int> DropPod_Minimum;
	Nullable<int> DropPod_Maximum;
	Valueable<double> DropPod_Veterancy;
	ValueableVector<TechnoTypeClass*> DropPod_Types;
	Valueable<int> Droppod_RetryCount;

	DroppodProperties DroppodProp;
#pragma endregion

#pragma region EMPField
	Valueable<int> EMPField_Duration;
#pragma endregion

#pragma region EMPulse / Fire
	Valueable<bool> EMPulse_Linked;
	Valueable<bool> EMPulse_TargetSelf;
	Valueable<int> EMPulse_PulseDelay;
	Nullable<AnimTypeClass*> EMPulse_PulseBall;
	ValueableVector<BuildingTypeClass*> EMPulse_Cannons;
	Valueable<bool> EMPulse_SuspendOthers;
	Valueable<int> EMPulse_WeaponIndex;

#pragma endregion

#pragma region Genetic Mutator
	Nullable<bool> Mutate_Explosion;
	Valueable<bool> Mutate_IgnoreCyborg;
	Valueable<bool> Mutate_IgnoreNotHuman;
	Valueable<bool> Mutate_KillNatural;
#pragma endregion

#pragma region Hunter Seeker
	Nullable<UnitTypeClass*> HunterSeeker_Type;
	Valueable<bool> HunterSeeker_RandomOnly;
	ValueableVector<BuildingTypeClass*> HunterSeeker_Buildings;
	Valueable<bool> HunterSeeker_AllowAttachedBuildingAsFallback;
#pragma endregion

#pragma region Lightning Storm
	Nullable<int> Weather_Duration;
	Nullable<int> Weather_HitDelay;
	Nullable<int> Weather_ScatterDelay;
	Valueable<int> Weather_ScatterCount;
	Nullable<int> Weather_Separation;
	Valueable<int> Weather_CloudHeight;
	Nullable<int> Weather_RadarOutage;
	Valueable<int> Weather_DebrisMin;
	Valueable<int> Weather_DebrisMax;
	Nullable<bool> Weather_PrintText;
	Valueable<bool> Weather_IgnoreLightningRod;
	Nullable<AnimTypeClass*> Weather_BoltExplosion;
	NullableVector<AnimTypeClass*> Weather_Clouds;
	NullableVector<AnimTypeClass*> Weather_Bolts;
	NullableVector<AnimTypeClass*> Weather_Debris;
	NullableIdxVector<VocClass> Weather_Sounds;
	Valueable<AffectedHouse> Weather_RadarOutageAffects;

	Valueable<bool> Weather_UseSeparateState;
	ValueableVector<BuildingTypeClass*> Weather_LightningRodTypes;
#pragma endregion

#pragma region  Generic Paradrop
	PhobosMap<AbstractTypeClass*, std::vector<ParadropData>> ParaDropDatas;
#pragma endregion

#pragma region Generic Protection
	Nullable<int> Protect_Duration;
	Nullable<int> Protect_PlayFadeSoundTime;
	Nullable<int> Protect_PowerOutageDuration;
	Valueable<bool> Protect_IsForceShield;
#pragma endregion

#pragma region Sonar
	Valueable<int> Sonar_Delay;
#pragma endregion

#pragma region Unit Delivery
	ValueableVector<TechnoTypeClass*> SW_Deliverables;
	ValueableVector<int> SW_DeliverableCounts;
	ValueableVector<FacingType> SW_Deliverables_Facing;
	Valueable<bool> SW_DeliverBuildups;
	Valueable<bool> SW_BaseNormal;
	Valueable<OwnerHouseKind> SW_OwnerHouse;
	Nullable<bool> SW_DeliverableScatter;
#pragma endregion

#pragma region Lighting
	Nullable<bool> Lighting_Enabled;
	Nullable<int> Lighting_Ambient;
	Nullable<int> Lighting_Green;
	Nullable<int> Lighting_Blue;
	Nullable<int> Lighting_Red;
#pragma endregion

#pragma region Money
	Valueable<int> Money_Amount;
	Valueable<int> Money_DrainAmount;
	Valueable<int> Money_DrainDelay;
#pragma endregion

	CustomPalette SidebarPalette; //PaletteManager::Mode::Default
	PhobosPCXFile SidebarPCX;

	ValueableIdxVector<SuperWeaponTypeClass> SW_ResetType;
	ValueableVector<int> SW_Require;
	ValueableVector<TechnoTypeClass*> Aux_Techno;
	ValueableVector<BuildingTypeClass*> SW_Lauchsites;

#pragma region MeteorShower
	Valueable<int> MeteorCounts;
	Valueable<int> MeteorImactCounts;
	Valueable<int> MeteorAddImpactChance;
	Valueable<int> MeteorKindChance;
	Valueable<int> MeteorImpactKindChance;

	Valueable<AnimTypeClass*> MeteorSmall;
	Valueable<AnimTypeClass*> MeteorLarge;

	Valueable<VoxelAnimTypeClass*> MeteorImpactSmall;
	Valueable<VoxelAnimTypeClass*> MeteorImpactLarge;
#pragma endregion

#pragma region IonCannon
	Valueable<bool> IonCannon_Ripple;
	Valueable<AnimTypeClass*> IonCannon_Blast;
	Valueable<AnimTypeClass*> IonCannon_Beam;
	Valueable<int> IonCannon_BlastHeight;
	Valueable<int> IonCannon_BeamHeight;
	Valueable<int> IonCannon_FireAtPercentage;
#pragma endregion

#pragma region LaserStrike
	Valueable<int> LaserStrikeDuration;
	Valueable<int> LaserStrikeRadius;
	Valueable<int> LaserStrikeMax;
	Valueable<int> LaserStrikeMin;
	Valueable<int> LaserStrikeMaxRadius;
	Valueable<int> LaserStrikeMinRadius;
	Valueable<int> LaserStrikeRadiusReduce;
	Valueable<int> LaserStrikeRadiusReduceAcceleration;
	Valueable<int> LaserStrikeRadiusReduceMax;
	Valueable<int> LaserStrikeRadiusReduceMin;
	Valueable<int> LaserStrikeROF;
	Valueable<int> LaserStrikeScatter_Max;
	Valueable<int> LaserStrikeScatter_Min;
	Valueable<int> LaserStrikeScatter_Max_IncreaseMax;
	Valueable<int> LaserStrikeScatter_Max_IncreaseMin;
	Valueable<int> LaserStrikeScatter_Max_Increase;
	Valueable<int> LaserStrikeScatter_Min_IncreaseMax;
	Valueable<int> LaserStrikeScatter_Min_IncreaseMin;
	Valueable<int> LaserStrikeScatter_Min_Increase;
	Valueable<int> LaserStrikeLines;
	Valueable<int> LaserStrikeAngle;
	Valueable<int> LaserStrikeAngleAcceleration;
	Valueable<int> LaserStrikeAngleMax;
	Valueable<int> LaserStrikeAngleMin;
	Valueable<WeaponTypeClass*> LaserStrikeZeroRadius_Weapon;
	Valueable<ColorStruct> LaserStrikeInnerColor;
	Valueable<ColorStruct> LaserStrikeOuterColor;
	Valueable<ColorStruct> LaserStrikeOuterSpread;
	Valueable<int> LaserStrikeLaserDuration;
	Valueable<int> LaserStrikeLaserHeight;
	Valueable<int> LaserStrikeThickness;
	Valueable<int> LaserStrikeRate;
#pragma endregion

#pragma region GenericWarheadSW
	Valueable<bool> Generic_Warhead_Detonate;
#pragma endregion

	Valueable<bool> UseWeeds;
	Valueable<int> UseWeeds_Amount;
	Valueable<bool> UseWeeds_StorageTimer;
	Valueable<int> UseWeeds_ReadinessAnimationPercentage;

	ValueableIdxVector<SuperWeaponTypeClass> SW_Link;
	Valueable<bool> SW_Link_Grant;
	Valueable<bool> SW_Link_Ready;
	Valueable<bool> SW_Link_Reset;
	ValueableVector<float> SW_Link_RollChances;
	Valueable<CSFText> Message_LinkedSWAcquired;
	NullableIdx<VoxClass> EVA_LinkedSWAcquired;

	Valueable<bool> CrateGoodies;
	Nullable<bool> SuperWeaponSidebar_Allow;
	DWORD SuperWeaponSidebar_PriorityHouses;
	DWORD SuperWeaponSidebar_RequiredHouses;

	Valueable<int> TabIndex;

	Valueable<int> BattlePoints_Amount;
	Valueable<int> BattlePoints_DrainAmount;
	Valueable<int> BattlePoints_DrainDelay;

	Valueable<int> SuperWeaponSidebar_Significance;

	Valueable<int> Music_Theme;
	Valueable<int> Music_Duration;
	Valueable<AffectedHouse> Music_AffectedHouses;

#pragma endregion

public:

	SWTypeExtData(SuperWeaponTypeClass* pObj) : AbstractTypeExtData(pObj),
		EVA_Activated(-1),
		EVA_Ready(-1),
		EVA_Detected(-1),
		EVA_Detected_Simple(false),
		EVA_InsufficientFunds(-1),
		EVA_InsufficientBattlePoints(-1),
		EVA_SelectTarget(-1),
		Message_Launch(),
		Message_CannotFire(),
		Message_Activate(),
		Message_Abort(),
		Message_InsufficientFunds(),
		Message_InsufficientBattlePoints(),
		Message_Impatient(),
		Message_Detected(),
		Message_Ready(),
		Message_FirerColor(false),
		Message_ColorScheme(-1),
		Text_Preparing(),
		Text_Hold(),
		Text_Ready(),
		Text_Charging(),
		Text_Active(),
		SW_RadarEvent(true),
		UIDescription(),
		CameoPriority(0),
		LimboDelivery_Types(),
		LimboDelivery_IDs(),
		LimboDelivery_RollChances(),
		LimboKill_Affected(AffectedHouse::Owner),
		LimboKill_IDs(),
		LimboDelivery_RandomWeightsData(),
		RandomBuffer(0.0),
		SW_Next(),
		SW_Next_RealLaunch(true),
		SW_Next_IgnoreInhibitors(false),
		SW_Next_IgnoreDesignators(true),
		SW_Next_RollChances(),
		SW_Next_RandomWeightsData(),
		SW_Link_RandomWeightsData(),
		SW_Inhibitors_Houses(AffectedHouse::Enemies),
		SW_Inhibitors(),
		SW_AnyInhibitor(false),
		SW_Designators_Houses(AffectedHouse::Owner),
		SW_Designators(),
		SW_AnyDesignator(false),
		ShowDesignatorRange(true),
		SW_Suppressors(),
		SW_AnySuppressor(false),
		SW_Attractors(),
		SW_AnyAttractor(false),
		SW_RangeMinimum(-1.0),
		SW_RangeMaximum(-1.0),
		SW_RequiredHouses(0xFFFFFFFFu),
		SW_ForbiddenHouses(0u),
		SW_AuxBuildings(),
		SW_NegBuildings(),
		SW_InitialReady(false),
		SW_AlwaysGranted(false),
		Detonate_Warhead(nullptr),
		Detonate_Weapon(nullptr),
		Detonate_Damage(),
		Detonate_AtFirer(false),
		GClock_Shape(),
		GClock_Transculency(),
		GClock_Palette(CustomPalette::PaletteMode::Default),
		ChargeTimer(false),
		ChargeTimer_Backwards(false),
		SW_Priority(0),
		SW_Damage(),
		CursorType((int)MouseCursorType::Attack),
		NoCursorType((int)MouseCursorType::NoMove),
		SW_Range(SWRange()),
		SW_AITargetingMode(SuperWeaponAITargetingMode::None),
		SW_Group(0),
		SW_Shots(-1),
		SW_AutoFire(false),
		SW_AutoFire_CheckAvail(false),
		SW_AllowPlayer(true),
		SW_AllowAI(true),
		SW_ChargeToDrainRatio(),
		SW_AffectsHouse(AffectedHouse::All),
		SW_AnimVisibility(AffectedHouse::All),
		SW_AnimHeight(0),
		HandledType(NewSuperType::Invalid),
		LastAction(Action::None),
		SW_AITargetingConstrain(),
		SW_AIRequiresTarget(),
		SW_AIRequiresHouse(),
		SW_AITargetingPreference(),
		SW_FireToShroud(true),
		SW_UseAITargeting(false),
		SW_RequiresTarget(SuperWeaponTarget::None),
		SW_RequiresHouse(AffectedHouse::None),
		SW_Warhead(),
		SW_Anim(),
		SW_Sound(),
		SW_ActivationSound(),
		SW_Power(),
		SW_AffectsTarget(SuperWeaponTarget::All),
		SW_PostDependent(),
		SW_Deferment(),
		SW_MaxCount(-1),
		SW_ShowCameo(true),
		SW_VirtualCharge(false),
		SW_TimerVisibility(AffectedHouse::All),
		SW_ManualFire(true),
		SW_Unstoppable(false),
		Converts_UseSWRange(false),
		ConvertsPair(),
		Convert_SucceededAnim(nullptr),
		Nuke_Payload(nullptr),
		Nuke_PsiWarning(nullptr),
		Nuke_TakeOff(),
		Nuke_SiloLaunch(true),
		SpyPlanes_TypeIndex(),
		SpyPlanes_Count(),
		SpyPlanes_Mission(),
		SpyPlanes_Rank(),
		Battery_Overpower(),
		Battery_KeepOnline(),
		Chronosphere_BlastSrc(),
		Chronosphere_BlastDest(),
		Chronosphere_KillOrganic(true),
		Chronosphere_KillTeleporters(true),
		Chronosphere_AffectUndeployable(false),
		Chronosphere_AffectBuildings(false),
		Chronosphere_AffectUnwarpable(false),
		Chronosphere_AffectIronCurtain(false),
		Chronosphere_BlowUnplaceable(true),
		Chronosphere_ReconsiderBuildings(false),
		Chronosphere_Delay(),
		Chronosphere_KillCargo(false),
		Dominator_Capture(true),
		Dominator_FireAtPercentage(),
		Dominator_FirstAnimHeight(1),
		Dominator_SecondAnimHeight(1),
		Dominator_FirstAnim(),
		Dominator_SecondAnim(),
		Dominator_ControlAnim(),
		Dominator_Ripple(true),
		Dominator_CaptureMindControlled(true),
		Dominator_CapturePermaMindControlled(true),
		Dominator_CaptureImmuneToPsionics(false),
		Dominator_PermanentCapture(true),
		DropPod_Minimum(),
		DropPod_Maximum(),
		DropPod_Veterancy(2.0),
		DropPod_Types(),
		Droppod_RetryCount(3),
		DroppodProp(),
		EMPField_Duration(100),
		EMPulse_Linked(false),
		EMPulse_TargetSelf(false),
		EMPulse_PulseDelay(32),
		EMPulse_PulseBall(),
		EMPulse_Cannons(),
		EMPulse_SuspendOthers(false),
		EMPulse_WeaponIndex(0),
		Mutate_Explosion(),
		Mutate_IgnoreCyborg(false),
		Mutate_IgnoreNotHuman(true),
		Mutate_KillNatural(true),
		HunterSeeker_Type(),
		HunterSeeker_RandomOnly(false),
		HunterSeeker_Buildings(),
		HunterSeeker_AllowAttachedBuildingAsFallback(false),
		Weather_Duration(),
		Weather_HitDelay(),
		Weather_ScatterDelay(),
		Weather_ScatterCount(1),
		Weather_Separation(),
		Weather_CloudHeight(-1),
		Weather_RadarOutage(),
		Weather_DebrisMin(0),
		Weather_DebrisMax(1),
		Weather_PrintText(),
		Weather_IgnoreLightningRod(false),
		Weather_BoltExplosion(),
		Weather_Clouds(),
		Weather_Bolts(),
		Weather_Debris(),
		Weather_Sounds(),
		Weather_RadarOutageAffects(AffectedHouse::All),
		Weather_UseSeparateState(false),
		Weather_LightningRodTypes(),
		ParaDropDatas(),
		Protect_Duration(),
		Protect_PlayFadeSoundTime(),
		Protect_PowerOutageDuration(),
		Protect_IsForceShield(false),
		Sonar_Delay(0),
		SW_Deliverables(),
		SW_DeliverableCounts(),
		SW_Deliverables_Facing(),
		SW_DeliverBuildups(true),
		SW_BaseNormal(true),
		SW_OwnerHouse(OwnerHouseKind::Default),
		SW_DeliverableScatter(),
		Lighting_Enabled(),
		Lighting_Ambient(),
		Lighting_Green(),
		Lighting_Blue(),
		Lighting_Red(),
		Money_Amount(0),
		Money_DrainAmount(0),
		Money_DrainDelay(0),
		SidebarPalette(CustomPalette::PaletteMode::Default),
		SidebarPCX(),
		SW_ResetType(),
		SW_Require(),
		Aux_Techno(),
		SW_Lauchsites(),
		MeteorCounts(15),
		MeteorImactCounts(5),
		MeteorAddImpactChance(10),
		MeteorKindChance(30),
		MeteorImpactKindChance(50),
		MeteorSmall(nullptr),
		MeteorLarge(nullptr),
		MeteorImpactSmall(nullptr),
		MeteorImpactLarge(nullptr),
		IonCannon_Ripple(true),
		IonCannon_Blast(nullptr),
		IonCannon_Beam(nullptr),
		IonCannon_BlastHeight(0),
		IonCannon_BeamHeight(0),
		IonCannon_FireAtPercentage(0),
		LaserStrikeDuration(1000),
		LaserStrikeRadius(4096),
		LaserStrikeMax(2),
		LaserStrikeMin(1),
		LaserStrikeMaxRadius(-1),
		LaserStrikeMinRadius(-1),
		LaserStrikeRadiusReduce(20),
		LaserStrikeRadiusReduceAcceleration(0),
		LaserStrikeRadiusReduceMax(0),
		LaserStrikeRadiusReduceMin(0),
		LaserStrikeROF(0),
		LaserStrikeScatter_Max(0),
		LaserStrikeScatter_Min(0),
		LaserStrikeScatter_Max_IncreaseMax(0),
		LaserStrikeScatter_Max_IncreaseMin(0),
		LaserStrikeScatter_Max_Increase(0),
		LaserStrikeScatter_Min_IncreaseMax(0),
		LaserStrikeScatter_Min_IncreaseMin(0),
		LaserStrikeScatter_Min_Increase(0),
		LaserStrikeLines(8),
		LaserStrikeAngle(2),
		LaserStrikeAngleAcceleration(0),
		LaserStrikeAngleMax(0),
		LaserStrikeAngleMin(0),
		LaserStrikeZeroRadius_Weapon(nullptr),
		LaserStrikeInnerColor({ 255, 0, 0 }),
		LaserStrikeOuterColor({ 255, 0, 0 }),
		LaserStrikeOuterSpread({ 255, 0, 0 }),
		LaserStrikeLaserDuration(3),
		LaserStrikeLaserHeight(20000),
		LaserStrikeThickness(10),
		LaserStrikeRate(0),
		Generic_Warhead_Detonate(false),
		UseWeeds(false),
		UseWeeds_Amount(RulesClass::Instance->WeedCapacity),
		UseWeeds_StorageTimer(false),
		UseWeeds_ReadinessAnimationPercentage(90),
		SW_Link(),
		SW_Link_Grant(false),
		SW_Link_Ready(false),
		SW_Link_Reset(false),
		SW_Link_RollChances(),
		Message_LinkedSWAcquired(),
		EVA_LinkedSWAcquired(),
		CrateGoodies(false),
		SuperWeaponSidebar_Allow(),
		SuperWeaponSidebar_PriorityHouses(0u),
		SuperWeaponSidebar_RequiredHouses(0xFFFFFFFFu),
		TabIndex(1),
		BattlePoints_Amount(0),
		BattlePoints_DrainAmount(0),
		BattlePoints_DrainDelay(0),
		SuperWeaponSidebar_Significance(0),
		Music_Theme(-1),
		Music_Duration(0),
		Music_AffectedHouses(AffectedHouse::All)
	{
			this->EVA_InsufficientFunds = VoxClass::FindIndexById(GameStrings::EVA_InsufficientFunds);
			this->EVA_SelectTarget = VoxClass::FindIndexById(GameStrings::EVA_SelectTarget);

			this->AbsType = SuperWeaponTypeClass::AbsID;
			this->Text_Ready = GameStrings::TXT_READY();
			this->Text_Hold = GameStrings::TXT_HOLD();
			this->Text_Charging = GameStrings::TXT_CHARGING();
			this->Text_Active = GameStrings::TXT_FIRESTORM_ON();
			this->Message_CannotFire = "MSG:CannotFire";

	}

	SWTypeExtData(SuperWeaponTypeClass* pObj, noinit_t nn) : AbstractTypeExtData(pObj, nn) { }

	virtual ~SWTypeExtData()
	{
		SuperWeaponTypeClass* pCopy = SWTypeExtData::CurrentSWType;
		if (This() == SWTypeExtData::CurrentSWType)
			pCopy = nullptr;

		SWTypeExtData::CurrentSWType = pCopy;
	};

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override
	{
		this->AbstractTypeExtData::InvalidatePointer(ptr, bRemoved);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->AbstractTypeExtData::LoadFromStream(Stm);
		this->Serialize(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		const_cast<SWTypeExtData*>(this)->AbstractTypeExtData::SaveToStream(Stm);
		const_cast<SWTypeExtData*>(this)->Serialize(Stm);
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const
	{
		this->AbstractTypeExtData::CalculateCRC(crc);
	}

	virtual SuperWeaponTypeClass* This() const override { return reinterpret_cast<SuperWeaponTypeClass*>(this->AbstractTypeExtData::This()); }
	virtual const SuperWeaponTypeClass* This_Const() const override { return reinterpret_cast<const SuperWeaponTypeClass*>(this->AbstractTypeExtData::This_Const()); }

	virtual bool LoadFromINI(CCINIClass* pINI, bool parseFailAddr);
	virtual bool WriteToINI(CCINIClass* pINI) const { return true; }

	void Initialize();

public:

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

	bool PreParse(CCINIClass* pINI);

	OPTIONALINLINE const char* get_ID(){
		return this->Name.data();
	}

	//with arg(s)

	// check shootamount
	bool CanFire(HouseClass* pOwner) const;
	bool CanFireAt(HouseClass* pOwner, const CellStruct& coords, bool manual);

	bool IsAnimVisible(HouseClass* pFirer) const;
	bool IsHouseAffected(HouseClass* pFirer, HouseClass* pHouse);
	bool IsHouseAffected(HouseClass* pFirer, HouseClass* pHouse, AffectedHouse value);
	bool Launch(SWTypeHandler* pNewType, SuperClass* pSuper, CellStruct const cell, bool const isPlayer);
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

	SWTypeHandler* GetNewSWType() const;

	void ApplyLinkedSW(SuperClass* pSW);
public:

	//statics
	static void Deactivate(SuperClass* pSuper, CellStruct const cell, bool const isPlayer);
	static bool Activate(SuperClass* pSuper, CellStruct const cell, bool const isPlayer);
	static AffectedHouse GetRelation(HouseClass* pFirer, HouseClass* pHouse);
	static Action GetAction(SuperWeaponTypeClass* pSuper, CellStruct* pTarget);
	static bool TryFire(SuperClass* pThis, bool IsPlayer);
	static bool IsTargetConstraintsEligible(SuperClass* pThis, bool IsPlayer);
	static TargetResult PickSuperWeaponTarget(SWTypeHandler* pNewType, const TargetingData* pTargeting, const SuperClass* pSuper);
	static void ApplyBattlePoints(SuperClass* pSW);
	static bool IsResourceAvailable(SuperClass* pSuper);
	static bool LauchSuper(SuperClass* pSuper);
	static bool DrawDarken(SuperClass* pSuper);

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
	static const AITargetingModeInfo AITargetingModes[];
	static SuperWeaponTypeClass* CurrentSWType;

public:

	static void LimboDeliver(BuildingTypeClass* pType, HouseClass* pOwner, int ID);
	static void WeightedRollsHandler(std::vector<int>& nResult, Valueable<double>& RandomBuffer, const ValueableVector<float>& rolls, const ValueableVector<ValueableVector<int>>& weights, size_t size);
	static void Launch(SuperClass* pFired, HouseClass* pHouse, SWTypeExtData* pLauncherTypeExt, int pLaunchedType, const CellStruct& cell, bool IsPlayer);
	static void ClearChronoAnim(SuperClass* pThis);
	static void CreateChronoAnim(SuperClass* pThis, const CoordStruct& Coords, AnimTypeClass* pAnimType);
	static bool ChangeLighting(SuperWeaponTypeClass* pCustom = nullptr);
	static LightingColor GetLightingColor(SuperWeaponTypeClass* pCustom = nullptr);
	static double GetSuperChargePercent(SuperClass* pSuper, bool backward = false);

};

class SWTypeExtContainer final : public Container<SWTypeExtData>
{
public:
	static SWTypeExtContainer Instance;

public:

	static void InvalidatePointer(AbstractClass* ptr, bool bRemoved);
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};

class NOVTABLE FakeSuperWeaponTypeClass : public SuperWeaponTypeClass
{
public:
	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, BOOL clearDirty);

	bool _ReadFromINI(CCINIClass* pINI);

	SWTypeExtData* _GetExtData() {
		return *reinterpret_cast<SWTypeExtData**>((DWORD)this + AbstractExtOffset);
	}

};
static_assert(sizeof(FakeSuperWeaponTypeClass) == sizeof(SuperWeaponTypeClass), "Invalid Size !");
