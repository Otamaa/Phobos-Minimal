#pragma once
#include <WarheadTypeClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>
#include <New/Type/ShieldTypeClass.h>
#include <New/Entity/LauchSWData.h>

#ifdef COMPILE_PORTED_DP_FEATURES
#include <Misc/DynamicPatcher/Others/DamageText.h>
#include <Misc/DynamicPatcher/AttachedAffects/Effects/PaintBall/PaintBall.h>
#endif
class WarheadTypeExt
{
public:
	using base_type = WarheadTypeClass;

	class ExtData final : public Extension<WarheadTypeClass>
	{
	public:

		Valueable<bool> SpySat;
		Valueable<bool> BigGap;
		Valueable<int> TransactMoney;
		Nullable<int> TransactMoney_Ally;
		Nullable<int> TransactMoney_Enemy;
		Valueable<bool> Transact_AffectsEnemies;
		Valueable<bool> Transact_AffectsAlly;
		Valueable<bool> Transact_AffectsOwner;
		Valueable<bool> TransactMoney_Display;
		Valueable<AffectedHouse> TransactMoney_Display_Houses;
		Valueable<bool> TransactMoney_Display_AtFirer;
		Valueable<Point2D> TransactMoney_Display_Offset;

		Valueable<int> StealMoney;
		Valueable<AffectedHouse> Steal_Display_Houses;
		Valueable<bool> Steal_Display;
		Valueable<Point2D> Steal_Display_Offset;

		ValueableVector<AnimTypeClass*> SplashList;
		Valueable<bool> SplashList_PickRandom;
		Valueable<bool> RemoveDisguise;
		Valueable<bool> RemoveMindControl;
		Valueable<bool> AnimList_PickRandom;
		Valueable<bool> DecloakDamagedTargets;
		Valueable<bool> ShakeIsLocal;

		Valueable<double> Crit_Chance;
		Valueable<bool> Crit_ApplyChancePerTarget;
		Valueable<int> Crit_ExtraDamage;
		Nullable<WarheadTypeClass*> Crit_Warhead;
		Valueable<AffectedTarget> Crit_Affects;
		ValueableVector<AnimTypeClass*> Crit_AnimList;
		Nullable<bool> Crit_AnimList_PickRandom;
		Valueable<bool> Crit_AnimOnAffectedTargets;
		Valueable<double> Crit_AffectBelowPercent;
		Valueable<bool> Crit_SuppressOnIntercept;

		double RandomBuffer;
		bool HasCrit;

		Nullable<AnimTypeClass*> MindControl_Anim;

		// Ares tags
		// http://ares-developers.github.io/Ares-docs/new/warheads/general.html
		Valueable<bool> AffectsEnemies;
		Nullable<bool> AffectsOwner;
		Valueable<bool> EffectsRequireDamage;
		Valueable<bool> EffectsRequireVerses;
		Valueable<bool> AllowZeroDamage;

		Valueable<bool> Shield_Penetrate;
		Valueable<bool> Shield_Break;
		Nullable<AnimTypeClass*> Shield_BreakAnim;
		Nullable<AnimTypeClass*> Shield_HitAnim;
		Nullable<WeaponTypeClass*> Shield_BreakWeapon;

		Nullable<double> Shield_AbsorbPercent;
		Nullable<double> Shield_PassPercent;

		Valueable<int> Shield_Respawn_Duration;
		Valueable<double> Shield_Respawn_Amount;
		Valueable<int> Shield_Respawn_Rate;

	private:
		Valueable<double> Shield_Respawn_Rate_InMinutes;
		Valueable<double> Shield_SelfHealing_Rate_InMinutes;

	public:

		Valueable<bool> Shield_Respawn_ResetTimer;
		Valueable<int> Shield_SelfHealing_Duration;
		Nullable<double> Shield_SelfHealing_Amount;
		Valueable<int> Shield_SelfHealing_Rate;
		Valueable<bool> Shield_SelfHealing_ResetTimer;

		ValueableVector<ShieldTypeClass*> Shield_AttachTypes;
		ValueableVector<ShieldTypeClass*> Shield_RemoveTypes;
		Valueable<bool> Shield_ReplaceOnly;
		Valueable<bool> Shield_ReplaceNonRespawning;
		Valueable<bool> Shield_InheritStateOnReplace;
		Valueable<int> Shield_MinimumReplaceDelay;
		ValueableVector<ShieldTypeClass*> Shield_AffectTypes;

		Valueable<bool> Transact;
		Valueable<int> Transact_Experience_Value;
		Valueable<int> Transact_Experience_Source_Flat;
		Valueable<double> Transact_Experience_Source_Percent;
		Valueable<bool> Transact_Experience_Source_Percent_CalcFromTarget;
		Valueable<int> Transact_Experience_Target_Flat;
		Valueable<double> Transact_Experience_Target_Percent;
		Valueable<bool> Transact_Experience_Target_Percent_CalcFromSource;
		Valueable<bool> Transact_SpreadAmongTargets;

		Valueable<int> NotHuman_DeathSequence;
		Nullable<bool> AllowDamageOnSelf;
		Valueable<bool> Debris_Conventional;
		Valueable<int> GattlingStage;
		Valueable<int> GattlingRateUp;
		Valueable<int> ReloadAmmo;

		Valueable<double> MindControl_Threshold;
		Valueable<bool> MindControl_Threshold_Inverse;
		Nullable<int> MindControl_AlternateDamage;
		Nullable<WarheadTypeClass*> MindControl_AlternateWarhead;
		Valueable<bool> MindControl_CanKill;

		Valueable<bool> DetonateOnAllMapObjects;
		Valueable<bool> DetonateOnAllMapObjects_RequireVerses;
		Valueable<AffectedTarget> DetonateOnAllMapObjects_AffectTargets;
		Valueable<AffectedHouse> DetonateOnAllMapObjects_AffectHouses;
		ValueableVector<TechnoTypeClass*> DetonateOnAllMapObjects_AffectTypes;
		ValueableVector<TechnoTypeClass*> DetonateOnAllMapObjects_IgnoreTypes;

		bool WasDetonatedOnAllMapObjects;

		#pragma region Otamaa
		Nullable<AnimTypeClass*> NotHuman_DeathAnim;

		Valueable<bool> IsNukeWarhead;

		Valueable<bool> Remover;
		Valueable<AnimTypeClass*> Remover_Anim;

		PhobosMap<int, AnimTypeClass*> ArmorHitAnim;

		NullableVector<AnimTypeClass*> DebrisAnimTypes;
		NullableVector<AnimTypeClass*> SquidSplash;

		Valueable<AnimTypeClass*> TemporalExpiredAnim;
		Valueable<bool> TemporalExpiredApplyDamage;
		Valueable<double> TemporalDetachDamageFactor;

		Valueable<bool> Parasite_DisableRocking;
		NullableIdx<AnimTypeClass> Parasite_GrappleAnimIndex;
		Nullable<ParticleSystemTypeClass*> Parasite_ParticleSys;
		Valueable<bool> Parasite_TreatInfantryAsVehicle;

		Nullable<int> Flammability;

		std::vector<LauchSWData> Launchs;

		Valueable<bool> PermaMC;

	#ifdef COMPILE_PORTED_DP_FEATURES_
		PhobosMap<int, DamageTextTypeData> DamageTextPerArmor;
	#endif
	#ifdef COMPILE_PORTED_DP_FEATURES
		Valueable<int> PaintBallDuration;
		PaintballType PaintBallData;
	#endif
		#pragma endregion
		ExtData(WarheadTypeClass* OwnerObject) : Extension<WarheadTypeClass>(OwnerObject)
			, SpySat { false }
			, BigGap { false }
			, TransactMoney { 0 }
			, TransactMoney_Ally { }
			, TransactMoney_Enemy { }
			, Transact_AffectsEnemies { false }
			, Transact_AffectsAlly { false }
			, Transact_AffectsOwner { true }
			, TransactMoney_Display { false }
			, TransactMoney_Display_Houses { AffectedHouse::All }
			, TransactMoney_Display_AtFirer { false }
			, TransactMoney_Display_Offset { { 0, 0 } }

			, StealMoney{ 0 }
			, Steal_Display_Houses{ AffectedHouse::All }
			, Steal_Display{ false }
			, Steal_Display_Offset{ { 0, 0 } }

			, SplashList {}
			, SplashList_PickRandom { false }
			, RemoveDisguise { false }
			, RemoveMindControl { false }
			, AnimList_PickRandom { false }
			, DecloakDamagedTargets { true }
			, ShakeIsLocal { false }

			, Crit_Chance { 0.0 }
			, Crit_ApplyChancePerTarget { false }
			, Crit_ExtraDamage { 0 }
			, Crit_Warhead {}
			, Crit_Affects { AffectedTarget::All }
			, Crit_AnimList {}
			, Crit_AnimList_PickRandom {}
			, Crit_AnimOnAffectedTargets { false }
			, Crit_AffectBelowPercent { 1.0 }
			, Crit_SuppressOnIntercept { false }

			, RandomBuffer { 0.0 }
			, HasCrit { false }

			, MindControl_Anim {}

			, AffectsEnemies { true }
			, AffectsOwner {}
			, EffectsRequireDamage { false }
			, EffectsRequireVerses { true }
			, AllowZeroDamage { false }
			, Shield_Penetrate { false }
			, Shield_Break { false }
			, Shield_BreakAnim {}
			, Shield_HitAnim {}
			, Shield_BreakWeapon {}
			, Shield_AbsorbPercent {}
			, Shield_PassPercent {}

			, Shield_Respawn_Duration { 0 }
			, Shield_Respawn_Amount { 0.0 }
			, Shield_Respawn_Rate { -1 }
			, Shield_Respawn_Rate_InMinutes { -1.0 }
			, Shield_SelfHealing_Rate_InMinutes { -1.0 }
			, Shield_Respawn_ResetTimer { false }
			, Shield_SelfHealing_Duration { 0 }
			, Shield_SelfHealing_Amount { }
			, Shield_SelfHealing_Rate { -1 }
			, Shield_SelfHealing_ResetTimer { false }
			, Shield_AttachTypes {}
			, Shield_RemoveTypes {}
			, Shield_ReplaceOnly { false }
			, Shield_ReplaceNonRespawning { false }
			, Shield_InheritStateOnReplace { false }
			, Shield_MinimumReplaceDelay { 0 }
			, Shield_AffectTypes {}

			, Transact { false }
			, Transact_Experience_Value { 1 }
			, Transact_Experience_Source_Flat { 0 }
			, Transact_Experience_Source_Percent { 0.0 }
			, Transact_Experience_Source_Percent_CalcFromTarget { false }
			, Transact_Experience_Target_Flat { 0 }
			, Transact_Experience_Target_Percent { 0.0 }
			, Transact_Experience_Target_Percent_CalcFromSource { false }
			, Transact_SpreadAmongTargets { false }

			, NotHuman_DeathSequence { -1 }
			, AllowDamageOnSelf{ }
			, Debris_Conventional{ false }
			, GattlingStage { 0 }
			, GattlingRateUp { 0 }
			, ReloadAmmo { 0 }

			, MindControl_Threshold { 1.0 }
			, MindControl_Threshold_Inverse { false }
			, MindControl_AlternateDamage {}
			, MindControl_AlternateWarhead {}
			, MindControl_CanKill { false }

			, DetonateOnAllMapObjects { false }
			, DetonateOnAllMapObjects_RequireVerses { false }
			, DetonateOnAllMapObjects_AffectTargets { AffectedTarget::All }
			, DetonateOnAllMapObjects_AffectHouses { AffectedHouse::All }
			, DetonateOnAllMapObjects_AffectTypes {}
			, DetonateOnAllMapObjects_IgnoreTypes {}
			, WasDetonatedOnAllMapObjects { false }

			, NotHuman_DeathAnim { }
			, IsNukeWarhead { false }
			, Remover { false }
			, Remover_Anim { nullptr }
			, ArmorHitAnim { }
			, DebrisAnimTypes {}
			, SquidSplash {}
			, TemporalExpiredAnim { nullptr }
			, TemporalExpiredApplyDamage { false }
			, TemporalDetachDamageFactor { 1.0 }
			, Parasite_DisableRocking {}
			, Parasite_GrappleAnimIndex {}
			, Parasite_ParticleSys {}
			, Parasite_TreatInfantryAsVehicle { false }

			, Flammability {}

			, Launchs {}
			, PermaMC { false }
#ifdef COMPILE_PORTED_DP_FEATURES_
			,DamageTextPerArmor { }

#endif
#ifdef COMPILE_PORTED_DP_FEATURES
			, PaintBallDuration { -1 }
			, PaintBallData { }
#endif
		{ }

	private:
		void DetonateOnOneUnit(HouseClass* pHouse, TechnoClass* pTarget, TechnoClass* pOwner = nullptr , bool BulletFound = false, bool bulletWasIntercepted = false);

		void ApplyRemoveDisguiseToInf(HouseClass* pHouse, TechnoClass* pTarget);
		void ApplyRemoveMindControl(HouseClass* pHouse, TechnoClass* pTarget);
		void ApplyCrit(HouseClass* pHouse, TechnoClass* pTarget, TechnoClass* Owner);
		void ApplyShieldModifiers(TechnoClass* pTarget);

		void ApplyGattlingStage(TechnoClass* pTarget, int Stage);
		void ApplyGattlingRateUp(TechnoClass* pTarget, int RateUp);
		void ApplyReloadAmmo(TechnoClass* pTarget, int ReloadAmount);

		//Otamaa
		void applyTransactMoney(TechnoClass* pOwner, HouseClass* pHouse, BulletClass* pBullet, CoordStruct const& coords);
		void applyStealMoney(TechnoClass* const Owner, TechnoClass* const Target);
		void applyPermaMC(HouseClass* const Owner, AbstractClass* const Target);

		void DetonateOnAllUnits(HouseClass* pHouse, const CoordStruct coords, const float cellSpread, TechnoClass* pOwner);
		void TransactOnOneUnit(TechnoClass* pTarget, TechnoClass* pOwner, int targets);
		void TransactOnAllUnits(std::vector<TechnoClass*>& nVec , HouseClass* pHouse, TechnoClass* pOwner);
		int TransactGetValue(TechnoClass* pTarget, TechnoClass* pOwner, int flat, double percent, bool calcFromTarget);
		std::pair<std::vector<int>, std::vector<int>>  TransactGetSourceAndTarget(TechnoClass* pTarget, TechnoTypeClass* pTargetType, TechnoClass* pOwner, TechnoTypeClass* pOwnerType, int targets);
		int TransactOneValue(TechnoClass* pTechno, TechnoTypeClass* pTechnoType, int transactValue, TransactValueType valueType);

	public:
		void Detonate(TechnoClass* pOwner, HouseClass* pHouse, BulletClass* pBullet, CoordStruct coords);
		bool CanTargetHouse(HouseClass* pHouse, TechnoClass* pTechno);
		void InterceptBullets(TechnoClass* pOwner, WeaponTypeClass* pWeapon, CoordStruct coords);
		bool CanAffectHouse(HouseClass* pOwnerHouse, HouseClass* pTargetHouse);
		bool CanDealDamage(TechnoClass* pTechno, int damageIn, int distanceFromEpicenter, int& DamageResult, bool effectsRequireDamage = false);
		bool CanDealDamage(TechnoClass* pTechno);
		bool EligibleForFullMapDetonation(TechnoClass* pTechno, HouseClass* pOwner);

		virtual ~ExtData() = default;
		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void Initialize() override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

		virtual size_t Size() const { return sizeof(*this); }
	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<WarheadTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override;
	};

	static ExtContainer ExtMap;
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static void DetonateAt(WarheadTypeClass* pThis, ObjectClass* pTarget, TechnoClass* pOwner, int damage);
	static void DetonateAt(WarheadTypeClass* pThis, const CoordStruct& coords, TechnoClass* pOwner, int damage);
};
