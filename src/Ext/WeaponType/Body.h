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

#include <Misc/DynamicPatcher/Others/DamageText.h>
#include <Misc/DynamicPatcher/Weapon/AttachFireData.h>


class WeaponTypeExtData final
{
public:
	static constexpr size_t Canary = 0x23222222;
	using base_type = WeaponTypeClass;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
public:
	Valueable<double> DiskLaser_Radius { 38.2 };
	Valueable<int> DiskLaser_Circumference { 240 };
	Nullable<RadTypeClass*> RadType {};
	Valueable<bool> Rad_NoOwner { true };

	Nullable<int> Strafing_Shots { };
	Valueable<bool> Strafing_SimulateBurst { false };
	Valueable<AffectedTarget> CanTarget { AffectedTarget::All };
	Valueable<AffectedHouse> CanTargetHouses { AffectedHouse::All };
	ValueableVector<int> Burst_Delays {};
	Valueable<AreaFireTarget> AreaFire_Target { AreaFireTarget::Base };
	Nullable<WeaponTypeClass*> FeedbackWeapon {};
	Valueable<bool> Laser_IsSingleColor { false };
	Valueable<double> Trajectory_Speed { 100.0 };

	Valueable<bool> Abductor { false };
	Valueable<AnimTypeClass*> Abductor_AnimType { nullptr };
	Valueable <bool> Abductor_ChangeOwner { false };
	Valueable<double> Abductor_AbductBelowPercent { 100 };
	Valueable<bool> Abductor_Temporal { false };
	Valueable<int> Abductor_MaxHealth { 0 };
	Valueable<bool> Abductor_CheckAbductableWhenTargeting { false };

	Nullable<AnimTypeClass*>DelayedFire_Anim {};
	Valueable<int> DelayedFire_Anim_LoopCount { 1 };
	Valueable<bool> DelayedFire_Anim_UseFLH { true };
	Valueable<int> DelayedFire_DurationTimer { 0 };
	Valueable<bool> Burst_FireWithinSequence { false };
	Nullable<PartialVector2D<int>> ROF_RandomDelay {};
	Valueable<bool> OmniFire_TurnToTarget { false };

	Valueable<int>Xhi { 0 };
	Valueable<int>Xlo { 0 };
	Valueable<int>Yhi { 0 };
	Valueable<int>Ylo { 0 };
	Valueable<bool> ShakeLocal { false };

	ValueableVector<AnimTypeClass*> OccupantAnims {};
	Valueable<bool> OccupantAnim_UseMultiple { false };
	Valueable<bool> Range_IgnoreVertical { false };

	// brought back from TS
	Valueable<Leptons> ProjectileRange { Leptons(100000) };
	Nullable<bool> Decloak_InstantFire {};
	Valueable<AnimTypeClass*> Feedback_Anim { nullptr };
	Valueable<CoordStruct> Feedback_Anim_Offset { { 0, 0, 0 } };
	Valueable<bool> Feedback_Anim_UseFLH { true };
	Valueable<bool> DestroyTechnoAfterFiring { false };
	Valueable<bool> RemoveTechnoAfterFiring { false };
	Valueable<AnimTypeClass*> OpentoppedAnim { nullptr };
	Nullable<Point2D> DiskLaser_FiringOffset {};

	Nullable<double> Targeting_Health_Percent {};
	Valueable<bool> Targeting_Health_Percent_Below { true };

	Valueable<float> RockerPitch { 0.0f };
	AttachFireData MyAttachFireDatas {};

	Valueable<int> Ammo { 1 };
	Valueable<bool> IsDetachedRailgun { false };

	// TS Lasers
	Valueable<bool> Wave_IsHouseColor { false };
	Valueable<bool> Wave_IsLaser { false };
	Valueable<bool> Wave_IsBigLaser { false };
	Nullable<ColorStruct> Wave_Color {};
	Nullable<Point3D> Wave_Intent {};
	bool Wave_Reverse[5] { false };

	// custom Ivan Bombs
	Valueable<bool> Ivan_KillsBridges { true };
	Valueable<bool> Ivan_Detachable { true };
	Nullable<int> Ivan_Damage {};
	Nullable<int> Ivan_Delay {};
	NullableIdx<VocClass> Ivan_TickingSound {};
	NullableIdx<VocClass> Ivan_AttachSound {};
	Nullable<WarheadTypeClass*> Ivan_WH {};
	Nullable<SHPStruct*> Ivan_Image {};
	Nullable<int> Ivan_FlickerRate {};
	Nullable<bool> Ivan_CanDetonateTimeBomb {};
	Nullable<bool> Ivan_CanDetonateDeathBomb {};
	Valueable<bool> Ivan_DetonateOnSell { false };

	Valueable<bool> Ivan_DeathBombOnAllies { false };
	Valueable<bool> Ivan_DeathBomb { false };

	//
	Nullable<bool> ApplyDamage {}; // whether Damage should be applied even if IsSonic=yes or UseFireParticles=yes

	ValueableIdx<CursorTypeClass> Cursor_Attack { (int)MouseCursorType::Attack };
	ValueableIdx<CursorTypeClass> Cursor_AttackOutOfRange { (int)MouseCursorType::AttackOutOfRange };

	Nullable<BoltData> WeaponBolt_Data {};

	Nullable<ColorStruct> Bolt_Color1 {};
	Nullable<ColorStruct> Bolt_Color2 {};
	Nullable<ColorStruct> Bolt_Color3 {};

	Valueable<bool> Bolt_Disable1 { false };
	Valueable<bool> Bolt_Disable2 { false };
	Valueable<bool> Bolt_Disable3 { false };
	Valueable<int> Bolt_Arcs { 8 };

	Nullable<ParticleSystemTypeClass*> Bolt_ParticleSys {};
	Valueable<int> Laser_Thickness { -1 };

	ValueableVector<WarheadTypeClass*> ExtraWarheads {};
	ValueableVector<int> ExtraWarheads_DamageOverrides {};

	Valueable<double> Burst_Retarget { 0.0 };
	Nullable<bool> KickOutPassenger {};

	Nullable<ColorStruct> Beam_Color {};
	Valueable<int> Beam_Duration { 15 };
	Valueable<double> Beam_Amplitude { 40.0 };
	Valueable<bool> Beam_IsHouseColor { false };

	Valueable<bool> Bolt_ParticleSys_Enabled { true };
	Nullable<WarheadTypeClass*> AmbientDamage_Warhead {};
	Valueable<bool> AmbientDamage_IgnoreTarget { false };

	WeaponTypeExtData() noexcept = default;
	~WeaponTypeExtData() noexcept = default;

	void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);
	void Initialize();
	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	bool inline IsWave() const
	{
		auto const pThis = this->AttachedToObject;
		return this->Wave_IsLaser || this->Wave_IsBigLaser || pThis->IsSonic || pThis->IsMagBeam;
	}

	int inline GetProjectileRange() const
	{
		return this->ProjectileRange.Get();
	}

	ColorStruct GetBeamColor() const;

private:
	template <typename T>
	void Serialize(T& Stm);

public:

	static int nOldCircumference;
	static PhobosMap<EBolt*, const WeaponTypeExtData*> boltWeaponTypeExt;

	static int GetBurstDelay(WeaponTypeClass* pThis, int burstIndex);
	static void DetonateAt(WeaponTypeClass* pThis, AbstractClass* pTarget, TechnoClass* pOwner, bool AddDamage, HouseClass* HouseInveoker);
	static void DetonateAt(WeaponTypeClass* pThis, AbstractClass* pTarget, TechnoClass* pOwner, int damage, bool AddDamage, HouseClass* HouseInveoker);
	static void DetonateAt(WeaponTypeClass* pThis, const CoordStruct& coords, TechnoClass* pOwner, bool AddDamage, HouseClass* HouseInveoker);
	static void DetonateAt(WeaponTypeClass* pThis, const CoordStruct& coords, TechnoClass* pOwner, int damage, bool AddDamage, HouseClass* HouseInveoker);
	static void DetonateAt(WeaponTypeClass* pThis, const CoordStruct& coords, AbstractClass* pTarget, TechnoClass* pOwner, int damage, bool AddDamage, HouseClass* HouseInveoker);

	static EBolt* CreateBolt(WeaponTypeClass* pWeapon);
	static EBolt* CreateBolt(WeaponTypeExtData* pWeapon = nullptr);

	static void FireRadBeam(TechnoClass* pFirer, WeaponTypeClass* pWeapon, CoordStruct& source, CoordStruct& target);
	static void FireEbolt(TechnoClass* pFirer, WeaponTypeClass* pWeapon, CoordStruct& source, CoordStruct& target, int idx);
};

class WeaponTypeExtContainer final :public Container<WeaponTypeExtData>
{
public:
	static WeaponTypeExtContainer Instance;

	CONSTEXPR_NOCOPY_CLASSB(WeaponTypeExtContainer, WeaponTypeExtData, "WeaponTypeClass");

public:

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
	static void Clear();
};