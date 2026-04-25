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
	
	

public:
#pragma region ClassMembers
	// ============================================================
	// Large aggregates: CustomPalette, PhobosFixedString, IndexBitfield
	// ============================================================
	CustomPalette GClock_Palette { CustomPalette::PaletteMode::Default };
	CustomPalette SidebarPalette { CustomPalette::PaletteMode::Default };
	PhobosPCXFile SidebarPCX {};
	PhobosFixedString<0x19> SW_PostDependent {};
	IndexBitfield<HouseTypeClass*> SW_RequiredHouses { 0xFFFFFFFFu };
	IndexBitfield<HouseTypeClass*> SW_ForbiddenHouses { 0u };
	DroppodProperties DroppodProp {};

	// ============================================================
	// Large aggregates: Valueable<CSFText>
	// ============================================================
	Valueable<CSFText> Message_Launch {};
	Valueable<CSFText> Message_CannotFire {};
	Valueable<CSFText> Message_Activate {};
	Valueable<CSFText> Message_Abort {};
	Valueable<CSFText> Message_InsufficientFunds {};
	Valueable<CSFText> Message_InsufficientBattlePoints {};
	Valueable<CSFText> Message_Impatient {};
	Valueable<CSFText> Message_Detected {};
	Valueable<CSFText> Message_Ready {};
	Valueable<CSFText> Text_Preparing {};
	Valueable<CSFText> Text_Hold {};
	Valueable<CSFText> Text_Ready {};
	Valueable<CSFText> Text_Charging {};
	Valueable<CSFText> Text_Active {};
	Valueable<CSFText> UIDescription {};
	Valueable<CSFText> Message_LinkedSWAcquired {};

	// ============================================================
	// Maps
	// ============================================================
	PhobosMap<AbstractTypeClass*, std::vector<ParadropData>> ParaDropDatas {};

	// ============================================================
	// Vectors of vectors
	// ============================================================
	std::vector<std::vector<int>> LimboDelivery_RandomWeightsData {};
	std::vector<std::vector<int>> SW_Next_RandomWeightsData {};
	std::vector<std::vector<int>> SW_Link_RandomWeightsData {};
	std::vector<TechnoTypeConvertData> ConvertsPair {};

	// ============================================================
	// 24-byte aligned: Vectors
	// ============================================================
	ValueableVector<BuildingTypeClass*> LimboDelivery_Types {};
	ValueableVector<int> LimboDelivery_IDs {};
	ValueableVector<float> LimboDelivery_RollChances {};
	ValueableVector<int> LimboKill_IDs {};
	ValueableVector<int> LimboKill_Counts {};
	ValueableIdxVector<SuperWeaponTypeClass*> SW_Next {};
	ValueableVector<float> SW_Next_RollChances {};
	ValueableVector<TechnoTypeClass*> SW_Inhibitors {};
	ValueableVector<TechnoTypeClass*> SW_Designators {};
	ValueableVector<TechnoTypeClass*> SW_Suppressors {};
	ValueableVector<TechnoTypeClass*> SW_Attractors {};
	ValueableVector<BuildingTypeClass*> SW_AuxBuildings {};
	ValueableVector<BuildingTypeClass*> SW_NegBuildings {};
	ValueableVector<TechnoTypeClass*> SW_AITargeting_PsyDom_AllowTypes {};
	ValueableVector<TechnoTypeClass*> SW_AITargeting_PsyDom_DisallowTypes {};
	ValueableVector<AircraftTypeClass*> SpyPlanes_TypeIndex {};
	ValueableVector<int> SpyPlanes_Count {};
	ValueableVector<Mission> SpyPlanes_Mission {};
	ValueableVector<Rank> SpyPlanes_Rank {};
	ValueableVector<BuildingTypeClass*> Battery_Overpower {};
	ValueableVector<BuildingTypeClass*> Battery_KeepOnline {};
	ValueableVector<TechnoTypeClass*> DropPod_Types {};
	ValueableVector<BuildingTypeClass*> EMPulse_Cannons {};
	ValueableVector<BuildingTypeClass*> HunterSeeker_Buildings {};
	NullableVector<AnimTypeClass*> Weather_Clouds {};
	NullableVector<AnimTypeClass*> Weather_Bolts {};
	NullableVector<AnimTypeClass*> Weather_Debris {};
	NullableIdxVector<VocClass> Weather_Sounds {};
	ValueableVector<BuildingTypeClass*> Weather_LightningRodTypes {};
	ValueableVector<TechnoTypeClass*> SW_Deliverables {};
	ValueableVector<int> SW_DeliverableCounts {};
	ValueableVector<FacingType> SW_Deliverables_Facing {};
	ValueableIdxVector<SuperWeaponTypeClass> SW_ResetType {};
	ValueableVector<int> SW_Require {};
	ValueableVector<TechnoTypeClass*> Aux_Techno {};
	ValueableVector<BuildingTypeClass*> SW_Lauchsites {};
	ValueableIdxVector<SuperWeaponTypeClass> SW_Link {};
	ValueableVector<float> SW_Link_RollChances {};

	// ============================================================
	// Valueable<SWRange> (likely 8+ bytes)
	// ============================================================
	Valueable<SWRange> SW_Range { SWRange() };

	// ============================================================
	// 8-byte aligned: Valueable<pointer>
	// ============================================================
	Valueable<WarheadTypeClass*> Detonate_Warhead { nullptr };
	Valueable<WeaponTypeClass*> Detonate_Weapon { nullptr };
	Valueable<SHPStruct*> GClock_Shape {};
	Valueable<WeaponTypeClass*> Nuke_Payload { nullptr };
	Valueable<AnimTypeClass*> Nuke_PsiWarning { nullptr };
	Valueable<AnimTypeClass*> Convert_SucceededAnim { nullptr };
	Valueable<AnimTypeClass*> MeteorSmall { nullptr };
	Valueable<AnimTypeClass*> MeteorLarge { nullptr };
	Valueable<VoxelAnimTypeClass*> MeteorImpactSmall { nullptr };
	Valueable<VoxelAnimTypeClass*> MeteorImpactLarge { nullptr };
	Valueable<AnimTypeClass*> IonCannon_Blast { nullptr };
	Valueable<AnimTypeClass*> IonCannon_Beam { nullptr };
	Valueable<WeaponTypeClass*> LaserStrikeZeroRadius_Weapon { nullptr };

	// ============================================================
	// Nullable<pointer> (pointer + bool + padding ≈ 16 bytes)
	// ============================================================
	Nullable<AnimTypeClass*> Nuke_TakeOff {};
	Nullable<AnimTypeClass*> Chronosphere_BlastSrc {};
	Nullable<AnimTypeClass*> Chronosphere_BlastDest {};
	Nullable<AnimTypeClass*> Dominator_FirstAnim {};
	Nullable<AnimTypeClass*> Dominator_SecondAnim {};
	Nullable<AnimTypeClass*> Dominator_ControlAnim {};
	Nullable<UnitTypeClass*> HunterSeeker_Type {};
	Nullable<AnimTypeClass*> Weather_BoltExplosion {};
	Nullable<AnimTypeClass*> EMPulse_PulseBall {};
	Nullable<WarheadTypeClass*> SW_Warhead {};
	Nullable<AnimTypeClass*> SW_Anim {};

	// ============================================================
	// Nullable<double> (~16 bytes)
	// ============================================================
	Nullable<double> SW_ChargeToDrainRatio {};

	// ============================================================
	// Valueable<double> (8 bytes each)
	// ============================================================
	Valueable<double> RandomBuffer { 0.0 };
	Valueable<double> SW_RangeMinimum { -1.0 };
	Valueable<double> SW_RangeMaximum { -1.0 };
	Valueable<double> DropPod_Veterancy { 2.0 };

	// ============================================================
	// Nullable<int> (~8 bytes each)
	// ============================================================
	Nullable<int> Detonate_Damage {};
	Nullable<int> SW_Damage {};
	Nullable<int> SW_Power {};
	Nullable<int> SW_Deferment {};
	Nullable<int> Chronosphere_Delay {};
	Nullable<int> Dominator_FireAtPercentage {};
	Nullable<int> DropPod_Minimum {};
	Nullable<int> DropPod_Maximum {};
	Nullable<int> Weather_Duration {};
	Nullable<int> Weather_HitDelay {};
	Nullable<int> Weather_ScatterDelay {};
	Nullable<int> Weather_Separation {};
	Nullable<int> Weather_RadarOutage {};
	Nullable<int> Protect_Duration {};
	Nullable<int> Protect_PlayFadeSoundTime {};
	Nullable<int> Protect_PowerOutageDuration {};
	Nullable<int> Lighting_Ambient {};
	Nullable<int> Lighting_Green {};
	Nullable<int> Lighting_Blue {};
	Nullable<int> Lighting_Red {};

	// ============================================================
	// Nullable<TargetingConstraints/SuperWeaponTarget/etc> (~8 bytes)
	// ============================================================
	Nullable<TargetingConstraints> SW_AITargetingConstrain {};
	Nullable<SuperWeaponTarget> SW_AIRequiresTarget {};
	Nullable<AffectedHouse> SW_AIRequiresHouse {};
	Nullable<TargetingPreference> SW_AITargetingPreference {};

	// ============================================================
	// Nullable<bool> (~2-4 bytes)
	// ============================================================
	Nullable<bool> Mutate_Explosion {};
	Nullable<bool> Weather_PrintText {};
	Nullable<bool> SW_DeliverableScatter {};
	Nullable<bool> Lighting_Enabled {};
	Nullable<bool> SuperWeaponSidebar_Allow {};

	// ============================================================
	// NullableIdx (~8 bytes each)
	// ============================================================
	NullableIdx<VocClass> SW_Sound {};
	NullableIdx<VocClass> SW_ActivationSound {};
	NullableIdx<VocClass> EVA_LinkedSWAcquired {};

	// ============================================================
	// ValueableIdx (4 bytes each)
	// ============================================================
	ValueableIdx<VoxClass> EVA_Activated { -1 };
	ValueableIdx<VoxClass> EVA_Ready { -1 };
	ValueableIdx<VoxClass> EVA_Detected { -1 };
	ValueableIdx<VoxClass> EVA_InsufficientFunds { -1 };
	ValueableIdx<VoxClass> EVA_InsufficientBattlePoints { -1 };
	ValueableIdx<VoxClass> EVA_SelectTarget { -1 };
	ValueableIdx<ColorScheme> Message_ColorScheme { -1 };
	ValueableIdx<CursorTypeClass*> CursorType { (int)MouseCursorType::Attack };
	ValueableIdx<CursorTypeClass*> NoCursorType { (int)MouseCursorType::NoMove };

	// ============================================================
	// Valueable<ColorStruct> (3-4 bytes each)
	// ============================================================
	Valueable<ColorStruct> LaserStrikeInnerColor { { 255, 0, 0 } };
	Valueable<ColorStruct> LaserStrikeOuterColor { { 255, 0, 0 } };
	Valueable<ColorStruct> LaserStrikeOuterSpread { { 255, 0, 0 } };

	// ============================================================
	// Valueable<int> (4 bytes each)
	// ============================================================
	Valueable<int> CameoPriority { 0 };
	Valueable<int> SW_Priority { 0 };
	Valueable<int> SW_Group { 0 };
	Valueable<int> SW_Shots { -1 };
	Valueable<int> SW_AnimHeight { 0 };
	Valueable<int> SW_MaxCount { -1 };
	Valueable<int> Dominator_FirstAnimHeight { 1 };
	Valueable<int> Dominator_SecondAnimHeight { 1 };
	Valueable<int> Droppod_RetryCount { 3 };
	Valueable<int> EMPField_Duration { 100 };
	Valueable<int> EMPulse_PulseDelay { 32 };
	Valueable<int> EMPulse_WeaponIndex { 0 };
	Valueable<int> Weather_ScatterCount { 1 };
	Valueable<int> Weather_CloudHeight { -1 };
	Valueable<int> Weather_DebrisMin { 0 };
	Valueable<int> Weather_DebrisMax { 1 };
	Valueable<int> Sonar_Delay { 0 };
	Valueable<int> Money_Amount { 0 };
	Valueable<int> Money_DrainAmount { 0 };
	Valueable<int> Money_DrainDelay { 0 };
	Valueable<int> MeteorCounts { 15 };
	Valueable<int> MeteorImactCounts { 5 };
	Valueable<int> MeteorAddImpactChance { 10 };
	Valueable<int> MeteorKindChance { 30 };
	Valueable<int> MeteorImpactKindChance { 50 };
	Valueable<int> IonCannon_BlastHeight { 0 };
	Valueable<int> IonCannon_BeamHeight { 0 };
	Valueable<int> IonCannon_FireAtPercentage { 0 };
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
	Valueable<int> LaserStrikeLaserDuration { 3 };
	Valueable<int> LaserStrikeLaserHeight { 20000 };
	Valueable<int> LaserStrikeThickness { 10 };
	Valueable<int> LaserStrikeRate { 0 };
	Valueable<int> UseWeeds_Amount { RulesClass::Instance->WeedCapacity };
	Valueable<int> UseWeeds_ReadinessAnimationPercentage { 90 };
	Valueable<int> TabIndex { 1 };
	Valueable<int> BattlePoints_Amount { 0 };
	Valueable<int> BattlePoints_DrainAmount { 0 };
	Valueable<int> BattlePoints_DrainDelay { 0 };
	Valueable<int> SuperWeaponSidebar_Significance { 0 };

	// ============================================================
	// DWORD (4 bytes each)
	// ============================================================
	DWORD SuperWeaponSidebar_PriorityHouses { 0u };
	DWORD SuperWeaponSidebar_RequiredHouses { 0xFFFFFFFFu };

	// ============================================================
	// Valueable<enum> (4 bytes each)
	// ============================================================
	Valueable<TranslucencyLevel> GClock_Transculency {};
	Valueable<AffectedHouse> LimboKill_Affected { AffectedHouse::Owner };
	Valueable<AffectedHouse> SW_Inhibitors_Houses { AffectedHouse::Enemies };
	Valueable<AffectedHouse> SW_Designators_Houses { AffectedHouse::Owner };
	Valueable<SuperWeaponAITargetingMode> SW_AITargetingMode { SuperWeaponAITargetingMode::None };
	Valueable<AffectedHouse> SW_AffectsHouse { AffectedHouse::All };
	Valueable<AffectedHouse> SW_AnimVisibility { AffectedHouse::All };
	Valueable<SuperWeaponTarget> SW_RequiresTarget { SuperWeaponTarget::None };
	Valueable<AffectedHouse> SW_RequiresHouse { AffectedHouse::None };
	Valueable<SuperWeaponTarget> SW_AffectsTarget { SuperWeaponTarget::All };
	Nullable<AffectedHouse> SW_TimerVisibility { };
	Valueable<OwnerHouseKind> SW_OwnerHouse { OwnerHouseKind::Default };
	Valueable<AffectedHouse> Weather_RadarOutageAffects { AffectedHouse::All };
	NewSuperType HandledType { NewSuperType::Invalid };
	Action LastAction { Action::None };

	// ============================================================
	// Valueable<bool> (1 byte each, packed together at the end)
	// ============================================================
	Valueable<bool> EVA_Detected_Simple { false };
	Valueable<bool> Message_FirerColor { false };
	Valueable<bool> SW_RadarEvent { true };
	Valueable<bool> SW_Next_RealLaunch { true };
	Valueable<bool> SW_Next_IgnoreInhibitors { false };
	Valueable<bool> SW_Next_IgnoreDesignators { true };
	Valueable<bool> SW_AnyInhibitor { false };
	Valueable<bool> SW_AnyDesignator { false };
	Valueable<bool> ShowDesignatorRange { true };
	Valueable<bool> SW_AnySuppressor { false };
	Valueable<bool> SW_AnyAttractor { false };
	Valueable<bool> SW_InitialReady { false };
	Valueable<bool> SW_AlwaysGranted { false };
	Valueable<bool> Detonate_AtFirer { false };
	Valueable<bool> ChargeTimer { false };
	Valueable<bool> ChargeTimer_Backwards { false };
	Valueable<bool> SW_AutoFire { false };
	Valueable<bool> SW_AutoFire_CheckAvail { false };
	Valueable<bool> SW_AllowPlayer { true };
	Valueable<bool> SW_AllowAI { true };
	Valueable<bool> SW_FireToShroud { true };
	Valueable<bool> SW_UseAITargeting { false };
	Valueable<bool> SW_AITargeting_PsyDom_SkipChecks { false };
	Valueable<bool> SW_AITargeting_PsyDom_AllowAir { false };
	Valueable<bool> SW_AITargeting_PsyDom_AllowInvulnerable { false };
	Valueable<bool> SW_AITargeting_PsyDom_AllowCloak { false };
	Valueable<bool> SW_ShowCameo { true };
	Valueable<bool> SW_VirtualCharge { false };
	Valueable<bool> SW_ManualFire { true };
	Valueable<bool> SW_Unstoppable { false };
	Valueable<bool> Converts_UseSWRange { false };
	Valueable<bool> Nuke_SiloLaunch { true };
	Valueable<bool> Chronosphere_KillOrganic { true };
	Valueable<bool> Chronosphere_KillTeleporters { true };
	Valueable<bool> Chronosphere_AffectUndeployable { false };
	Valueable<bool> Chronosphere_AffectBuildings { false };
	Valueable<bool> Chronosphere_AffectUnwarpable { false };
	Valueable<bool> Chronosphere_AffectIronCurtain { false };
	Valueable<bool> Chronosphere_BlowUnplaceable { true };
	Valueable<bool> Chronosphere_ReconsiderBuildings { false };
	Valueable<bool> Chronosphere_KillCargo { false };
	Valueable<bool> Dominator_Capture { true };
	Valueable<bool> Dominator_Ripple { true };
	Valueable<bool> Dominator_CaptureMindControlled { true };
	Valueable<bool> Dominator_CapturePermaMindControlled { true };
	Valueable<bool> Dominator_CaptureImmuneToPsionics { false };
	Valueable<bool> Dominator_PermanentCapture { true };
	Valueable<bool> EMPulse_Linked { false };
	Valueable<bool> EMPulse_TargetSelf { false };
	Valueable<bool> EMPulse_SuspendOthers { false };
	Valueable<bool> Mutate_IgnoreCyborg { false };
	Valueable<bool> Mutate_IgnoreNotHuman { true };
	Valueable<bool> Mutate_KillNatural { true };
	Valueable<bool> HunterSeeker_RandomOnly { false };
	Valueable<bool> HunterSeeker_AllowAttachedBuildingAsFallback { false };
	Valueable<bool> Weather_IgnoreLightningRod { false };
	Valueable<bool> Weather_UseSeparateState { false };
	Valueable<bool> Protect_IsForceShield { false };
	Valueable<bool> SW_DeliverBuildups { true };
	Valueable<bool> SW_BaseNormal { true };
	Valueable<bool> IonCannon_Ripple { true };
	Valueable<bool> Generic_Warhead_Detonate { false };
	Valueable<bool> UseWeeds { false };
	Valueable<bool> UseWeeds_StorageTimer { false };
	Valueable<bool> SW_Link_Grant { false };
	Valueable<bool> SW_Link_Ready { false };
	Valueable<bool> SW_Link_Reset { false };
	Valueable<bool> CrateGoodies { false };
#pragma endregion

public:
	SWTypeExtData(SuperWeaponTypeClass* pObj) : AbstractTypeExtData(pObj)	
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

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved, AbstractType  type) override
	{
		this->AbstractTypeExtData::InvalidatePointer(ptr, bRemoved, type);
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
	Iterator<TechnoClass*> GetPotentialAITargets(HouseClass* pTarget) const;
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

	virtual bool LoadAll(const PhobosStreamReader& stm) { return true; }
	virtual bool SaveAll(PhobosStreamWriter& stm){ return true; }

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
