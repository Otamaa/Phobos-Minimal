#include "Body.h"

#include <TechnoTypeClass.h>
#include <StringTable.h>

#include <Ext/BuildingType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/House/Body.h>

#include <Utilities/GeneralUtils.h>
#include <Utilities/Cast.h>
#include <Utilities/EnumFunctions.h>

TechnoTypeExt::ExtContainer TechnoTypeExt::ExtMap;
double TechnoTypeExt::TurretMultiOffsetDefaultMult = 1.0;
double TechnoTypeExt::TurretMultiOffsetOneByEightMult = 0.125;

void TechnoTypeExt::ExtData::Initialize()
{
	Is_Cow = strcmp(Get()->ID, "COW") == 0;

	this->ShieldType = ShieldTypeClass::FindOrAllocate(DEFAULT_STR2);

	if (Is_AircraftType(Get()))
	{
		this->CustomMissileTrailerAnim = AnimTypeClass::Find(GameStrings::V3TRAIL());
		this->CustomMissileTakeoffAnim = AnimTypeClass::Find(GameStrings::V3TAKEOFF());
	}

	this->Promote_Elite_Eva = VoxClass::FindIndexById(GameStrings::EVA_UnitPromoted());
	this->Promote_Vet_Eva = VoxClass::FindIndexById(GameStrings::EVA_UnitPromoted());

	OreGathering_Anims.reserve(1);
	OreGathering_Tiberiums.reserve(1);
	OreGathering_FramesPerDir.reserve(1);
	LaserTrailData.reserve(4);
}

AnimTypeClass* TechnoTypeExt::GetSinkAnim(TechnoClass* pThis)
{
	return TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())->SinkAnim.Get(RulesClass::Instance->Wake);
}

double TechnoTypeExt::GetTunnelSpeed(TechnoClass* pThis, RulesClass* pRules)
{
	return TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())->Tunnel_Speed.Get(pRules->TunnelSpeed);
}

//void TechnoTypeExt::ExtData::ApplyTurretOffset(Matrix3D* mtx, double factor)
//{
//	auto const pOffs = this->TurretOffset.GetEx();
//	float x = static_cast<float>(pOffs->X * factor);
//	float y = static_cast<float>(pOffs->Y * factor);
//	float z = static_cast<float>(pOffs->Z * factor);
//
//	mtx->Translate(x, y, z);
//}
//
//void TechnoTypeExt::ApplyTurretOffset(TechnoTypeClass* pType, Matrix3D* mtx, double factor)
//{
//	if (const auto ext = TechnoTypeExt::ExtMap.Find(pType)) {
//
//		auto const pOffs = ext->TurretOffset.GetEx();
//		float x = static_cast<float>(pOffs->X * factor);
//		float y = static_cast<float>(pOffs->Y * factor);
//		float z = static_cast<float>(pOffs->Z * factor);
//
//		mtx->Translate(x, y, z);
//	}
//}

// Ares 0.A source
const char* TechnoTypeExt::ExtData::GetSelectionGroupID() const
{
	return GeneralUtils::IsValidString(this->GroupAs) ? this->GroupAs : this->Get()->ID;
}

const char* TechnoTypeExt::GetSelectionGroupID(ObjectTypeClass* pType)
{
	if (auto pTType = type_cast<TechnoTypeClass*>(pType))
		return TechnoTypeExt::ExtMap.Find(pTType)->GetSelectionGroupID();

	return pType->ID;
}

bool TechnoTypeExt::HasSelectionGroupID(ObjectTypeClass* pType, const std::string& pID)
{
	const auto id = TechnoTypeExt::GetSelectionGroupID(pType);

	return (IS_SAME_STR_(id, pID.c_str()));
}

bool TechnoTypeExt::ExtData::IsCountedAsHarvester() const
{
	const auto pThis = this->Get();

	if (!pThis)
		return false;

	UnitTypeClass* pUnit = nullptr;

	if (Is_UnitType(pThis))
		pUnit = static_cast<UnitTypeClass*>(pThis);

	if (this->Harvester_Counted.Get(pThis->Enslaves || (pUnit && (pUnit->Harvester || pUnit->Enslaves))))
		return true;

	return false;
}

void TechnoTypeExt::GetBurstFLHs(TechnoTypeClass* pThis, INI_EX& exArtINI, const char* pArtSection,
	std::vector<std::vector<CoordStruct>>& nFLH, std::vector<std::vector<CoordStruct>>& nEFlh, const char* pPrefixTag)
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
			Nullable<CoordStruct> FLH { };
			FLH.Read(exArtINI, pArtSection, tempBufferFLH);

			_snprintf_s(tempBufferFLH, sizeof(tempBufferFLH), "Elite%sFLH.Burst%d", tempBuffer, j);
			Nullable<CoordStruct> eliteFLH { };
			eliteFLH.Read(exArtINI, pArtSection, tempBufferFLH);

			if (FLH.isset() && !eliteFLH.isset())
				eliteFLH = FLH;
			else if (!FLH.isset() && !eliteFLH.isset())
				break;

			nFLH[i].push_back(FLH.Get());
			nEFlh[i].push_back(eliteFLH.Get());
		}
	}
};

void TechnoTypeExt::GetFLH(INI_EX& exArtINI, const char* pArtSection, Nullable<CoordStruct>& nFlh, Nullable<CoordStruct>& nEFlh, const char* pFlag)
{
	char tempBuffer[32];
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%sFLH", pFlag);
	nFlh.Read(exArtINI, pArtSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "Elite%sFLH", pFlag);
	nEFlh.Read(exArtINI, pArtSection, tempBuffer);
}

void TechnoTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI)
{
	auto pThis = this->Get();
	const auto pArtIni = &CCINIClass::INI_Art();
	const char* pSection = pThis->ID;
	const char* pArtSection = pThis->ImageFile;

	if (!pINI->GetSection(pSection))
		return;

	INI_EX exArtINI(pArtIni);
	INI_EX exINI(pINI);

	this->Survivors_PassengerChance.Read(exINI, pSection, "Survivor.%sPassengerChance");
	this->HealthBar_Hide.Read(exINI, pSection, "HealthBar.Hide");
	this->UIDescription.Read(exINI, pSection, "UIDescription");
	this->LowSelectionPriority.Read(exINI, pSection, "LowSelectionPriority");
	this->MindControlRangeLimit.Read(exINI, pSection, "MindControlRangeLimit");

	this->Phobos_EliteAbilities.Read(exINI, pSection, GameStrings::EliteAbilities(), EnumFunctions::PhobosAbilityType_ToStrings);
	this->Phobos_VeteranAbilities.Read(exINI, pSection, GameStrings::VeteranAbilities(), EnumFunctions::PhobosAbilityType_ToStrings);

	this->ImmuneToEMP.Read(exINI, pSection, "ImmuneToEMP");

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
	this->Interceptor_ConsiderWeaponRange.Read(exINI, pSection, "Interceptor.ConsiderWaponRange");
	this->Interceptor_OnlyTargetBullet.Read(exINI, pSection, "Interceptor.OnlyTargetBullet");

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

	//TODO : Tag name Change
	this->Death_NoAmmo.Read(exINI, pSection, "Death.NoAmmo");
	this->Death_Countdown.Read(exINI, pSection, "Death.Countdown");

	Nullable<bool> Death_Peaceful;
	Death_Peaceful.Read(exINI, pSection, "Death.Peaceful");

	this->Death_Method.Read(exINI, pSection, "Death.Method");

	if (Death_Peaceful.isset())
		this->Death_Method = Death_Peaceful.Get() ? KillMethod::Vanish : KillMethod::Explode;

	this->AutoDeath_Nonexist.Read(exINI, pSection, "AutoDeath.Nonexist");
	this->AutoDeath_Nonexist_Any.Read(exINI, pSection, "AutoDeath.Nonexist.Any");
	this->AutoDeath_Nonexist_House.Read(exINI, pSection, "AutoDeath.Nonexist.House");
	this->AutoDeath_Nonexist_AllowLimboed.Read(exINI, pSection, "AutoDeath.Nonexist.AllowLimboed");
	this->AutoDeath_Exist.Read(exINI, pSection, "AutoDeath.Exist");
	this->AutoDeath_Exist_Any.Read(exINI, pSection, "AutoDeath.Exist.Any");
	this->AutoDeath_Exist_House.Read(exINI, pSection, "AutoDeath.Exist.House");
	this->AutoDeath_Exist_AllowLimboed.Read(exINI, pSection, "AutoDeath.Exist.AllowLimboed");
	this->AutoDeath_VanishAnimation.Read(exINI, pSection, "AutoDeath.VanishAnimation");

	this->Death_WithMaster.Read(exINI, pSection, "Death.WithSlaveOwner");
	this->Slaved_ReturnTo.Read(exINI, pSection, "Slaved.ReturnTo");

	//

	this->ShieldType.Read(exINI, pSection, "ShieldType", true);
	this->CameoPriority.Read(exINI, pSection, "CameoPriority");

	this->WarpOut.Read(exINI, pSection, GameStrings::WarpOut());
	this->WarpIn.Read(exINI, pSection, GameStrings::WarpIn());
	this->WarpAway.Read(exINI, pSection, GameStrings::WarpAway());
	this->ChronoTrigger.Read(exINI, pSection, GameStrings::ChronoTrigger());
	this->ChronoDistanceFactor.Read(exINI, pSection, GameStrings::ChronoDistanceFactor());
	this->ChronoMinimumDelay.Read(exINI, pSection, GameStrings::ChronoMinimumDelay());
	this->ChronoRangeMinimum.Read(exINI, pSection, GameStrings::ChronoRangeMinimum());
	this->ChronoDelay.Read(exINI, pSection, GameStrings::ChronoDelay());

	this->WarpInWeapon.Read(exINI, pSection, "WarpInWeapon", true);
	this->WarpInMinRangeWeapon.Read(exINI, pSection, "WarpInMinRangeWeapon", true);
	this->WarpOutWeapon.Read(exINI, pSection, "WarpOutWeapon", true);
	this->WarpInWeapon_UseDistanceAsDamage.Read(exINI, pSection, "WarpInWeapon.UseDistanceAsDamage");

	this->OreGathering_Anims.Read(exINI, pSection, "OreGathering.Anims");
	this->OreGathering_Tiberiums.Read(exINI, pSection, "OreGathering.Tiberiums");
	this->OreGathering_FramesPerDir.Read(exINI, pSection, "OreGathering.FramesPerDir");

	this->DestroyAnim_Random.Read(exINI, pSection, "DestroyAnim.Random");
	this->NotHuman_RandomDeathSequence.Read(exINI, pSection, "NotHuman.RandomDeathSequence");

	auto const& [canParse, resetValue] = PassengerDeletionTypeClass::CanParse(exINI, pSection);

	if (canParse)
	{
		if (!this->PassengerDeletionType)
			this->PassengerDeletionType = std::make_unique<PassengerDeletionTypeClass>(this->Get());

		this->PassengerDeletionType->LoadFromINI(pINI, pSection);
	}
	else if (resetValue)
	{
		this->PassengerDeletionType.reset();
	}

	this->DefaultDisguise.Read(exINI, pSection, "DefaultDisguise");

	this->OpenTopped_RangeBonus.Read(exINI, pSection, "OpenTopped.RangeBonus");
	this->OpenTopped_DamageMultiplier.Read(exINI, pSection, "OpenTopped.DamageMultiplier");
	this->OpenTopped_WarpDistance.Read(exINI, pSection, "OpenTopped.WarpDistance");
	this->OpenTopped_IgnoreRangefinding.Read(exINI, pSection, "OpenTopped.IgnoreRangefinding");
	this->OpenTopped_AllowFiringIfDeactivated.Read(exINI, pSection, "OpenTopped.AllowFiringIfDeactivated");
	this->OpenTopped_ShareTransportTarget.Read(exINI, pSection, "OpenTopped.ShareTransportTarget");

	this->AutoFire.Read(exINI, pSection, "AutoFire");
	this->AutoFire_TargetSelf.Read(exINI, pSection, "AutoFire.TargetSelf");

	this->NoSecondaryWeaponFallback.Read(exINI, pSection, "NoSecondaryWeaponFallback");
	this->NoSecondaryWeaponFallback_AllowAA.Read(exINI, pSection, "NoSecondaryWeaponFallback.AllowAA");

	this->JumpjetAllowLayerDeviation.Read(exINI, pSection, "JumpjetAllowLayerDeviation");
	this->JumpjetTurnToTarget.Read(exINI, pSection, "JumpjetTurnToTarget");
	this->JumpjetCrash_Rotate.Read(exINI, pSection, "JumpjetCrashRotate");

	this->DeployingAnim_AllowAnyDirection.Read(exINI, pSection, "DeployingAnim.AllowAnyDirection");
	this->DeployingAnim_KeepUnitVisible.Read(exINI, pSection, "DeployingAnim.KeepUnitVisible");
	this->DeployingAnim_ReverseForUndeploy.Read(exINI, pSection, "DeployingAnim.ReverseForUndeploy");
	this->DeployingAnim_UseUnitDrawer.Read(exINI, pSection, "DeployingAnim.UseUnitDrawer");

	this->SelfHealGainType.Read(exINI, pSection, "SelfHealGainType");

	// Ares 0.2
	this->RadarJamRadius.Read(exINI, pSection, "RadarJamRadius");

	// Ares 0.9
	this->InhibitorRange.Read(exINI, pSection, "InhibitorRange");
	this->DesignatorRange.Read(exINI, pSection, "DesignatorRange");

	// Ares 0.A
	this->GroupAs.Read(pINI, pSection, "GroupAs");

	// Ares 0.C
	this->NoAmmoWeapon.Read(exINI, pSection, "NoAmmoWeapon");
	this->NoAmmoAmount.Read(exINI, pSection, "NoAmmoAmount");

	this->EnemyUIName.Read(exINI, pSection, "EnemyUIName");

	this->ForceWeapon_Naval_Decloaked.Read(exINI, pSection, "ForceWeapon.Naval.Decloaked");
	this->ForceWeapon_UnderEMP.Read(exINI, pSection, "ForceWeapon.UnderEMP");
	this->ForceWeapon_Cloaked.Read(exINI, pSection, "ForceWeapon.Cloaked");
	this->ForceWeapon_Disguised.Read(exINI, pSection, "ForceWeapon.Disguised");

	this->Ammo_Shared.Read(exINI, pSection, "Ammo.Shared");
	this->Ammo_Shared_Group.Read(exINI, pSection, "Ammo.Shared.Group");
	this->Passengers_SyncOwner.Read(exINI, pSection, "Passengers.SyncOwner");
	this->Passengers_SyncOwner_RevertOnExit.Read(exINI, pSection, "Passengers.SyncOwner.RevertOnExit");

	this->UseDisguiseMovementSpeed.Read(exINI, pSection, "UseDisguiseMovementSpeed");
	this->Insignia.Read(exINI, pSection, "Insignia.%s");
	this->InsigniaFrame.Read(exINI, pSection, "InsigniaFrame.%s");
	this->Insignia_ShowEnemy.Read(exINI, pSection, "Insignia.ShowEnemy");
	this->InsigniaFrames.Read(exINI, pSection, "InsigniaFrames");
	this->InsigniaDrawOffset.Read(exINI, pSection, "Insignia.DrawOffset");
	this->InitialStrength_Cloning.Read(exINI, pSection, "InitialStrength.Cloning");

	this->SHP_SelectBrdSHP.Read(exINI, pSection, "SelectBrd.SHP");
	this->SHP_SelectBrdPAL.Read(pINI, pSection, "SelectBrd.PAL");
	this->UseCustomSelectBrd.Read(exINI, pSection, "UseCustomSelectBrd");
	this->SelectBrd_Frame.Read(exINI, pSection, "SelectBrd.Frame");
	this->SelectBrd_DrawOffset.Read(exINI, pSection, "SelectBrd.DrawOffset");
	this->SelectBrd_TranslucentLevel.Read(exINI, pSection, "SelectBrd.TranslucentLevel");
	this->SelectBrd_ShowEnemy.Read(exINI, pSection, "SelectBrd.ShowEnemy");

	this->MobileRefinery.Read(exINI, pSection, "MobileRefinery");
	this->MobileRefinery_TransRate.Read(exINI, pSection, "MobileRefinery.TransRate");
	this->MobileRefinery_CashMultiplier.Read(exINI, pSection, "MobileRefinery.CashMultiplier");
	this->MobileRefinery_AmountPerCell.Read(exINI, pSection, "MobileRefinery.AmountPerCell");
	this->MobileRefinery_FrontOffset.Read(exINI, pSection, "MobileRefinery.FrontOffset");
	this->MobileRefinery_LeftOffset.Read(exINI, pSection, "MobileRefinery.LeftOffset");
	this->MobileRefinery_Display.Read(exINI, pSection, "MobileRefinery.Display");
	this->MobileRefinery_DisplayColor.Read(exINI, pSection, "MobileRefinery.DisplayColor");
	this->MobileRefinery_Anims.Read(exINI, pSection, "MobileRefinery.Anims");
	this->MobileRefinery_AnimMove.Read(exINI, pSection, "MobileRefinery.AnimMove");
	this->Explodes_KillPassengers.Read(exINI, pSection, "Explodes.KillPassengers");

	this->DeployFireWeapon.Read(exINI, pSection, "DeployFireWeapon");
	this->RevengeWeapon.Read(exINI, pSection, "RevengeWeapon", true);
	this->RevengeWeapon_AffectsHouses.Read(exINI, pSection, "RevengeWeapon.AffectsHouses");
	this->TargetZoneScanType.Read(exINI, pSection, "TargetZoneScanType");

	this->GrapplingAttack.Read(exINI, pSection, "Parasite.GrapplingAttack");

#pragma region Otamaa
	this->DontShake.Read(exINI, pSection, "DontShakeScreen");
	this->DiskLaserChargeUp.Read(exINI, pSection, GameStrings::DiskLaserChargeUp());

	this->DrainMoneyFrameDelay.Read(exINI, pSection, GameStrings::DrainMoneyFrameDelay());
	this->DrainMoneyAmount.Read(exINI, pSection, GameStrings::DrainMoneyAmount());
	this->DrainMoney_Display.Read(exINI, pSection, "DrainMoney.Display");
	this->DrainMoney_Display_Houses.Read(exINI, pSection, "DrainMoney.Display.Houses");
	this->DrainMoney_Display_AtFirer.Read(exINI, pSection, "DrainMoney.Display.AtFirer");
	this->DrainMoney_Display_Offset.Read(exINI, pSection, "DrainMoney.Display.Offset");
	this->DrainAnimationType.Read(exINI, pSection, GameStrings::DrainAnimationType());

	this->TalkBubbleTime.Read(exINI, pSection, GameStrings::TalkBubbleTime());

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
	this->Overload_Warhead.Read(exINI, pSection, "Overload.Warhead", true);

	this->Landing_Anim.Read(exINI, pSection, "Landing.Anim");
	this->Landing_AnimOnWater.Read(exINI, pSection, "Landing.AnimOnWater");

	this->FacingRotation_Disable.Read(exINI, pSection, "FacingRotation.Disabled");
	this->FacingRotation_DisalbeOnEMP.Read(exINI, pSection, "FacingRotation.DisabledOnEMP");
	this->FacingRotation_DisalbeOnDeactivated.Read(exINI, pSection, "FacingRotation.DisabledOnDeactivated");
	this->FacingRotation_DisableOnDriverKilled.Read(exINI, pSection, "FacingRotation.DisabledOnDriverKilled"); // condition disabled , require Ares 3.0 ++

	this->Draw_MindControlLink.Read(exINI, pSection, "MindControl.DrawLink");

	this->DeathWeapon.Read(exINI, pSection, "%s.DeathWeapon");
	this->Disable_C4WarheadExp.Read(exINI, pSection, "Crash.DisableC4WarheadExplosion");
	this->GClock_Shape.Read(exINI, pSection, "GClock.Shape");
	this->GClock_Transculency.Read(exINI, pSection, "GClock.Transculency");
	this->GClock_Palette.Read(pINI, pSection, "GClock.Palette");

	this->ROF_Random.Read(exINI, pSection, "ROF.AddRandom");
	this->Rof_RandomMinMax.Read(exINI, pSection, "ROF.RandomMinMax");

	this->CreateSound_Enable.Read(exINI, pSection, "CreateSound.Enable");

	this->Eva_Complete.Read(exINI, pSection, "EVA.Complete");
	this->VoiceCreate.Read(exINI, pSection, "VoiceCreate");

	this->SlaveFreeSound_Enable.Read(exINI, pSection, "SlaveFreeSound.Enable");
	this->SlaveFreeSound.Read(exINI, pSection, "SlaveFreeSound");
	this->SinkAnim.Read(exINI, pSection, "Sink.Anim");
	this->Tunnel_Speed.Read(exINI, pSection, "TunnelSpeed");
	this->HoverType.Read(exINI, pSection, "HoverType");

	this->Gattling_Overload.Read(exINI, pSection, "Gattling.Overload");
	this->Gattling_Overload_Damage.Read(exINI, pSection, "Gattling.Overload.Damage");
	this->Gattling_Overload_Frames.Read(exINI, pSection, "Gattling.Overload.Frames");
	this->Gattling_Overload_DeathSound.Read(exINI, pSection, "Gattling.Overload.DeathSound");
	this->Gattling_Overload_ParticleSys.Read(exINI, pSection, "Gattling.Overload.ParticleSys");
	this->Gattling_Overload_ParticleSysCount.Read(exINI, pSection, "Gattling.Overload.ParticleSysCount");
	this->Gattling_Overload_Warhead.Read(exINI, pSection, "Gattling.Overload.Warhead", true);

	this->IsHero.Read(exINI, pSection, "Hero"); //TODO : Move to InfType Ext
	this->IsDummy.Read(exINI, pSection, "Dummy");

	{
		// UpdateCode Disabled 
		// TODO : what will happen if the vectors for different state have different item count ?
		// that will trigger crash because of out of bound idx 
		// so disable these untill i can figure out better codes

		this->FireSelf_Weapon.Read(exINI, pSection, "FireSelf.Weapon");
		this->FireSelf_ROF.Read(exINI, pSection, "FireSelf.ROF");
		this->FireSelf_Weapon_GreenHeath.Read(exINI, pSection, "FireSelf.Weapon.GreenHealth");
		this->FireSelf_ROF_GreenHeath.Read(exINI, pSection, "FireSelf.ROF.GreenHealth");
		this->FireSelf_Weapon_YellowHeath.Read(exINI, pSection, "FireSelf.Weapon.YellowHealth");
		this->FireSelf_ROF_YellowHeath.Read(exINI, pSection, "FireSelf.ROF.YellowHealth");
		this->FireSelf_Weapon_RedHeath.Read(exINI, pSection, "FireSelf.Weapon.RedHealth");
		this->FireSelf_ROF_RedHeath.Read(exINI, pSection, "FireSelf.ROF.RedHealth");
	}

	this->AllowFire_IroncurtainedTarget.Read(exINI, pSection, "Firing.AllowICedTargetForAI");

#ifdef COMPILE_PORTED_DP_FEATURES
	this->VirtualUnit.Read(exINI, pSection, "VirtualUnit");
	this->MyExtraFireData.ReadRules(exINI, pSection);
	this->MyGiftBoxData.Read(exINI, pSection);
	//this->MyJJData.Read(exINI, pSection);
	this->MyPassangersData.Read(exINI, pSection);
	this->MySpawnSupportDatas.Read(exINI, pSection);
	this->DamageSelfData.Read(exINI, pSection);
#endif

	this->IronCurtain_SyncDeploysInto.Read(exINI, pSection, "IronCurtain.KeptOnDeploy");
	this->IronCurtain_Effect.Read(exINI, pSection, "IronCurtain.Flag");
	this->IronCurtain_KillWarhead.Read(exINI, pSection, "IronCurtain.KillWarhead", true);

	this->SellSound.Read(exINI, pSection, "SellSound");
	this->EVA_Sold.Read(exINI, pSection, "EVA.Sold");
	this->EngineerCaptureDelay.Read(exINI, pSection, "Engineer.CaptureDelay"); // code unfinished

	this->CommandLine_Move_Color.Read(exINI, pSection, "ActionLine.Move.Color");
	this->CommandLine_Attack_Color.Read(exINI, pSection, "ActionLine.Attack.Color");
	this->CloakMove.Read(exINI, pSection, "Cloak.Move");
	this->PassiveAcquire_AI.Read(exINI, pSection, "CanPassiveAquire.AI");
	this->TankDisguiseAsTank.Read(exINI, pSection, "Disguise.AsTank"); // code disabled , crash
	this->DisguiseDisAllowed.Read(exINI, pSection, "Disguise.Allowed");  // code disabled , crash
	this->ChronoDelay_Immune.Read(exINI, pSection, "ChronoDelay.Immune");
	this->Unit_AI_AlternateType.Read(exINI, pSection, "AIAlternateType");

	this->Riparius_FrameIDx.Read(exINI, pSection, "Storage0FrameIdx");
	this->Cruentus_FrameIDx.Read(exINI, pSection, "Storage1FrameIdx");
	this->Vinifera_FrameIDx.Read(exINI, pSection, "Storage2FrameIdx");
	this->Aboreus_FrameIDx.Read(exINI, pSection, "Storage3FrameIdx");

	this->CrushLevel.Read(exINI, pSection, "%sCrushLevel");
	this->CrushableLevel.Read(exINI, pSection, "%sCrushableLevel");
	this->DeployCrushableLevel.Read(exINI, pSection, "%sDeployCrushableLevel");
	this->Experience_KillerMultiple.Read(exINI, pSection, "Experience.KillerMultiple");
	this->Experience_VictimMultiple.Read(exINI, pSection, "Experience.VictimMultiple");
	this->NavalRangeBonus.Read(exINI, pSection, "NavalRangeBonus");
	this->AI_LegalTarget.Read(exINI, pSection, "AI.LegalTarget");
	this->DeployFire_UpdateFacing.Read(exINI, pSection, "DeployFire.CheckFacing");
	this->Fake_Of.Read(exINI, pSection, "FakeOf");
	this->CivilianEnemy.Read(exINI, pSection, "CivilianEnemy");
	this->ImmuneToBerserk.Read(exINI, pSection, "ImmuneToBerzerk");
	this->Berzerk_Modifier.Read(exINI, pSection, "Berzerk.Modifier");
	this->IgnoreToProtect.Read(exINI, pSection, "ToProtect.Ignore");
	this->TargetLaser_Time.Read(exINI, pSection, "TargetLaser.Time");
	this->TargetLaser_WeaponIdx.Read(exINI, pSection, "TargetLaser.WeaponIndexes");
	this->AdjustCrushProperties();

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

		this->LaserTrailData.emplace_back(trail.Get(), flh.Get(), isOnTurret.Get());
	}

#pragma region FLHs
	TechnoTypeExt::GetBurstFLHs(pThis, exArtINI, pArtSection, WeaponBurstFLHs, EliteWeaponBurstFLHs, "");
	TechnoTypeExt::GetBurstFLHs(pThis, exArtINI, pArtSection, DeployedWeaponBurstFLHs, EliteDeployedWeaponBurstFLHs, "Deployed");
	TechnoTypeExt::GetBurstFLHs(pThis, exArtINI, pArtSection, CrouchedWeaponBurstFLHs, EliteCrouchedWeaponBurstFLHs, "Prone");

	TechnoTypeExt::GetFLH(exArtINI, pArtSection, PronePrimaryFireFLH, E_PronePrimaryFireFLH, "PronePrimaryFire");
	TechnoTypeExt::GetFLH(exArtINI, pArtSection, ProneSecondaryFireFLH, E_ProneSecondaryFireFLH, "ProneSecondaryFire");
	TechnoTypeExt::GetFLH(exArtINI, pArtSection, DeployedPrimaryFireFLH, E_DeployedPrimaryFireFLH, "DeployedPrimaryFire");
	TechnoTypeExt::GetFLH(exArtINI, pArtSection, DeployedSecondaryFireFLH, E_DeployedSecondaryFireFLH, "DeployedSecondaryFire");

	for (size_t i = 0;; i++)
	{
		char key[0x39];
		Nullable<CoordStruct> alternateFLH;
		sprintf_s(key, "AlternateFLH%u", i);
		alternateFLH.Read(exArtINI, pArtSection, key);

		if (i >= 5U && !alternateFLH.isset())
			break;

		this->AlternateFLHs.size() < i
			? this->AlternateFLHs[i] = alternateFLH.Get()
			: this->AlternateFLHs.emplace_back(alternateFLH.Get());
	}
#pragma endregion FLHs

	this->ConsideredNaval.Read(exINI, pSection, "ConsideredNaval");
	this->ConsideredVehicle.Read(exINI, pSection, "ConsideredVehicle");

#pragma region Prereq
	// Prerequisite.RequiredTheaters contains a list of theader names
	const char* key_prereqTheaters = "Prerequisite.RequiredTheaters";
	char* context = nullptr;
	pINI->ReadString(pSection, key_prereqTheaters, "", Phobos::readBuffer);

	for (char* cur = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context); cur; cur = strtok_s(nullptr, Phobos::readDelims, &context))
	{
		cur = CRT::strtrim(cur);
		int index = Theater::FindIndex(cur);
		if (index != -1)
			Prerequisite_RequiredTheaters.push_back(index);
	}

	// Prerequisite with Generic Prerequistes support.
	// Note: I have no idea of what could happen in all the game engine logics if I push the negative indexes of the Ares generic prerequisites directly into the original Prerequisite tag... for that reason this tag is duplicated for working with it
	const char* key_prereqs = "Prerequisite";
	context = nullptr;
	pINI->ReadString(pSection, key_prereqs, "", Phobos::readBuffer);

	for (char* cur = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context); cur; cur = strtok_s(nullptr, Phobos::readDelims, &context))
	{
		cur = CRT::strtrim(cur);
		int idx = TechnoTypeClass::FindIndexById(cur);

		if (idx >= 0)
		{
			Prerequisite.push_back(idx);
		}
		else
		{
			int index = HouseExt::FindGenericPrerequisite(cur);
			if (index < 0)
				Prerequisite.push_back(index);
		}
	}

	// Prerequisite.Negative with Generic Prerequistes support
	const char* key_prereqsNegative = "Prerequisite.Negative";
	context = nullptr;
	pINI->ReadString(pSection, key_prereqsNegative, "", Phobos::readBuffer);

	for (char* cur = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context); cur; cur = strtok_s(nullptr, Phobos::readDelims, &context))
	{
		cur = CRT::strtrim(cur);
		int idx = TechnoTypeClass::FindIndexById(cur);

		if (idx >= 0)
		{
			Prerequisite_Negative.push_back(idx);
		}
		else
		{
			int index = HouseExt::FindGenericPrerequisite(cur);
			if (index < 0)
				Prerequisite_Negative.push_back(index);
		}
	}

	// Prerequisite.ListX with Generic Prerequistes support
	this->Prerequisite_Lists.Read(exINI, pSection, "Prerequisite.Lists");

	if (Prerequisite_Lists.Get() > 0)
	{
		for (int i = 1; i <= Prerequisite_Lists.Get(); i++)
		{
			char keySection[32];
			_snprintf_s(keySection, sizeof(keySection), "Prerequisite.List%d", i);

			std::vector<int> objectsList;
			char* context2 = nullptr;
			pINI->ReadString(pSection, keySection, "", Phobos::readBuffer);

			for (char* cur = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context2); cur; cur = strtok_s(nullptr, Phobos::readDelims, &context2))
			{
				cur = CRT::strtrim(cur);
				int idx = TechnoTypeClass::FindIndexById(cur);

				if (idx >= 0)
				{
					objectsList.push_back(idx);
				}
				else
				{
					int index = HouseExt::FindGenericPrerequisite(cur);
					if (index < 0)
						objectsList.push_back(index);
				}
			}

			Prerequisite_ListVector.push_back(objectsList);
		}
	}
#pragma endregion Prereq

	this->AttachedEffect.Read(exINI);

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

	this->Spawner_SpawnOffsets.Read(exArtINI, pArtSection, "SpawnOffset");
	this->Spawner_SpawnOffsets_OverrideWeaponFLH.Read(exArtINI, pArtSection, "SpawnOffsetOverrideFLH");

	LineTrailData::LoadFromINI(this->LineTrailData, exArtINI, pArtSection);

#ifdef COMPILE_PORTED_DP_FEATURES

	TechnoTypeExt::GetFLH(exArtINI, pArtSection, PrimaryCrawlFLH, Elite_PrimaryCrawlFLH, "PrimaryCrawling");
	TechnoTypeExt::GetFLH(exArtINI, pArtSection, SecondaryCrawlFLH, Elite_SecondaryCrawlFLH, "SecondaryCrawling");

	this->MyExtraFireData.ReadArt(exArtINI, pArtSection);
	this->MySpawnSupportFLH.Read(exArtINI, pArtSection);
	this->Trails.Read(exArtINI, pArtSection, true);
#endif

#pragma endregion
}


void TechnoTypeExt::ExtData::LoadFromINIFile_Aircraft(CCINIClass* pINI)
{
	auto pThis = Get();
	const char* pSection = pThis->ID;
	INI_EX exINI(pINI);

	this->CrashSpinLevelRate.Read(exINI, pSection, "CrashSpin.LevelRate");
	this->CrashSpinVerticalRate.Read(exINI, pSection, "CrashSpin.VerticalRate");

	this->SpyplaneCameraSound.Read(exINI, pSection, "SpyPlaneCameraSound");
	this->ParadropRadius.Read(exINI, pSection, "Paradrop.ApproachRadius");
	this->ParadropOverflRadius.Read(exINI, pSection, "Paradrop.OverflyRadius");
	this->Paradrop_DropPassangers.Read(exINI, pSection, "Paradrop.DropPassangers");

	// Disabled , rare but can crash after S/L
	this->Paradrop_MaxAttempt.Read(exINI, pSection, "Paradrop.MaxApproachAttempt");
	//

	this->IsCustomMissile.Read(exINI, pSection, "Missile.Custom");
	this->CustomMissileData.Read(exINI, pSection, "Missile");
	this->CustomMissileData.GetEx()->Type = static_cast<AircraftTypeClass*>(pThis);
	this->CustomMissileRaise.Read(exINI, pSection, "Missile.%sRaiseBeforeLaunching");
	this->CustomMissileOffset.Read(exINI, pSection, "Missile.CoordOffset");
	this->CustomMissileWarhead.Read(exINI, pSection, "Missile.Warhead");
	this->CustomMissileEliteWarhead.Read(exINI, pSection, "Missile.EliteWarhead");
	this->CustomMissileTakeoffAnim.Read(exINI, pSection, "Missile.TakeOffAnim");
	this->CustomMissileTrailerAnim.Read(exINI, pSection, "Missile.TrailerAnim");
	this->CustomMissileTrailerSeparation.Read(exINI, pSection, "Missile.TrailerSeparation");
	this->CustomMissileWeapon.Read(exINI, pSection, "Missile.Weapon");
	this->CustomMissileEliteWeapon.Read(exINI, pSection, "Missile.EliteWeapon");

	this->AttackingAircraftSightRange.Read(exINI, pSection, "AttackingAircraftSightRange");
	this->CrashWeapon_s.Read(exINI, pSection, "Crash.Weapon", true);
	this->CrashWeapon.Read(exINI, pSection, "Crash.%sWeapon");
	this->NoAirportBound_DisableRadioContact.Read(exINI, pSection, "NoAirportBound.DisableRadioContact");

	this->TakeOff_Anim.Read(exINI, pSection, "TakeOff.Anim");
	this->PoseDir.Read(exINI, pSection, GameStrings::PoseDir());
	this->Firing_IgnoreGravity.Read(exINI, pSection, "Firing.IgnoreGravity");
	//No code
	this->Aircraft_DecreaseAmmo.Read(exINI, pSection, "Firing.ReplaceFiringMode");
	this->CurleyShuffle.Read(exINI, pSection, "CurleyShuffle");

	// #346, #464, #970, #1014
	this->PassengersGainExperience.Read(exINI, pSection, "Experience.PromotePassengers");
	this->ExperienceFromPassengers.Read(exINI, pSection, "Experience.FromPassengers");
	this->PassengerExperienceModifier.Read(exINI, pSection, "Experience.PassengerModifier");
	this->MindControlExperienceSelfModifier.Read(exINI, pSection, "Experience.MindControlSelfModifier");
	this->MindControlExperienceVictimModifier.Read(exINI, pSection, "Experience.MindControlVictimModifier");
	this->SpawnExperienceOwnerModifier.Read(exINI, pSection, "Experience.SpawnOwnerModifier");
	this->SpawnExperienceSpawnModifier.Read(exINI, pSection, "Experience.SpawnModifier");
	this->ExperienceFromAirstrike.Read(exINI, pSection, "Experience.FromAirstrike");
	this->AirstrikeExperienceModifier.Read(exINI, pSection, "Experience.AirstrikeModifier");
	this->Promote_IncludePassengers.Read(exINI, pSection, "Promote.IncludePassengers");
	this->Promote_Elite_Eva.Read(exINI, pSection, "EVA.ElitePromoted");
	this->Promote_Vet_Eva.Read(exINI, pSection, "EVA.VeteranPromoted");
	this->Promote_Elite_Sound.Read(exINI, pSection, "Promote.EliteFlash");
	this->Promote_Vet_Sound.Read(exINI, pSection, "Promote.VeteranFlash");
	this->Promote_Elite_Flash.Read(exINI, pSection, "Promote.EliteSound");
	this->Promote_Vet_Flash.Read(exINI, pSection, "Promote.VeteranSound");
	this->Promote_Vet_Flash.Read(exINI, pSection, "Promote.VeteranType");
	this->Promote_Elite_Type.Read(exINI, pSection, "Promote.EliteType");
	this->Promote_Vet_Exp.Read(exINI, pSection, "Promote.VeteranExperience");
	this->Promote_Elite_Exp.Read(exINI, pSection, "Promote.EliteExperience");

#ifdef COMPILE_PORTED_DP_FEATURES
	this->MissileHoming.Read(exINI, pSection, "Missile.Homing");
	this->MyDiveData.Read(exINI, pSection);
	this->MyPutData.Read(exINI, pSection);
	this->MyFighterData.Read(exINI, pSection, pThis);
#endif
}

void TechnoTypeExt::ExtData::LoadFromINIFile_EvaluateSomeVariables(CCINIClass* pINI)
{
	auto pThis = Get();
	const char* pSection = pThis->ID;
	INI_EX exINI(pINI);

#ifdef COMPILE_PORTED_DP_FEATURES
	this->MyExtraFireData.ReadRules(exINI, pSection);
#endif
}


void TechnoTypeExt::ExtData::AdjustCrushProperties()
{
	auto const pThis = Get();

	if (this->CrushLevel.Rookie <= 0)
	{
		if (pThis->OmniCrusher)
			this->CrushLevel.Rookie = 10;
		else if (pThis->Crusher)
			this->CrushLevel.Rookie = 5;
		else
			this->CrushLevel.Rookie = 0;
	}
	if (this->CrushLevel.Veteran <= 0)
	{
		if (!pThis->OmniCrusher && pThis->VeteranAbilities.CRUSHER)
			this->CrushLevel.Veteran = 5;
		else
			this->CrushLevel.Veteran = this->CrushLevel.Rookie;
	}
	if (this->CrushLevel.Elite <= 0)
	{
		if (!pThis->OmniCrusher && pThis->EliteAbilities.CRUSHER)
			this->CrushLevel.Elite = 5;
		else
			this->CrushLevel.Elite = this->CrushLevel.Veteran;
	}
	if (!pThis->Crusher && (this->CrushLevel.Rookie > 0 || this->CrushLevel.Veteran > 0 || this->CrushLevel.Elite > 0) &&
		Is_UnitType(pThis))
		pThis->Crusher = true;

	if (this->CrushableLevel.Rookie <= 0)
	{
		if (pThis->OmniCrushResistant)
			this->CrushableLevel.Rookie = 10;
		else if (!pThis->Crushable)
			this->CrushableLevel.Rookie = 5;
		else
			this->CrushableLevel.Rookie = 0;
	}
	if (this->CrushableLevel.Veteran <= 0)
		this->CrushableLevel.Veteran = this->CrushableLevel.Rookie;
	if (this->CrushableLevel.Elite <= 0)
		this->CrushableLevel.Elite = this->CrushableLevel.Veteran;

	if (const auto pInfType = abstract_cast<InfantryTypeClass*>(pThis))
	{
		if (this->DeployCrushableLevel.Rookie <= 0)
		{
			if (!pInfType->DeployedCrushable)
				this->DeployCrushableLevel.Rookie = 5;
			else
				this->DeployCrushableLevel.Rookie = this->CrushableLevel.Rookie;
		}
		if (this->DeployCrushableLevel.Veteran <= 0)
			this->DeployCrushableLevel.Veteran = this->DeployCrushableLevel.Rookie;
		if (this->DeployCrushableLevel.Elite <= 0)
			this->DeployCrushableLevel.Elite = this->DeployCrushableLevel.Veteran;
	}
}

template <typename T>
void TechnoTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->HealthBar_Hide)
		.Process(this->UIDescription)
		.Process(this->LowSelectionPriority)
		.Process(this->MindControlRangeLimit)
		.Process(this->Phobos_EliteAbilities)
		.Process(this->Phobos_VeteranAbilities)
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
		.Process(this->Interceptor_ConsiderWeaponRange)
		.Process(this->Interceptor_OnlyTargetBullet)
		.Process(this->GroupAs)
		.Process(this->RadarJamRadius)
		.Process(this->InhibitorRange)
		.Process(this->DesignatorRange)
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
		.Process(this->Death_Method)
		.Process(this->AutoDeath_Nonexist)
		.Process(this->AutoDeath_Nonexist_House)
		.Process(this->AutoDeath_Nonexist_Any)
		.Process(this->AutoDeath_Nonexist_AllowLimboed)

		.Process(this->AutoDeath_Exist)
		.Process(this->AutoDeath_Exist_House)
		.Process(this->AutoDeath_Exist_Any)
		.Process(this->AutoDeath_Exist_AllowLimboed)
		.Process(this->AutoDeath_VanishAnimation)

		.Process(this->Death_WithMaster)
		.Process(this->Slaved_ReturnTo)

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
		.Process(this->PassengerDeletionType)

		.Process(this->OpenTopped_RangeBonus)
		.Process(this->OpenTopped_DamageMultiplier)
		.Process(this->OpenTopped_WarpDistance)
		.Process(this->OpenTopped_IgnoreRangefinding)
		.Process(this->OpenTopped_AllowFiringIfDeactivated)
		.Process(this->OpenTopped_ShareTransportTarget)

		.Process(this->AutoFire)
		.Process(this->AutoFire_TargetSelf)
		.Process(this->NoSecondaryWeaponFallback)
		.Process(this->NoSecondaryWeaponFallback_AllowAA)
		.Process(this->NoAmmoWeapon)
		.Process(this->NoAmmoAmount)
		.Process(this->JumpjetAllowLayerDeviation)
		.Process(this->JumpjetTurnToTarget)
		.Process(this->JumpjetCrash_Rotate)
		.Process(this->DeployingAnim_AllowAnyDirection)
		.Process(this->DeployingAnim_KeepUnitVisible)
		.Process(this->DeployingAnim_ReverseForUndeploy)
		.Process(this->DeployingAnim_UseUnitDrawer)
		.Process(this->SelfHealGainType)
		.Process(this->EnemyUIName)
		.Process(this->ForceWeapon_Naval_Decloaked)
		.Process(this->ForceWeapon_UnderEMP)
		.Process(this->ForceWeapon_Cloaked)
		.Process(this->ForceWeapon_Disguised)
		.Process(this->ImmuneToEMP)
		.Process(this->Ammo_Shared)
		.Process(this->Ammo_Shared_Group)
		.Process(this->Passengers_SyncOwner)
		.Process(this->Passengers_SyncOwner_RevertOnExit)
		.Process(this->Aircraft_DecreaseAmmo)
		.Process(this->UseDisguiseMovementSpeed)
		.Process(this->Insignia)
		.Process(this->InsigniaFrame)
		.Process(this->Insignia_ShowEnemy)
		.Process(this->InsigniaFrames)
		.Process(this->InsigniaDrawOffset)
		.Process(this->InitialStrength_Cloning)
		.Process(this->UseCustomSelectBrd)

		.Process(this->SHP_SelectBrdSHP)
		.Process(this->SHP_SelectBrdPAL)
		.Process(this->UseCustomSelectBrd)
		.Process(this->SelectBrd_Frame)
		.Process(this->SelectBrd_DrawOffset)
		.Process(this->SelectBrd_TranslucentLevel)
		.Process(this->SelectBrd_ShowEnemy)

		.Process(this->MobileRefinery)
		.Process(this->MobileRefinery_TransRate)
		.Process(this->MobileRefinery_CashMultiplier)
		.Process(this->MobileRefinery_AmountPerCell)
		.Process(this->MobileRefinery_FrontOffset)
		.Process(this->MobileRefinery_LeftOffset)
		.Process(this->MobileRefinery_Display)
		.Process(this->MobileRefinery_DisplayColor)
		.Process(this->MobileRefinery_Anims)
		.Process(this->MobileRefinery_AnimMove)
		.Process(this->Explodes_KillPassengers)

		.Process(this->DeployFireWeapon)
		.Process(this->RevengeWeapon)
		.Process(this->RevengeWeapon_AffectsHouses)
		.Process(this->TargetZoneScanType)

		.Process(this->GrapplingAttack)

		.Process(this->PronePrimaryFireFLH)
		.Process(this->ProneSecondaryFireFLH)

		.Process(this->E_PronePrimaryFireFLH)
		.Process(this->ProneSecondaryFireFLH)

		.Process(this->E_DeployedPrimaryFireFLH)
		.Process(this->E_DeployedSecondaryFireFLH)

		.Process(this->IronCurtain_SyncDeploysInto)
		.Process(this->IronCurtain_Effect)
		.Process(this->IronCurtain_KillWarhead)

		.Process(this->SellSound)
		.Process(this->EVA_Sold)

		.Process(this->CrouchedWeaponBurstFLHs)
		.Process(this->EliteCrouchedWeaponBurstFLHs)
		.Process(this->DeployedWeaponBurstFLHs)
		.Process(this->EliteDeployedWeaponBurstFLHs)

		.Process(this->AlternateFLHs)
		.Process(this->Spawner_SpawnOffsets)
		.Process(this->Spawner_SpawnOffsets_OverrideWeaponFLH)
#pragma region Otamaa
		.Process(this->FacingRotation_Disable)
		.Process(this->FacingRotation_DisalbeOnEMP)
		.Process(this->FacingRotation_DisalbeOnDeactivated)
		.Process(this->FacingRotation_DisableOnDriverKilled)
		.Process(this->Is_Cow)

		.Process(this->DontShake)

		.Process(this->DiskLaserChargeUp)

		.Process(this->DrainAnimationType)
		.Process(this->DrainMoneyFrameDelay)
		.Process(this->DrainMoneyAmount)
		.Process(this->DrainMoney_Display)
		.Process(this->DrainMoney_Display_Houses)
		.Process(this->DrainMoney_Display_AtFirer)
		.Process(this->DrainMoney_Display_Offset)

		.Process(this->TalkBubbleTime)

		.Process(this->AttackingAircraftSightRange)

		.Process(this->SpyplaneCameraSound)

		.Process(this->ParadropRadius)
		.Process(this->ParadropOverflRadius)
		.Process(this->Paradrop_DropPassangers)
		.Process(this->Paradrop_MaxAttempt)

		.Process(this->IsCustomMissile)
		.Process(this->CustomMissileData)
		.Process(this->CustomMissileWarhead)
		.Process(this->CustomMissileEliteWarhead)
		.Process(this->CustomMissileTakeoffAnim)
		.Process(this->CustomMissileTrailerAnim)
		.Process(this->CustomMissileTrailerSeparation)
		.Process(this->CustomMissileWeapon)
		.Process(this->CustomMissileEliteWeapon)
		.Process(this->CustomMissileRaise)
		.Process(this->CustomMissileOffset)

		.Process(this->Draw_MindControlLink)

		.Process(this->Overload_Count)
		.Process(this->Overload_Damage)
		.Process(this->Overload_Frames)
		.Process(this->Overload_DeathSound)
		.Process(this->Overload_ParticleSys)
		.Process(this->Overload_ParticleSysCount)
		.Process(this->Overload_Warhead)

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

		.Process(this->Gattling_Overload)
		.Process(this->Gattling_Overload_Damage)
		.Process(this->Gattling_Overload_Frames)
		.Process(this->Gattling_Overload_DeathSound)
		.Process(this->Gattling_Overload_ParticleSys)
		.Process(this->Gattling_Overload_ParticleSysCount)
		.Process(this->Gattling_Overload_Warhead)

		.Process(this->IsHero)
		.Process(this->IsDummy)

		.Process(this->FireSelf_Weapon)
		.Process(this->FireSelf_ROF)
		.Process(this->FireSelf_Weapon_GreenHeath)
		.Process(this->FireSelf_ROF_GreenHeath)
		.Process(this->FireSelf_Weapon_YellowHeath)
		.Process(this->FireSelf_ROF_YellowHeath)
		.Process(this->FireSelf_Weapon_RedHeath)
		.Process(this->FireSelf_ROF_RedHeath)
		.Process(this->AllowFire_IroncurtainedTarget)
		.Process(this->EngineerCaptureDelay)
		.Process(this->CommandLine_Move_Color)
		.Process(this->CommandLine_Attack_Color)
		.Process(this->CloakMove)
		.Process(this->PassiveAcquire_AI)
		.Process(this->TankDisguiseAsTank)
		.Process(this->DisguiseDisAllowed)
		.Process(this->ChronoDelay_Immune)
		.Process(this->LineTrailData)
		.Process(this->PoseDir)
		.Process(this->Firing_IgnoreGravity)
		.Process(this->Survivors_PassengerChance)
		.Process(this->Unit_AI_AlternateType)

		.Process(this->Prerequisite_RequiredTheaters)
		.Process(this->Prerequisite)
		.Process(this->Prerequisite_Negative)
		.Process(this->Prerequisite_Lists)
		.Process(this->Prerequisite_ListVector)
		.Process(this->ConsideredNaval)
		.Process(this->ConsideredVehicle)

#ifdef COMPILE_PORTED_DP_FEATURES
		.Process(this->VirtualUnit)

		.Process(this->PrimaryCrawlFLH)
		.Process(this->Elite_PrimaryCrawlFLH)
		.Process(this->SecondaryCrawlFLH)
		.Process(this->Elite_SecondaryCrawlFLH)
		.Process(this->MissileHoming)

#endif
		.Process(this->Riparius_FrameIDx)
		.Process(this->Cruentus_FrameIDx)
		.Process(this->Vinifera_FrameIDx)
		.Process(this->Aboreus_FrameIDx)

		.Process(this->CrushLevel)
		.Process(this->CrushableLevel)
		.Process(this->DeployCrushableLevel)

		.Process(this->Experience_KillerMultiple)
		.Process(this->Experience_VictimMultiple)
		.Process(this->NavalRangeBonus)
		.Process(this->AI_LegalTarget)
		.Process(this->DeployFire_UpdateFacing)
		.Process(this->Fake_Of)
		.Process(this->CivilianEnemy)
		.Process(this->ImmuneToBerserk)
		.Process(this->Berzerk_Modifier)
		.Process(this->IgnoreToProtect)
		.Process(this->TargetLaser_Time)
		.Process(this->TargetLaser_WeaponIdx)
		.Process(this->CurleyShuffle)

		.Process(this->PassengersGainExperience)
		.Process(this->ExperienceFromPassengers)
		.Process(this->PassengerExperienceModifier)
		.Process(this->MindControlExperienceSelfModifier)
		.Process(this->MindControlExperienceVictimModifier)
		.Process(this->SpawnExperienceOwnerModifier)
		.Process(this->SpawnExperienceSpawnModifier)
		.Process(this->ExperienceFromAirstrike)
		.Process(this->AirstrikeExperienceModifier)
		.Process(this->Promote_IncludePassengers)
		.Process(this->Promote_Elite_Eva)
		.Process(this->Promote_Vet_Eva)
		.Process(this->Promote_Elite_Sound)
		.Process(this->Promote_Vet_Sound)
		.Process(this->Promote_Elite_Flash)
		.Process(this->Promote_Vet_Flash)

		.Process(this->Promote_Vet_Type)
		.Process(this->Promote_Elite_Type)
		.Process(this->Promote_Vet_Exp)
		.Process(this->Promote_Elite_Exp)
#pragma endregion
		;
#ifdef COMPILE_PORTED_DP_FEATURES
	this->MyExtraFireData.Serialize(Stm);
	this->MyDiveData.Serialize(Stm);
	this->MyPutData.Serialize(Stm);
	this->MyGiftBoxData.Serialize(Stm);
	//this->MyJJData.Serialize(Stm);
	this->MyPassangersData.Serialize(Stm);
	this->MySpawnSupportFLH.Serialize(Stm);
	this->MySpawnSupportDatas.Serialize(Stm);
	this->Trails.Serialize(Stm);
	this->MyFighterData.Serialize(Stm);
	this->DamageSelfData.Serialize(Stm);
#endif

	Stm.Process(this->AttachedEffect)
		;
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
	GET(TechnoTypeClass* const, pItem, ESI);
	TechnoTypeExt::ExtMap.Allocate(pItem);
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

DEFINE_HOOK_AGAIN(0x716132, TechnoTypeClass_LoadFromINI, 0x5) // this should make the techno unusable ? becase the game will return false when it
DEFINE_HOOK(0x716123, TechnoTypeClass_LoadFromINI, 0x5)
{
	GET(TechnoTypeClass*, pItem, EBP);
	GET_STACK(CCINIClass*, pINI, 0x380);

	//if (R->Origin() == 0x716132) {
	//	Debug::Log("Failed to find TechnoType %s from TechnoType::LoadFromINI with AbsType %s ! \n", pItem->get_ID(), pItem->GetThisClassName());
	//}

	if (auto ptr = TechnoTypeExt::ExtMap.Find(pItem))
	{
		ptr->LoadFromINI(pINI);
	}

	return 0;
}

//hook before stuffs got pop-ed to remove crash possibility
DEFINE_HOOK(0x41CD74, AircraftTypeClass_LoadFromINI, 0x6)
{
	GET(AircraftTypeClass*, pItem, ESI);
	GET(CCINIClass* const, pINI, EBX);

	R->AL(pINI->ReadBool(pItem->ID, GameStrings::FlyBack(), R->CL()));

	if (auto pExt = TechnoTypeExt::ExtMap.Find(pItem))
		pExt->LoadFromINIFile_Aircraft(pINI);

	return 0x41CD82;
}