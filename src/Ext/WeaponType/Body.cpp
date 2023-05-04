#include "Body.h"

#include <Ext/BulletType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Utilities/Macro.h>

WeaponTypeExt::ExtContainer WeaponTypeExt::ExtMap;
WeaponTypeClass* WeaponTypeExt::Temporal_WP = nullptr;
int WeaponTypeExt::nOldCircumference = DiskLaserClass::Radius;

void WeaponTypeExt::ExtData::Initialize()
{
}

void WeaponTypeExt::ExtData::InitializeConstants()
{
	Burst_Delays.reserve(10);
	this->RadType = RadTypeClass::Find(RADIATION_SECTION);
}

// =============================
// load / save

void WeaponTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI)
{
	auto pThis = this->Get();
	const char* pSection = pThis->ID;

	if (!pINI->GetSection(pSection))
		return;

	INI_EX exINI(pINI);

	{ // DiskLaser_Radius
		this->DiskLaser_Radius.Read(exINI, pSection, "DiskLaser.Radius");
		this->DiskLaser_Circumference = (int)(this->DiskLaser_Radius * Math::Pi * 2);
	}

	this->Bolt_Disable1.Read(exINI, pSection, "Bolt.Disable1");
	this->Bolt_Disable2.Read(exINI, pSection, "Bolt.Disable2");
	this->Bolt_Disable3.Read(exINI, pSection, "Bolt.Disable3");

	if (!Phobos::Otamaa::DisableCustomRadSite)
	{
		this->RadType.Read(exINI, pSection, "RadType");
		this->Rad_NoOwner.Read(exINI, pSection, "Rad.NoOwner");
	}

	this->Strafing_Shots.Read(exINI, pSection, "Strafing.Shots");
	this->Strafing_SimulateBurst.Read(exINI, pSection, "Strafing.SimulateBurst");
	this->CanTarget.Read(exINI, pSection, "CanTarget");
	this->CanTargetHouses.Read(exINI, pSection, "CanTargetHouses");
	this->Burst_Delays.Read(exINI, pSection, "Burst.Delays");
	this->AreaFire_Target.Read(exINI, pSection, "AreaFire.Target");
	this->FeedbackWeapon.Read(exINI, pSection, "FeedbackWeapon", true);
	this->Laser_IsSingleColor.Read(exINI, pSection, "IsSingleColor");
	this->Trajectory_Speed.Read(exINI, pSection, "Trajectory.Speed");

	this->Abductor.Read(exINI, pSection, "Abductor");
	this->Abductor_AnimType.Read(exINI, pSection, "Abductor.Anim");
	this->Abductor_ChangeOwner.Read(exINI, pSection, "Abductor.ChangeOwner");
	this->Abductor_AbductBelowPercent.Read(exINI, pSection, "Abductor.AbductBelowPercent");

	this->DelayedFire_Anim.Read(exINI, pSection, "DelayedFire.Anim");
	this->DelayedFire_Anim_LoopCount.Read(exINI, pSection, "DelayedFire.Anim.LoopCount");
	this->DelayedFire_Anim_UseFLH.Read(exINI, pSection, "DelayedFire.Anim.UseFLH");
	this->DelayedFire_DurationTimer.Read(exINI, pSection, "DelayedFire.DurationTimer");
	this->Burst_FireWithinSequence.Read(exINI, pSection, "Burst.FireWithinSequence");
	this->ROF_RandomDelay.Read(exINI, pSection, "ROF.RandomDelay");
	this->OmniFire_TurnToTarget.Read(exINI, pSection, "OmniFire.TurnToTarget");
#pragma region Otamaa
	this->Ylo.Read(exINI, pSection, GameStrings::ShakeYlo());
	this->Yhi.Read(exINI, pSection, GameStrings::ShakeYhi());
	this->Xhi.Read(exINI, pSection, GameStrings::ShakeXhi());
	this->Xlo.Read(exINI, pSection, GameStrings::ShakeXlo());
	this->ShakeLocal.Read(exINI, pSection, "Shake.Local");

	this->OccupantAnims.Read(exINI, pSection, "OccupantAnims");
	this->OccupantAnim_UseMultiple.Read(exINI, pSection, "OccupantAnim.UseAnimsInstead");

	this->Range_IgnoreVertical.Read(exINI, pSection, "Range.IgnoreVertical");
	// brought back from TS
	this->ProjectileRange.Read(exINI, pSection, "ProjectileRange");
	this->Decloak_InstantFire.Read(exINI, pSection, "DecloakInstantFire");

	this->Feedback_Anim.Read(exINI, pSection, "FeedbackAnim");
	this->Feedback_Anim_Offset.Read(exINI, pSection, "FeedbackAnim.Offset");
	this->Feedback_Anim_UseFLH.Read(exINI, pSection, "FeedbackAnim.UseFLH");

	this->DestroyTechnoAfterFiring.Read(exINI, pSection, "DeleteAfterFiring");
	this->RemoveTechnoAfterFiring.Read(exINI, pSection, "RemoveAfterFiring");
	this->OpentoppedAnim.Read(exINI, pSection, "OpenToppedAnim", true);
	this->DiskLaser_FiringOffset.Read(exINI, pSection, "DiskLaser.FiringOffset");

	this->Targeting_Health_Percent.Read(exINI, pSection, "Targeting.TargetHealthPercent");
	this->Targeting_Health_Percent_Below.Read(exINI, pSection, "Targeting.TargetHealthPercentCheckBelowPercent");

#ifdef COMPILE_PORTED_DP_FEATURES
	this->RockerPitch.Read(exINI, pSection, "RockerPitch");

	if (this->RockerPitch > 0.0f)
	{
		this->RockerPitch = 1.0f;
		this->RockerPitch = this->RockerPitch.Get() * (Math::PI / 2);
	}


	this->MyAttachFireDatas.Read(exINI, pSection);
#endif
#pragma endregion

	this->Ammo.Read(exINI, pSection, "Ammo");
	this->IsDetachedRailgun.Read(exINI, pSection, "IsDetachedRailgun");

	this->Wave_IsLaser.Read(exINI, pSection, "Wave.IsLaser");
	this->Wave_IsBigLaser.Read(exINI, pSection, "Wave.IsBigLaser");
	this->Wave_IsHouseColor.Read(exINI, pSection, "Wave.IsHouseColor");

	if (this->IsWave() && !this->Wave_IsHouseColor) {
		this->Wave_Color.Read(exINI, pSection, "Wave.Color");
		this->Wave_Intent.Read(exINI, pSection, "Wave.Intensity");
	}

	static constexpr std::array<const char* const, sizeof(this->Wave_Reverse)> WaveReverseAgainst
	{
	{   { "Wave.ReverseAgainstVehicles" } , { "Wave.ReverseAgainstAircraft" } ,
		{ "Wave.ReverseAgainstBuildings" } , { "Wave.ReverseAgainstInfantry" } ,
		{ "Wave.ReverseAgainstOthers"}
	}
	};

	for (size_t i = 0; i < WaveReverseAgainst.size(); ++i) {
		this->Wave_Reverse[i] = pINI->ReadBool(pSection, WaveReverseAgainst[i], this->Wave_Reverse[i]);
	}

	this->Ivan_KillsBridges.Read(exINI, pSection, "IvanBomb.DestroysBridges");
	this->Ivan_Detachable.Read(exINI, pSection, "IvanBomb.Detachable");
	this->Ivan_Damage.Read(exINI, pSection, "IvanBomb.Damage");
	this->Ivan_Delay.Read(exINI, pSection, "IvanBomb.Delay");
	this->Ivan_FlickerRate.Read(exINI, pSection, "IvanBomb.FlickerRate");
	this->Ivan_TickingSound.Read(exINI, pSection, "IvanBomb.TickingSound");
	this->Ivan_AttachSound.Read(exINI, pSection, "IvanBomb.AttachSound");
	this->Ivan_WH.Read(exINI, pSection, "IvanBomb.Warhead");
	this->Ivan_Image.Read(exINI, pSection, "IvanBomb.Image");
	this->Ivan_CanDetonateTimeBomb.Read(exINI, pSection, "IvanBomb.CanDetonateTimeBomb");
	this->Ivan_CanDetonateDeathBomb.Read(exINI, pSection, "IvanBomb.CanDetonateDeathBomb");
	this->Ivan_DetonateOnSell.Read(exINI, pSection, "IvanBomb.DetonateOnSell");
	this->ApplyDamage.Read(exINI, pSection, "ApplyDamage");

	this->Cursor_Attack.Read(exINI, pSection, "Cursor.Attack");
	this->Cursor_AttackOutOfRange.Read(exINI, pSection, "Cursor.AttackOutOfRange");
}

template <typename T>
void WeaponTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->DiskLaser_Radius)
		.Process(this->DiskLaser_Circumference)
		.Process(this->Rad_NoOwner)
		.Process(this->Bolt_Disable1)
		.Process(this->Bolt_Disable2)
		.Process(this->Bolt_Disable3)
		.Process(this->Strafing_Shots)
		.Process(this->Strafing_SimulateBurst)
		.Process(this->CanTarget)
		.Process(this->CanTargetHouses)
		.Process(this->RadType)
		.Process(this->Burst_Delays)
		.Process(this->AreaFire_Target)
		.Process(this->FeedbackWeapon)
		.Process(this->Laser_IsSingleColor)
		.Process(this->Trajectory_Speed)
		.Process(this->Abductor)
		.Process(this->Abductor_AnimType)
		.Process(this->Abductor_ChangeOwner)
		.Process(this->Abductor_AbductBelowPercent)
		.Process(this->DelayedFire_Anim)
		.Process(this->DelayedFire_Anim_LoopCount)
		.Process(this->DelayedFire_Anim_UseFLH)
		.Process(this->DelayedFire_DurationTimer)
		.Process(this->Burst_FireWithinSequence)
		.Process(this->ROF_RandomDelay)
		.Process(this->OmniFire_TurnToTarget)
		.Process(this->Ylo)
		.Process(this->Xlo)
		.Process(this->Xhi)
		.Process(this->Yhi)
		.Process(this->ShakeLocal)
		.Process(this->OccupantAnims)
		.Process(this->OccupantAnim_UseMultiple)
		.Process(this->Range_IgnoreVertical)
		.Process(this->ProjectileRange)
		.Process(this->Decloak_InstantFire)
		.Process(this->Feedback_Anim)
		.Process(this->Feedback_Anim_Offset)
		.Process(this->Feedback_Anim_UseFLH)
		.Process(this->DestroyTechnoAfterFiring)
		.Process(this->RemoveTechnoAfterFiring)
		.Process(this->OpentoppedAnim)
		.Process(this->DiskLaser_FiringOffset)
		.Process(this->Targeting_Health_Percent)
		.Process(this->Targeting_Health_Percent_Below)
#ifdef COMPILE_PORTED_DP_FEATURES
		.Process(this->RockerPitch)
#endif
		.Process(this->Ammo)
		.Process(this->IsDetachedRailgun)

		.Process(this->Wave_IsHouseColor)
		.Process(this->Wave_IsLaser)
		.Process(this->Wave_IsBigLaser)
		.Process(this->Wave_Color)
		.Process(this->Wave_Intent)
		.Process(this->Wave_Reverse)

		.Process(this->Ivan_KillsBridges)
		.Process(this->Ivan_Detachable)
		.Process(this->Ivan_Damage)
		.Process(this->Ivan_Delay)
		.Process(this->Ivan_TickingSound)
		.Process(this->Ivan_AttachSound)
		.Process(this->Ivan_WH)
		.Process(this->Ivan_Image)
		.Process(this->Ivan_FlickerRate)
		.Process(this->Ivan_CanDetonateTimeBomb)
		.Process(this->Ivan_CanDetonateDeathBomb)
		.Process(this->Ivan_DetonateOnSell)
		.Process(this->ApplyDamage)
		.Process(this->Cursor_Attack)
		.Process(this->Cursor_AttackOutOfRange)
		;

#ifdef COMPILE_PORTED_DP_FEATURES
	MyAttachFireDatas.Serialize(Stm);
#endif
};

int WeaponTypeExt::GetBurstDelay(WeaponTypeClass* pThis, int burstIndex)
{
	auto const pExt = WeaponTypeExt::ExtMap.Find(pThis);

	if (burstIndex == 0)
		return 0;

	if (!pExt->Burst_Delays.empty())
	{
		const size_t nSize = pExt->Burst_Delays.size();

		if (nSize > (unsigned)burstIndex)
			return pExt->Burst_Delays[burstIndex - 1];
		else 
			return pExt->Burst_Delays[nSize - 1];
	}

	return -1;
}

void WeaponTypeExt::ExtContainer::InvalidatePointer(void* ptr, bool bRemoved)
{
	AnnounceInvalidPointer(WeaponTypeExt::Temporal_WP, ptr);
}

void WeaponTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<WeaponTypeClass>::LoadFromStream(Stm);
	this->Serialize(Stm);

}

void WeaponTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<WeaponTypeClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool WeaponTypeExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Process(Temporal_WP)
		.Process(nOldCircumference)
		.Success();
}

bool WeaponTypeExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Process(Temporal_WP)
		.Process(nOldCircumference)
		.Success();
}

void WeaponTypeExt::DetonateAt(WeaponTypeClass* pThis, AbstractClass* pTarget, TechnoClass* pOwner)
{
	WeaponTypeExt::DetonateAt(pThis, pTarget, pOwner, pThis->Damage);
}

void WeaponTypeExt::DetonateAt(WeaponTypeClass* pThis, AbstractClass* pTarget, TechnoClass* pOwner, int damage)
{
	if(pThis->Warhead->NukeMaker) {
		if (!pTarget) {
			Debug::Log("WeaponTypeExt::DetonateAt , cannot execute when invalid Target is present , need to be avail ! \n");
			return;
		}
	}

	auto pBulletTypeExt = BulletTypeExt::ExtMap.Find(pThis->Projectile);
	auto pExt = WeaponTypeExt::ExtMap.Find(pThis);

	if (BulletClass* pBullet = pBulletTypeExt->CreateBullet(pTarget, pOwner,
		damage, pThis->Warhead, pThis->Speed, pExt->GetProjectileRange(), pThis->Bright || pThis->Warhead->Bright, true))
	{
		pBullet->SetWeaponType(pThis);
		BulletExt::DetonateAt(pBullet, pTarget, pOwner);
	}
}

void WeaponTypeExt::DetonateAt(WeaponTypeClass* pThis, const CoordStruct& coords, TechnoClass* pOwner)
{
	WeaponTypeExt::DetonateAt(pThis, coords, pOwner, pThis->Damage);
}

void WeaponTypeExt::DetonateAt(WeaponTypeClass* pThis, const CoordStruct& coords, TechnoClass* pOwner, int damage)
{
	if(!coords) {
		Debug::Log("WeaponTypeExt::DetonateAt Coords empty ! ");
		return;
	}

	WeaponTypeExt::DetonateAt(pThis, MapClass::Instance->GetCellAt(coords) , pOwner, damage);
}

void WeaponTypeExt::DetonateAt(WeaponTypeClass* pThis, const CoordStruct& coords, AbstractClass* pTarget, TechnoClass* pOwner, int damage)
{
	if (pThis->Warhead->NukeMaker)
	{
		if (!pTarget)
		{
			Debug::Log("WeaponTypeExt::DetonateAt , cannot execute when invalid Target is present , need to be avail ! \n");
			return;
		}
	}

	auto pBulletTypeExt = BulletTypeExt::ExtMap.Find(pThis->Projectile);
	auto pExt = WeaponTypeExt::ExtMap.Find(pThis);

	if (BulletClass* pBullet = pBulletTypeExt->CreateBullet(pTarget, pOwner,
		damage, pThis->Warhead, pThis->Speed, pExt->GetProjectileRange(), pThis->Bright || pThis->Warhead->Bright, true))
	{
		pBullet->SetWeaponType(pThis);
		BulletExt::DetonateAt(pBullet, pTarget, pOwner, coords);
	}
}
// =============================
// container

WeaponTypeExt::ExtContainer::ExtContainer() : Container("WeaponTypeClass") { }
WeaponTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks
//
DEFINE_HOOK(0x771EE9, WeaponTypeClass_CTOR, 0x5)
{
	GET(WeaponTypeClass*, pItem, ESI);
	WeaponTypeExt::ExtMap.Allocate(pItem);
	return 0;
}

DEFINE_HOOK(0x77311D, WeaponTypeClass_SDDTOR, 0x6)
{
	GET(WeaponTypeClass*, pItem, ESI);
	WeaponTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x772EB0, WeaponTypeClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x772CD0, WeaponTypeClass_SaveLoad_Prefix, 0x7)
{
	GET_STACK(WeaponTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	WeaponTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x772EA6, WeaponTypeClass_Load_Suffix, 0x6)
{
	WeaponTypeExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x772F8C, WeaponTypeClass_Save, 0x5)
{
	WeaponTypeExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK_AGAIN(0x7729C7, WeaponTypeClass_LoadFromINI, 0x5)
DEFINE_HOOK_AGAIN(0x7729D6, WeaponTypeClass_LoadFromINI, 0x5)
DEFINE_HOOK(0x7729B0, WeaponTypeClass_LoadFromINI, 0x5)
{
	GET(WeaponTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0xE4);

	WeaponTypeExt::ExtMap.LoadFromINI(pItem, pINI);

	return 0;
}
