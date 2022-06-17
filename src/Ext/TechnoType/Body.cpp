#include "Body.h"

#include <TechnoTypeClass.h>
#include <StringTable.h>

#include <Ext/BuildingType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Techno/Body.h>

#include <Utilities/GeneralUtils.h>
#include <Utilities/Cast.h>

template<> const DWORD Extension<TechnoTypeClass>::Canary = 0x11111111;
TechnoTypeExt::ExtContainer TechnoTypeExt::ExtMap;

void TechnoTypeExt::ExtData::Initialize()
{
	Is_Cow = strcmp(OwnerObject()->ID, "COW") == 0;

	this->ShieldType = ShieldTypeClass::FindOrAllocate(NONE_STR);
	OreGathering_Anims.reserve(1);
	OreGathering_Tiberiums.reserve(1);
	OreGathering_FramesPerDir.reserve(1);
	LaserTrailData.reserve(4);
}

void TechnoTypeExt::ExtData::ApplyTurretOffset(Matrix3D* mtx, double factor)
{
	float x = static_cast<float>(this->TurretOffset.GetEx()->X * factor);
	float y = static_cast<float>(this->TurretOffset.GetEx()->Y * factor);
	float z = static_cast<float>(this->TurretOffset.GetEx()->Z * factor);

	mtx->Translate(x, y, z);
}

void TechnoTypeExt::ApplyTurretOffset(TechnoTypeClass* pType, Matrix3D* mtx, double factor)
{
	if (auto ext = TechnoTypeExt::ExtMap.Find(pType))
		ext->ApplyTurretOffset(mtx, factor);
}

// Ares 0.A source
const char* TechnoTypeExt::ExtData::GetSelectionGroupID() const
{
	return GeneralUtils::IsValidString(this->GroupAs) ? this->GroupAs : this->OwnerObject()->ID;
}

const char* TechnoTypeExt::GetSelectionGroupID(ObjectTypeClass* pType)
{
	if (auto pExt = TechnoTypeExt::ExtMap.Find(type_cast<TechnoTypeClass*>(pType)))
		return pExt->GetSelectionGroupID();

	return pType->ID;
}

bool TechnoTypeExt::HasSelectionGroupID(ObjectTypeClass* pType, const char* pID)
{
	auto id = TechnoTypeExt::GetSelectionGroupID(pType);

	return (IS_SAME_STR_(id, pID));
}

bool TechnoTypeExt::ExtData::IsCountedAsHarvester()
{
	auto pThis = this->OwnerObject();
	UnitTypeClass* pUnit = nullptr;

	if (pThis->WhatAmI() == AbstractType::UnitType)
		pUnit = abstract_cast<UnitTypeClass*>(pThis);

	if (this->Harvester_Counted.Get(pThis->Enslaves || pUnit && (pUnit->Harvester || pUnit->Enslaves)))
		return true;

	return false;
}

void TechnoTypeExt::GetBurstFLHs(TechnoTypeClass* pThis, INI_EX &exArtINI, const char* pArtSection,
	std::vector<DynamicVectorClass<CoordStruct>>& nFLH, std::vector<DynamicVectorClass<CoordStruct>>& nEFlh, const char* pPrefixTag)
{
	char tempBuffer[32];
	char tempBufferFLH[48];

	bool parseMultiWeapons = pThis->TurretCount > 0 && pThis->WeaponCount > 0;
	auto weaponCount = parseMultiWeapons ? pThis->WeaponCount : 2;
	nFLH.resize(weaponCount);
	nEFlh.resize(weaponCount);

	for (int i = 0; i < weaponCount; i++)
	{
		for (int j = 0; j < INT_MAX; j++)
		{
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "%sWeapon%d", pPrefixTag, i + 1);
			auto prefix = parseMultiWeapons ? tempBuffer : i > 0 ? "%sSecondaryFire" : "%sPrimaryFire";
			_snprintf_s(tempBuffer, sizeof(tempBuffer), prefix, pPrefixTag);

			_snprintf_s(tempBufferFLH, sizeof(tempBufferFLH), "%sFLH.Burst%d", tempBuffer, j);
			Nullable<CoordStruct> FLH;
			FLH.Read(exArtINI, pArtSection, tempBufferFLH);

			_snprintf_s(tempBufferFLH, sizeof(tempBufferFLH), "Elite%sFLH.Burst%d", tempBuffer, j);
			Nullable<CoordStruct> eliteFLH;
			eliteFLH.Read(exArtINI, pArtSection, tempBufferFLH);

			if (FLH.isset() && !eliteFLH.isset())
				eliteFLH = FLH;
			else if (!FLH.isset() && !eliteFLH.isset())
				break;

			nFLH[i].AddItem(FLH.Get());
			nEFlh[i].AddItem(eliteFLH.Get());
		}
	}
};

void TechnoTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI)
{
	auto pThis = this->OwnerObject();
	const auto pArtIni = &CCINIClass::INI_Art();
	const char* pSection = pThis->ID;
	const char* pArtSection = pThis->ImageFile;

	if (!pINI->GetSection(pSection))
		return;

	INI_EX exArtINI(pArtIni);
	INI_EX exINI(pINI);

	this->HealthBar_Hide.Read(exINI, pSection, "HealthBar.Hide");
	this->UIDescription.Read(exINI, pSection, "UIDescription");
	this->LowSelectionPriority.Read(exINI, pSection, "LowSelectionPriority");
	this->MindControlRangeLimit.Read(exINI, pSection, "MindControlRangeLimit");
	this->Interceptor.Read(exINI, pSection, "Interceptor");
	this->Interceptor_CanTargetHouses.Read(exINI, pSection, "Interceptor.CanTargetHouses");
	this->Interceptor_GuardRange.Read(exINI, pSection, "Interceptor.%sGuardRange");
	this->Interceptor_MinimumGuardRange.Read(exINI, pSection, "Interceptor.%sMinimumGuardRange");
	this->Interceptor_Weapon.Read(exINI, pSection, "Interceptor.Weapon");
	this->Interceptor_DeleteOnIntercept.Read(exINI, pSection, "Interceptor.DeleteOnIntercept");
	this->Interceptor_WeaponOverride.Read(exINI, pSection, "Interceptor.WeaponOverride");
	this->Interceptor_WeaponReplaceProjectile.Read(exINI, pSection, "Interceptor.WeaponReplaceProjectile");
	this->Interceptor_WeaponCumulativeDamage.Read(exINI, pSection, "Interceptor.WeaponCumulativeDamage");
	this->Interceptor_KeepIntact.Read(exINI, pSection, "Interceptor.KeepIntact");
	this->Powered_KillSpawns.Read(exINI, pSection, "Powered.KillSpawns");
	this->Spawn_LimitedRange.Read(exINI, pSection, "Spawner.LimitRange");
	this->Spawn_LimitedExtraRange.Read(exINI, pSection, "Spawner.ExtraLimitRange");
	this->Spawner_DelayFrames.Read(exINI, pSection, "Spawner.DelayFrames");
	this->Harvester_Counted.Read(exINI, pSection, "Harvester.Counted");
	this->Promote_IncludeSpawns.Read(exINI, pSection, "Promote.IncludeSpawns");
	this->ImmuneToCrit.Read(exINI, pSection, "ImmuneToCrit");
	this->MultiMindControl_ReleaseVictim.Read(exINI, pSection, "MultiMindControl.ReleaseVictim");
	this->NoManualMove.Read(exINI, pSection, "NoManualMove");
	this->InitialStrength.Read(exINI, pSection, "InitialStrength");
	this->Death_NoAmmo.Read(exINI, pSection, "Death.NoAmmo");
	this->Death_Countdown.Read(exINI, pSection, "Death.Countdown");
	this->Death_Peaceful.Read(exINI, pSection, "Death.Peaceful");
	this->Death_WithMaster.Read(exINI, pSection, "Death.WithSlaveOwner");
	this->ShieldType.Read(exINI, pSection, "ShieldType", true);
	this->CameoPriority.Read(exINI, pSection, "CameoPriority");

	this->WarpOut.Read(exINI, pSection, "WarpOut");
	this->WarpIn.Read(exINI, pSection, "WarpIn");
	this->WarpAway.Read(exINI, pSection, "WarpAway");
	this->ChronoTrigger.Read(exINI, pSection, "ChronoTrigger");
	this->ChronoDistanceFactor.Read(exINI, pSection, "ChronoDistanceFactor");
	this->ChronoMinimumDelay.Read(exINI, pSection, "ChronoMinimumDelay");
	this->ChronoRangeMinimum.Read(exINI, pSection, "ChronoRangeMinimum");
	this->ChronoDelay.Read(exINI, pSection, "ChronoDelay");

	this->WarpInWeapon.Read(exINI, pSection, "WarpInWeapon", true);
	this->WarpInMinRangeWeapon.Read(exINI, pSection, "WarpInMinRangeWeapon", true);
	this->WarpOutWeapon.Read(exINI, pSection, "WarpOutWeapon", true);
	this->WarpInWeapon_UseDistanceAsDamage.Read(exINI, pSection, "WarpInWeapon.UseDistanceAsDamage");

	this->OreGathering_Anims.Read(exINI, pSection, "OreGathering.Anims");
	this->OreGathering_Tiberiums.Read(exINI, pSection, "OreGathering.Tiberiums");
	this->OreGathering_FramesPerDir.Read(exINI, pSection, "OreGathering.FramesPerDir");

	this->DestroyAnim_Random.Read(exINI, pSection, "DestroyAnim.Random");
	this->NotHuman_RandomDeathSequence.Read(exINI, pSection, "NotHuman.RandomDeathSequence");

	this->PassengerDeletion_Soylent.Read(exINI, pSection, "PassengerDeletion.Soylent");
	this->PassengerDeletion_SoylentMultiplier.Read(exINI, pSection, "PassengerDeletion.SoylentMultiplier");
	this->PassengerDeletion_SoylentFriendlies.Read(exINI, pSection, "PassengerDeletion.SoylentFriendlies");
	this->PassengerDeletion_ReportSound.Read(exINI, pSection, "PassengerDeletion.ReportSound");
	this->PassengerDeletion_Rate_SizeMultiply.Read(exINI, pSection, "PassengerDeletion.Rate.SizeMultiply");
	this->PassengerDeletion_Rate.Read(exINI, pSection, "PassengerDeletion.Rate");
	this->PassengerDeletion_UseCostAsRate.Read(exINI, pSection, "PassengerDeletion.UseCostAsRate");
	this->PassengerDeletion_CostMultiplier.Read(exINI, pSection, "PassengerDeletion.CostMultiplier");
	this->PassengerDeletion_Anim.Read(exINI, pSection, "PassengerDeletion.Anim");
	this->PassengerDeletion_DisplaySoylent.Read(exINI, pSection, "PassengerDeletion.DisplaySoylent");
	this->PassengerDeletion_DisplaySoylentToHouses.Read(exINI, pSection, "PassengerDeletion.DisplaySoylentToHouses");
	this->PassengerDeletion_DisplaySoylentOffset.Read(exINI, pSection, "PassengerDeletion.DisplaySoylentOffset");

	this->DefaultDisguise.Read(exINI, pSection, "DefaultDisguise");

	this->OpenTopped_RangeBonus.Read(exINI, pSection, "OpenTopped.RangeBonus");
	this->OpenTopped_DamageMultiplier.Read(exINI, pSection, "OpenTopped.DamageMultiplier");
	this->OpenTopped_WarpDistance.Read(exINI, pSection, "OpenTopped.WarpDistance");
	this->OpenTopped_IgnoreRangefinding.Read(exINI, pSection, "OpenTopped.IgnoreRangefinding");

	this->AutoFire.Read(exINI, pSection, "AutoFire");
	this->AutoFire_TargetSelf.Read(exINI, pSection, "AutoFire.TargetSelf");

	this->NoSecondaryWeaponFallback.Read(exINI, pSection, "NoSecondaryWeaponFallback");

	this->JumpjetAllowLayerDeviation.Read(exINI, pSection, "JumpjetAllowLayerDeviation");
	this->JumpjetTurnToTarget.Read(exINI, pSection, "JumpjetTurnToTarget");
	this->DeployingAnim_AllowAnyDirection.Read(exINI, pSection, "DeployingAnim.AllowAnyDirection");
	this->DeployingAnim_KeepUnitVisible.Read(exINI, pSection, "DeployingAnim.KeepUnitVisible");
	this->DeployingAnim_ReverseForUndeploy.Read(exINI, pSection, "DeployingAnim.ReverseForUndeploy");
	this->DeployingAnim_UseUnitDrawer.Read(exINI, pSection, "DeployingAnim.UseUnitDrawer");

	this->SelfHealGainType.Read(exINI, pSection, "SelfHealGainType");

	// Ares 0.2
	this->RadarJamRadius.Read(exINI, pSection, "RadarJamRadius");

	// Ares 0.9
	this->InhibitorRange.Read(exINI, pSection, "InhibitorRange");

	// Ares 0.A
	this->GroupAs.Read(pINI, pSection, "GroupAs");

	// Ares 0.C
	this->NoAmmoWeapon.Read(exINI, pSection, "NoAmmoWeapon");
	this->NoAmmoAmount.Read(exINI, pSection, "NoAmmoAmount");

	this->EnemyUIName.Read(exINI, pSection, "EnemyUIName");

	this->ForceWeapon_Naval_Decloaked.Read(exINI, pSection, "ForceWeapon.Naval.Decloaked");
	this->Ammo_Shared.Read(exINI, pSection, "Ammo.Shared");
	this->Ammo_Shared_Group.Read(exINI, pSection, "Ammo.Shared.Group");
	this->Passengers_SyncOwner.Read(exINI, pSection, "Passengers.SyncOwner");
	this->Passengers_SyncOwner_RevertOnExit.Read(exINI, pSection, "Passengers.SyncOwner.RevertOnExit");

	this->UseDisguiseMovementSpeed.Read(exINI, pSection, "UseDisguiseMovementSpeed");
	this->Insignia.Read(exINI, pSection, "Insignia.%s");
	this->InsigniaFrame.Read(exINI, pSection, "InsigniaFrame.%s");
	this->Insignia_ShowEnemy.Read(exINI, pSection, "Insignia.ShowEnemy");
	this->InsigniaFrames.Read(exINI, pSection, "InsigniaFrames");
	this->InitialStrength_Cloning.Read(exINI, pSection, "InitialStrength.Cloning");

	this->SHP_SelectBrdSHP.Read(exINI, pSection, "SelectBrd.SHP");
	this->SHP_SelectBrdPAL.Read(pINI, pSection, "SelectBrd.PAL");
	this->UseCustomSelectBrd.Read(exINI, pSection, "UseCustomSelectBrd");
	this->SelectBrd_Frame.Read(exINI, pSection, "SelectBrd.Frame");
	this->SelectBrd_DrawOffset.Read(exINI, pSection, "SelectBrd.DrawOffset");
	this->SelectBrd_TranslucentLevel.Read(exINI, pSection, "SelectBrd.TranslucentLevel");
	this->SelectBrd_ShowEnemy.Read(exINI, pSection, "SelectBrd.ShowEnemy");

#pragma region Otamaa
	this->DontShake.Read(exINI, pSection, "DontShakeScreen");
	this->DiskLaserChargeUp.Read(exINI, pSection, "DiskLaserChargeUp");
	this->DrainAnimationType.Read(exINI, pSection, "DrainAnimationType");

	this->TalkBubbleTime.Read(exINI, pSection, "TalkBubbleTime");

	if (auto pAircraftType = specific_cast<AircraftTypeClass*>(pThis))
	{
		this->SpyplaneCameraSound.Read(exINI, pSection, "SpyplaneCameraSound");
		this->ParadropRadius.Read(exINI, pSection, "Paradrop.ApproachRadius");
		this->ParadropOverflRadius.Read(exINI, pSection, "Paradrop.OverflyRadius");
		this->Paradrop_DropPassangers.Read(exINI, pSection, "Paradrop.DropPassangers");
		this->Paradrop_MaxAttempt.Read(exINI, pSection, "Paradrop.MaxApproachAttempt");

		this->IsCustomMissile.Read(exINI, pSection, "Missile.Custom");
		this->CustomMissileData.Read(exINI, pSection, "Missile");
		this->CustomMissileData.GetEx()->Type = pAircraftType;
		this->CustomMissileRaise.Read(exINI, pSection, "Missile.%sRaiseBeforeLaunching");
		this->AttackingAircraftSightRange.Read(exINI, pSection, "AttackingAircraftSightRange");
		this->CrashWeapon_s.Read(exINI, pSection, "Crash.Weapon",true);
		this->CrashWeapon.Read(exINI, pSection, "Crash.%sWeapon");
		this->NoAirportBound_DisableRadioContact.Read(exINI, pSection, "NoAirportBound.DisableRadioContact");

		this->TakeOff_Anim.Read(exINI, pSection, "TakeOff.Anim");
#ifdef COMPILE_PORTED_DP_FEATURES
		this->MissileHoming.Read(exINI, pSection, "Missile.Homing");
		this->MyDiveData.Read(exINI, pSection);
		this->MyPutData.Read(exINI, pSection);
		this->MyFighterData.Read(exINI, pSection, pThis);

#endif
	}
	//pipshape
	this->HealthBarSHP.Read(exINI, pSection, "HealthBarSHP");

	//pipbar
	this->HealthBarSHP_Selected.Read(exINI, pSection, "HealthBarSHP.Selected");
	this->HealthBarSHPBracketOffset.Read(exINI, pSection, "HealthBarSHP.BracketOffset");
	this->HealthBarSHP_HealthFrame.Read(exINI, pSection, "HealthBarSHP.HealthFrame");
	this->HealthBarSHP_Palette.Read(pINI, pSection, "HealthBarSHP.Palette");
	this->HealthBarSHP_PointOffset.Read(exINI, pSection, "HealthBarSHP.Point2DOffset");
	this->HealthbarRemap.Read(exINI, pSection, "HealthBarSHP.Remap");

	this->PipShapes02.Read(exINI, pSection, "PipShapes.Foot");
	this->PipGarrison.Read(exINI, pSection, "PipShapes.Garrison");
	this->PipGarrison_FrameIndex.Read(exINI, pSection, "PipShapes.GarrisonFrameIndex");
	this->PipGarrison_Palette.Read(pINI, pSection, "PipShapes.GarrisonPalette");
	this->PipShapes01.Read(exINI, pSection, "PipShapes.Building");

	this->Is_Cow.Read(exINI, pSection, "IsCow");

	this->HealthNumber_SHP.Read(exINI, pSection, "HealthNumber.Shape");
	this->HealthNumber_Show.Read(exINI, pSection, "HealthNumber.Show");
	this->HealthNumber_Percent.Read(exINI, pSection, "HealthNumber.Percent");
	this->Healnumber_Offset.Read(exINI, pSection, "HealthNumber.Offset");
	this->Healnumber_Decrement.Read(exINI, pSection, "HealthNumber.Decrement");

	this->ParasiteExit_Sound.Read(exINI, pSection, "Parasite.ExitSound");

	this->Overload_Count.Read(exINI, pSection, "Overload.Count");
	this->Overload_Damage.Read(exINI, pSection, "Overload.Damage");
	this->Overload_Frames.Read(exINI, pSection, "Overload.Frames");
	this->Overload_DeathSound.Read(exINI, pSection, "Overload.DeathSound");
	this->Overload_ParticleSys.Read(exINI, pSection, "Overload.ParticleSys");
	this->Overload_ParticleSysCount.Read(exINI, pSection, "Overload.ParticleSysCount");

	this->Landing_Anim.Read(exINI, pSection, "Landing.Anim");
	this->Landing_AnimOnWater.Read(exINI, pSection, "Landing.AnimOnWater");

	this->FacingRotation_Disable.Read(exINI, pSection, "FacingRotation.Disabled");
	this->FacingRotation_DisalbeOnEMP.Read(exINI, pSection, "FacingRotation.DisabledOnEMP");
	this->FacingRotation_DisalbeOnDeactivated.Read(exINI, pSection, "FacingRotation.DisabledOnDeactivated");

	this->Draw_MindControlLink.Read(exINI, pSection, "MindControll.DrawLink");

	this->DeathWeapon.Read(exINI, pSection, "%s.DeathWeapon");
	this->Disable_C4WarheadExp.Read(exINI, pSection, "Crash.DisableC4WarheadExplosion");
	this->GClock_Shape.Read(exINI, pSection, "GClock.Shape");
	this->GClock_Transculency.Read(exINI, pSection, "GClock.Transculency");
	this->GClock_Palette.Read(pINI, pSection, "GClock.Palette");

	this->ROF_Random.Read(exINI, pSection, "ROF.AddRandom");
	this->Rof_RandomMinMax.Read(exINI, pSection, "ROF.RandomMinMax");

	this->CrashSpinLevelRate.Read(exINI, pSection, "CrashSpin.LevelRate");
	this->CrashSpinVerticalRate.Read(exINI, pSection, "CrashSpin.VerticalRate");

	this->CreateSound_Enable.Read(exINI, pSection, "CreateSound.Enable");

	this->Eva_Complete.Read(exINI.GetINI(), pSection, "EVA.Complete");
	this->VoiceCreate.Read(exINI, pSection, "VoiceCreate");

	this->SlaveFreeSound_Enable.Read(exINI, pSection, "SlaveFreeSound.Enable");
	this->SlaveFreeSound.Read(exINI, pSection, "SlaveFreeSound");
	this->SinkAnim.Read(exINI, pSection, "Sink.Anim");
	this->Tunnel_Speed.Read(exINI, pSection, "TunnelSpeed");
	this->HoverType.Read(exINI, pSection, "HoverType");
#ifdef COMPILE_PORTED_DP_FEATURES
	this->VirtualUnit.Read(exINI, pSection, "VirtualUnit");
	this->MyExtraFireData.ReadRules(exINI, pSection);
	this->MyGiftBoxData.Read(exINI, pSection);
	this->MyJJData.Read(exINI, pSection);
	this->MyPassangersData.Read(exINI, pSection);
	this->MySpawnSupportDatas.Read(exINI, pSection);
#endif

#pragma endregion

	// Art tags
	if (!pArtIni->GetSection(pArtSection))
		return;

	this->TurretOffset.Read(exArtINI, pArtSection, "TurretOffset");

	char tempBuffer[32];
	for (size_t i = 0; ; ++i)
	{
		NullableIdx<LaserTrailTypeClass> trail;
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "LaserTrail%d.Type", i);
		trail.Read(exArtINI, pArtSection, tempBuffer);

		if (!trail.isset())
			break;

		Valueable<CoordStruct> flh;
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "LaserTrail%d.FLH", i);
		flh.Read(exArtINI, pArtSection, tempBuffer);

		Valueable<bool> isOnTurret;
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "LaserTrail%d.IsOnTurret", i);
		isOnTurret.Read(exArtINI, pArtSection, tempBuffer);

		auto nTrail = trail.Get();
		auto nFLH = flh.Get();
		auto bOnTur = isOnTurret.Get();

		this->LaserTrailData.emplace_back(nTrail, nFLH, bOnTur);
	}

	TechnoTypeExt::GetBurstFLHs(pThis, exArtINI, pArtSection, WeaponBurstFLHs, EliteWeaponBurstFLHs, "");
	TechnoTypeExt::GetBurstFLHs(pThis, exArtINI, pArtSection, DeployedWeaponBurstFLHs, EliteDeployedWeaponBurstFLHs, "Deployed");
	TechnoTypeExt::GetBurstFLHs(pThis, exArtINI, pArtSection, CrouchedWeaponBurstFLHs, EliteCrouchedWeaponBurstFLHs, "Prone");

	this->PronePrimaryFireFLH.Read(exArtINI, pArtSection, "PronePrimaryFireFLH");
	this->ProneSecondaryFireFLH.Read(exArtINI, pArtSection, "ProneSecondaryFireFLH");

	this->E_PronePrimaryFireFLH.Read(exArtINI, pArtSection, "ElitePronePrimaryFireFLH");
	this->E_ProneSecondaryFireFLH.Read(exArtINI, pArtSection, "EliteProneSecondaryFireFLH");

	this->DeployedPrimaryFireFLH.Read(exArtINI, pArtSection, "DeployedPrimaryFireFLH");
	this->DeployedSecondaryFireFLH.Read(exArtINI, pArtSection, "DeployedSecondaryFireFLH");

	this->E_DeployedPrimaryFireFLH.Read(exArtINI, pArtSection, "EliteDeployedPrimaryFireFLH");
	this->E_DeployedSecondaryFireFLH.Read(exArtINI, pArtSection, "EliteDeployedSecondaryFireFLH");

#pragma region Otamaa
	char HitCoord_tempBuffer[32];
	for (size_t i = 0; ; ++i)
	{
		Nullable<CoordStruct> nHitBuff;
		_snprintf_s(HitCoord_tempBuffer, sizeof(HitCoord_tempBuffer), "HitCoordOffset%d", i);
		nHitBuff.Read(exArtINI, pArtSection, HitCoord_tempBuffer);

		if (!nHitBuff.isset() || !nHitBuff.Get())
			break;

		this->HitCoordOffset.push_back(nHitBuff.Get());
	}

	this->HitCoordOffset_Random.Read(exArtINI, pArtSection, "HitCoordOffset.Random");

#ifdef COMPILE_PORTED_DP_FEATURES
	PrimaryCrawlFLH.Read(exArtINI, pArtSection, "PrimaryCrawlingFLH");
	Elite_PrimaryCrawlFLH.Read(exArtINI, pArtSection, "ElitePrimaryCrawlingFLH");

	SecondaryCrawlFLH.Read(exArtINI, pArtSection, "SecondaryCrawlingFLH");
	Elite_SecondaryCrawlFLH.Read(exArtINI, pArtSection, "EliteSecondaryCrawlingFLH");

	this->MyExtraFireData.ReadArt(exArtINI, pArtSection);
	this->MySpawnSupportFLH.Read(exArtINI, pArtSection);
	this->Trails.Read(exArtINI, pArtSection, true);
#endif

#pragma endregion
}

template <typename T>
void TechnoTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->HealthBar_Hide)
		.Process(this->UIDescription)
		.Process(this->LowSelectionPriority)
		.Process(this->MindControlRangeLimit)
		.Process(this->Interceptor)
		.Process(this->Interceptor_CanTargetHouses)
		.Process(this->Interceptor_GuardRange)
		.Process(this->Interceptor_MinimumGuardRange)
		.Process(this->Interceptor_Weapon)
		.Process(this->Interceptor_DeleteOnIntercept)
		.Process(this->Interceptor_WeaponOverride)
		.Process(this->Interceptor_WeaponReplaceProjectile)
		.Process(this->Interceptor_WeaponCumulativeDamage)
		.Process(this->Interceptor_KeepIntact)
		.Process(this->GroupAs)
		.Process(this->RadarJamRadius)
		.Process(this->InhibitorRange)
		.Process(this->TurretOffset)
		.Process(this->Powered_KillSpawns)
		.Process(this->Spawn_LimitedRange)
		.Process(this->Spawn_LimitedExtraRange)
		.Process(this->Spawner_DelayFrames)
		.Process(this->Harvester_Counted)
		.Process(this->Promote_IncludeSpawns)
		.Process(this->ImmuneToCrit)
		.Process(this->MultiMindControl_ReleaseVictim)
		.Process(this->CameoPriority)
		.Process(this->NoManualMove)
		.Process(this->InitialStrength)
		.Process(this->Death_NoAmmo)
		.Process(this->Death_Countdown)
		.Process(this->Death_Peaceful)
		.Process(this->Death_WithMaster)
		.Process(this->ShieldType)
		.Process(this->WarpOut)
		.Process(this->WarpIn)
		.Process(this->WarpAway)
		.Process(this->ChronoTrigger)
		.Process(this->ChronoDistanceFactor)
		.Process(this->ChronoMinimumDelay)
		.Process(this->ChronoRangeMinimum)
		.Process(this->ChronoDelay)
		.Process(this->WarpInWeapon)
		.Process(this->WarpInMinRangeWeapon)
		.Process(this->WarpOutWeapon)
		.Process(this->WarpInWeapon_UseDistanceAsDamage)
		.Process(this->OreGathering_Anims)
		.Process(this->OreGathering_Tiberiums)
		.Process(this->OreGathering_FramesPerDir)
		.Process(this->LaserTrailData)
		.Process(this->DestroyAnim_Random)
		.Process(this->NotHuman_RandomDeathSequence)
		.Process(this->DefaultDisguise)
		.Process(this->WeaponBurstFLHs)
		.Process(this->EliteWeaponBurstFLHs)
		.Process(this->PassengerDeletion_Soylent)
		.Process(this->PassengerDeletion_SoylentMultiplier)
		.Process(this->PassengerDeletion_SoylentFriendlies)
		.Process(this->PassengerDeletion_Rate)
		.Process(this->PassengerDeletion_ReportSound)
		.Process(this->PassengerDeletion_Rate_SizeMultiply)
		.Process(this->PassengerDeletion_UseCostAsRate)
		.Process(this->PassengerDeletion_CostMultiplier)
		.Process(this->PassengerDeletion_Anim)
		.Process(this->PassengerDeletion_DisplaySoylent)
		.Process(this->PassengerDeletion_DisplaySoylentToHouses)
		.Process(this->PassengerDeletion_DisplaySoylentOffset)
		.Process(this->OpenTopped_RangeBonus)
		.Process(this->OpenTopped_DamageMultiplier)
		.Process(this->OpenTopped_WarpDistance)
		.Process(this->OpenTopped_IgnoreRangefinding)
		.Process(this->AutoFire)
		.Process(this->AutoFire_TargetSelf)
		.Process(this->NoSecondaryWeaponFallback)
		.Process(this->NoAmmoWeapon)
		.Process(this->NoAmmoAmount)
		.Process(this->JumpjetAllowLayerDeviation)
		.Process(this->JumpjetTurnToTarget)
		.Process(this->DeployingAnim_AllowAnyDirection)
		.Process(this->DeployingAnim_KeepUnitVisible)
		.Process(this->DeployingAnim_ReverseForUndeploy)
		.Process(this->DeployingAnim_UseUnitDrawer)
		.Process(this->SelfHealGainType)
		.Process(this->EnemyUIName)
		.Process(this->ForceWeapon_Naval_Decloaked)
		.Process(this->Ammo_Shared)
		.Process(this->Ammo_Shared_Group)
		.Process(this->Passengers_SyncOwner)
		.Process(this->Passengers_SyncOwner_RevertOnExit)
		.Process(this->UseDisguiseMovementSpeed)
		.Process(this->Insignia)
		.Process(this->InsigniaFrame)
		.Process(this->Insignia_ShowEnemy)
		.Process(this->InsigniaFrames)
		.Process(this->InitialStrength_Cloning)
		.Process(this->UseCustomSelectBrd)

		.Process(this->SHP_SelectBrdSHP)
		.Process(this->SHP_SelectBrdPAL)
		.Process(this->UseCustomSelectBrd)
		.Process(this->SelectBrd_Frame)
		.Process(this->SelectBrd_DrawOffset)
		.Process(this->SelectBrd_TranslucentLevel)
		.Process(this->SelectBrd_ShowEnemy)

		.Process(this->PronePrimaryFireFLH)
		.Process(this->ProneSecondaryFireFLH)

		.Process(this->E_PronePrimaryFireFLH)
		.Process(this->ProneSecondaryFireFLH)

		.Process(this->E_DeployedPrimaryFireFLH)
		.Process(this->E_DeployedSecondaryFireFLH)


		.Process(this->E_DeployedPrimaryFireFLH)
		.Process(this->E_DeployedSecondaryFireFLH)


		.Process(this->CrouchedWeaponBurstFLHs)
		.Process(this->EliteCrouchedWeaponBurstFLHs)
		.Process(this->DeployedWeaponBurstFLHs)
		.Process(this->EliteDeployedWeaponBurstFLHs)

#pragma region Otamaa
		.Process(this->FacingRotation_Disable)
		.Process(this->FacingRotation_DisalbeOnEMP)
		.Process(this->FacingRotation_DisalbeOnDeactivated)

		.Process(this->Is_Cow)

		.Process(this->DontShake)

		.Process(this->DiskLaserChargeUp)
		.Process(this->DrainAnimationType)

		.Process(this->TalkBubbleTime)

		.Process(this->AttackingAircraftSightRange)

		.Process(this->SpyplaneCameraSound)

		.Process(this->ParadropRadius)
		.Process(this->ParadropOverflRadius)
		.Process(this->Paradrop_DropPassangers)
		.Process(this->Paradrop_MaxAttempt)

		.Process(this->IsCustomMissile)
		.Process(this->CustomMissileRaise)
		.Process(this->CustomMissileData)

		.Process(this->Draw_MindControlLink)

		.Process(this->Overload_Count)
		.Process(this->Overload_Damage)
		.Process(this->Overload_Frames)
		.Process(this->Overload_DeathSound)
		.Process(this->Overload_ParticleSys)
		.Process(this->Overload_ParticleSysCount)

		.Process(this->Landing_Anim)
		.Process(this->Landing_AnimOnWater)
		.Process(this->TakeOff_Anim)

		.Process(this->HitCoordOffset)
		.Process(this->HitCoordOffset_Random)

		.Process(this->DeathWeapon)
		.Process(this->CrashWeapon)
		.Process(this->CrashWeapon_s)
		.Process(this->Disable_C4WarheadExp)



		.Process(this->CrashSpinLevelRate)
		.Process(this->CrashSpinVerticalRate)

		.Process(this->ParasiteExit_Sound)

		.Process(this->PipShapes01)
		.Process(this->PipShapes02)
		.Process(this->PipGarrison)
		.Process(this->PipGarrison_FrameIndex)
		.Process(this->PipGarrison_Palette)

		.Process(this->HealthNumber_Show)
		.Process(this->HealthNumber_Percent)
		.Process(this->Healnumber_Offset)
		.Process(this->HealthNumber_SHP)
		.Process(this->Healnumber_Decrement)

		.Process(this->HealthBarSHP)
		.Process(this->HealthBarSHP_Selected)
		.Process(this->HealthBarSHPBracketOffset)
		.Process(this->HealthBarSHP_HealthFrame)
		.Process(this->HealthBarSHP_Palette)
		.Process(this->HealthBarSHP_PointOffset)
		.Process(this->HealthbarRemap)

		.Process(this->GClock_Shape)
		.Process(this->GClock_Transculency)
		.Process(this->GClock_Palette)

		.Process(this->ROF_Random)
		.Process(this->Rof_RandomMinMax)

		.Process(this->Eva_Complete)
		.Process(this->VoiceCreate)
		.Process(this->CreateSound_Enable)

		.Process(this->SlaveFreeSound_Enable)
		.Process(this->SlaveFreeSound)
		.Process(this->NoAirportBound_DisableRadioContact)
		.Process(this->SinkAnim)
		.Process(this->Tunnel_Speed)
		.Process(this->HoverType)
#ifdef COMPILE_PORTED_DP_FEATURES
		.Process(this->VirtualUnit)

		.Process(this->PrimaryCrawlFLH)
		.Process(this->Elite_PrimaryCrawlFLH)
		.Process(this->SecondaryCrawlFLH)
		.Process(this->Elite_SecondaryCrawlFLH)
		.Process(this->MissileHoming)
#endif


#pragma endregion
		;
#ifdef COMPILE_PORTED_DP_FEATURES
		this->MyExtraFireData.Serialize(Stm);
		this->MyDiveData.Serialize(Stm);
		this->MyPutData.Serialize(Stm);
		this->MyGiftBoxData.Serialize(Stm);
		this->MyJJData.Serialize(Stm);
		this->MyPassangersData.Serialize(Stm);
		this->MySpawnSupportFLH.Serialize(Stm);
		this->MySpawnSupportDatas.Serialize(Stm);
		this->Trails.Serialize(Stm);
		this->MyFighterData.Serialize(Stm);
#endif
}
void TechnoTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<TechnoTypeClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void TechnoTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<TechnoTypeClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool TechnoTypeExt::ExtData::LaserTrailDataEntry::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return this->Serialize(stm);
}

bool TechnoTypeExt::ExtData::LaserTrailDataEntry::Save(PhobosStreamWriter& stm) const
{
	return const_cast<LaserTrailDataEntry*>(this)->Serialize(stm);
}

template <typename T>
bool TechnoTypeExt::ExtData::LaserTrailDataEntry::Serialize(T& stm)
{
	return stm
		.Process(idxType)
		.Process(FLH)
		.Process(IsOnTurret)
		.Success();
}

void TechnoTypeExt::ExtContainer::InvalidatePointer(void* ptr, bool bRemoved) { }

bool TechnoTypeExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool TechnoTypeExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

TechnoTypeExt::ExtContainer::ExtContainer() : Container("TechnoTypeClass") { }
TechnoTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x711835, TechnoTypeClass_CTOR, 0x5)
{
	GET(TechnoTypeClass*, pItem, ESI);

	TechnoTypeExt::ExtMap.FindOrAllocate(pItem);

	return 0;
}

DEFINE_HOOK(0x711AE0, TechnoTypeClass_DTOR, 0x5)
{
	GET(TechnoTypeClass*, pItem, ECX);

	TechnoTypeExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x716DC0, TechnoTypeClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x7162F0, TechnoTypeClass_SaveLoad_Prefix, 0x6)
{
	GET_STACK(TechnoTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	TechnoTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x716DAC, TechnoTypeClass_Load_Suffix, 0xA)
{
	TechnoTypeExt::ExtMap.LoadStatic();

	return 0;
}

DEFINE_HOOK(0x717094, TechnoTypeClass_Save_Suffix, 0x5)
{
	TechnoTypeExt::ExtMap.SaveStatic();

	return 0;
}

DEFINE_HOOK_AGAIN(0x716132, TechnoTypeClass_LoadFromINI, 0x5)
DEFINE_HOOK(0x716123, TechnoTypeClass_LoadFromINI, 0x5)
{
	GET(TechnoTypeClass*, pItem, EBP);
	GET_STACK(CCINIClass*, pINI, 0x380);

	TechnoTypeExt::ExtMap.LoadFromINI(pItem, pINI);

	return 0;
}
