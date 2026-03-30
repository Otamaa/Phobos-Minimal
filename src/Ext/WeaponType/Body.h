#pragma once

#include <WeaponTypeClass.h>
#include <DiskLaserClass.h>

#include <Ext/AbstractType/Body.h>

#include <New/Type/RadTypeClass.h>
#include <New/Type/CursorTypeClass.h>

#include <New/PhobosAttachedAffect/PhobosAttachEffectTypeClass.h>
#include <New/PhobosAttachedAffect/AEAttachInfoTypeClass.h>

#include <New/Entity/ElectricBoltClass.h>


class WeaponTypeExtData final : public AbstractTypeExtData
{
public:
	using base_type = WeaponTypeClass;
	static COMPILETIMEEVAL const char* ClassName = "WeaponTypeExtData";
	static COMPILETIMEEVAL const char* BaseClassName = "WeaponTypeClass";
	
	

public:

#pragma region ClassMember
	Valueable<double> DiskLaser_Radius { 38.2 };
	Valueable<int> DiskLaser_Circumference { 240 };
	Nullable<RadTypeClass*> RadType {};
	Valueable<bool> Rad_NoOwner { true };

	Nullable<int> Strafing_Shots {};
	Valueable<bool> Strafing_SimulateBurst { false };
	Nullable<bool> Strafing {};
	Valueable<bool> Strafing_UseAmmoPerShot { false };
	Nullable<int> Strafing_EndDelay {};
	Valueable<bool> Strafing_TargetCell { false };

	Valueable<AffectedTarget> CanTarget { AffectedTarget::All };
	Valueable<AffectedHouse> CanTargetHouses { AffectedHouse::All };
	ValueableVector<int> Burst_Delays {};
	Valueable<AreaFireTarget> AreaFire_Target { AreaFireTarget::Base };
	Valueable<WeaponTypeClass*> FeedbackWeapon { nullptr };
	Valueable<bool> Laser_IsSingleColor { false };
	Nullable<double> Trajectory_Speed {};

	Valueable<bool> Abductor { false };
	Valueable<AnimTypeClass*> Abductor_AnimType { nullptr };
	Valueable<bool> Abductor_ChangeOwner { false };
	Valueable<double> Abductor_AbductBelowPercent { 100.0 };
	Valueable<bool> Abductor_Temporal { false };
	Valueable<int> Abductor_MaxHealth { 0 };
	Valueable<bool> Abductor_CheckAbductableWhenTargeting { false };

	Valueable<bool> Burst_FireWithinSequence { false };
	Valueable<bool> Burst_NoDelay { false };
	Nullable<PartialVector2D<int>> ROF_RandomDelay {};
	ValueableVector<int> ChargeTurret_Delays {};
	Valueable<bool> OmniFire_TurnToTarget { false };

	Valueable<int> Xhi { 0 };
	Valueable<int> Xlo { 0 };
	Valueable<int> Yhi { 0 };
	Valueable<int> Ylo { 0 };
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
	Valueable<int> Ammo { 1 };
	Valueable<bool> IsDetachedRailgun { false };

	// TS Lasers
	Valueable<bool> Wave_IsHouseColor { false };
	Valueable<bool> Wave_IsLaser { false };
	Valueable<bool> Wave_IsBigLaser { false };
	Nullable<ColorStruct> Wave_Color {};
	Nullable<Point3D> Wave_Intent {};
	std::array<bool, 5> Wave_Reverse {};

	// custom Ivan Bombs
	Valueable<bool> Ivan_KillsBridges { true };
	Valueable<bool> Ivan_Detachable { true };
	Nullable<int> Ivan_Damage {};
	Nullable<int> Ivan_Delay {};
	NullableIdx<VocClass> Ivan_TickingSound {};
	NullableIdx<VocClass> Ivan_AttachSound {};
	Nullable<WarheadTypeClass*> Ivan_WH {};
	Nullable<bool> Ivan_AttachToCenter {};
	Nullable<SHPStruct*> Ivan_Image {};
	Nullable<int> Ivan_FlickerRate {};
	Nullable<bool> Ivan_CanDetonateTimeBomb {};
	Nullable<bool> Ivan_CanDetonateDeathBomb {};
	Valueable<bool> Ivan_DetonateOnSell { false };

	Valueable<bool> Ivan_DeathBombOnAllies { false };
	Valueable<bool> Ivan_DeathBomb { false };

	//
	Nullable<bool> ApplyDamage {};

	ValueableIdx<CursorTypeClass> Cursor_Attack { (int)MouseCursorType::Attack };
	ValueableIdx<CursorTypeClass> Cursor_AttackOutOfRange { (int)MouseCursorType::AttackOutOfRange };

	//Nullable<BoltData> WeaponBolt_Data;

	std::array<Nullable<ColorStruct>, 3> Bolt_Colors {};
	std::array<Valueable<bool>, 3> Bolt_Disables {};

	Valueable<int> Bolt_Arcs { 8 };

	Valueable<int> Bolt_Duration { 17 };

	Nullable<ParticleSystemTypeClass*> Bolt_ParticleSys {};
	Nullable<bool> Bolt_FollowFLH {};

	Valueable<int> Laser_Thickness { -1 };

	ValueableVector<WarheadTypeClass*> ExtraWarheads {};
	ValueableVector<int> ExtraWarheads_DamageOverrides {};
	ValueableVector<double> ExtraWarheads_DetonationChances {};
	ValueableVector<bool> ExtraWarheads_FullDetonation {};

	Valueable<double> Burst_Retarget { 0.0 };
	Nullable<bool> KickOutPassenger {};

	Nullable<ColorStruct> Beam_Color {};
	Valueable<int> Beam_Duration { 15 };
	Valueable<double> Beam_Amplitude { 40.0 };
	Valueable<bool> Beam_IsHouseColor { false };

	Nullable<WarheadTypeClass*> AmbientDamage_Warhead {};
	Valueable<bool> AmbientDamage_IgnoreTarget { false };

	//Nullable<bool> BlockageTargetingBypassDamageOverride;
	Nullable<double> RecoilForce {};

	ValueableVector<PhobosAttachEffectTypeClass*> AttachEffect_RequiredTypes {};
	ValueableVector<PhobosAttachEffectTypeClass*> AttachEffect_DisallowedTypes {};
	std::vector<std::string> AttachEffect_RequiredGroups {};
	std::vector<std::string> AttachEffect_DisallowedGroups {};
	ValueableVector<int> AttachEffect_RequiredMinCounts {};
	ValueableVector<int> AttachEffect_RequiredMaxCounts {};
	ValueableVector<int> AttachEffect_DisallowedMinCounts {};
	ValueableVector<int> AttachEffect_DisallowedMaxCounts {};
	Valueable<bool> AttachEffect_CheckOnFirer { false };
	Valueable<bool> AttachEffect_IgnoreFromSameSource { false };

	Valueable<bool> FireOnce_ResetSequence { true };

	AEAttachInfoTypeClass AttachEffects {};
	Valueable<bool> AttachEffect_Enable { false };
	Valueable<int> NoRepeatFire { 0 };

	bool SkipWeaponPicking { true };

	Valueable<Leptons> KeepRange {};
	Valueable<bool> KeepRange_AllowAI { false };
	Valueable<bool> KeepRange_AllowPlayer { false };
	Valueable<int> KeepRange_EarlyStopFrame { 0 };

	Valueable<bool> VisualScatter { false };
	Nullable<Leptons> VisualScatter_Min {};
	Nullable<Leptons> VisualScatter_Max {};

	Valueable<bool> TurretRecoil_Suppress { false };

	Valueable<double> CanTarget_MaxHealth { 1.0 };
	Valueable<double> CanTarget_MinHealth { 0.0 };
	Valueable<AffectedVeterancy> CanTargetVeterancy { AffectedVeterancy::All };

	Nullable<PartialVector2D<int>> DelayedFire_Duration {};
	Valueable<bool> DelayedFire_SkipInTransport { false };
	Valueable<AnimTypeClass*> DelayedFire_Animation { nullptr };
	Nullable<AnimTypeClass*> DelayedFire_OpenToppedAnimation {};
	Valueable<bool> DelayedFire_AnimIsAttached { true };
	Valueable<bool> DelayedFire_CenterAnimOnFirer { false };
	Valueable<bool> DelayedFire_RemoveAnimOnNoDelay { false };
	Valueable<bool> DelayedFire_PauseFiringSequence { false };
	Valueable<bool> DelayedFire_OnlyOnInitialBurst { false };
	Valueable<bool> DelayedFire_InitialBurstSymmetrical { false };
	Nullable<CoordStruct> DelayedFire_AnimOffset {};
	Valueable<bool> DelayedFire_AnimOnTurret { true };

	Valueable<bool> OnlyAttacker { false };

	Nullable<Leptons> ChasingExtraRange {};
	Nullable<Leptons> PrefiringExtraRange {};
	Nullable<bool> PrefiringExtraRange_IncludeBurst {};
	Nullable<Leptons> ExtraRange_FirerMoving {};
	Nullable<bool> AttackFriendlies {};
	Nullable<bool> AttackCursorOnFriendlies {};
	Nullable<bool> AttackNoThreatBuildings {};
	Nullable<bool> Anim_Update {};
	Nullable<bool> CanTarget_IronCurtained {};
	Nullable<bool> AutoTarget_IronCurtained {};

	Nullable<bool> CylinderRangefinding {};
#pragma endregion

public:

	WeaponTypeExtData(WeaponTypeClass* pObj) : AbstractTypeExtData(pObj)		
	{
		this->AbsType = WeaponTypeClass::AbsID;
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

	WeaponTypeClass* This() const { return reinterpret_cast<WeaponTypeClass*>(this->AttachedToObject); }
	const WeaponTypeClass* This_Const() const { return reinterpret_cast<const WeaponTypeClass*>(this->AttachedToObject); }

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
	static void DetonateAt1(WeaponTypeClass* pThis, AbstractClass* pTarget, TechnoClass* pOwner, bool AddDamage, HouseClass* HouseInveoker);
	static void DetonateAt2(WeaponTypeClass* pThis, AbstractClass* pTarget, TechnoClass* pOwner, int damage, bool AddDamage, HouseClass* HouseInveoker);
	static void DetonateAt3(WeaponTypeClass* pThis, const CoordStruct& coords, TechnoClass* pOwner, bool AddDamage, HouseClass* HouseInveoker);
	static void DetonateAt4(WeaponTypeClass* pThis, const CoordStruct& coords, TechnoClass* pOwner, int damage, bool AddDamage, HouseClass* HouseInveoker);
	static void DetonateAt5(WeaponTypeClass* pThis, const CoordStruct& coords, AbstractClass* pTarget, TechnoClass* pOwner, int damage, bool AddDamage, HouseClass* HouseInveoker);

	static void FireRadBeam(TechnoClass* pFirer, WeaponTypeClass* pWeapon, CoordStruct& source, CoordStruct& target);
	static void FireEbolt(TechnoClass* pFirer, WeaponTypeClass* pWeapon, CoordStruct& source, CoordStruct& target, int idx);

	bool IsVeterancyInThreshold(TechnoClass* pTarget) const;

	//return lepton
	static int GetRangeWithModifiers(WeaponTypeClass* pThis, TechnoClass* pFirer, std::optional<int> fallback = std::nullopt);
	static int GetTechnoKeepRange(WeaponTypeClass* pThis, TechnoClass* pFirer, bool isMinimum);

	static std::array<double, 16> cosLUT;
	static std::array<double, 16> sinLUT;
	static bool LutsInitialized;

	static void calculateCircuferences();
};

class WeaponTypeExtContainer final :public Container<WeaponTypeExtData>
	, public ReadWriteContainerInterfaces<WeaponTypeExtData>
{
public:
	static COMPILETIMEEVAL const char* ClassName = "WeaponTypeExtContainer";
	using base_t = Container<WeaponTypeExtData>;
	using ext_t = WeaponTypeExtData;

public:
	static WeaponTypeExtContainer Instance;

	virtual bool LoadAll(const json& root);
	virtual bool SaveAll(json& root);

	virtual void LoadFromINI(ext_t::base_type* key, CCINIClass* pINI, bool parseFailAddr);
	virtual void WriteToINI(ext_t::base_type* key, CCINIClass* pINI);
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