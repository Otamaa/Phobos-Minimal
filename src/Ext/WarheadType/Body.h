#pragma once
#include <WarheadTypeClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>
#include <New/Type/ShieldTypeClass.h>
#include <New/Entity/LauchSWData.h>

#include <New/Type/ArmorTypeClass.h>
#include <New/Type/ImmunityTypeClass.h>

#include <Misc/DynamicPatcher/Others/DamageText.h>
#include <Misc/DynamicPatcher/AttachedAffects/Effects/PaintBall/PaintBall.h>

#include <New/AnonymousType/AresAttachEffectTypeClass.h>
#include <Utilities/VersesData.h>

typedef std::vector<std::tuple< std::vector<int>, std::vector<int>, TransactValueType>> TransactData;

struct args_ReceiveDamage;
class ArmorTypeClass;
class WarheadTypeExtData final
{
public:
	static constexpr size_t Canary = 0x22242222;
	using base_type = WarheadTypeClass;
	//static constexpr size_t ExtOffset = 0x1CC; //ares

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
public:

	Valueable<int> Reveal { 0 };
	Valueable<bool> BigGap { false };
	Valueable<int> TransactMoney { 0 };
	Nullable<int> TransactMoney_Ally { };
	Nullable<int> TransactMoney_Enemy { };
	Valueable<bool> Transact_AffectsEnemies { false };
	Valueable<bool> Transact_AffectsAlly { false };
	Valueable<bool> Transact_AffectsOwner { true };
	Valueable<bool> TransactMoney_Display { false };
	Valueable<AffectedHouse> TransactMoney_Display_Houses { AffectedHouse::All };
	Valueable<bool> TransactMoney_Display_AtFirer { false };
	Valueable<Point2D> TransactMoney_Display_Offset { { 0, 0 } };

	Valueable<int> StealMoney { 0 };
	Valueable<AffectedHouse> Steal_Display_Houses { AffectedHouse::All };
	Valueable<bool> Steal_Display { false };
	Valueable<Point2D> Steal_Display_Offset { { 0, 0 } };

	NullableVector<AnimTypeClass*> SplashList {};
	Valueable<bool> SplashList_PickRandom { false };
	Valueable<bool> SplashList_CreateAll { false };
	Valueable<int> SplashList_CreationInterval { 0 };

	Valueable<bool> RemoveDisguise { false };
	Valueable<bool> RemoveMindControl { false };
	Nullable<bool> AnimList_PickRandom { };
	Valueable<bool> AnimList_CreateAll { false };
	Valueable<int> AnimList_CreationInterval { 0 };
	Valueable<bool> AnimList_ShowOnZeroDamage { false };
	Valueable<bool> DecloakDamagedTargets { true };
	Valueable<bool> ShakeIsLocal { false };

	Valueable<double> Crit_Chance { 0.0 };
	Valueable<bool> Crit_ApplyChancePerTarget { false };
	Valueable<int> Crit_ExtraDamage { 0 };
	Nullable<WarheadTypeClass*> Crit_Warhead {};
	Valueable<AffectedTarget> Crit_Affects { AffectedTarget::All };
	Valueable<AffectedHouse> Crit_AffectsHouses { AffectedHouse::All };
	ValueableVector<AnimTypeClass*> Crit_AnimList {};
	Nullable<bool> Crit_AnimList_PickRandom {};
	Valueable<bool> Crit_AnimOnAffectedTargets { false };
	Valueable<double> Crit_AffectBelowPercent { 1.0 };
	Valueable<bool> Crit_SuppressOnIntercept { false };
	NullablePromotable<double> Crit_GuaranteeAfterHealthTreshold {};

	double RandomBuffer { 0.0 };
	bool HasCrit { false };

	Nullable<AnimTypeClass*> MindControl_Anim {};

	// Ares tags
	// http://ares-developers.github.io/Ares-docs/new/warheads/general.html
	Valueable<bool> AffectsEnemies { true };
	Nullable<bool> AffectsOwner {};
	Valueable<bool> EffectsRequireDamage { false };
	Valueable<bool> EffectsRequireVerses { true };
	Valueable<bool> AllowZeroDamage { false };

	Valueable<bool> Shield_Penetrate { false };
	Valueable<bool> Shield_Break { false };
	Nullable<AnimTypeClass*> Shield_BreakAnim {};
	Nullable<AnimTypeClass*> Shield_HitAnim {};
	Nullable<WeaponTypeClass*> Shield_BreakWeapon {};

	Nullable<double> Shield_AbsorbPercent {};
	Nullable<double> Shield_PassPercent {};
	Nullable<int> Shield_ReceivedDamage_Minimum {};
	Nullable<int> Shield_ReceivedDamage_Maximum {};

	Valueable<int> Shield_Respawn_Duration { 0 };
	Valueable<double> Shield_Respawn_Amount { 0.0 };
	Valueable<int> Shield_Respawn_Rate { -1 };

private:
	Valueable<double> Shield_Respawn_Rate_InMinutes { -1.0 };
	Valueable<double> Shield_SelfHealing_Rate_InMinutes { -1.0 };

public:

	Valueable<bool> Shield_Respawn_RestartTimer { false };
	Valueable<int> Shield_SelfHealing_Duration { 0 };
	Nullable<double> Shield_SelfHealing_Amount { };
	Valueable<int> Shield_SelfHealing_Rate { -1 };
	Nullable<bool> Shield_SelfHealing_RestartInCombat {};
	Valueable<int> Shield_SelfHealing_RestartInCombatDelay { -1 };
	Valueable<bool> Shield_SelfHealing_RestartTimer { false };

	ValueableVector<ShieldTypeClass*> Shield_AttachTypes { };
	ValueableVector<ShieldTypeClass*> Shield_RemoveTypes { };

	Valueable<bool> Shield_ReplaceOnly { false };
	Valueable<bool> Shield_ReplaceNonRespawning { false };
	Valueable<bool> Shield_InheritStateOnReplace { false };
	Valueable<int> Shield_MinimumReplaceDelay { 0 };
	ValueableVector<ShieldTypeClass*> Shield_AffectTypes { };

	NullableVector<ShieldTypeClass*> Shield_Penetrate_Types { };
	NullableVector<ShieldTypeClass*> Shield_Break_Types { };
	NullableVector<ShieldTypeClass*> Shield_Respawn_Types { };
	NullableVector<ShieldTypeClass*> Shield_SelfHealing_Types { };

	Valueable<bool> Transact { false };
	Valueable<int> Transact_Experience_Value { 1 };
	Valueable<int> Transact_Experience_Source_Flat { 0 };
	Valueable<double> Transact_Experience_Source_Percent { 0.0 };
	Valueable<bool> Transact_Experience_Source_Percent_CalcFromTarget { false };
	Valueable<int> Transact_Experience_Target_Flat { 0 };
	Valueable<double> Transact_Experience_Target_Percent { 0.0 };
	Valueable<bool> Transact_Experience_Target_Percent_CalcFromSource { false };
	Valueable<bool> Transact_SpreadAmongTargets { false };
	Valueable<bool> Transact_Experience_IgnoreNotTrainable { true };

	Nullable<int> NotHuman_DeathSequence { };
	Nullable<bool> AllowDamageOnSelf { };
	Valueable<bool> Debris_Conventional { false };
	Valueable<int> GattlingStage { 0 };
	Valueable<int> GattlingRateUp { 0 };
	Valueable<int> ReloadAmmo { 0 };

	Valueable<bool> MindControl_UseTreshold { false };
	Valueable<double> MindControl_Threshold { 1.0 };
	Valueable<bool> MindControl_Threshold_Inverse { };
	Nullable<int> MindControl_AlternateDamage { };
	Nullable<WarheadTypeClass*> MindControl_AlternateWarhead { };
	Valueable<bool> MindControl_CanKill { false };

	Valueable<bool> DetonateOnAllMapObjects { false };
	Valueable<bool> DetonateOnAllMapObjects_RequireVerses { false };
	Valueable<AffectedTarget> DetonateOnAllMapObjects_AffectTargets { AffectedTarget::All };
	Valueable<AffectedHouse> DetonateOnAllMapObjects_AffectHouses { AffectedHouse::All };
	ValueableVector<TechnoTypeClass*> DetonateOnAllMapObjects_AffectTypes {};
	ValueableVector<TechnoTypeClass*> DetonateOnAllMapObjects_IgnoreTypes {};

	Nullable<WeaponTypeClass*> RevengeWeapon {};
	Valueable<int> RevengeWeapon_GrantDuration { 0 };
	Valueable<AffectedHouse> RevengeWeapon_AffectsHouses { AffectedHouse::All };
	Valueable<bool> RevengeWeapon_Cumulative { false };
	Valueable<int> RevengeWeapon_MaxCount { -1 };

	bool WasDetonatedOnAllMapObjects { false };
	bool Splashed { false };
	int RemainingAnimCreationInterval { 0 };

	Nullable<AnimTypeClass*> NotHuman_DeathAnim { };

	Valueable<bool> IsNukeWarhead { false };
	Valueable<AnimTypeClass*> PreImpactAnim {};
	Valueable<int> NukeFlashDuration { };

	Valueable<bool> Remover { false };
	Valueable<AnimTypeClass*> Remover_Anim { nullptr };

	PhobosMap<ArmorTypeClass*, AnimTypeClass*> ArmorHitAnim {};

	NullableVector<AnimTypeClass*> DebrisAnimTypes {};
	NullableVector<AnimTypeClass*> SquidSplash {};

	Valueable<AnimTypeClass*> TemporalExpiredAnim { nullptr };
	Valueable<bool> TemporalExpiredApplyDamage { false };
	Valueable<double> TemporalDetachDamageFactor { 1.0 };

	Valueable<bool> Parasite_DisableRocking { false };
	Nullable<AnimTypeClass*> Parasite_GrappleAnim {};
	Nullable<ParticleSystemTypeClass*> Parasite_ParticleSys {};
	Nullable<bool> Parasite_TreatInfantryAsVehicle {};
	Nullable<WeaponTypeClass*> Parasite_InvestationWP {};
	Nullable<double> Parasite_Damaging_Chance {};

	Nullable<int> Flammability {};

	std::vector<LauchSWData> Launchs {};

	Valueable<bool> PermaMC { false };
	ValueableIdx<VocClass> Sound { -1 };

	Valueable<bool> Converts { false };

	ValueableVector<TechnoTypeConvertData> ConvertsPair {};
	Valueable<AnimTypeClass*> Convert_SucceededAnim { nullptr };

	ValueableVector<AnimTypeClass*> DeadBodies {};

	Nullable<double> AffectEnemies_Damage_Mod {};
	Nullable<double> AffectOwner_Damage_Mod {};
	Nullable<double> AffectAlly_Damage_Mod {};

	PhobosFixedString<32U> AttachTag {};
	Valueable<bool> AttachTag_Imposed { false };
	NullableVector<TechnoTypeClass*> AttachTag_Types {};
	NullableVector<TechnoTypeClass*> AttachTag_Ignore {};


	Valueable<bool> RecalculateDistanceDamage { false };
	Valueable<bool> RecalculateDistanceDamage_IgnoreMaxDamage { false };
	Valueable<int> RecalculateDistanceDamage_Add { 0 };
	Valueable<double> RecalculateDistanceDamage_Add_Factor { 1.0 };
	Valueable<double> RecalculateDistanceDamage_Multiply { 1.0 };
	Valueable<double> RecalculateDistanceDamage_Multiply_Factor { 1.0 };
	Valueable<int> RecalculateDistanceDamage_Max { INT_MAX };
	Valueable<int> RecalculateDistanceDamage_Min { -INT_MAX };
	Valueable<bool> RecalculateDistanceDamage_Display { false };
	Valueable<bool> RecalculateDistanceDamage_Display_AtFirer { false };
	Valueable<Point2D> RecalculateDistanceDamage_Display_Offset { Point2D::Empty };
	Valueable<bool> RecalculateDistanceDamage_ProcessVerses { false };

	AresAttachEffectTypeClass AttachedEffect {};
	ValueableVector<WeaponTypeClass*> DetonatesWeapons {};
	ValueableVector<int> LimboKill_IDs {};
	Valueable<AffectedHouse> LimboKill_Affected { AffectedHouse::Owner };

	Valueable<AnimTypeClass*> InfDeathAnim { nullptr };

	Promotable<int> Culling_BelowHP { 0, -1, -2 };
	Promotable<int> Culling_Chance { 100 };

	Valueable<bool> RelativeDamage { false };
	Valueable<int> RelativeDamage_AirCraft { 0 };
	Valueable<int> RelativeDamage_Unit { 0 };
	Valueable<int> RelativeDamage_Infantry { 0 };
	Valueable<int> RelativeDamage_Building { 0 };
	Valueable<int> RelativeDamage_Terrain { 0 };

	std::vector<VersesData> Verses {};

	Nullable<int> Berzerk_dur {};
	Valueable<int> Berzerk_cap { -1 };
	Valueable<bool> Berzerk_dealDamage { false };

	Nullable<bool> IC_Flash {};
	Valueable<bool> PreventScatter { false };
	NullableIdx<VocClass> DieSound_Override {};
	NullableIdx<VocClass> VoiceSound_Override {};

	Valueable<bool> SuppressDeathWeapon_Vehicles { false };
	Valueable<bool> SuppressDeathWeapon_Infantry { false };
	ValueableVector<TechnoTypeClass*> SuppressDeathWeapon {};
	ValueableVector<TechnoTypeClass*> SuppressDeathWeapon_Exclude {};
	Nullable<double> SuppressDeathWeapon_Chance {};
	Promotable<double> DeployedDamage { 1.00 };

	Nullable<AnimTypeClass*> Temporal_WarpAway {};
	Valueable<bool> Supress_LostEva { false };
	Valueable<double> Temporal_HealthFactor { 1.0 };

	PhobosMap<InfantryTypeClass* , AnimTypeClass*> InfDeathAnims {};

	Valueable<int> Sonar_Duration { 0 };
	Valueable<int> DisableWeapons_Duration { 0 };
	Valueable<int> Flash_Duration { 0 };

	NullableIdx<ImmunityTypeClass> ImmunityType {};

	Valueable<bool> Malicious { true };
	Valueable<bool> PreImpact_Moves { false };
	Valueable<bool> Conventional_IgnoreUnits { false };

	Valueable<bool> InflictLocomotor { false };
	Valueable<bool> RemoveInflictedLocomotor { false };

	Nullable<int> Rocker_Damage {};

	Valueable<int> PaintBallDuration { -1 };
	PaintballType PaintBallData { };

	ValueableIdx<SuperWeaponTypeClass> NukePayload_LinkedSW { -1 };
	Valueable<int> IC_Duration { 0 };
	Valueable<int> IC_Cap { -1 };

#pragma region Ion
	Valueable<bool> Ion { false };
	Nullable<int> Ripple_Radius {};
	Nullable<AnimTypeClass*> Ion_Beam {};
	Nullable<AnimTypeClass*> Ion_Blast {};
	Valueable<bool> Ion_AllowWater { false };
	Valueable<bool> Ion_Rocking { true };
	Nullable<WarheadTypeClass*> Ion_WH {};
	Nullable<int> Ion_Damage {};
#pragma endregion

	ValueableVector<ParticleSystemTypeClass*> DetonateParticleSystem {};
	Nullable<bool> BridgeAbsoluteDestroyer {};
	Valueable<int> DamageAirThreshold { 0 };
	Valueable<int> CellSpread_MaxAffect { -1 };

	Valueable<int> EMP_Duration { 0 };
	Valueable<int> EMP_Cap { -1 };
	Valueable<AnimTypeClass*> EMP_Sparkles { nullptr };

	Nullable<bool> CanRemoveParasytes { };
	Valueable<bool> CanRemoveParasytes_KickOut { false };
	Valueable<int> CanRemoveParasytes_KickOut_Paralysis { -1 };
	NullableIdx<VocClass> CanRemoveParasytes_ReportSound { };
	Nullable<AnimTypeClass*> CanRemoveParasytes_KickOut_Anim { nullptr };

	Valueable<bool> Webby { false };
	ValueableVector<AnimTypeClass*> Webby_Anims {};
	Valueable<int> Webby_Duration { 0 };
	Valueable<int> Webby_Cap { -1 };
	Valueable<int> Webby_Duration_Variation { 0 };

	NullablePromotable<int> SelfHealing_CombatDelay { };

	Valueable<bool> KillDriver { false };
	Valueable<double> KillDriver_KillBelowPercent { 1.00 };
	Valueable<OwnerHouseKind> KillDriver_Owner { OwnerHouseKind::Special };
	Valueable<bool> KillDriver_ResetVeterancy { false };
	Valueable<double> KillDriver_Chance { 100.0 };

	Valueable<bool> ApplyModifiersOnNegativeDamage { false };

public:

	WarheadTypeExtData() noexcept = default;
	~WarheadTypeExtData() noexcept = default;

	void InitializeConstant();
	void ApplyRemoveDisguise(HouseClass* pHouse, TechnoClass* pTarget) const;
	void ApplyRemoveMindControl(HouseClass* pHouse, TechnoClass* pTarget) const;

	bool applyPermaMC(HouseClass* const Owner, AbstractClass* const Target) const;

	void ApplyLocomotorInfliction(TechnoClass* pTarget) const;
	void ApplyLocomotorInflictionReset(TechnoClass* pTarget) const;

	void applyIronCurtain(TechnoClass* items, HouseClass* Owner, int damage) const;
	void applyIronCurtain(const CoordStruct& coords, HouseClass* pOwner, int damage) const;

private:

	void EvaluateArmor(WarheadTypeClass* OwnerObject);
	void DetonateOnOneUnit(HouseClass* pHouse, TechnoClass* pTarget, int damage, TechnoClass* pOwner = nullptr, BulletClass* pBullet = nullptr, bool bulletWasIntercepted = false);

	void ApplyCrit(HouseClass* pHouse, TechnoClass* pTarget, TechnoClass* Owner);
	void ApplyShieldModifiers(TechnoClass* pTarget) const;

	void ApplyGattlingStage(TechnoClass* pTarget, int Stage) const;
	void ApplyGattlingRateUp(TechnoClass* pTarget, int RateUp) const;
	void ApplyReloadAmmo(TechnoClass* pTarget, int ReloadAmount) const;

	void ApplyAttachTag(TechnoClass* pTarget) const;
	void ApplyDirectional(BulletClass* pBullet, TechnoClass* pTarget) const;

	void applyWebby(TechnoClass* pTarget, HouseClass* pKillerHouse, TechnoClass* pKillerTech) const;
	//Otamaa
	void applyTransactMoney(TechnoClass* pOwner, HouseClass* pHouse, BulletClass* pBullet, CoordStruct const& coords) const;
	void applyStealMoney(TechnoClass* const Owner, TechnoClass* const Target) const;

	void ApplyUpgrade(HouseClass* pHouse, TechnoClass* pTarget) const;

	//void DetonateOnAllUnits(HouseClass* pHouse, const CoordStruct coords, const float cellSpread, TechnoClass* pOwner);
	void TransactOnOneUnit(TechnoClass* pTarget, TechnoClass* pOwner, int targets);
	void TransactOnAllUnits(std::vector<TechnoClass*>& nVec, HouseClass* pHouse, TechnoClass* pOwner);
	int TransactGetValue(TechnoClass* pTarget, TechnoClass* pOwner, int flat, double percent, bool calcFromTarget);
	TransactData TransactGetSourceAndTarget(TechnoClass* pTarget, TechnoTypeClass* pTargetType, TechnoClass* pOwner, TechnoTypeClass* pOwnerType, int targets);
	int TransactOneValue(TechnoClass* pTechno, TechnoTypeClass* pTechnoType, int transactValue, TransactValueType valueType);

public:
	void Detonate(TechnoClass* pOwner, HouseClass* pHouse, BulletClass* pBullet, CoordStruct coords, int damage);
	bool CanTargetHouse(HouseClass* pHouse, TechnoClass* pTechno) const;
	void InterceptBullets(TechnoClass* pOwner, WeaponTypeClass* pWeapon, CoordStruct coords) const;
	bool CanAffectHouse(HouseClass* pOwnerHouse, HouseClass* pTargetHouse) const;
	bool CanDealDamage(TechnoClass* pTechno, int damageIn, int distanceFromEpicenter, int& DamageResult, bool effectsRequireDamage = false) const;
	bool CanDealDamage(TechnoClass* pTechno, bool Bypass = false, bool SkipVerses = false) const;
	FullMapDetonateResult EligibleForFullMapDetonation(TechnoClass* pTechno, HouseClass* pOwner) const;
	void ApplyDamageMult(TechnoClass* pVictim, args_ReceiveDamage* pArgs) const;
	void ApplyRecalculateDistanceDamage(ObjectClass* pVictim, args_ReceiveDamage* pArgs) const;
	void ApplyRevengeWeapon(TechnoClass* pTarget) const;
	bool applyCulling(TechnoClass* pSource, ObjectClass* pTarget) const;
	void applyRelativeDamage(ObjectClass* pTarget, args_ReceiveDamage* pArgs) const;
	bool GoBerzerkFor(FootClass* pVictim, int* damage) const;
	bool ApplySuppressDeathWeapon(TechnoClass* pVictim) const;

	VersesData& GetVerses(Armor armor)
	{
		return this->Verses[static_cast<int>(armor)];
	}

	const VersesData& GetVerses(Armor armor) const
	{
		return this->Verses[static_cast<int>(armor)];
	}

	void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);
	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }
	void Initialize();

	AnimTypeClass* GetArmorHitAnim(int Armor);

private:
	template <typename T>
	void Serialize(T& Stm);
public:
	static PhobosMap<IonBlastClass*, WarheadTypeExtData*> IonBlastExt;
	static void DetonateAt(
		WarheadTypeClass* pThis,
		ObjectClass* pTarget,
		TechnoClass* pOwner,
		int damage,
		bool targetCell = false,
		HouseClass* pFiringHouse = nullptr
	);

	static void DetonateAt(
		WarheadTypeClass* pThis,
		const CoordStruct& coords,
		TechnoClass* pOwner,
		int damage,
		bool targetCell = false,
		HouseClass* pFiringHouse = nullptr
	);

	static void DetonateAt(
		WarheadTypeClass* pThis,
		AbstractClass* pTarget,
		const CoordStruct& coords,
		TechnoClass* pOwner,
		int damage,
		HouseClass* pFiringHouse = nullptr
	);

	static void CreateIonBlast(WarheadTypeClass* pThis, const CoordStruct& coords);

	static void applyEMP(WarheadTypeClass* pWH, const CoordStruct& coords, TechnoClass* source);
};

class WarheadTypeExtContainer final : public Container<WarheadTypeExtData>
{
public:
	static WarheadTypeExtContainer Instance;

	CONSTEXPR_NOCOPY_CLASSB(WarheadTypeExtContainer, WarheadTypeExtData, "WarheadTypeClass");
public:

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
	static void Clear();
};

