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
	static COMPILETIMEEVAL const char* ClassName = "SWTypeExtData";
	static COMPILETIMEEVAL const char* BaseClassName = "SuperWeaponTypeClass";
	static COMPILETIMEEVAL unsigned Marker = UuidFirstPart<base_type>::value;
	static COMPILETIMEEVAL auto Marker_str = to_hex_string<Marker>();

public:
#pragma region ClassMembers

	// ============================================================
	// Large aggregates: CustomPalette, PhobosFixedString, IndexBitfield
	// ============================================================
	CustomPalette GClock_Palette;
	CustomPalette SidebarPalette;
	PhobosPCXFile SidebarPCX;
	PhobosFixedString<0x19> SW_PostDependent;
	IndexBitfield<HouseTypeClass*> SW_RequiredHouses;
	IndexBitfield<HouseTypeClass*> SW_ForbiddenHouses;
	DroppodProperties DroppodProp;

	// ============================================================
	// Large aggregates: Valueable<CSFText>
	// ============================================================
	Valueable<CSFText> Message_Launch;
	Valueable<CSFText> Message_CannotFire;
	Valueable<CSFText> Message_Activate;
	Valueable<CSFText> Message_Abort;
	Valueable<CSFText> Message_InsufficientFunds;
	Valueable<CSFText> Message_InsufficientBattlePoints;
	Valueable<CSFText> Message_Impatient;
	Valueable<CSFText> Message_Detected;
	Valueable<CSFText> Message_Ready;
	Valueable<CSFText> Text_Preparing;
	Valueable<CSFText> Text_Hold;
	Valueable<CSFText> Text_Ready;
	Valueable<CSFText> Text_Charging;
	Valueable<CSFText> Text_Active;
	Valueable<CSFText> UIDescription;
	Valueable<CSFText> Message_LinkedSWAcquired;

	// ============================================================
	// Maps
	// ============================================================
	PhobosMap<AbstractTypeClass*, std::vector<ParadropData>> ParaDropDatas;

	// ============================================================
	// Vectors of vectors
	// ============================================================
	std::vector<std::vector<int>> LimboDelivery_RandomWeightsData;
	std::vector<std::vector<int>> SW_Next_RandomWeightsData;
	std::vector<std::vector<int>> SW_Link_RandomWeightsData;
	std::vector<TechnoTypeConvertData> ConvertsPair;

	// ============================================================
	// 24-byte aligned: Vectors
	// ============================================================
	ValueableVector<BuildingTypeClass*> LimboDelivery_Types;
	ValueableVector<int> LimboDelivery_IDs;
	ValueableVector<float> LimboDelivery_RollChances;
	ValueableVector<int> LimboKill_IDs;
	ValueableIdxVector<SuperWeaponTypeClass*> SW_Next;
	ValueableVector<float> SW_Next_RollChances;
	ValueableVector<TechnoTypeClass*> SW_Inhibitors;
	ValueableVector<TechnoTypeClass*> SW_Designators;
	ValueableVector<TechnoTypeClass*> SW_Suppressors;
	ValueableVector<TechnoTypeClass*> SW_Attractors;
	ValueableVector<BuildingTypeClass*> SW_AuxBuildings;
	ValueableVector<BuildingTypeClass*> SW_NegBuildings;
	ValueableVector<TechnoTypeClass*> SW_AITargeting_PsyDom_AllowTypes;
	ValueableVector<TechnoTypeClass*> SW_AITargeting_PsyDom_DisallowTypes;
	ValueableVector<AircraftTypeClass*> SpyPlanes_TypeIndex;
	ValueableVector<int> SpyPlanes_Count;
	ValueableVector<Mission> SpyPlanes_Mission;
	ValueableVector<Rank> SpyPlanes_Rank;
	ValueableVector<BuildingTypeClass*> Battery_Overpower;
	ValueableVector<BuildingTypeClass*> Battery_KeepOnline;
	ValueableVector<TechnoTypeClass*> DropPod_Types;
	ValueableVector<BuildingTypeClass*> EMPulse_Cannons;
	ValueableVector<BuildingTypeClass*> HunterSeeker_Buildings;
	NullableVector<AnimTypeClass*> Weather_Clouds;
	NullableVector<AnimTypeClass*> Weather_Bolts;
	NullableVector<AnimTypeClass*> Weather_Debris;
	NullableIdxVector<VocClass> Weather_Sounds;
	ValueableVector<BuildingTypeClass*> Weather_LightningRodTypes;
	ValueableVector<TechnoTypeClass*> SW_Deliverables;
	ValueableVector<int> SW_DeliverableCounts;
	ValueableVector<FacingType> SW_Deliverables_Facing;
	ValueableIdxVector<SuperWeaponTypeClass> SW_ResetType;
	ValueableVector<int> SW_Require;
	ValueableVector<TechnoTypeClass*> Aux_Techno;
	ValueableVector<BuildingTypeClass*> SW_Lauchsites;
	ValueableIdxVector<SuperWeaponTypeClass> SW_Link;
	ValueableVector<float> SW_Link_RollChances;

	// ============================================================
	// Valueable<SWRange> (likely 8+ bytes)
	// ============================================================
	Valueable<SWRange> SW_Range;

	// ============================================================
	// 8-byte aligned: Valueable<pointer>
	// ============================================================
	Valueable<WarheadTypeClass*> Detonate_Warhead;
	Valueable<WeaponTypeClass*> Detonate_Weapon;
	Valueable<SHPStruct*> GClock_Shape;
	Valueable<WeaponTypeClass*> Nuke_Payload;
	Valueable<AnimTypeClass*> Nuke_PsiWarning;
	Valueable<AnimTypeClass*> Convert_SucceededAnim;
	Valueable<AnimTypeClass*> MeteorSmall;
	Valueable<AnimTypeClass*> MeteorLarge;
	Valueable<VoxelAnimTypeClass*> MeteorImpactSmall;
	Valueable<VoxelAnimTypeClass*> MeteorImpactLarge;
	Valueable<AnimTypeClass*> IonCannon_Blast;
	Valueable<AnimTypeClass*> IonCannon_Beam;
	Valueable<WeaponTypeClass*> LaserStrikeZeroRadius_Weapon;

	// ============================================================
	// Nullable<pointer> (pointer + bool + padding ≈ 16 bytes)
	// ============================================================
	Nullable<AnimTypeClass*> Nuke_TakeOff;
	Nullable<AnimTypeClass*> Chronosphere_BlastSrc;
	Nullable<AnimTypeClass*> Chronosphere_BlastDest;
	Nullable<AnimTypeClass*> Dominator_FirstAnim;
	Nullable<AnimTypeClass*> Dominator_SecondAnim;
	Nullable<AnimTypeClass*> Dominator_ControlAnim;
	Nullable<UnitTypeClass*> HunterSeeker_Type;
	Nullable<AnimTypeClass*> Weather_BoltExplosion;
	Nullable<AnimTypeClass*> EMPulse_PulseBall;
	Nullable<WarheadTypeClass*> SW_Warhead;
	Nullable<AnimTypeClass*> SW_Anim;

	// ============================================================
	// Nullable<double> (~16 bytes)
	// ============================================================
	Nullable<double> SW_ChargeToDrainRatio;

	// ============================================================
	// Valueable<double> (8 bytes each)
	// ============================================================
	Valueable<double> RandomBuffer;
	Valueable<double> SW_RangeMinimum;
	Valueable<double> SW_RangeMaximum;
	Valueable<double> DropPod_Veterancy;

	// ============================================================
	// Nullable<int> (~8 bytes each)
	// ============================================================
	Nullable<int> Detonate_Damage;
	Nullable<int> SW_Damage;
	Nullable<int> SW_Power;
	Nullable<int> SW_Deferment;
	Nullable<int> Chronosphere_Delay;
	Nullable<int> Dominator_FireAtPercentage;
	Nullable<int> DropPod_Minimum;
	Nullable<int> DropPod_Maximum;
	Nullable<int> Weather_Duration;
	Nullable<int> Weather_HitDelay;
	Nullable<int> Weather_ScatterDelay;
	Nullable<int> Weather_Separation;
	Nullable<int> Weather_RadarOutage;
	Nullable<int> Protect_Duration;
	Nullable<int> Protect_PlayFadeSoundTime;
	Nullable<int> Protect_PowerOutageDuration;
	Nullable<int> Lighting_Ambient;
	Nullable<int> Lighting_Green;
	Nullable<int> Lighting_Blue;
	Nullable<int> Lighting_Red;

	// ============================================================
	// Nullable<TargetingConstraints/SuperWeaponTarget/etc> (~8 bytes)
	// ============================================================
	Nullable<TargetingConstraints> SW_AITargetingConstrain;
	Nullable<SuperWeaponTarget> SW_AIRequiresTarget;
	Nullable<AffectedHouse> SW_AIRequiresHouse;
	Nullable<TargetingPreference> SW_AITargetingPreference;

	// ============================================================
	// Nullable<bool> (~2-4 bytes)
	// ============================================================
	Nullable<bool> Mutate_Explosion;
	Nullable<bool> Weather_PrintText;
	Nullable<bool> SW_DeliverableScatter;
	Nullable<bool> Lighting_Enabled;
	Nullable<bool> SuperWeaponSidebar_Allow;

	// ============================================================
	// NullableIdx (~8 bytes each)
	// ============================================================
	NullableIdx<VocClass> SW_Sound;
	NullableIdx<VocClass> SW_ActivationSound;
	NullableIdx<VocClass> EVA_LinkedSWAcquired;

	// ============================================================
	// ValueableIdx (4 bytes each)
	// ============================================================
	ValueableIdx<VoxClass> EVA_Activated;
	ValueableIdx<VoxClass> EVA_Ready;
	ValueableIdx<VoxClass> EVA_Detected;
	ValueableIdx<VoxClass> EVA_InsufficientFunds;
	ValueableIdx<VoxClass> EVA_InsufficientBattlePoints;
	ValueableIdx<VoxClass> EVA_SelectTarget;
	ValueableIdx<ColorScheme> Message_ColorScheme;
	ValueableIdx<CursorTypeClass*> CursorType;
	ValueableIdx<CursorTypeClass*> NoCursorType;

	// ============================================================
	// Valueable<ColorStruct> (3-4 bytes each)
	// ============================================================
	Valueable<ColorStruct> LaserStrikeInnerColor;
	Valueable<ColorStruct> LaserStrikeOuterColor;
	Valueable<ColorStruct> LaserStrikeOuterSpread;

	// ============================================================
	// Valueable<int> (4 bytes each)
	// ============================================================
	Valueable<int> CameoPriority;
	Valueable<int> SW_Priority;
	Valueable<int> SW_Group;
	Valueable<int> SW_Shots;
	Valueable<int> SW_AnimHeight;
	Valueable<int> SW_MaxCount;
	Valueable<int> Dominator_FirstAnimHeight;
	Valueable<int> Dominator_SecondAnimHeight;
	Valueable<int> Droppod_RetryCount;
	Valueable<int> EMPField_Duration;
	Valueable<int> EMPulse_PulseDelay;
	Valueable<int> EMPulse_WeaponIndex;
	Valueable<int> Weather_ScatterCount;
	Valueable<int> Weather_CloudHeight;
	Valueable<int> Weather_DebrisMin;
	Valueable<int> Weather_DebrisMax;
	Valueable<int> Sonar_Delay;
	Valueable<int> Money_Amount;
	Valueable<int> Money_DrainAmount;
	Valueable<int> Money_DrainDelay;
	Valueable<int> MeteorCounts;
	Valueable<int> MeteorImactCounts;
	Valueable<int> MeteorAddImpactChance;
	Valueable<int> MeteorKindChance;
	Valueable<int> MeteorImpactKindChance;
	Valueable<int> IonCannon_BlastHeight;
	Valueable<int> IonCannon_BeamHeight;
	Valueable<int> IonCannon_FireAtPercentage;
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
	Valueable<int> LaserStrikeLaserDuration;
	Valueable<int> LaserStrikeLaserHeight;
	Valueable<int> LaserStrikeThickness;
	Valueable<int> LaserStrikeRate;
	Valueable<int> UseWeeds_Amount;
	Valueable<int> UseWeeds_ReadinessAnimationPercentage;
	Valueable<int> TabIndex;
	Valueable<int> BattlePoints_Amount;
	Valueable<int> BattlePoints_DrainAmount;
	Valueable<int> BattlePoints_DrainDelay;
	Valueable<int> SuperWeaponSidebar_Significance;
	Valueable<int> Music_Theme;
	Valueable<int> Music_Duration;

	// ============================================================
	// DWORD (4 bytes each)
	// ============================================================
	DWORD SuperWeaponSidebar_PriorityHouses;
	DWORD SuperWeaponSidebar_RequiredHouses;

	// ============================================================
	// Valueable<enum> (4 bytes each)
	// ============================================================
	Valueable<TranslucencyLevel> GClock_Transculency;
	Valueable<AffectedHouse> LimboKill_Affected;
	Valueable<AffectedHouse> SW_Inhibitors_Houses;
	Valueable<AffectedHouse> SW_Designators_Houses;
	Valueable<SuperWeaponAITargetingMode> SW_AITargetingMode;
	Valueable<AffectedHouse> SW_AffectsHouse;
	Valueable<AffectedHouse> SW_AnimVisibility;
	Valueable<SuperWeaponTarget> SW_RequiresTarget;
	Valueable<AffectedHouse> SW_RequiresHouse;
	Valueable<SuperWeaponTarget> SW_AffectsTarget;
	Valueable<AffectedHouse> SW_TimerVisibility;
	Valueable<OwnerHouseKind> SW_OwnerHouse;
	Valueable<AffectedHouse> Weather_RadarOutageAffects;
	Valueable<AffectedHouse> Music_AffectedHouses;
	NewSuperType HandledType;
	Action LastAction;

	// ============================================================
	// Valueable<bool> (1 byte each, packed together at the end)
	// ============================================================
	Valueable<bool> EVA_Detected_Simple;
	Valueable<bool> Message_FirerColor;
	Valueable<bool> SW_RadarEvent;
	Valueable<bool> SW_Next_RealLaunch;
	Valueable<bool> SW_Next_IgnoreInhibitors;
	Valueable<bool> SW_Next_IgnoreDesignators;
	Valueable<bool> SW_AnyInhibitor;
	Valueable<bool> SW_AnyDesignator;
	Valueable<bool> ShowDesignatorRange;
	Valueable<bool> SW_AnySuppressor;
	Valueable<bool> SW_AnyAttractor;
	Valueable<bool> SW_InitialReady;
	Valueable<bool> SW_AlwaysGranted;
	Valueable<bool> Detonate_AtFirer;
	Valueable<bool> ChargeTimer;
	Valueable<bool> ChargeTimer_Backwards;
	Valueable<bool> SW_AutoFire;
	Valueable<bool> SW_AutoFire_CheckAvail;
	Valueable<bool> SW_AllowPlayer;
	Valueable<bool> SW_AllowAI;
	Valueable<bool> SW_FireToShroud;
	Valueable<bool> SW_UseAITargeting;
	Valueable<bool> SW_AITargeting_PsyDom_SkipChecks;
	Valueable<bool> SW_AITargeting_PsyDom_AllowAir;
	Valueable<bool> SW_AITargeting_PsyDom_AllowInvulnerable;
	Valueable<bool> SW_AITargeting_PsyDom_AllowCloak;
	Valueable<bool> SW_ShowCameo;
	Valueable<bool> SW_VirtualCharge;
	Valueable<bool> SW_ManualFire;
	Valueable<bool> SW_Unstoppable;
	Valueable<bool> Converts_UseSWRange;
	Valueable<bool> Nuke_SiloLaunch;
	Valueable<bool> Chronosphere_KillOrganic;
	Valueable<bool> Chronosphere_KillTeleporters;
	Valueable<bool> Chronosphere_AffectUndeployable;
	Valueable<bool> Chronosphere_AffectBuildings;
	Valueable<bool> Chronosphere_AffectUnwarpable;
	Valueable<bool> Chronosphere_AffectIronCurtain;
	Valueable<bool> Chronosphere_BlowUnplaceable;
	Valueable<bool> Chronosphere_ReconsiderBuildings;
	Valueable<bool> Chronosphere_KillCargo;
	Valueable<bool> Dominator_Capture;
	Valueable<bool> Dominator_Ripple;
	Valueable<bool> Dominator_CaptureMindControlled;
	Valueable<bool> Dominator_CapturePermaMindControlled;
	Valueable<bool> Dominator_CaptureImmuneToPsionics;
	Valueable<bool> Dominator_PermanentCapture;
	Valueable<bool> EMPulse_Linked;
	Valueable<bool> EMPulse_TargetSelf;
	Valueable<bool> EMPulse_SuspendOthers;
	Valueable<bool> Mutate_IgnoreCyborg;
	Valueable<bool> Mutate_IgnoreNotHuman;
	Valueable<bool> Mutate_KillNatural;
	Valueable<bool> HunterSeeker_RandomOnly;
	Valueable<bool> HunterSeeker_AllowAttachedBuildingAsFallback;
	Valueable<bool> Weather_IgnoreLightningRod;
	Valueable<bool> Weather_UseSeparateState;
	Valueable<bool> Protect_IsForceShield;
	Valueable<bool> SW_DeliverBuildups;
	Valueable<bool> SW_BaseNormal;
	Valueable<bool> IonCannon_Ripple;
	Valueable<bool> Generic_Warhead_Detonate;
	Valueable<bool> UseWeeds;
	Valueable<bool> UseWeeds_StorageTimer;
	Valueable<bool> SW_Link_Grant;
	Valueable<bool> SW_Link_Ready;
	Valueable<bool> SW_Link_Reset;
	Valueable<bool> CrateGoodies;

#pragma endregion

public:
	SWTypeExtData(SuperWeaponTypeClass* pObj) : AbstractTypeExtData(pObj)
		// Large aggregates
		, GClock_Palette(CustomPalette::PaletteMode::Default)
		, SidebarPalette(CustomPalette::PaletteMode::Default)
		, SidebarPCX()
		, SW_PostDependent()
		, SW_RequiredHouses(0xFFFFFFFFu)
		, SW_ForbiddenHouses(0u)
		, DroppodProp()
		// CSFText
		, Message_Launch()
		, Message_CannotFire()
		, Message_Activate()
		, Message_Abort()
		, Message_InsufficientFunds()
		, Message_InsufficientBattlePoints()
		, Message_Impatient()
		, Message_Detected()
		, Message_Ready()
		, Text_Preparing()
		, Text_Hold()
		, Text_Ready()
		, Text_Charging()
		, Text_Active()
		, UIDescription()
		, Message_LinkedSWAcquired()
		// Maps
		, ParaDropDatas()
		// Vectors of vectors
		, LimboDelivery_RandomWeightsData()
		, SW_Next_RandomWeightsData()
		, SW_Link_RandomWeightsData()
		, ConvertsPair()
		// Vectors
		, LimboDelivery_Types()
		, LimboDelivery_IDs()
		, LimboDelivery_RollChances()
		, LimboKill_IDs()
		, SW_Next()
		, SW_Next_RollChances()
		, SW_Inhibitors()
		, SW_Designators()
		, SW_Suppressors()
		, SW_Attractors()
		, SW_AuxBuildings()
		, SW_NegBuildings()
		, SW_AITargeting_PsyDom_AllowTypes()
		, SW_AITargeting_PsyDom_DisallowTypes()
		, SpyPlanes_TypeIndex()
		, SpyPlanes_Count()
		, SpyPlanes_Mission()
		, SpyPlanes_Rank()
		, Battery_Overpower()
		, Battery_KeepOnline()
		, DropPod_Types()
		, EMPulse_Cannons()
		, HunterSeeker_Buildings()
		, Weather_Clouds()
		, Weather_Bolts()
		, Weather_Debris()
		, Weather_Sounds()
		, Weather_LightningRodTypes()
		, SW_Deliverables()
		, SW_DeliverableCounts()
		, SW_Deliverables_Facing()
		, SW_ResetType()
		, SW_Require()
		, Aux_Techno()
		, SW_Lauchsites()
		, SW_Link()
		, SW_Link_RollChances()
		// SWRange
		, SW_Range(SWRange())
		// Valueable<pointer>
		, Detonate_Warhead(nullptr)
		, Detonate_Weapon(nullptr)
		, GClock_Shape()
		, Nuke_Payload(nullptr)
		, Nuke_PsiWarning(nullptr)
		, Convert_SucceededAnim(nullptr)
		, MeteorSmall(nullptr)
		, MeteorLarge(nullptr)
		, MeteorImpactSmall(nullptr)
		, MeteorImpactLarge(nullptr)
		, IonCannon_Blast(nullptr)
		, IonCannon_Beam(nullptr)
		, LaserStrikeZeroRadius_Weapon(nullptr)
		// Nullable<pointer>
		, Nuke_TakeOff()
		, Chronosphere_BlastSrc()
		, Chronosphere_BlastDest()
		, Dominator_FirstAnim()
		, Dominator_SecondAnim()
		, Dominator_ControlAnim()
		, HunterSeeker_Type()
		, Weather_BoltExplosion()
		, EMPulse_PulseBall()
		, SW_Warhead()
		, SW_Anim()
		// Nullable<double>
		, SW_ChargeToDrainRatio()
		// Valueable<double>
		, RandomBuffer(0.0)
		, SW_RangeMinimum(-1.0)
		, SW_RangeMaximum(-1.0)
		, DropPod_Veterancy(2.0)
		// Nullable<int>
		, Detonate_Damage()
		, SW_Damage()
		, SW_Power()
		, SW_Deferment()
		, Chronosphere_Delay()
		, Dominator_FireAtPercentage()
		, DropPod_Minimum()
		, DropPod_Maximum()
		, Weather_Duration()
		, Weather_HitDelay()
		, Weather_ScatterDelay()
		, Weather_Separation()
		, Weather_RadarOutage()
		, Protect_Duration()
		, Protect_PlayFadeSoundTime()
		, Protect_PowerOutageDuration()
		, Lighting_Ambient()
		, Lighting_Green()
		, Lighting_Blue()
		, Lighting_Red()
		// Nullable<enum>
		, SW_AITargetingConstrain()
		, SW_AIRequiresTarget()
		, SW_AIRequiresHouse()
		, SW_AITargetingPreference()
		// Nullable<bool>
		, Mutate_Explosion()
		, Weather_PrintText()
		, SW_DeliverableScatter()
		, Lighting_Enabled()
		, SuperWeaponSidebar_Allow()
		// NullableIdx
		, SW_Sound()
		, SW_ActivationSound()
		, EVA_LinkedSWAcquired()
		// ValueableIdx
		, EVA_Activated(-1)
		, EVA_Ready(-1)
		, EVA_Detected(-1)
		, EVA_InsufficientFunds(-1)
		, EVA_InsufficientBattlePoints(-1)
		, EVA_SelectTarget(-1)
		, Message_ColorScheme(-1)
		, CursorType((int)MouseCursorType::Attack)
		, NoCursorType((int)MouseCursorType::NoMove)
		// Valueable<ColorStruct>
		, LaserStrikeInnerColor({ 255, 0, 0 })
		, LaserStrikeOuterColor({ 255, 0, 0 })
		, LaserStrikeOuterSpread({ 255, 0, 0 })
		// Valueable<int>
		, CameoPriority(0)
		, SW_Priority(0)
		, SW_Group(0)
		, SW_Shots(-1)
		, SW_AnimHeight(0)
		, SW_MaxCount(-1)
		, Dominator_FirstAnimHeight(1)
		, Dominator_SecondAnimHeight(1)
		, Droppod_RetryCount(3)
		, EMPField_Duration(100)
		, EMPulse_PulseDelay(32)
		, EMPulse_WeaponIndex(0)
		, Weather_ScatterCount(1)
		, Weather_CloudHeight(-1)
		, Weather_DebrisMin(0)
		, Weather_DebrisMax(1)
		, Sonar_Delay(0)
		, Money_Amount(0)
		, Money_DrainAmount(0)
		, Money_DrainDelay(0)
		, MeteorCounts(15)
		, MeteorImactCounts(5)
		, MeteorAddImpactChance(10)
		, MeteorKindChance(30)
		, MeteorImpactKindChance(50)
		, IonCannon_BlastHeight(0)
		, IonCannon_BeamHeight(0)
		, IonCannon_FireAtPercentage(0)
		, LaserStrikeDuration(1000)
		, LaserStrikeRadius(4096)
		, LaserStrikeMax(2)
		, LaserStrikeMin(1)
		, LaserStrikeMaxRadius(-1)
		, LaserStrikeMinRadius(-1)
		, LaserStrikeRadiusReduce(20)
		, LaserStrikeRadiusReduceAcceleration(0)
		, LaserStrikeRadiusReduceMax(0)
		, LaserStrikeRadiusReduceMin(0)
		, LaserStrikeROF(0)
		, LaserStrikeScatter_Max(0)
		, LaserStrikeScatter_Min(0)
		, LaserStrikeScatter_Max_IncreaseMax(0)
		, LaserStrikeScatter_Max_IncreaseMin(0)
		, LaserStrikeScatter_Max_Increase(0)
		, LaserStrikeScatter_Min_IncreaseMax(0)
		, LaserStrikeScatter_Min_IncreaseMin(0)
		, LaserStrikeScatter_Min_Increase(0)
		, LaserStrikeLines(8)
		, LaserStrikeAngle(2)
		, LaserStrikeAngleAcceleration(0)
		, LaserStrikeAngleMax(0)
		, LaserStrikeAngleMin(0)
		, LaserStrikeLaserDuration(3)
		, LaserStrikeLaserHeight(20000)
		, LaserStrikeThickness(10)
		, LaserStrikeRate(0)
		, UseWeeds_Amount(RulesClass::Instance->WeedCapacity)
		, UseWeeds_ReadinessAnimationPercentage(90)
		, TabIndex(1)
		, BattlePoints_Amount(0)
		, BattlePoints_DrainAmount(0)
		, BattlePoints_DrainDelay(0)
		, SuperWeaponSidebar_Significance(0)
		, Music_Theme(-1)
		, Music_Duration(0)
		// DWORD
		, SuperWeaponSidebar_PriorityHouses(0u)
		, SuperWeaponSidebar_RequiredHouses(0xFFFFFFFFu)
		// Valueable<enum>
		, GClock_Transculency()
		, LimboKill_Affected(AffectedHouse::Owner)
		, SW_Inhibitors_Houses(AffectedHouse::Enemies)
		, SW_Designators_Houses(AffectedHouse::Owner)
		, SW_AITargetingMode(SuperWeaponAITargetingMode::None)
		, SW_AffectsHouse(AffectedHouse::All)
		, SW_AnimVisibility(AffectedHouse::All)
		, SW_RequiresTarget(SuperWeaponTarget::None)
		, SW_RequiresHouse(AffectedHouse::None)
		, SW_AffectsTarget(SuperWeaponTarget::All)
		, SW_TimerVisibility(AffectedHouse::All)
		, SW_OwnerHouse(OwnerHouseKind::Default)
		, Weather_RadarOutageAffects(AffectedHouse::All)
		, Music_AffectedHouses(AffectedHouse::All)
		, HandledType(NewSuperType::Invalid)
		, LastAction(Action::None)
		// Valueable<bool>
		, EVA_Detected_Simple(false)
		, Message_FirerColor(false)
		, SW_RadarEvent(true)
		, SW_Next_RealLaunch(true)
		, SW_Next_IgnoreInhibitors(false)
		, SW_Next_IgnoreDesignators(true)
		, SW_AnyInhibitor(false)
		, SW_AnyDesignator(false)
		, ShowDesignatorRange(true)
		, SW_AnySuppressor(false)
		, SW_AnyAttractor(false)
		, SW_InitialReady(false)
		, SW_AlwaysGranted(false)
		, Detonate_AtFirer(false)
		, ChargeTimer(false)
		, ChargeTimer_Backwards(false)
		, SW_AutoFire(false)
		, SW_AutoFire_CheckAvail(false)
		, SW_AllowPlayer(true)
		, SW_AllowAI(true)
		, SW_FireToShroud(true)
		, SW_UseAITargeting(false)
		, SW_AITargeting_PsyDom_SkipChecks(false)
		, SW_AITargeting_PsyDom_AllowAir(false)
		, SW_AITargeting_PsyDom_AllowInvulnerable(false)
		, SW_AITargeting_PsyDom_AllowCloak(false)
		, SW_ShowCameo(true)
		, SW_VirtualCharge(false)
		, SW_ManualFire(true)
		, SW_Unstoppable(false)
		, Converts_UseSWRange(false)
		, Nuke_SiloLaunch(true)
		, Chronosphere_KillOrganic(true)
		, Chronosphere_KillTeleporters(true)
		, Chronosphere_AffectUndeployable(false)
		, Chronosphere_AffectBuildings(false)
		, Chronosphere_AffectUnwarpable(false)
		, Chronosphere_AffectIronCurtain(false)
		, Chronosphere_BlowUnplaceable(true)
		, Chronosphere_ReconsiderBuildings(false)
		, Chronosphere_KillCargo(false)
		, Dominator_Capture(true)
		, Dominator_Ripple(true)
		, Dominator_CaptureMindControlled(true)
		, Dominator_CapturePermaMindControlled(true)
		, Dominator_CaptureImmuneToPsionics(false)
		, Dominator_PermanentCapture(true)
		, EMPulse_Linked(false)
		, EMPulse_TargetSelf(false)
		, EMPulse_SuspendOthers(false)
		, Mutate_IgnoreCyborg(false)
		, Mutate_IgnoreNotHuman(true)
		, Mutate_KillNatural(true)
		, HunterSeeker_RandomOnly(false)
		, HunterSeeker_AllowAttachedBuildingAsFallback(false)
		, Weather_IgnoreLightningRod(false)
		, Weather_UseSeparateState(false)
		, Protect_IsForceShield(false)
		, SW_DeliverBuildups(true)
		, SW_BaseNormal(true)
		, IonCannon_Ripple(true)
		, Generic_Warhead_Detonate(false)
		, UseWeeds(false)
		, UseWeeds_StorageTimer(false)
		, SW_Link_Grant(false)
		, SW_Link_Ready(false)
		, SW_Link_Reset(false)
		, CrateGoodies(false)
	{
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

	SuperWeaponTypeClass* This() const { return reinterpret_cast<SuperWeaponTypeClass*>(this->AttachedToObject); }
	const SuperWeaponTypeClass* This_Const() const { return reinterpret_cast<const SuperWeaponTypeClass*>(this->AttachedToObject); }

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
	, public ReadWriteContainerInterfaces<SWTypeExtData>
{
public:
	static COMPILETIMEEVAL const char* ClassName = "SWTypeExtContainer";
	using base_t = Container<SWTypeExtData>;
	using ext_t = SWTypeExtData;

public:
	static SWTypeExtContainer Instance;

public:

	virtual bool LoadAll(const json& root);
	virtual bool SaveAll(json& root);
	virtual void Clear();

	virtual void LoadFromINI(ext_t::base_type* key, CCINIClass* pINI, bool parseFailAddr);
	virtual void WriteToINI(ext_t::base_type * key, CCINIClass* pINI);

	static void InvalidatePointer(AbstractClass* ptr, bool bRemoved);

};

class NOVTABLE FakeSuperWeaponTypeClass : public SuperWeaponTypeClass
{
public:

	bool _ReadFromINI(CCINIClass* pINI);

	SWTypeExtData* _GetExtData() {
		return *reinterpret_cast<SWTypeExtData**>((DWORD)this + AbstractExtOffset);
	}

};
static_assert(sizeof(FakeSuperWeaponTypeClass) == sizeof(SuperWeaponTypeClass), "Invalid Size !");
