#pragma once
#include <BulletClass.h>
#include <WeaponTypeClass.h>

#include <Helpers/Macro.h>
//#include <Utilities/Container.h>
#include <Ext/Abstract/Body.h>
#include <Utilities/TemplateDef.h>

#include <New/Type/RadTypeClass.h>
#include <New/Type/CursorTypeClass.h>

#include <New/Entity/ElectricBoltClass.h>

#ifdef COMPILE_PORTED_DP_FEATURES
#include <Misc/DynamicPatcher/Others/DamageText.h>
#include <Misc/DynamicPatcher/Weapon/AttachFireData.h>
#endif

class WeaponTypeExt
{
public:
	static int nOldCircumference;

	class ExtData final : public Extension<WeaponTypeClass>
	{
	public:
		static constexpr size_t Canary = 0x22222222;
		using base_type = WeaponTypeClass;

	public:

		Valueable<double> DiskLaser_Radius;
		Valueable<int> DiskLaser_Circumference;
		Nullable<RadTypeClass*> RadType;
		Valueable<bool> Rad_NoOwner;

		Valueable<int> Strafing_Shots;
		Valueable<bool> Strafing_SimulateBurst;
		Valueable<AffectedTarget> CanTarget;
		Valueable<AffectedHouse> CanTargetHouses;
		ValueableVector<int> Burst_Delays;
		Valueable<AreaFireTarget> AreaFire_Target;
		Nullable<WeaponTypeClass*> FeedbackWeapon;
		Valueable<bool> Laser_IsSingleColor;
		Valueable<double> Trajectory_Speed;

		Valueable<bool> Abductor;
		Valueable<AnimTypeClass*> Abductor_AnimType;
		Valueable <bool> Abductor_ChangeOwner;
		Valueable<double> Abductor_AbductBelowPercent;
		Valueable<bool> Abductor_Temporal;
		Valueable<int> Abductor_MaxHealth;

		Nullable<AnimTypeClass*>DelayedFire_Anim;
		Valueable<int> DelayedFire_Anim_LoopCount;
		Valueable<bool> DelayedFire_Anim_UseFLH;
		Valueable<int> DelayedFire_DurationTimer;
		Valueable<bool> Burst_FireWithinSequence;
		Nullable<PartialVector2D<int>> ROF_RandomDelay;
		Valueable<bool> OmniFire_TurnToTarget;

#pragma region Otamaa
		Valueable<int>Xhi;
		Valueable<int>Xlo;
		Valueable<int>Yhi;
		Valueable<int>Ylo;
		Valueable<bool> ShakeLocal;

		ValueableVector<AnimTypeClass*> OccupantAnims;
		Valueable<bool> OccupantAnim_UseMultiple;
		Valueable<bool>Range_IgnoreVertical;
		// brought back from TS
		Valueable<Leptons> ProjectileRange;
		Nullable<bool> Decloak_InstantFire;
		Valueable<AnimTypeClass*> Feedback_Anim;
		Valueable<CoordStruct> Feedback_Anim_Offset;
		Valueable<bool> Feedback_Anim_UseFLH;
		Valueable<bool> DestroyTechnoAfterFiring;
		Valueable<bool> RemoveTechnoAfterFiring;
		Valueable<AnimTypeClass*> OpentoppedAnim;
		Nullable<Point2D> DiskLaser_FiringOffset;

		Nullable<double> Targeting_Health_Percent;
		Valueable<bool> Targeting_Health_Percent_Below;

		#ifdef  COMPILE_PORTED_DP_FEATURES
		Valueable<float> RockerPitch;
		AttachFireData MyAttachFireDatas;
		#endif

#pragma endregion

		Valueable<int> Ammo;
		Valueable<bool> IsDetachedRailgun;
		
		// TS Lasers
		Valueable<bool> Wave_IsHouseColor;
		Valueable<bool> Wave_IsLaser;
		Valueable<bool> Wave_IsBigLaser;
		Nullable<ColorStruct> Wave_Color;
		Nullable<Point3D> Wave_Intent;
		bool   Wave_Reverse[5];

		// custom Ivan Bombs
		Valueable<bool> Ivan_KillsBridges;
		Valueable<bool> Ivan_Detachable;
		Nullable<int> Ivan_Damage;
		Nullable<int> Ivan_Delay;
		NullableIdx<VocClass> Ivan_TickingSound;
		NullableIdx<VocClass> Ivan_AttachSound;
		Nullable<WarheadTypeClass*> Ivan_WH;
		Nullable<SHPStruct*> Ivan_Image;
		Nullable<int> Ivan_FlickerRate;
		Nullable<bool> Ivan_CanDetonateTimeBomb;
		Nullable<bool> Ivan_CanDetonateDeathBomb;
		Valueable<bool> Ivan_DetonateOnSell;
		//
		Nullable<bool> ApplyDamage; // whether Damage should be applied even if IsSonic=yes or UseFireParticles=yes

		ValueableIdx<CursorTypeClass> Cursor_Attack;
		ValueableIdx<CursorTypeClass> Cursor_AttackOutOfRange;

		Nullable<BoltData> WeaponBolt_Data;

		Nullable<ColorStruct> Bolt_Color1;
		Nullable<ColorStruct> Bolt_Color2;
		Nullable<ColorStruct> Bolt_Color3;

		Valueable<bool> Bolt_Disable1;
		Valueable<bool> Bolt_Disable2;
		Valueable<bool> Bolt_Disable3;

		Nullable<ParticleSystemTypeClass*> Bolt_ParticleSys;

		ExtData(WeaponTypeClass* OwnerObject) : Extension<WeaponTypeClass>(OwnerObject)
			, DiskLaser_Radius { 38.2 }
			, DiskLaser_Circumference { 240 }
			, RadType {}
			, Rad_NoOwner { true }
			, Strafing_Shots { 5 }
			, Strafing_SimulateBurst { false }
			, CanTarget { AffectedTarget::All }
			, CanTargetHouses { AffectedHouse::All }
			, Burst_Delays {}
			, AreaFire_Target { AreaFireTarget::Base }
			, FeedbackWeapon {}
			, Laser_IsSingleColor { false }
			, Trajectory_Speed { 100.0 }
			, Abductor { false }
			, Abductor_AnimType { nullptr }
			, Abductor_ChangeOwner { false }
			, Abductor_AbductBelowPercent { 100 }
			, Abductor_Temporal { false }
			, Abductor_MaxHealth { 0 }

			, DelayedFire_Anim { }
			, DelayedFire_Anim_LoopCount { 1 }
			, DelayedFire_Anim_UseFLH { true }
			, DelayedFire_DurationTimer { 0 }
			, Burst_FireWithinSequence { false }
			, ROF_RandomDelay {}
			, OmniFire_TurnToTarget { false }
			, Xhi { 0 }
			, Xlo { 0 }
			, Yhi { 0 }
			, Ylo { 0 }
			, ShakeLocal { false }
			, OccupantAnims {}
			, OccupantAnim_UseMultiple { false }
			, Range_IgnoreVertical { false }
			, ProjectileRange { Leptons(100000) }
			, Decloak_InstantFire { }
			, Feedback_Anim { nullptr }
			, Feedback_Anim_Offset{{ 0,0,0 } }
			, Feedback_Anim_UseFLH { true }
			, DestroyTechnoAfterFiring { false }
			, RemoveTechnoAfterFiring { false }
			, OpentoppedAnim { nullptr }
			, DiskLaser_FiringOffset { }
			, Targeting_Health_Percent { }
			, Targeting_Health_Percent_Below { true }
			 #ifdef COMPILE_PORTED_DP_FEATURES
			, RockerPitch { 0.0f }
			, MyAttachFireDatas { }
			 #endif
			, Ammo { 1 }
			, IsDetachedRailgun { false }

			, Wave_IsHouseColor { false }
			, Wave_IsLaser { false }
			, Wave_IsBigLaser { false }
			, Wave_Color {  }
			, Wave_Intent { }
			, Wave_Reverse { false }

			, Ivan_KillsBridges { true }
			, Ivan_Detachable { true }
			, Ivan_Damage { }
			, Ivan_Delay { }
			, Ivan_TickingSound { }
			, Ivan_AttachSound { }
			, Ivan_WH { }
			, Ivan_Image { }
			, Ivan_FlickerRate { }
			, Ivan_CanDetonateTimeBomb { }
			, Ivan_CanDetonateDeathBomb { }
			, Ivan_DetonateOnSell { false }
			, ApplyDamage { }
			, Cursor_Attack { (int)MouseCursorType::Attack }
			, Cursor_AttackOutOfRange { (int)MouseCursorType::AttackOutOfRange }
			, WeaponBolt_Data {}
			, Bolt_Color1 { }
			, Bolt_Color2 { }
			, Bolt_Color3 { }
			, Bolt_Disable1 { false }
			, Bolt_Disable2 { false }
			, Bolt_Disable3 { false }
			, Bolt_ParticleSys { }
		{ }

		virtual ~ExtData() override  = default;

		void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);
		void Initialize();
		void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
		void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

		bool IsWave() const {
			auto const pThis = this->OwnerObject();
			return this->Wave_IsLaser || this->Wave_IsBigLaser || pThis->IsSonic || pThis->IsMagBeam;
		}

		int GetProjectileRange() const {
			return this->ProjectileRange.Get();
		}

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	static std::unordered_map<EBolt*, const WeaponTypeExt::ExtData*> boltWeaponTypeExt;

	class ExtContainer final :public Container<WeaponTypeExt::ExtData>
	{
	public:
		ExtContainer();
		~ExtContainer();

		static bool LoadGlobals(PhobosStreamReader& Stm)
		{
			return Stm
				.Process(nOldCircumference)
				.Success();
		}

		static bool SaveGlobals(PhobosStreamWriter& Stm)
		{
			return Stm
				.Process(nOldCircumference)
				.Success();
		}

		static void Clear()
		{
			boltWeaponTypeExt.clear();
		}
	};

	static ExtContainer ExtMap;

	static int GetBurstDelay(WeaponTypeClass* pThis , int burstIndex);
	static void DetonateAt(WeaponTypeClass* pThis, AbstractClass* pTarget, TechnoClass* pOwner, bool AddDamage);
	static void DetonateAt(WeaponTypeClass* pThis, AbstractClass* pTarget, TechnoClass* pOwner, int damage, bool AddDamage);
	static void DetonateAt(WeaponTypeClass* pThis, const CoordStruct& coords, TechnoClass* pOwner, bool AddDamage);
	static void DetonateAt(WeaponTypeClass* pThis, const CoordStruct& coords, TechnoClass* pOwner, int damage, bool AddDamage);
	static void DetonateAt(WeaponTypeClass* pThis, const CoordStruct& coords, AbstractClass* pTarget, TechnoClass* pOwner, int damage , bool AddDamage);
};
