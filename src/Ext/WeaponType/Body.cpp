#include "Body.h"

#include <Ext/BulletType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Utilities/Macro.h>

#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>


#include <EBolt.h>

#pragma region defines
int WeaponTypeExtData::nOldCircumference { DiskLaserClass::Radius };
#pragma endregion

void WeaponTypeExtData::Initialize()
{
	Burst_Delays.reserve(10);
	this->RadType = RadTypeClass::Array.begin()->get();
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

#ifdef _Enable
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
#endif

	{
		static constexpr std::array<std::pair<const char*, const char*> , 3u> Bolt_Tags {{
			{ "Bolt.Color1" , "Bolt.Disable1" } ,
			{ "Bolt.Color2" , "Bolt.Disable2" } ,
			{ "Bolt.Color3" , "Bolt.Disable3" }
		}};

		for(int i = 0; i < 3; ++i) {
			this->Bolt_Colors[i].Read(exINI, pSection, Bolt_Tags[i].first);
			this->Bolt_Disables[i].Read(exINI, pSection, Bolt_Tags[i].second);
		}

		this->Bolt_Arcs.Read(exINI, pSection, "Bolt.Arcs");
		this->Bolt_Duration.Read(exINI, pSection, "Bolt.Duration");
		this->Bolt_ParticleSys_Enabled.Read(exINI, pSection, "Bolt.DisableParticleSystems");
		this->Bolt_ParticleSys.Read(exINI, pSection, "Bolt.ParticleSystem");
		this->Bolt_FollowFLH.Read(exINI, pSection, "Bolt.FollowFLH");
	}

	this->RadType.Read(exINI, pSection, "RadType" , true);
	this->Rad_NoOwner.Read(exINI, pSection, "Rad.NoOwner");

	this->Strafing.Read(exINI, pSection, "Strafing");
	this->Strafing_Shots.Read(exINI, pSection, "Strafing.Shots");
	this->Strafing_SimulateBurst.Read(exINI, pSection, "Strafing.SimulateBurst");
	this->Strafing_UseAmmoPerShot.Read(exINI, pSection, "Strafing.UseAmmoPerShot");
	this->Strafing_EndDelay.Read(exINI, pSection, "Strafing.EndDelay");

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

	this->Burst_FireWithinSequence.Read(exINI, pSection, "Burst.FireWithinSequence");
	this->Burst_NoDelay.Read(exINI, pSection, "Burst.NoDelay");
	this->ROF_RandomDelay.Read(exINI, pSection, "ROF.RandomDelay");
	this->ChargeTurret_Delays.Read(exINI, pSection, "ChargeTurret.Delays");
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
		COMPILETIMEEVAL auto halfpi = (Math::PI / 2);
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

	static COMPILETIMEEVAL std::array<const char* const, sizeof(this->Wave_Reverse)> WaveReverseAgainst
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
	this->Laser_Thickness.Read(exINI, pSection, "LaserThickness");

	this->ExtraWarheads.Read(exINI, pSection, "ExtraWarheads");
	this->ExtraWarheads_DamageOverrides.Read(exINI, pSection, "ExtraWarheads.DamageOverrides");
	this->ExtraWarheads_DetonationChances.Read(exINI, pSection, "ExtraWarheads.DetonationChances");
	this->ExtraWarheads_FullDetonation.Read(exINI, pSection, "ExtraWarheads.FullDetonation");
	this->Burst_Retarget.Read(exINI, pSection, "Burst.Retarget");
	this->KickOutPassenger.Read(exINI, pSection, "KickOutPassenger");

	this->Beam_Duration.Read(exINI, pSection, "Beam.Duration");
	this->Beam_Amplitude.Read(exINI, pSection, "Beam.Amplitude");
	this->Beam_IsHouseColor.Read(exINI, pSection, "Beam.IsHouseColor");
	this->Beam_Color.Read(exINI, pSection, "Beam.Color");

	this->AmbientDamage_Warhead.Read(exINI, pSection, "AmbientDamage.Warhead");
	this->AmbientDamage_IgnoreTarget.Read(exINI, pSection, "AmbientDamage.IgnoreTarget");
	this->RecoilForce.Read(exINI, pSection, "RecoilForce");
	//this->BlockageTargetingBypassDamageOverride.Read(exINI, pSection, "BlockageTargetingBypassDamageOverride");

	// AttachEffect
	this->AttachEffects.LoadFromINI(pINI, pSection);

	this->AttachEffect_RequiredTypes.Read(exINI, pSection, "AttachEffect.RequiredTypes");
	this->AttachEffect_DisallowedTypes.Read(exINI, pSection, "AttachEffect.DisallowedTypes");
	exINI.ParseList(this->AttachEffect_RequiredGroups, pSection, "AttachEffect.RequiredGroups");
	exINI.ParseList(this->AttachEffect_DisallowedGroups, pSection, "AttachEffect.DisallowedGroups");
	this->AttachEffect_RequiredMinCounts.Read(exINI, pSection, "AttachEffect.RequiredMinCounts");
	this->AttachEffect_RequiredMaxCounts.Read(exINI, pSection, "AttachEffect.RequiredMaxCounts");
	this->AttachEffect_DisallowedMinCounts.Read(exINI, pSection, "AttachEffect.DisallowedMinCounts");
	this->AttachEffect_DisallowedMaxCounts.Read(exINI, pSection, "AttachEffect.DisallowedMaxCounts");
	this->AttachEffect_CheckOnFirer.Read(exINI, pSection, "AttachEffect.CheckOnFirer");
	this->AttachEffect_IgnoreFromSameSource.Read(exINI, pSection, "AttachEffect.IgnoreFromSameSource");

	this->AttachEffect_Enable = (this->AttachEffects.AttachTypes.size() > 0 || this->AttachEffects.RemoveTypes.size() > 0 || this->AttachEffects.RemoveGroups.size() > 0);

	this->FireOnce_ResetSequence.Read(exINI, pSection, "FireOnce.ResetSequence");
	this->NoRepeatFire.Read(exINI, pSection, "NoRepeatFire");

	this->KeepRange.Read(exINI, pSection, "KeepRange");
	this->KeepRange_AllowAI.Read(exINI, pSection, "KeepRange.AllowAI");
	this->KeepRange_AllowPlayer.Read(exINI, pSection, "KeepRange.AllowPlayer");
	this->VisualScatter.Read(exINI, pSection, "VisualScatter");
	this->TurretRecoil_Suppress.Read(exINI, pSection, "TurretRecoil.Suppress");

	this->CanTarget_MaxHealth.Read(exINI, pSection, "CanTarget.MaxHealth");
	this->CanTarget_MinHealth.Read(exINI, pSection, "CanTarget.MinHealth");

	this->DelayedFire_Duration.Read(exINI, pSection, "DelayedFire.Duration");
	this->DelayedFire_AnimOnTurret.Read(exINI, pSection, "DelayedFire.AnimOnTurret");
	this->DelayedFire_SkipInTransport.Read(exINI, pSection, "DelayedFire.SkipInTransport");
	this->DelayedFire_Animation.Read(exINI, pSection, "DelayedFire.Animation");
	this->DelayedFire_OpenToppedAnimation.Read(exINI, pSection, "DelayedFire.OpenToppedAnimation");
	this->DelayedFire_AnimIsAttached.Read(exINI, pSection, "DelayedFire.AnimIsAttached");
	this->DelayedFire_CenterAnimOnFirer.Read(exINI, pSection, "DelayedFire.CenterAnimOnFirer");
	this->DelayedFire_RemoveAnimOnNoDelay.Read(exINI, pSection, "DelayedFire.RemoveAnimOnNoDelay");
	this->DelayedFire_PauseFiringSequence.Read(exINI, pSection, "DelayedFire.PauseFiringSequence");
	this->DelayedFire_OnlyOnInitialBurst.Read(exINI, pSection, "DelayedFire.OnlyOnInitialBurst");
	this->DelayedFire_AnimOffset.Read(exINI, pSection, "DelayedFire.AnimOffset");


	this->SkipWeaponPicking = true;
	if (this->CanTarget != AffectedTarget::All ||
		this->CanTargetHouses != AffectedHouse::All
		|| this->Targeting_Health_Percent.isset()
		|| this->AttachEffect_RequiredTypes.size()
		|| this->AttachEffect_RequiredGroups.size()
		|| this->AttachEffect_DisallowedTypes.size()
		|| this->AttachEffect_DisallowedGroups.size()
		|| this->CanTarget_MaxHealth < 1.0 || this->CanTarget_MinHealth > 0.0
		)
	{
		this->SkipWeaponPicking = false;
	}
}

int WeaponTypeExtData::GetRangeWithModifiers(WeaponTypeClass* pThis, TechnoClass* pFirer, std::optional<int> fallback)
{
	int range = fallback.has_value() ? fallback.value()  : 0;

	if (!pThis && !pFirer)
		return range;
	else if (pFirer && pFirer->CanOccupyFire())
		range = (RulesClass::Instance->OccupyWeaponRange + pFirer->GetOccupyRangeBonus()) * Unsorted::LeptonsPerCell;
	else if (pThis && pFirer){
		auto pFirerExt = TechnoExtContainer::Instance.Find(pFirer);
		int range_ = pThis->Range;

		if(pFirerExt->AdditionalRange.isset())
			range_+= pFirerExt->AdditionalRange;

		range = range_;
	}
	else
		return range;

	if (range == -512)
		return range;

	auto pTechno = pFirer;

	if (pTechno->Transporter && pTechno->Transporter->IsAlive && pTechno->Transporter->GetTechnoType()->OpenTopped) {
		auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pTechno->Transporter->GetTechnoType());

		if (pTypeExt->OpenTopped_UseTransportRangeModifiers)
			pTechno = pTechno->Transporter;
	}

	{
		auto pExt = TechnoExtContainer::Instance.Find(pTechno);
		if (pExt->AE.ExtraRange.Enabled()){
			range = pExt->AE.ExtraRange.Get(range, pThis);
		}
	}
	return MaxImpl(range, 0);
}

#include <SpawnManagerClass.h>

int WeaponTypeExtData::GetTechnoKeepRange(WeaponTypeClass* pThis, TechnoClass* pFirer, bool isMinimum) {
	const auto pExt = WeaponTypeExtContainer::Instance.Find(pThis);
	const auto keepRange = pExt->KeepRange;

	if (!keepRange.Get() || !pFirer || pFirer->Transporter)
		return 0;

	const auto absType = pFirer->WhatAmI();

	if (absType != AbstractType::Infantry && absType != AbstractType::Unit)
		return 0;

	const auto pHouse = pFirer->Owner;

	if (pHouse && pHouse->IsControlledByHuman())
	{
		if (!pExt->KeepRange_AllowPlayer)
			return 0;
	}
	else if (!pExt->KeepRange_AllowAI)
	{
		return 0;
	}

	if (!pFirer->DiskLaserTimer.InProgress())
	{
		const auto spawnManager = pFirer->SpawnManager;

		if (!spawnManager || spawnManager->Status != SpawnManagerStatus::CoolDown)
			return 0;

		const auto spawnsNumber = pFirer->GetTechnoType()->SpawnsNumber;

		for (int i = 0; i < spawnsNumber; i++)
		{
			const auto status = spawnManager->SpawnedNodes[i]->Status;

			if (status == SpawnNodeStatus::TakeOff || status == SpawnNodeStatus::Returning)
				return 0;
		}
	}

	if (isMinimum)
		return (keepRange.Get() > 0) ? keepRange.Get() : 0;

	if (keepRange.Get() > 0)
		return 0;

	const auto checkRange = -keepRange.Get() - 128;
	const auto pTarget = pFirer->Target;

	if (pTarget && pFirer->DistanceFrom(pTarget) >= checkRange)
		return (checkRange > 443) ? checkRange : 443; // 1.73 * Unsorted::LeptonsPerCell

	return -keepRange.Get();
}

#include <New/PhobosAttachedAffect/Functions.h>

bool WeaponTypeExtData::HasRequiredAttachedEffects(TechnoClass* pTarget, TechnoClass* pFirer)
{
	bool hasRequiredTypes = !this->AttachEffect_RequiredTypes.empty();
	bool hasDisallowedTypes = !this->AttachEffect_DisallowedTypes.empty();
	bool hasRequiredGroups = !this->AttachEffect_RequiredGroups.empty();
	bool hasDisallowedGroups = !this->AttachEffect_DisallowedGroups.empty();

	if (hasRequiredTypes || hasDisallowedTypes || hasRequiredGroups || hasDisallowedGroups)
	{
		auto pTechno = pTarget;
		if (this->AttachEffect_CheckOnFirer && pFirer)
			pTechno = pFirer;

		if(!pTechno || !pTechno->IsAlive)
			return true;

		//auto const pTechnoExt = TechnoExtContainer::Instance.Find(pTechno);

		if (hasDisallowedTypes && PhobosAEFunctions::HasAttachedEffects(pTechno, this->AttachEffect_DisallowedTypes, false, this->AttachEffect_IgnoreFromSameSource, pFirer, this->AttachedToObject->Warhead, &this->AttachEffect_DisallowedMinCounts, &this->AttachEffect_DisallowedMaxCounts))
			return false;

		if (hasDisallowedGroups) {
			auto group = PhobosAttachEffectTypeClass::GetTypesFromGroups(this->AttachEffect_DisallowedGroups);
			if(PhobosAEFunctions::HasAttachedEffects(pTechno, group, false, this->AttachEffect_IgnoreFromSameSource, pFirer, this->AttachedToObject->Warhead, &this->AttachEffect_DisallowedMinCounts, &this->AttachEffect_DisallowedMaxCounts))
				return false;
		}

		if (hasRequiredTypes && !PhobosAEFunctions::HasAttachedEffects(pTechno, this->AttachEffect_RequiredTypes, true, this->AttachEffect_IgnoreFromSameSource, pFirer, this->AttachedToObject->Warhead, &this->AttachEffect_RequiredMinCounts, &this->AttachEffect_RequiredMaxCounts))
			return false;

		if (hasRequiredGroups){
			auto req_group = PhobosAttachEffectTypeClass::GetTypesFromGroups(this->AttachEffect_RequiredGroups);
			if (!PhobosAEFunctions::HasAttachedEffects(pTechno, req_group, true, this->AttachEffect_IgnoreFromSameSource, pFirer, this->AttachedToObject->Warhead, &this->AttachEffect_RequiredMinCounts, &this->AttachEffect_RequiredMaxCounts))
				return false;
		}
	}

	return true;
}

bool WeaponTypeExtData::IsHealthInThreshold(ObjectClass* pTarget) const {
	if(!(this->CanTarget_MinHealth > 0.0 || this->CanTarget_MaxHealth < 1.0))
		return true;

	return TechnoExtData::IsHealthInThreshold(pTarget, this->CanTarget_MinHealth, this->CanTarget_MaxHealth);
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
		.Process(this->Bolt_Disables[0])
		.Process(this->Bolt_Disables[1])
		.Process(this->Bolt_Disables[2])
		.Process(this->Bolt_Arcs)
		.Process(this->Bolt_Duration)
		.Process(this->Strafing)
		.Process(this->Strafing_Shots)
		.Process(this->Strafing_SimulateBurst)
		.Process(this->Strafing_UseAmmoPerShot)
		.Process(this->Strafing_EndDelay)
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
		.Process(this->Burst_FireWithinSequence)
		.Process(this->Burst_NoDelay)
		.Process(this->ROF_RandomDelay)
		.Process(this->ChargeTurret_Delays)
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
#ifdef _Enable
		.Process(this->WeaponBolt_Data)
#endif
		.Process(this->Bolt_Colors[0])
		.Process(this->Bolt_Colors[1])
		.Process(this->Bolt_Colors[2])
		.Process(this->Bolt_ParticleSys)
		.Process(this->Bolt_FollowFLH)
		.Process(this->Laser_Thickness)

		.Process(this->ExtraWarheads)
		.Process(this->ExtraWarheads_DamageOverrides)
		.Process(this->ExtraWarheads_DetonationChances)
		.Process(this->ExtraWarheads_FullDetonation)
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
		.Process(this->AttachEffect_CheckOnFirer)
		.Process(this->AttachEffect_IgnoreFromSameSource)

		.Process(this->FireOnce_ResetSequence)

		.Process(this->AttachEffects)
		.Process(this->AttachEffect_Enable)
		.Process(this->NoRepeatFire)

		.Process(this->SkipWeaponPicking)

		.Process(this->KeepRange)
		.Process(this->KeepRange_AllowAI)
		.Process(this->KeepRange_AllowPlayer)
		.Process(this->VisualScatter)
		.Process(this->TurretRecoil_Suppress)

		.Process(this->CanTarget_MaxHealth)
		.Process(this->CanTarget_MinHealth)

		.Process(this->DelayedFire_Duration)
		.Process(this->DelayedFire_SkipInTransport)
		.Process(this->DelayedFire_Animation)
		.Process(this->DelayedFire_OpenToppedAnimation)
		.Process(this->DelayedFire_AnimIsAttached)
		.Process(this->DelayedFire_CenterAnimOnFirer)
		.Process(this->DelayedFire_RemoveAnimOnNoDelay)
		.Process(this->DelayedFire_PauseFiringSequence)
		.Process(this->DelayedFire_OnlyOnInitialBurst)
		.Process(this->DelayedFire_AnimOffset)
		.Process(this->DelayedFire_AnimOnTurret)
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
	// if (pThis->Warhead->NukeMaker)
	// {
	// 	if (!pTarget)
	// 	{
	// 		Debug::LogInfo("WeaponTypeExtData::DetonateAt , cannot execute when invalid Target is present , need to be avail ! ");
	// 		return;
	// 	}
	// }

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
		Debug::LogInfo("WeaponTypeExtData::DetonateAt Coords empty ! ");
		return;
	}

	WeaponTypeExtData::DetonateAt(pThis, MapClass::Instance->GetCellAt(coords), pOwner, damage, AddDamage , HouseInveoker);
}

void WeaponTypeExtData::DetonateAt(WeaponTypeClass* pThis, const CoordStruct& coords, AbstractClass* pTarget, TechnoClass* pOwner, int damage, bool AddDamage, HouseClass* HouseInveoker)
{
	// if (pThis->Warhead->NukeMaker)
	// {
	// 	if (!pTarget)
	// 	{
	// 		Debug::LogInfo("WeaponTypeExtData::DetonateAt , cannot execute when invalid Target is present , need to be avail ! ");
	// 		return;
	// 	}
	// }

	auto pBulletTypeExt = BulletTypeExtContainer::Instance.Find(pThis->Projectile);
	auto pExt = WeaponTypeExtContainer::Instance.Find(pThis);

	if (BulletClass* pBullet = pBulletTypeExt->CreateBullet(pTarget, pOwner,
		damage, pThis->Warhead, pThis->Speed, pExt->GetProjectileRange(), pThis->Bright || pThis->Warhead->Bright, AddDamage))
	{
		pBullet->SetWeaponType(pThis);
		BulletExtData::DetonateAt(pBullet, pTarget, pOwner, coords , HouseInveoker);
	}
}

// =============================
// container
WeaponTypeExtContainer WeaponTypeExtContainer::Instance;

bool WeaponTypeExtContainer::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Process(WeaponTypeExtData::nOldCircumference)
		.Success();
}

bool WeaponTypeExtContainer::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Process(WeaponTypeExtData::nOldCircumference)
		.Success();
}

void WeaponTypeExtContainer::Clear()
{
}

// =============================
// container hooks
//

ASMJIT_PATCH(0x771EE0, WeaponTypeClass_CTOR, 0x6)
{
	GET(WeaponTypeClass*, pItem, ESI);
	WeaponTypeExtContainer::Instance.Allocate(pItem);
	return 0;
}

ASMJIT_PATCH(0x77311D, WeaponTypeClass_SDDTOR, 0x6)
{
	GET(WeaponTypeClass*, pItem, ESI);
	WeaponTypeExtContainer::Instance.Remove(pItem);
	return 0;
}

#include <Misc/Hooks.Otamaa.h>

HRESULT __stdcall FakeWeaponTypeClass::_Load(IStream* pStm)
{

	WeaponTypeExtContainer::Instance.PrepareStream(this, pStm);
	HRESULT res = this->WeaponTypeClass::Load(pStm);

	if (SUCCEEDED(res))
		WeaponTypeExtContainer::Instance.LoadStatic();

	return res;
}

HRESULT __stdcall FakeWeaponTypeClass::_Save(IStream* pStm, bool clearDirty)
{

	WeaponTypeExtContainer::Instance.PrepareStream(this, pStm);
	HRESULT res = this->WeaponTypeClass::Save(pStm, clearDirty);

	if (SUCCEEDED(res))
		WeaponTypeExtContainer::Instance.SaveStatic();

	return res;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F73CC, FakeWeaponTypeClass::_Load)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F73D0, FakeWeaponTypeClass::_Save)


ASMJIT_PATCH(0x7729B0, WeaponTypeClass_LoadFromINI, 0x5)
{
	GET(WeaponTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0xE4);

	WeaponTypeExtContainer::Instance.LoadFromINI(pItem, pINI, R->Origin() == 0x7729D6);

	return 0;
}ASMJIT_PATCH_AGAIN(0x7729C7, WeaponTypeClass_LoadFromINI, 0x5)
ASMJIT_PATCH_AGAIN(0x7729D6, WeaponTypeClass_LoadFromINI, 0x5)
