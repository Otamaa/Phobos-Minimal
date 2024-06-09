#include "Body.h"

#include <Ext/BulletType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Utilities/Macro.h>

#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>

int WeaponTypeExtData::nOldCircumference = DiskLaserClass::Radius;
PhobosMap<EBolt*, const WeaponTypeExtData*> WeaponTypeExtData::boltWeaponTypeExt;

void WeaponTypeExtData::Initialize()
{
	Burst_Delays.reserve(10);
	this->RadType = RadTypeClass::Array[0].get();
}

// =============================
// load / save

void WeaponTypeExtData::LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr)
{
	auto pThis = this->AttachedToObject;
	const char* pSection = pThis->ID;

	if (parseFailAddr)
		return;

	INI_EX exINI(pINI);

	{ // DiskLaser_Radius
		this->DiskLaser_Radius.Read(exINI, pSection, "DiskLaser.Radius");
		this->DiskLaser_Circumference = (int)(this->DiskLaser_Radius * Math::Pi * 2);
	}

	int Bolt_Count;
	if (detail::read(Bolt_Count , exINI, pSection, "Bolt.Count") && Bolt_Count > 0)
	{
		BoltData data { Bolt_Count };

		auto disabled = data.Disabled.begin();
		for (int i = 0; i < Bolt_Count; ++i)
		{
			bool temp {};
			std::string _number = std::to_string(i + 1);
			detail::read<bool>(temp, exINI, pSection, (std::string("Bolt.Disable") + _number).c_str());
			data.Disabled[i] = temp;
			detail::read(data.ColorData[i], exINI, pSection, (std::string("Bolt.Color") + _number).c_str());
		}

		this->WeaponBolt_Data = data;
	}
	else
	{
		this->Bolt_Color1.Read(exINI, pSection, "Bolt.Color1");
		this->Bolt_Color2.Read(exINI, pSection, "Bolt.Color2");
		this->Bolt_Color3.Read(exINI, pSection, "Bolt.Color3");
		this->Bolt_Disable1.Read(exINI, pSection, "Bolt.Disable1");
		this->Bolt_Disable2.Read(exINI, pSection, "Bolt.Disable2");
		this->Bolt_Disable3.Read(exINI, pSection, "Bolt.Disable3");
		this->Bolt_Arcs.Read(exINI, pSection, "Bolt.Arcs");
	}

	if (!Phobos::Otamaa::DisableCustomRadSite)
	{
		this->RadType.Read(exINI, pSection, "RadType" , true);
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
	this->Abductor_Temporal.Read(exINI, pSection, "Abductor.Temporal");
	this->Abductor_MaxHealth.Read(exINI, pSection, "Abductor.MaxHealth");
	this->Abductor_CheckAbductableWhenTargeting.Read(exINI, pSection, "Abductor.CheckAbductableWhenTargeting");

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

	this->RockerPitch.Read(exINI, pSection, "RockerPitch");

	if (this->RockerPitch > 0.0f) {
		constexpr auto halfpi = (Math::PI / 2);
		this->RockerPitch = 1.0f * halfpi;
	}

	this->MyAttachFireDatas.Read(exINI, pSection);

#pragma endregion

	this->Ammo.Read(exINI, pSection, "Ammo");
	this->IsDetachedRailgun.Read(exINI, pSection, "IsDetachedRailgun");

	this->Wave_IsLaser.Read(exINI, pSection, "Wave.IsLaser");
	this->Wave_IsBigLaser.Read(exINI, pSection, "Wave.IsBigLaser");
	this->Wave_IsHouseColor.Read(exINI, pSection, "Wave.IsHouseColor");

	if (this->IsWave() && !this->Wave_IsHouseColor)
	{
		this->Wave_Color.Read(exINI, pSection, "Wave.Color");
		this->Wave_Intent.Read(exINI, pSection, "Wave.Intensity");
	}

	static constexpr std::array<const char* const, sizeof(this->Wave_Reverse)> WaveReverseAgainst
	{
		{   { "Wave.ReverseAgainstVehicles" }, { "Wave.ReverseAgainstAircraft" },
			{ "Wave.ReverseAgainstBuildings" }, { "Wave.ReverseAgainstInfantry" },
			{ "Wave.ReverseAgainstOthers" }
		}
	};

	for (size_t i = 0; i < WaveReverseAgainst.size(); ++i)
	{
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
	this->Ivan_DeathBombOnAllies.Read(exINI, pSection, "IvanBomb.DeathBombOnAllies");
	this->Ivan_DeathBomb.Read(exINI, pSection, "IvanBomb.DeathBomb");
	this->Ivan_DetonateOnSell.Read(exINI, pSection, "IvanBomb.DetonateOnSell");
	this->ApplyDamage.Read(exINI, pSection, "ApplyDamage");

	this->Cursor_Attack.Read(exINI, pSection, "Cursor.Attack");
	this->Cursor_AttackOutOfRange.Read(exINI, pSection, "Cursor.AttackOutOfRange");

	this->Bolt_ParticleSys.Read(exINI, pSection, "Bolt.ParticleSystem");
	this->Laser_Thickness.Read(exINI, pSection, "LaserThickness");

	this->ExtraWarheads.Read(exINI, pSection, "ExtraWarheads");
	this->ExtraWarheads_DamageOverrides.Read(exINI, pSection, "ExtraWarheads.DamageOverrides");
	this->ExtraWarheads_DetonationChances.Read(exINI, pSection, "ExtraWarheads.DetonationChances");
	this->Burst_Retarget.Read(exINI, pSection, "Burst.Retarget");
	this->KickOutPassenger.Read(exINI, pSection, "KickOutPassenger");

	this->Beam_Duration.Read(exINI, pSection, "Beam.Duration");
	this->Beam_Amplitude.Read(exINI, pSection, "Beam.Amplitude");
	this->Beam_IsHouseColor.Read(exINI, pSection, "Beam.IsHouseColor");
	this->Beam_Color.Read(exINI, pSection, "Beam.Color");

	this->Bolt_ParticleSys_Enabled.Read(exINI, pSection, "Bolt.DisableParticleSystems");

	this->AmbientDamage_Warhead.Read(exINI, pSection, "AmbientDamage.Warhead");
	this->AmbientDamage_IgnoreTarget.Read(exINI, pSection, "AmbientDamage.IgnoreTarget");
	this->RecoilForce.Read(exINI, pSection, "RecoilForce");
	//this->BlockageTargetingBypassDamageOverride.Read(exINI, pSection, "BlockageTargetingBypassDamageOverride");

	this->AttachEffect_RequiredTypes.Read(exINI, pSection, "AttachEffect.RequiredTypes");
	this->AttachEffect_DisallowedTypes.Read(exINI, pSection, "AttachEffect.DisallowedTypes");
	exINI.ParseStringList(this->AttachEffect_RequiredGroups, pSection, "AttachEffect.RequiredGroups");
	exINI.ParseStringList(this->AttachEffect_DisallowedGroups, pSection, "AttachEffect.DisallowedGroups");
	this->AttachEffect_RequiredMinCounts.Read(exINI, pSection, "AttachEffect.RequiredMinCounts");
	this->AttachEffect_RequiredMaxCounts.Read(exINI, pSection, "AttachEffect.RequiredMaxCounts");
	this->AttachEffect_DisallowedMinCounts.Read(exINI, pSection, "AttachEffect.DisallowedMinCounts");
	this->AttachEffect_DisallowedMaxCounts.Read(exINI, pSection, "AttachEffect.DisallowedMaxCounts");
	this->AttachEffect_IgnoreFromSameSource.Read(exINI, pSection, "AttachEffect.IgnoreFromSameSource");
}

int WeaponTypeExtData::GetRangeWithModifiers(WeaponTypeClass* pThis, TechnoClass* pFirer, std::optional<int> fallback)
{
	int range = fallback.has_value() ? fallback.value()  : 0;

	if (!pThis && !pFirer)
		return range;
	else if (pFirer && pFirer->CanOccupyFire())
		range = RulesClass::Instance->OccupyWeaponRange * Unsorted::LeptonsPerCell;
	else if (pThis && pFirer)
		range = pThis->Range;
	else
		return range;

	if (range == -512)
		return range;

	auto pTechno = pFirer;

	if (pTechno->Transporter && pTechno->Transporter->GetTechnoType()->OpenTopped) {
		auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pTechno->Transporter->GetTechnoType());

		if (pTypeExt->OpenTopped_UseTransportRangeModifiers)
			pTechno = pTechno->Transporter;
	}

	{
		auto pExt = TechnoExtContainer::Instance.Find(pTechno);
		if (pExt->AE_ExtraRange.Enabled()){
			range = pExt->AE_ExtraRange.Get(range, pThis);
		}
	}
	return MaxImpl(range, 0);
}

#include <New/PhobosAttachedAffect/Functions.h>

bool WeaponTypeExtData::HasRequiredAttachedEffects(TechnoClass* pTechno, TechnoClass* pFirer)
{
	bool hasRequiredTypes = this->AttachEffect_RequiredTypes.size() > 0;
	bool hasDisallowedTypes = this->AttachEffect_DisallowedTypes.size() > 0;
	bool hasRequiredGroups = this->AttachEffect_RequiredGroups.size() > 0;
	bool hasDisallowedGroups = this->AttachEffect_DisallowedGroups.size() > 0;

	if (hasRequiredTypes || hasDisallowedTypes || hasRequiredGroups || hasDisallowedGroups)
	{
		auto const pTechnoExt = TechnoExtContainer::Instance.Find(pTechno);

		if (hasDisallowedTypes && PhobosAEFunctions::HasAttachedEffects(pTechno, this->AttachEffect_DisallowedTypes, false, this->AttachEffect_IgnoreFromSameSource, pFirer, this->AttachedToObject->Warhead, this->AttachEffect_DisallowedMinCounts, this->AttachEffect_DisallowedMaxCounts))
			return false;

		if (hasDisallowedGroups && PhobosAEFunctions::HasAttachedEffects(pTechno, PhobosAttachEffectTypeClass::GetTypesFromGroups(this->AttachEffect_DisallowedGroups), false, this->AttachEffect_IgnoreFromSameSource, pFirer, this->AttachedToObject->Warhead, this->AttachEffect_DisallowedMinCounts, this->AttachEffect_DisallowedMaxCounts))
			return false;

		if (hasRequiredTypes && !PhobosAEFunctions::HasAttachedEffects(pTechno, this->AttachEffect_RequiredTypes, true, this->AttachEffect_IgnoreFromSameSource, pFirer, this->AttachedToObject->Warhead, this->AttachEffect_RequiredMinCounts, this->AttachEffect_RequiredMaxCounts))
			return false;

		if (hasRequiredGroups &&
			!PhobosAEFunctions::HasAttachedEffects(pTechno, PhobosAttachEffectTypeClass::GetTypesFromGroups(this->AttachEffect_RequiredGroups), true, this->AttachEffect_IgnoreFromSameSource, pFirer, this->AttachedToObject->Warhead, this->AttachEffect_RequiredMinCounts, this->AttachEffect_RequiredMaxCounts))
			return false;
	}

	return true;
}

ColorStruct WeaponTypeExtData::GetBeamColor() const
{
	const auto pThis = this->AttachedToObject;
	const auto& result = this->Beam_Color;

	if (pThis->IsRadBeam || pThis->IsRadEruption) {
		if (pThis->Warhead && pThis->Warhead->Temporal) {
			return result.Get(RulesClass::Instance->ChronoBeamColor);
		}
	}

	return result.Get(RulesClass::Instance->RadColor);
}

template <typename T>
void WeaponTypeExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Initialized)
		.Process(this->DiskLaser_Radius)
		.Process(this->DiskLaser_Circumference)
		.Process(this->Rad_NoOwner)
		.Process(this->Bolt_Disable1)
		.Process(this->Bolt_Disable2)
		.Process(this->Bolt_Disable3)
		.Process(this->Bolt_Arcs)
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
		.Process(this->Abductor_Temporal)
		.Process(this->Abductor_MaxHealth)
		.Process(this->Abductor_CheckAbductableWhenTargeting)
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
		.Process(this->RockerPitch)
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
		.Process(this->Ivan_DeathBombOnAllies)
		.Process(this->Ivan_DeathBomb)
		.Process(this->ApplyDamage)
		.Process(this->Cursor_Attack)
		.Process(this->Cursor_AttackOutOfRange)
		.Process(this->WeaponBolt_Data)
		.Process(this->Bolt_Color1)
		.Process(this->Bolt_Color2)
		.Process(this->Bolt_Color3)
		.Process(this->Bolt_ParticleSys)
		.Process(this->Laser_Thickness)

		.Process(this->ExtraWarheads)
		.Process(this->ExtraWarheads_DamageOverrides)
		.Process(this->ExtraWarheads_DetonationChances)
		.Process(this->Burst_Retarget)
		.Process(this->KickOutPassenger)

		.Process(this->Beam_Color)
		.Process(this->Beam_Duration)
		.Process(this->Beam_Amplitude)
		.Process(this->Beam_IsHouseColor)

		.Process(this->Bolt_ParticleSys_Enabled)

		.Process(this->AmbientDamage_Warhead)
		.Process(this->AmbientDamage_IgnoreTarget)
		.Process(this->RecoilForce)
		//.Process(this->BlockageTargetingBypassDamageOverride)

		.Process(this->AttachEffect_RequiredTypes)
		.Process(this->AttachEffect_DisallowedTypes)
		.Process(this->AttachEffect_RequiredGroups)
		.Process(this->AttachEffect_DisallowedGroups)
		.Process(this->AttachEffect_RequiredMinCounts)
		.Process(this->AttachEffect_RequiredMaxCounts)
		.Process(this->AttachEffect_DisallowedMinCounts)
		.Process(this->AttachEffect_DisallowedMaxCounts)
		.Process(this->AttachEffect_IgnoreFromSameSource)
		;

	MyAttachFireDatas.Serialize(Stm);
};

int WeaponTypeExtData::GetBurstDelay(WeaponTypeClass* pThis, int burstIndex)
{
	auto const pExt = WeaponTypeExtContainer::Instance.Find(pThis);

	if (burstIndex == 0)
		return 0;

	if (!pExt->Burst_Delays.empty())
	{
		const size_t nSize = pExt->Burst_Delays.size();

		if (nSize > (size_t)burstIndex)
			return pExt->Burst_Delays[burstIndex - 1];
		else
			return pExt->Burst_Delays[nSize - 1];
	}

	return -1;
}

void WeaponTypeExtData::DetonateAt(WeaponTypeClass* pThis, AbstractClass* pTarget, TechnoClass* pOwner, bool AddDamage, HouseClass* HouseInveoker)
{
	WeaponTypeExtData::DetonateAt(pThis, pTarget, pOwner, pThis->Damage , AddDamage , HouseInveoker);
}

void WeaponTypeExtData::DetonateAt(WeaponTypeClass* pThis, AbstractClass* pTarget, TechnoClass* pOwner, int damage, bool AddDamage, HouseClass* HouseInveoker)
{
	if (pThis->Warhead->NukeMaker)
	{
		if (!pTarget)
		{
			Debug::Log("WeaponTypeExtData::DetonateAt , cannot execute when invalid Target is present , need to be avail ! \n");
			return;
		}
	}

	auto pBulletTypeExt = BulletTypeExtContainer::Instance.Find(pThis->Projectile);
	auto pExt = WeaponTypeExtContainer::Instance.Find(pThis);

	if (BulletClass* pBullet = pBulletTypeExt->CreateBullet(pTarget, pOwner,
		damage, pThis->Warhead, pThis->Speed, pExt->GetProjectileRange(), pThis->Bright || pThis->Warhead->Bright, AddDamage))
	{
		pBullet->SetWeaponType(pThis);
		BulletExtData::DetonateAt(pBullet, pTarget, pOwner, CoordStruct::Empty , HouseInveoker);
	}
}

void WeaponTypeExtData::DetonateAt(WeaponTypeClass* pThis, const CoordStruct& coords, TechnoClass* pOwner, bool AddDamage, HouseClass* HouseInveoker)
{
	WeaponTypeExtData::DetonateAt(pThis, coords, pOwner, pThis->Damage , AddDamage , HouseInveoker);
}

void WeaponTypeExtData::DetonateAt(WeaponTypeClass* pThis, const CoordStruct& coords, TechnoClass* pOwner, int damage, bool AddDamage, HouseClass* HouseInveoker)
{
	if (!coords.IsValid())
	{
		Debug::Log("WeaponTypeExtData::DetonateAt Coords empty ! ");
		return;
	}

	WeaponTypeExtData::DetonateAt(pThis, MapClass::Instance->GetCellAt(coords), pOwner, damage, AddDamage , HouseInveoker);
}

void WeaponTypeExtData::DetonateAt(WeaponTypeClass* pThis, const CoordStruct& coords, AbstractClass* pTarget, TechnoClass* pOwner, int damage, bool AddDamage, HouseClass* HouseInveoker)
{
	if (pThis->Warhead->NukeMaker)
	{
		if (!pTarget)
		{
			Debug::Log("WeaponTypeExtData::DetonateAt , cannot execute when invalid Target is present , need to be avail ! \n");
			return;
		}
	}

	auto pBulletTypeExt = BulletTypeExtContainer::Instance.Find(pThis->Projectile);
	auto pExt = WeaponTypeExtContainer::Instance.Find(pThis);

	if (BulletClass* pBullet = pBulletTypeExt->CreateBullet(pTarget, pOwner,
		damage, pThis->Warhead, pThis->Speed, pExt->GetProjectileRange(), pThis->Bright || pThis->Warhead->Bright, AddDamage))
	{
		pBullet->SetWeaponType(pThis);
		BulletExtData::DetonateAt(pBullet, pTarget, pOwner, coords , HouseInveoker);
	}
}

EBolt* WeaponTypeExtData::CreateBolt(WeaponTypeClass* pWeapon)
{
	return WeaponTypeExtData::CreateBolt(WeaponTypeExtContainer::Instance.Find(pWeapon));
}

EBolt* WeaponTypeExtData::CreateBolt(WeaponTypeExtData* pWeapon)
{
	auto ret = GameCreate<EBolt>();

	if (ret && pWeapon)
	{
		WeaponTypeExtData::boltWeaponTypeExt[ret] = pWeapon;
	}

	return ret;
}

// =============================
// container
WeaponTypeExtContainer WeaponTypeExtContainer::Instance;

bool WeaponTypeExtContainer::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Process(WeaponTypeExtData::nOldCircumference)
		.Process(WeaponTypeExtData::boltWeaponTypeExt)
		.Success();
}

bool WeaponTypeExtContainer::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Process(WeaponTypeExtData::nOldCircumference)
		.Process(WeaponTypeExtData::boltWeaponTypeExt)
		.Success();
}

void WeaponTypeExtContainer::Clear()
{
	WeaponTypeExtData::boltWeaponTypeExt.clear();
}

// =============================
// container hooks
//

DEFINE_HOOK(0x771EE0, WeaponTypeClass_CTOR, 0x6)
{
	GET(WeaponTypeClass*, pItem, ESI);
	WeaponTypeExtContainer::Instance.Allocate(pItem);
	return 0;
}

DEFINE_HOOK(0x77311D, WeaponTypeClass_SDDTOR, 0x6)
{
	GET(WeaponTypeClass*, pItem, ESI);
	WeaponTypeExtContainer::Instance.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x772EB0, WeaponTypeClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x772CD0, WeaponTypeClass_SaveLoad_Prefix, 0x7)
{
	GET_STACK(WeaponTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	WeaponTypeExtContainer::Instance.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x772EA6, WeaponTypeClass_Load_Suffix, 0x6)
{
	WeaponTypeExtContainer::Instance.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x772F8C, WeaponTypeClass_Save, 0x5)
{
	WeaponTypeExtContainer::Instance.SaveStatic();
	return 0;
}

DEFINE_HOOK_AGAIN(0x7729C7, WeaponTypeClass_LoadFromINI, 0x5)
DEFINE_HOOK_AGAIN(0x7729D6, WeaponTypeClass_LoadFromINI, 0x5)
DEFINE_HOOK(0x7729B0, WeaponTypeClass_LoadFromINI, 0x5)
{
	GET(WeaponTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0xE4);

	WeaponTypeExtContainer::Instance.LoadFromINI(pItem, pINI, R->Origin() == 0x7729D6);

	return 0;
}
