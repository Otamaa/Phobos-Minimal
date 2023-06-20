#pragma once
#include <SuperWeaponTypeClass.h>

#include <Helpers/Macro.h>
#include <Ext/Abstract/Body.h>
#include <Utilities/TemplateDefB.h>

#include <New/Type/CursorTypeClass.h>

enum class SWTargetFlags
{
	DisallowEmpty,
	AllowEmpty,
	CheckHousePower
};

struct LightingColor
{
	int Red, Green, Blue, Ambient;
	bool HasValue;
};

class ParadropPlane
{
public:
	Valueable<AircraftTypeClass*> Aircraft;
	ValueableVector<TechnoTypeClass*> Types;
	ValueableVector<int> Num;
};

template <>
struct Savegame::PhobosStreamObject<ParadropPlane>
{

	bool ReadFromStream(PhobosStreamReader& Stm, ParadropPlane& Value, bool RegisterForChange) const
	{
		return Stm
			.Process(Value.Aircraft, RegisterForChange)
			.Process(Value.Types, RegisterForChange)
			.Process(Value.Num, RegisterForChange)
			.Success();
	};

	bool WriteToStream(PhobosStreamWriter& Stm, const ParadropPlane& Value) const
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
	TargetingConstraint Constrain;
	TargetingPreference Preference;
};

struct TargetResult
{
	CellStruct Target;
	SWTargetFlags Flags;
};

class SuperClass;
class NewSWType;
class ColorScheme;
class SWTypeExt
{
public:
	static bool Handled;
	static SuperClass* TempSuper;
	static SuperClass* LauchData;
	static std::array<const AITargetingModeInfo, (size_t)SuperWeaponAITargetingMode::count> AITargetingModes;

	class ExtData final : public Extension<SuperWeaponTypeClass>
	{
	public:
		static constexpr size_t Canary = 0x11111111;
		using base_type = SuperWeaponTypeClass;

	public:
		// some of the stuffs were initiated depend on custom type it handle
		// some value may not accurate because it loaded internally instead of ini

		ValueableIdx<VoxClass> EVA_Activated;
		ValueableIdx<VoxClass> EVA_Ready;
		ValueableIdx<VoxClass> EVA_Detected;

		//TODO
		Valueable<CSFText> Message_Launch;
		Valueable<bool> Message_FirerColor;

		Valueable<bool> SW_RadarEvent;

		Valueable<CSFText> UIDescription;
		Valueable<int> CameoPriority;
		ValueableVector<BuildingTypeClass*> LimboDelivery_Types;
		ValueableVector<int> LimboDelivery_IDs;
		ValueableVector<float> LimboDelivery_RollChances;
		Valueable<AffectedHouse> LimboKill_Affected;
		ValueableVector<int> LimboKill_IDs;
		Valueable<double> RandomBuffer;

		ValueableIdxVector<SuperWeaponTypeClass*> SW_Next;
		Valueable<bool> SW_Next_RealLaunch;
		Valueable<bool> SW_Next_IgnoreInhibitors;
		Valueable<bool> SW_Next_IgnoreDesignators;
		ValueableVector<float> SW_Next_RollChances;

		std::vector<std::vector<int>> LimboDelivery_RandomWeightsData;
		std::vector<std::vector<int>> SW_Next_RandomWeightsData;
		ValueableVector<TechnoTypeClass*> SW_Inhibitors;
		Valueable<bool> SW_AnyInhibitor;
		ValueableVector<TechnoTypeClass*> SW_Designators;
		Valueable<bool> SW_AnyDesignator;

		Valueable<double> SW_RangeMinimum;
		Valueable<double> SW_RangeMaximum;
		IndexBitfield<HouseTypeClass*> SW_RequiredHouses;
		IndexBitfield<HouseTypeClass*> SW_ForbiddenHouses;
		ValueableVector<BuildingTypeClass*> SW_AuxBuildings;
		ValueableVector<BuildingTypeClass*> SW_NegBuildings;
		Valueable<bool> SW_InitialReady;
		Nullable<WarheadTypeClass*> Detonate_Warhead;
		Nullable<WeaponTypeClass*> Detonate_Weapon;
		Nullable<int> Detonate_Damage;
		Valueable<bool> Detonate_AtFirer;


#pragma region Otamaa
		Nullable<SHPStruct*> GClock_Shape;
		Nullable<int> GClock_Transculency;
		Valueable<PaletteManager*> GClock_Palette; //CustomPalette::PaletteMode::Default
		Valueable<bool> ChargeTimer;
		Valueable<bool> ChargeTimer_Backwards;
#pragma endregion

		Valueable<int> SW_Priority;
		Nullable<int> SW_Damage;

		ValueableIdx<CursorTypeClass*> CursorType;
		ValueableIdx<CursorTypeClass*> NoCursorType;

		Valueable<SWRange> SW_Range;

		ValueableIdx<ColorScheme> Message_ColorScheme;
		Valueable<SuperWeaponAITargetingMode> SW_AITargetingMode;

		Valueable<int> SW_Group;
		Valueable<int> SW_Shots;
		Valueable<bool> SW_AutoFire;
		Valueable<bool> SW_AllowPlayer;
		Valueable<bool> SW_AllowAI;

		Nullable<double> SW_ChargeToDrainRatio;
		Valueable<AffectedHouse> SW_AffectsHouse;
		Valueable<AffectedHouse> SW_AnimVisibility;
		Valueable<int> SW_AnimHeight;

		SuperWeaponType HandledType;

		Valueable<bool> Converts;
		ValueableVector<TechnoTypeConvertData> ConvertsPair;

#pragma region Nuke 
		Valueable<WeaponTypeClass*> Nuke_Payload;
		Valueable<AnimTypeClass*> Nuke_PsiWarning;
		Nullable<AnimTypeClass*> Nuke_TakeOff;
		Valueable<bool> Nuke_SiloLaunch;
#pragma endregion


		Nullable<TargetingConstraint> SW_AITargetingConstrain;
		Nullable<SuperWeaponTarget> SW_AIRequiresTarget;
		Nullable<AffectedHouse> SW_AIRequiresHouse;
		Nullable<TargetingPreference> SW_AITargetingPreference;
		Valueable<bool> SW_FireToShroud;
		Valueable<bool> SW_UseAITargeting;
		Valueable<CSFText> Message_CannotFire;
		Valueable<SuperWeaponTarget> SW_RequiresTarget;
		Valueable<AffectedHouse> SW_RequiresHouse;

		Nullable<WarheadTypeClass*> SW_Warhead;
		Nullable<AnimTypeClass*> SW_Anim;
		NullableIdx<VocClass> SW_Sound;
		NullableIdx<VocClass> SW_ActivationSound;

		ValueableVector<AircraftTypeClass*> SpyPlanes_TypeIndex;
		ValueableVector<int> SpyPlanes_Count;
		ValueableVector<Mission> SpyPlanes_Mission;
		ValueableVector<Rank> SpyPlanes_Rank;

		// TODO  : S/L reading init

		Nullable<int> SW_Power;

		ValueableVector<BuildingTypeClass*> Battery_Overpower;
		ValueableVector<BuildingTypeClass*> Battery_KeepOnline;

		Valueable<SuperWeaponTarget> SW_AffectsTarget;

		PhobosFixedString<0x19> SW_PostDependent;

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
#pragma endregion

		Nullable<int> SW_Deferment;

		//TODO S/L read
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

		Valueable<CSFText> Message_Activate;
		Valueable<CSFText> Message_Abort;

#pragma region Drop Pod
		Nullable<int> DropPod_Minimum;
		Nullable<int> DropPod_Maximum;
		Valueable<double> DropPod_Veterancy;
		ValueableVector<TechnoTypeClass*> DropPod_Types;
#pragma endregion

#pragma region EMPField
		Valueable<int> EMPField_Duration;
#pragma endregion

		Valueable<int> SW_MaxCount;

#pragma region EMPulse / Fire
		Valueable<bool> EMPulse_Linked;
		Valueable<bool> EMPulse_TargetSelf;
		Valueable<int> EMPulse_PulseDelay;
		Nullable<AnimTypeClass*> EMPulse_PulseBall;
		ValueableVector<BuildingTypeClass*> EMPulse_Cannons;
#pragma endregion

		// TODO : S/L
#pragma region Genetic Mutator
		Nullable<bool> Mutate_Explosion;
		Valueable<bool> Mutate_IgnoreCyborg;
		Valueable<bool> Mutate_IgnoreNotHuman;
		Valueable<bool> Mutate_KillNatural;
#pragma endregion

#pragma region Texts
		Valueable<CSFText> Text_Preparing {};
		Valueable<CSFText> Text_Hold {};
		Valueable<CSFText> Text_Ready {};
		Valueable<CSFText> Text_Charging {};
		Valueable<CSFText> Text_Active {};
#pragma endregion

		ValueableIdx<VoxClass> EVA_Impatient { -1 };
		ValueableIdx<VoxClass> EVA_InsufficientFunds { -1 };
		ValueableIdx<VoxClass> EVA_SelectTarget { -1 };

		Valueable<CSFText> Message_InsufficientFunds {};
		Valueable<CSFText> Message_Detected {};
		Valueable<CSFText> Message_Ready {};

#pragma region Hunter Seeker
		Nullable<UnitTypeClass*> HunterSeeker_Type  { };
		Valueable<bool> HunterSeeker_RandomOnly { false };
		ValueableVector<BuildingTypeClass*> HunterSeeker_Buildings { };
#pragma endregion

#pragma region Lightning Storm
		Nullable<int> Weather_Duration{  };
		Nullable<int> Weather_HitDelay{  };
		Nullable<int> Weather_ScatterDelay{  };
		Valueable<int> Weather_ScatterCount;
		Nullable<int> Weather_Separation{  };
		Valueable<int> Weather_CloudHeight;
		Nullable<int> Weather_RadarOutage{  };
		Valueable<int> Weather_DebrisMin{ 0 };
		Valueable<int> Weather_DebrisMax{ 1 };
		Nullable<bool> Weather_PrintText{  };
		Valueable<bool> Weather_IgnoreLightningRod{  };
		Nullable<AnimTypeClass*> Weather_BoltExplosion{  };
		NullableVector<AnimTypeClass*> Weather_Clouds{  };
		NullableVector<AnimTypeClass*> Weather_Bolts{  };
		NullableVector<AnimTypeClass*> Weather_Debris{  };
		NullableIdxVector<VocClass> Weather_Sounds{  };
		Valueable<AffectedHouse> Weather_RadarOutageAffects{ AffectedHouse::All };
#pragma endregion

#pragma region  Generic Paradrop
		PhobosMap<AbstractTypeClass*, std::vector<ParadropPlane*>> ParaDrop {};
		std::vector<std::unique_ptr<ParadropPlane>> ParaDropPlanes {};
#pragma endregion

#pragma region Generic Protection
		Nullable<int> Protect_Duration{  };
		Nullable<int> Protect_PlayFadeSoundTime{  };
		Nullable<int> Protect_PowerOutageDuration{  };
		Valueable<bool> Protect_IsForceShield{  };
#pragma endregion

#pragma region Sonar
		Valueable<int> Sonar_Delay { 0 };
#pragma endregion

#pragma region Unit Delivery
		ValueableVector<TechnoTypeClass*> SW_Deliverables {  };
		Valueable<bool> SW_DeliverBuildups { true };
		Valueable<OwnerHouseKind> SW_OwnerHouse{ OwnerHouseKind::Default  };
#pragma endregion

		Valueable<bool> SW_ShowCameo { true };

#pragma region Lighting
		Valueable<bool> Lighting_Enabled { false };
		Nullable<int> Lighting_Ambient {  };
		Nullable<int> Lighting_Green {  };
		Nullable<int> Lighting_Blue {  };
		Nullable<int> Lighting_Red {  };
#pragma endregion

		Valueable<bool> SW_VirtualCharge { false };
		Valueable<AffectedHouse> SW_TimerVisibility { AffectedHouse::All };

#pragma region Money
		Valueable<int> Money_Amount { 0  };
		Valueable<int> Money_DrainAmount { 0 };
		Valueable<int> Money_DrainDelay { 0 };
#pragma endregion

		Valueable<bool> SW_ManualFire { true };
		Valueable<bool> SW_Unstoppable { false };
		ExtData(SuperWeaponTypeClass* OwnerObject) : Extension<SuperWeaponTypeClass>(OwnerObject)
			, EVA_Activated { -1 }
			, EVA_Ready { -1 }
			, EVA_Detected { -1 }
			, Message_Launch { }
			, Message_FirerColor { false }
			, SW_RadarEvent { false }

			, UIDescription {}
			, CameoPriority { 0 }
			, LimboDelivery_Types {}
			, LimboDelivery_IDs {}
			, LimboDelivery_RollChances {}
			, LimboKill_Affected { AffectedHouse::Owner }
			, LimboKill_IDs {}
			, RandomBuffer { 0.0 }
			, SW_Next {}
			, SW_Next_RealLaunch { true }
			, SW_Next_IgnoreInhibitors { false }
			, SW_Next_IgnoreDesignators { true }
			, SW_Next_RollChances {}

			, LimboDelivery_RandomWeightsData {}
			, SW_Next_RandomWeightsData {}

			, SW_Inhibitors {}
			, SW_AnyInhibitor { false }
			, SW_Designators { }
			, SW_AnyDesignator { false }

			, SW_RangeMinimum { -1.0 }
			, SW_RangeMaximum { -1.0 }
			, SW_RequiredHouses { 0xFFFFFFFFu }
			, SW_ForbiddenHouses { 0u }
			, SW_AuxBuildings {}
			, SW_NegBuildings {}
			, SW_InitialReady { false }
			, Detonate_Warhead {}
			, Detonate_Weapon {}
			, Detonate_Damage {}
			, Detonate_AtFirer { false }

			, GClock_Shape { }
			, GClock_Transculency { }
			, GClock_Palette { }
			, ChargeTimer { false }
			, ChargeTimer_Backwards { false }

			, SW_Priority { 0 }
			, SW_Damage { }
			, CursorType { -1 }
			, NoCursorType { -1 }
			, SW_Range {}
			, Message_ColorScheme { -1 }

			, SW_AITargetingMode { SuperWeaponAITargetingMode::None }
			, SW_Group { 0 }
			, SW_Shots { -1 }
			, SW_AutoFire { false }
			, SW_AllowPlayer { true }
			, SW_AllowAI { true }
			, SW_ChargeToDrainRatio { }
			, SW_AffectsHouse { AffectedHouse::All }
			, SW_AnimVisibility { AffectedHouse::All }
			, SW_AnimHeight { 0 }

			, HandledType { SuperWeaponType::Invalid }

			, Converts { false }
			, ConvertsPair { }

			, Nuke_Payload { }
			, Nuke_PsiWarning { nullptr }
			, Nuke_TakeOff { }
			, Nuke_SiloLaunch { true }

			, SW_AITargetingConstrain {}
			, SW_AIRequiresTarget {}
			, SW_AIRequiresHouse {}
			, SW_AITargetingPreference {}
			, SW_FireToShroud { true }
			, SW_UseAITargeting { false }
			, Message_CannotFire {}
			, SW_RequiresTarget { SuperWeaponTarget::None }
			, SW_RequiresHouse { AffectedHouse::None }

			, SW_Warhead {}
			, SW_Anim {}
			, SW_Sound {}
			, SW_ActivationSound {}

			, SpyPlanes_TypeIndex {}
			, SpyPlanes_Count {}
			, SpyPlanes_Mission {}
			, SpyPlanes_Rank {}

			, SW_Power {}

			, Battery_Overpower {}
			, Battery_KeepOnline {}

			, SW_AffectsTarget { SuperWeaponTarget::All }

			, SW_PostDependent {}

#pragma region Chronosphere
			, Chronosphere_BlastSrc {}
			, Chronosphere_BlastDest {}
			, Chronosphere_KillOrganic { true }
			, Chronosphere_KillTeleporters { true }
			, Chronosphere_AffectUndeployable { false }
			, Chronosphere_AffectBuildings { false }
			, Chronosphere_AffectUnwarpable { false }
			, Chronosphere_AffectIronCurtain { false }
			, Chronosphere_BlowUnplaceable { true }
			, Chronosphere_ReconsiderBuildings { false }
			, Chronosphere_Delay {}
#pragma endregion
			, SW_Deferment {}

#pragma region Psychic Dominator
			, Dominator_Capture{ true }
			, Dominator_FireAtPercentage{  }
			, Dominator_FirstAnimHeight{ 1 }
			, Dominator_SecondAnimHeight{ 1 }
			, Dominator_FirstAnim{  }
			, Dominator_SecondAnim{  }
			, Dominator_ControlAnim{  }
			, Dominator_Ripple{ true }
			, Dominator_CaptureMindControlled{ true }
			, Dominator_CapturePermaMindControlled{ true  }
			, Dominator_CaptureImmuneToPsionics{ false }
			, Dominator_PermanentCapture{ true }
#pragma endregion

			, Message_Activate {}
			, Message_Abort{}
			, DropPod_Minimum {}
			, DropPod_Maximum {}
			, DropPod_Veterancy {2.0}
			, DropPod_Types {}

			, EMPField_Duration { 0 }
			, SW_MaxCount { -1 }

			, EMPulse_Linked { false }
			, EMPulse_TargetSelf { false }
			, EMPulse_PulseDelay { 32 }
			, EMPulse_PulseBall {}
			, EMPulse_Cannons {}

			, Mutate_Explosion {  }
			, Mutate_IgnoreCyborg { false }
			, Mutate_IgnoreNotHuman { true }
			, Mutate_KillNatural { true  }
		{ }


		void FireSuperWeapon(SuperClass* pSW, HouseClass* pHouse, const CellStruct* const pCell, bool IsCurrentPlayer);

		bool IsInhibitor(HouseClass* pOwner, TechnoClass* pTechno);
		bool HasInhibitor(HouseClass* pOwner, const CellStruct& Coords);
		bool IsInhibitorEligible(HouseClass* pOwner, const CellStruct& Coords, TechnoClass* pTechno);
		bool IsDesignator(HouseClass* pOwner, TechnoClass* pTechno) const;
		bool HasDesignator(HouseClass* pOwner, const CellStruct& coords) const;
		bool IsDesignatorEligible(HouseClass* pOwner, const CellStruct& coords, TechnoClass* pTechno) const;

		bool IsLaunchSiteEligible(const CellStruct& Coords, BuildingClass* pBuilding, bool ignoreRange) const;
		bool IsLaunchSite(BuildingClass* pBuilding) const;
		std::pair<double, double> GetLaunchSiteRange(BuildingClass* pBuilding = nullptr) const;
		bool IsAvailable(HouseClass* pHouse);

		void ApplyDetonation(HouseClass* pHouse, const CellStruct& cell);
		void ApplySWNext(SuperClass* pSW, const CellStruct& cell);

		void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);
		void LoadFromRulesFile(CCINIClass* pINI);
		virtual ~ExtData() override = default;
		void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
		void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }
		void Initialize();

		const char* get_ID();

		//with arg(s)

		// check shootamount
		bool CanFire(HouseClass* pOwner);
		bool CanFireAt(HouseClass* pOwner, const CellStruct& coords, bool manual);
		bool IsAnimVisible(HouseClass* pFirer);
		bool IsHouseAffected(HouseClass* pFirer, HouseClass* pHouse);
		bool IsHouseAffected(HouseClass* pFirer, HouseClass* pHouse, AffectedHouse value);
		bool Launch(NewSWType* pNewType, SuperClass* pSuper, CellStruct const cell, bool const isPlayer);
		void PrintMessage(const CSFText& message, HouseClass* pFirer);
		Iterator<TechnoClass*> GetPotentialAITargets(HouseClass* pTarget = nullptr) const;
		bool IsCellEligible(CellClass* pCell, SuperWeaponTarget allowed);
		bool IsTechnoEligible(TechnoClass* pTechno, SuperWeaponTarget allowed);
		bool IsTechnoAffected(TechnoClass* pTechno);

		//no arg(s)
		double GetChargeToDrainRatio() const;
		SuperWeaponTarget GetAIRequiredTarget() const;
		AffectedHouse GetAIRequiredHouse() const;
		TargetingConstraint GetAITargetingConstraint() const;
		TargetingPreference GetAITargetingPreference() const;
		bool UpdateLightingColor(LightingColor& Lighting) const;
		// is this an original type handled by a NewSWType?
		bool IsTypeRedirected() const;
		bool IsOriginalType() const;
		NewSWType* GetNewSWType() const;

		//statics 
		static bool Deactivate(SuperClass* pSuper, CellStruct const cell, bool const isPlayer);
		static bool Activate(SuperClass* pSuper, CellStruct const cell, bool const isPlayer);
		static AffectedHouse GetRelation(HouseClass* pFirer, HouseClass* pHouse);
		static Action GetAction(SuperWeaponTypeClass* pSuper, CellStruct* pTarget);
		static bool TryFire(SuperClass* pThis, bool IsPlayer);
		static bool IsTargetConstraintEligible(SuperClass* pThis, bool IsPlayer);
		static TargetResult PickSuperWeaponTarget(SuperClass* pSuper);

	private:

		void WeightedRollsHandler(std::vector<int>& nResult, std::vector<float>* chances, std::vector<std::vector<int>>* weights, size_t size);

		void ApplyLimboDelivery(HouseClass* pHouse);
		void ApplyLimboKill(HouseClass* pHouse);

		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<SWTypeExt::ExtData>
	{
	public:
		ExtContainer();
		~ExtContainer();

		static void InvalidatePointer(void* ptr, bool bRemoved);
		static  bool InvalidateIgnorable(void* ptr);
		static bool LoadGlobals(PhobosStreamReader& Stm);
		static bool SaveGlobals(PhobosStreamWriter& Stm);
		static void Clear();
	};

	static ExtContainer ExtMap;
	static void LimboDeliver(BuildingTypeClass* pType, HouseClass* pOwner, int ID);
	static void WeightedRollsHandler(std::vector<int>& nResult, Valueable<double>& RandomBuffer, const ValueableVector<float>& rolls, const ValueableVector<ValueableVector<int>>& weights, size_t size);
	static void Launch(HouseClass* pHouse, SWTypeExt::ExtData* pLauncherTypeExt, int pLaunchedType, const CellStruct& cell);
	static void ClearChronoAnim(SuperClass* pThis);
	static void CreateChronoAnim(SuperClass* pThis, const CoordStruct& Coords, AnimTypeClass* pAnimType);
	static bool ChangeLighting(SuperWeaponTypeClass* pCustom = nullptr);
	static LightingColor GetLightingColor(SuperWeaponTypeClass* pCustom = nullptr);
};