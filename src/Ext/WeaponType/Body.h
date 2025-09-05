#pragma once

#include <WeaponTypeClass.h>
#include <DiskLaserClass.h>

#include <Ext/AbstractType/Body.h>

#include <New/Type/RadTypeClass.h>
#include <New/Type/CursorTypeClass.h>

#include <New/PhobosAttachedAffect/PhobosAttachEffectTypeClass.h>
#include <New/PhobosAttachedAffect/AEAttachInfoTypeClass.h>

#include <New/Entity/ElectricBoltClass.h>

#include <Misc/DynamicPatcher/Others/DamageText.h>
#include <Misc/DynamicPatcher/Weapon/AttachFireData.h>


class WeaponTypeExtData final : public AbstractTypeExtData
{
public:
	using base_type = WeaponTypeClass;
	static constexpr unsigned Marker = UuidFirstPart<base_type>::value;

public:

#pragma region ClassMember

	Valueable<double> DiskLaser_Radius;
	Valueable<int> DiskLaser_Circumference;
	Nullable<RadTypeClass*> RadType;
	Valueable<bool> Rad_NoOwner;

	Nullable<int> Strafing_Shots;
	Valueable<bool> Strafing_SimulateBurst;
	Nullable<bool> Strafing;
	Valueable<bool> Strafing_UseAmmoPerShot;
	Nullable<int> Strafing_EndDelay;
	Valueable<bool> Strafing_TargetCell;

	Valueable<AffectedTarget> CanTarget;
	Valueable<AffectedHouse> CanTargetHouses;
	ValueableVector<int> Burst_Delays;
	Valueable<AreaFireTarget> AreaFire_Target;
	Valueable<WeaponTypeClass*> FeedbackWeapon;
	Valueable<bool> Laser_IsSingleColor;
	Nullable<double> Trajectory_Speed;

	Valueable<bool> Abductor;
	Valueable<AnimTypeClass*> Abductor_AnimType;
	Valueable <bool> Abductor_ChangeOwner;
	Valueable<double> Abductor_AbductBelowPercent;
	Valueable<bool> Abductor_Temporal;
	Valueable<int> Abductor_MaxHealth;
	Valueable<bool> Abductor_CheckAbductableWhenTargeting;

	Valueable<bool> Burst_FireWithinSequence;
	Valueable<bool> Burst_NoDelay;
	Nullable<PartialVector2D<int>> ROF_RandomDelay;
	ValueableVector<int> ChargeTurret_Delays;
	Valueable<bool> OmniFire_TurnToTarget;

	Valueable<int>Xhi;
	Valueable<int>Xlo;
	Valueable<int>Yhi;
	Valueable<int>Ylo;
	Valueable<bool> ShakeLocal;

	ValueableVector<AnimTypeClass*> OccupantAnims;
	Valueable<bool> OccupantAnim_UseMultiple;
	Valueable<bool> Range_IgnoreVertical;

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

	Valueable<float> RockerPitch;
	AttachFireData MyAttachFireDatas;

	Valueable<int> Ammo;
	Valueable<bool> IsDetachedRailgun;

	// TS Lasers
	Valueable<bool> Wave_IsHouseColor;
	Valueable<bool> Wave_IsLaser;
	Valueable<bool> Wave_IsBigLaser;
	Nullable<ColorStruct> Wave_Color;
	Nullable<Point3D> Wave_Intent;
	bool Wave_Reverse[5];

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

	Valueable<bool> Ivan_DeathBombOnAllies;
	Valueable<bool> Ivan_DeathBomb;

	//
	Nullable<bool> ApplyDamage; // whether Damage should be applied even if IsSonic=yes or UseFireParticles=yes

	ValueableIdx<CursorTypeClass> Cursor_Attack;
	ValueableIdx<CursorTypeClass> Cursor_AttackOutOfRange;

	//Nullable<BoltData> WeaponBolt_Data;

	Nullable<ColorStruct> Bolt_Colors[3];
	Valueable<bool> Bolt_Disables[3];

	Valueable<int> Bolt_Arcs;

	Valueable<int> Bolt_Duration;

	Nullable<ParticleSystemTypeClass*> Bolt_ParticleSys;
	Nullable<bool> Bolt_FollowFLH;

	Valueable<int> Laser_Thickness;

	ValueableVector<WarheadTypeClass*> ExtraWarheads;
	ValueableVector<int> ExtraWarheads_DamageOverrides;
	ValueableVector<double> ExtraWarheads_DetonationChances;
	ValueableVector<bool> ExtraWarheads_FullDetonation;

	Valueable<double> Burst_Retarget;
	Nullable<bool> KickOutPassenger;

	Nullable<ColorStruct> Beam_Color;
	Valueable<int> Beam_Duration;
	Valueable<double> Beam_Amplitude;
	Valueable<bool> Beam_IsHouseColor;

	Nullable<WarheadTypeClass*> AmbientDamage_Warhead;
	Valueable<bool> AmbientDamage_IgnoreTarget;

	//Nullable<bool> BlockageTargetingBypassDamageOverride;
	Nullable<double> RecoilForce;

	ValueableVector<PhobosAttachEffectTypeClass*> AttachEffect_RequiredTypes;
	ValueableVector<PhobosAttachEffectTypeClass*> AttachEffect_DisallowedTypes;
	std::vector<std::string> AttachEffect_RequiredGroups;
	std::vector<std::string> AttachEffect_DisallowedGroups;
	ValueableVector<int> AttachEffect_RequiredMinCounts;
	ValueableVector<int> AttachEffect_RequiredMaxCounts;
	ValueableVector<int> AttachEffect_DisallowedMinCounts;
	ValueableVector<int> AttachEffect_DisallowedMaxCounts;
	Valueable<bool> AttachEffect_CheckOnFirer;
	Valueable<bool> AttachEffect_IgnoreFromSameSource;

	Valueable<bool> FireOnce_ResetSequence;

	AEAttachInfoTypeClass AttachEffects;
	Valueable<bool> AttachEffect_Enable;
	Valueable<int> NoRepeatFire;

	bool SkipWeaponPicking;

	Valueable<Leptons> KeepRange;
	Valueable<bool> KeepRange_AllowAI;
	Valueable<bool> KeepRange_AllowPlayer;
	Valueable<int> KeepRange_EarlyStopFrame;

	Valueable<bool> VisualScatter;
	Valueable<bool> TurretRecoil_Suppress;

	Valueable<double> CanTarget_MaxHealth;
	Valueable<double> CanTarget_MinHealth;

	Nullable<PartialVector2D<int>> DelayedFire_Duration;
	Valueable<bool> DelayedFire_SkipInTransport;
	Valueable<AnimTypeClass*> DelayedFire_Animation;
	Nullable<AnimTypeClass*> DelayedFire_OpenToppedAnimation;
	Valueable<bool> DelayedFire_AnimIsAttached;
	Valueable<bool> DelayedFire_CenterAnimOnFirer;
	Valueable<bool> DelayedFire_RemoveAnimOnNoDelay;
	Valueable<bool> DelayedFire_PauseFiringSequence;
	Valueable<bool> DelayedFire_OnlyOnInitialBurst;
	Valueable<bool> DelayedFire_InitialBurstSymmetrical;
	Nullable<CoordStruct> DelayedFire_AnimOffset;
	Valueable<bool> DelayedFire_AnimOnTurret;

	Valueable<bool> OnlyAttacker;
#pragma endregion

public:

	WeaponTypeExtData(WeaponTypeClass* pObj) : AbstractTypeExtData(pObj),
		DiskLaser_Radius(38.2),
		DiskLaser_Circumference(240),
		Rad_NoOwner(true),
		Strafing_SimulateBurst(false),
		Strafing_UseAmmoPerShot(false),
		Strafing_TargetCell(),
		CanTarget(AffectedTarget::All),
		CanTargetHouses(AffectedHouse::All),
		AreaFire_Target(AreaFireTarget::Base),
		FeedbackWeapon(nullptr),
		Laser_IsSingleColor(false),
		Abductor(false),
		Abductor_AnimType(nullptr),
		Abductor_ChangeOwner(false),
		Abductor_AbductBelowPercent(100.0),
		Abductor_Temporal(false),
		Abductor_MaxHealth(0),
		Abductor_CheckAbductableWhenTargeting(false),
		Burst_FireWithinSequence(false),
		Burst_NoDelay(false),
		OmniFire_TurnToTarget(false),
		Xhi(0),
		Xlo(0),
		Yhi(0),
		Ylo(0),
		ShakeLocal(false),
		OccupantAnim_UseMultiple(false),
		Range_IgnoreVertical(false),
		ProjectileRange(Leptons(100000)),
		Feedback_Anim(nullptr),
		Feedback_Anim_Offset({ 0, 0, 0 }),
		Feedback_Anim_UseFLH(true),
		DestroyTechnoAfterFiring(false),
		RemoveTechnoAfterFiring(false),
		OpentoppedAnim(nullptr),
		Targeting_Health_Percent_Below(true),
		RockerPitch(0.0f),
		Ammo(1),
		IsDetachedRailgun(false),
		Wave_IsHouseColor(false),
		Wave_IsLaser(false),
		Wave_IsBigLaser(false),
		Ivan_KillsBridges(true),
		Ivan_Detachable(true),
		Ivan_DetonateOnSell(false),
		Ivan_DeathBombOnAllies(false),
		Ivan_DeathBomb(false),
		Cursor_Attack((int)MouseCursorType::Attack),
		Cursor_AttackOutOfRange((int)MouseCursorType::AttackOutOfRange),
		Bolt_Arcs(8),
		Bolt_Duration(17),
		Laser_Thickness(-1),
		Burst_Retarget(0.0),
		Beam_Duration(15),
		Beam_Amplitude(40.0),
		Beam_IsHouseColor(false),
		AmbientDamage_IgnoreTarget(false),
		AttachEffect_CheckOnFirer(false),
		AttachEffect_IgnoreFromSameSource(false),
		FireOnce_ResetSequence(true),
		AttachEffect_Enable(false),
		NoRepeatFire(0),
		SkipWeaponPicking(true),
		KeepRange_AllowAI(false),
		KeepRange_AllowPlayer(false),
		KeepRange_EarlyStopFrame(0),
		VisualScatter(false),
		TurretRecoil_Suppress(false),
		CanTarget_MaxHealth(1.0),
		CanTarget_MinHealth(0.0),
		DelayedFire_SkipInTransport(false),
		DelayedFire_Animation(nullptr),
		DelayedFire_AnimIsAttached(true),
		DelayedFire_CenterAnimOnFirer(false),
		DelayedFire_RemoveAnimOnNoDelay(false),
		DelayedFire_PauseFiringSequence(false),
		DelayedFire_OnlyOnInitialBurst(false),
		DelayedFire_InitialBurstSymmetrical(false),
		DelayedFire_AnimOnTurret(true),
		OnlyAttacker(false)
	{
		// Initialize Wave_Reverse array
		for (int i = 0; i < 5; ++i)
		{
			Wave_Reverse[i] = false;
		}

		// Initialize Bolt_Disables array
		for (int i = 0; i < 3; ++i)
		{
			Bolt_Disables[i] = false;
		}

		this->Initialize();
	}

	void Initialize();

	WeaponTypeExtData(WeaponTypeClass* pObj, noinit_t nn) : AbstractTypeExtData(pObj, nn) { }

	virtual ~WeaponTypeExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override { }

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->AbstractTypeExtData::LoadFromStream(Stm);
		this->Serialize(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		const_cast<WeaponTypeExtData*>(this)->AbstractTypeExtData::SaveToStream(Stm);
		const_cast<WeaponTypeExtData*>(this)->Serialize(Stm);
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const
	{
		this->AbstractTypeExtData::CalculateCRC(crc);
	}

	virtual WeaponTypeClass* This() const override { return reinterpret_cast<WeaponTypeClass*>(this->AbstractTypeExtData::This()); }
	virtual const WeaponTypeClass* This_Const() const override { return reinterpret_cast<const WeaponTypeClass*>(this->AbstractTypeExtData::This_Const()); }

	virtual bool LoadFromINI(CCINIClass* pINI, bool parseFailAddr);
	virtual bool WriteToINI(CCINIClass* pINI) const { return true; }

public:

	bool OPTIONALINLINE IsWave() const
	{
		auto const pThis = this->This();
		return this->Wave_IsLaser || this->Wave_IsBigLaser || pThis->IsSonic || pThis->IsMagBeam;
	}

	int OPTIONALINLINE GetProjectileRange() const
	{
		return this->ProjectileRange.Get();
	}

	ColorStruct GetBeamColor() const;
	bool HasRequiredAttachedEffects(TechnoClass* pTarget, TechnoClass* pFirer);
	bool IsHealthInThreshold(ObjectClass* pTarget) const;

private:
	template <typename T>
	void Serialize(T& Stm);

public:
	static int nOldCircumference;

	static int GetBurstDelay(WeaponTypeClass* pThis, int burstIndex);
	static void DetonateAt(WeaponTypeClass* pThis, AbstractClass* pTarget, TechnoClass* pOwner, bool AddDamage, HouseClass* HouseInveoker);
	static void DetonateAt(WeaponTypeClass* pThis, AbstractClass* pTarget, TechnoClass* pOwner, int damage, bool AddDamage, HouseClass* HouseInveoker);
	static void DetonateAt(WeaponTypeClass* pThis, const CoordStruct& coords, TechnoClass* pOwner, bool AddDamage, HouseClass* HouseInveoker);
	static void DetonateAt(WeaponTypeClass* pThis, const CoordStruct& coords, TechnoClass* pOwner, int damage, bool AddDamage, HouseClass* HouseInveoker);
	static void DetonateAt(WeaponTypeClass* pThis, const CoordStruct& coords, AbstractClass* pTarget, TechnoClass* pOwner, int damage, bool AddDamage, HouseClass* HouseInveoker);

	static void FireRadBeam(TechnoClass* pFirer, WeaponTypeClass* pWeapon, CoordStruct& source, CoordStruct& target);
	static void FireEbolt(TechnoClass* pFirer, WeaponTypeClass* pWeapon, CoordStruct& source, CoordStruct& target, int idx);

	//return lepton
	static int GetRangeWithModifiers(WeaponTypeClass* pThis, TechnoClass* pFirer, std::optional<int> fallback = std::nullopt);
	static int GetTechnoKeepRange(WeaponTypeClass* pThis, TechnoClass* pFirer, bool isMinimum);
};

class WeaponTypeExtContainer final :public Container<WeaponTypeExtData>
{
public:
	static WeaponTypeExtContainer Instance;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
	static void Clear();
};

class BulletTypeExtData;
class WarheadTypeExtData;
class FakeBulletTypeClass;
class FakeWarheadTypeClass;
class NOVTABLE FakeWeaponTypeClass : public WeaponTypeClass
{
public:

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, BOOL clearDirty);

	bool _ReadFromINI(CCINIClass* pINI);

	FORCEDINLINE WeaponTypeExtData* _GetExtData() {
		return *reinterpret_cast<WeaponTypeExtData**>(((DWORD)this) + AbstractExtOffset);
	}

	FORCEDINLINE BulletTypeExtData* _GetBulletTypeExtData() {
		return *reinterpret_cast<BulletTypeExtData**>(((DWORD)this->Projectile) + AbstractExtOffset);
	}

	FORCEDINLINE FakeBulletTypeClass* _GetBulletType() {
		return (FakeBulletTypeClass*)this->Projectile;
	}

	FORCEDINLINE FakeWarheadTypeClass* _GetWarheadType() {
		return (FakeWarheadTypeClass*)this->Warhead;
	}

	FORCEDINLINE WarheadTypeExtData* _GetWarheadTypeExtData() {
		return *reinterpret_cast<WarheadTypeExtData**>(((DWORD)this->Warhead) + AbstractExtOffset);
	}

};
static_assert(sizeof(FakeWeaponTypeClass) == sizeof(WeaponTypeClass), "Invalid Size !");