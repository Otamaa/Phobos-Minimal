#include "Body.h"

#include <TechnoTypeClass.h>
#include <StringTable.h>

#include <Ext/BuildingType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/House/Body.h>

#include <New/Type/TheaterTypeClass.h>
#include <New/Type/GenericPrerequisite.h>
#include <New/Type/DigitalDisplayTypeClass.h>

#include <Utilities/GeneralUtils.h>
#include <Utilities/Cast.h>
#include <Utilities/EnumFunctions.h>

void TechnoTypeExt::ExtData::Initialize()
{
	OreGathering_Anims.reserve(1);
	OreGathering_Tiberiums.reserve(1);
	OreGathering_FramesPerDir.reserve(1);
	LaserTrailData.reserve(4);
	LineTrailData.reserve(4);
	AutoDeath_Nonexist.reserve(5);
	AutoDeath_Exist.reserve(5);
	OreGathering_Anims.reserve(5);
	OreGathering_Tiberiums.reserve(5);
	OreGathering_FramesPerDir.reserve(5);
	AlternateFLHs.reserve(5);
	MobileRefinery_FrontOffset.reserve(2);
	MobileRefinery_LeftOffset.reserve(2);
	MobileRefinery_Anims.reserve(2);
	Overload_Count.reserve(3);
	Overload_Damage.reserve(3);
	Overload_Frames.reserve(3);
	HitCoordOffset.reserve(3);
	FireSelf_Weapon.reserve(3);
	FireSelf_ROF.reserve(3);
	FireSelf_Weapon_GreenHeath.reserve(3);
	FireSelf_ROF_GreenHeath.reserve(3);
	FireSelf_Weapon_YellowHeath.reserve(3);
	FireSelf_ROF_YellowHeath.reserve(3);
	FireSelf_Weapon_RedHeath.reserve(3);
	FireSelf_ROF_RedHeath.reserve(3);
	DisguiseDisAllowed.reserve(10);
	Prerequisite_RequiredTheaters.reserve(7);
	Prerequisite.reserve(10);
	Prerequisite_Negative.reserve(10);
	TargetLaser_WeaponIdx.reserve(TechnoTypeClass::MaxWeapons);
	PassengersWhitelist.reserve(10);
	PassengersBlacklist.reserve(10);
	ParticleSystems_DamageSmoke.reserve(4);
	ParticleSystems_DamageSparks.reserve(4);

	this->ShieldType = ShieldTypeClass::Array[0].get();

	this->SellSound = RulesClass::Instance->SellSound;
	auto Eva_ready = GameStrings::EVA_ConstructionComplete();
	auto Eva_sold = GameStrings::EVA_StructureSold() ;

	if (!Is_BuildingType(Get()))
	{
		Eva_ready = GameStrings::EVA_UnitReady();
		Eva_sold = GameStrings::EVA_UnitSold();

		if (Is_AircraftType(Get()))
		{
			this->CustomMissileTrailerAnim = AnimTypeClass::Find(GameStrings::V3TRAIL());
			this->CustomMissileTakeoffAnim = AnimTypeClass::Find(GameStrings::V3TAKEOFF());
			this->SmokeAnim = AnimTypeClass::Find(GameStrings::SGRYSMK1());
		}

		this->EVA_UnitLost = VoxClass::FindIndexById(GameStrings::EVA_UnitLost());
		const auto nPromotedEva = VoxClass::FindIndexById(GameStrings::EVA_UnitPromoted());
		this->Promote_Elite_Eva = nPromotedEva;
		this->Promote_Vet_Eva = nPromotedEva;
	}

	this->Eva_Complete = VoxClass::FindIndexById(Eva_ready);
	this->EVA_Sold = VoxClass::FindIndexById(Eva_sold);
}

AnimTypeClass* TechnoTypeExt::GetSinkAnim(TechnoClass* pThis)
{
	return TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())->SinkAnim.Get(RulesClass::Instance->Wake);
}

double TechnoTypeExt::GetTunnelSpeed(TechnoClass* pThis, RulesClass* pRules)
{
	return TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())->Tunnel_Speed.Get(pRules->TunnelSpeed);
}

VoxelStruct* TechnoTypeExt::GetBarrelsVoxelData(TechnoTypeClass* const pThis, size_t const nIdx)
{
	if (nIdx < TechnoTypeClass::MaxWeapons)
		return &pThis->ChargerBarrels[nIdx];

	const auto nAdditional = (nIdx - TechnoTypeClass::MaxWeapons);
	if (nAdditional > TechnoTypeExt::ExtMap.Find(pThis)->BarrelImageData.size())
		Debug::FatalErrorAndExit(__FUNCTION__" [%s] Size[%s] Is Bigger than BarrelData ! \n", pThis->ID, nAdditional);

	return &TechnoTypeExt::ExtMap.Find(pThis)->BarrelImageData
		[nAdditional];
}

VoxelStruct* TechnoTypeExt::GetTurretVoxelData(TechnoTypeClass* const pThis, size_t const nIdx)
{
	if (nIdx < TechnoTypeClass::MaxWeapons)
		return &pThis->ChargerTurrets[nIdx];

	const auto nAdditional = (nIdx - TechnoTypeClass::MaxWeapons);
	if (nAdditional > TechnoTypeExt::ExtMap.Find(pThis)->TurretImageData.size())
		Debug::FatalErrorAndExit(__FUNCTION__" [%s] Size[%d]  Is Bigger than TurretData ! \n", pThis->ID, nAdditional);

	return &TechnoTypeExt::ExtMap.Find(pThis)->TurretImageData
		[nAdditional];
}

// Ares 0.A source
const char* TechnoTypeExt::ExtData::GetSelectionGroupID() const
{
	return GeneralUtils::IsValidString(this->GroupAs) ? this->GroupAs : this->Get()->ID;
}

bool TechnoTypeExt::ExtData::IsGenericPrerequisite() const
{
	if (this->GenericPrerequisite.empty())
	{
		bool isGeneric = false;
		for (auto const& Prereq : GenericPrerequisite::Array) {
			if (Prereq->Alternates.Contains(this->OwnerObject())) {
				isGeneric = true;
				break;
			}
		}

		this->GenericPrerequisite = isGeneric;
	}

	return this->GenericPrerequisite;
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
	char tempBuffer[0x40];
	char tempBufferFLH[0x40];

	bool parseMultiWeapons = pThis->TurretCount > 0 && pThis->WeaponCount > 0;
	auto weaponCount = parseMultiWeapons ? pThis->WeaponCount : 2;
	nFLH.resize(weaponCount);
	nEFlh.resize(weaponCount);

	for (int i = 0; i < weaponCount; i++)
	{
		for (int j = 0; j < INT_MAX; j++)
		{
			IMPL_SNPRNINTF(tempBuffer, sizeof(tempBuffer), "%sWeapon%d", pPrefixTag, i + 1);
			auto prefix = parseMultiWeapons ? tempBuffer : i > 0 ? "%sSecondaryFire" : "%sPrimaryFire";
			IMPL_SNPRNINTF(tempBuffer, sizeof(tempBuffer), prefix, pPrefixTag);

			IMPL_SNPRNINTF(tempBufferFLH, sizeof(tempBufferFLH), "%sFLH.Burst%d", tempBuffer, j);
			Nullable<CoordStruct> FLH { };
			FLH.Read(exArtINI, pArtSection, tempBufferFLH);

			IMPL_SNPRNINTF(tempBufferFLH, sizeof(tempBufferFLH), "Elite%sFLH.Burst%d", tempBuffer, j);
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
	char tempBuffer[0x40];
	IMPL_SNPRNINTF(tempBuffer, sizeof(tempBuffer), "%sFLH", pFlag);
	nFlh.Read(exArtINI, pArtSection, tempBuffer);
	IMPL_SNPRNINTF(tempBuffer, sizeof(tempBuffer), "Elite%sFLH", pFlag);
	nEFlh.Read(exArtINI, pArtSection, tempBuffer);
}

void TechnoTypeExt::ExtData::LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr)
{
	auto pThis = this->Get();
	const auto pArtIni = &CCINIClass::INI_Art();
	const char* pSection = pThis->ID;
	const char* pArtSection = pThis->ImageFile;

	if (pINI->GetSection(pSection))
	{
		INI_EX exINI(pINI);

		this->Survivors_PassengerChance.Read(exINI, pSection, "Survivor.%sPassengerChance");
		this->HealthBar_Hide.Read(exINI, pSection, "HealthBar.Hide");
		this->UIDescription.Read(exINI, pSection, "UIDescription");
		this->LowSelectionPriority.Read(exINI, pSection, "LowSelectionPriority");
		this->MindControlRangeLimit.Read(exINI, pSection, "MindControlRangeLimit");

		this->Phobos_EliteAbilities.Read(exINI, pSection, GameStrings::EliteAbilities(), EnumFunctions::PhobosAbilityType_ToStrings);
		this->Phobos_VeteranAbilities.Read(exINI, pSection, GameStrings::VeteranAbilities(), EnumFunctions::PhobosAbilityType_ToStrings);

		this->E_ImmuneToType.Read(exINI, pSection, "EliteImmuneTo");
		this->V_ImmuneToType.Read(exINI, pSection, "VeteranImmuneTo");
		this->R_ImmuneToType.Read(exINI, pSection, "RookieImmuneTo");

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

		this->ShieldType.Read(exINI, pSection, "ShieldType");
		this->CameoPriority.Read(exINI, pSection, "CameoPriority");

		this->WarpOut.Read(exINI, pSection, "%s.WarpOut");
		this->WarpIn.Read(exINI, pSection, "%s.WarpIn");
		this->WarpAway.Read(exINI, pSection, "%s.WarpAway");
		this->ChronoTrigger.Read(exINI, pSection, "%s.ChronoTrigger");
		this->ChronoDistanceFactor.Read(exINI, pSection, "%s.ChronoDistanceFactor");
		this->ChronoMinimumDelay.Read(exINI, pSection, "%s.ChronoMinimumDelay");
		this->ChronoRangeMinimum.Read(exINI, pSection, "%s.ChronoRangeMinimum");
		this->ChronoDelay.Read(exINI, pSection, "%s.ChronoDelay");

		this->WarpInWeapon.Read(exINI, pSection, "%s.WarpInWeapon");
		this->WarpInMinRangeWeapon.Read(exINI, pSection, "%s.WarpInMinRangeWeapon");
		this->WarpOutWeapon.Read(exINI, pSection, "%s.WarpOutWeapon");
		this->WarpInWeapon_UseDistanceAsDamage.Read(exINI, pSection, "%s.WarpInWeapon.UseDistanceAsDamage");

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
		this->DrawInsignia.Read(exINI, pSection, "Insignia.Show");
		this->Insignia.Read(exINI, pSection, "Insignia.%s");
		this->InsigniaFrame.Read(exINI, pSection, "InsigniaFrame.%s");
		this->Insignia_ShowEnemy.Read(exINI, pSection, "Insignia.ShowEnemy");
		this->InsigniaFrames.Read(exINI, pSection, "InsigniaFrames");
		this->InsigniaDrawOffset.Read(exINI, pSection, "Insignia.DrawOffset");
		this->InitialStrength_Cloning.Read(exINI, pSection, "InitialStrength.Cloning");

		this->SHP_SelectBrdSHP.Read(exINI, pSection, "SelectBrd.SHP");
		this->SHP_SelectBrdPAL.Read(exINI, pSection, "SelectBrd.PAL");
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
		this->HealthBarSHP_Palette.Read(exINI, pSection, "HealthBarSHP.Palette");
		this->HealthBarSHP_PointOffset.Read(exINI, pSection, "HealthBarSHP.Point2DOffset");
		this->HealthbarRemap.Read(exINI, pSection, "HealthBarSHP.Remap");

		this->PipShapes02.Read(exINI, pSection, "PipShapes.Foot");
		this->PipGarrison.Read(exINI, pSection, "PipShapes.Garrison");
		this->PipGarrison_FrameIndex.Read(exINI, pSection, "PipShapes.GarrisonFrameIndex");
		this->PipGarrison_Palette.Read(exINI, pSection, "PipShapes.GarrisonPalette");
		this->PipShapes01.Read(exINI, pSection, "PipShapes.Building");

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
		this->GClock_Palette.Read(exINI, pSection, "GClock.Palette");

		this->ROF_Random.Read(exINI, pSection, "ROF.AddRandom");
		this->Rof_RandomMinMax.Read(exINI, pSection, "ROF.RandomMinMax");

		this->CreateSound_Enable.Read(exINI, pSection, "CreateSound.Enable");

		this->Eva_Complete.Read(exINI, pSection, "EVA.Complete");

		if (exINI.ReadString(pSection, "VoiceCreated")) {
			this->VoiceCreate = VocClass::FindIndexById(exINI.c_str());
		} else {
			this->VoiceCreate.Read(exINI, pSection, "VoiceCreate");
		}

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

		this->VirtualUnit.Read(exINI, pSection, "VirtualUnit");
		this->MyExtraFireData.ReadRules(exINI, pSection);
		this->MyGiftBoxData.Read(exINI, pSection);
		//this->MyJJData.Read(exINI, pSection);
		this->MyPassangersData.Read(exINI, pSection);
		this->MySpawnSupportDatas.Read(exINI, pSection);
		this->DamageSelfData.Read(exINI, pSection);

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
		this->ImmuneToBerserk.Read(exINI, pSection, "ImmuneToBerserk");
		this->Berzerk_Modifier.Read(exINI, pSection, "Berzerk.Modifier");
		this->IgnoreToProtect.Read(exINI, pSection, "ToProtect.Ignore");
		this->TargetLaser_Time.Read(exINI, pSection, "TargetLaser.Time");
		this->TargetLaser_WeaponIdx.Read(exINI, pSection, "TargetLaser.WeaponIndexes");
		this->AdjustCrushProperties();

		this->ConsideredNaval.Read(exINI, pSection, "ConsideredNaval");
		this->ConsideredVehicle.Read(exINI, pSection, "ConsideredVehicle");

#pragma region Prereq

		//TODO : PrerequisiteOverride

		this->Prerequisite_RequiredTheaters.Read(exINI, pSection, "Prerequisite.RequiredTheaters");

		// subtract the default list, get tag (not less than 0), add one back
		const auto nRead = pINI->ReadInteger(pSection, "Prerequisite.Lists", static_cast<int>(this->Prerequisite.size()) - 1);

		this->Prerequisite.resize(static_cast<size_t>(MaxImpl(nRead, 0) + 1));
		GenericPrerequisite::Parse(pINI, pSection, "Prerequisite", this->Prerequisite[0]);

		char flag[256];
		for (auto i = 0u; i < this->Prerequisite.size(); ++i) {
			IMPL_SNPRNINTF(flag, sizeof(flag), "Prerequisite.List%u", i);
			GenericPrerequisite::Parse(pINI, pSection, flag, this->Prerequisite[i]);
		}

		// Prerequisite.Negative with Generic Prerequistes support
		GenericPrerequisite::Parse(pINI, pSection, "Prerequisite.Negative", this->Prerequisite_Negative);

#pragma endregion Prereq

		char tempBuffer[0x40];

		if (pThis->Gunner && this->Insignia_Weapon.empty())
		{
			const int weaponCount = pThis->WeaponCount;
			this->Insignia_Weapon.resize(weaponCount);

			for (int i = 0; i < weaponCount; i++)
			{
				auto& Data = this->Insignia_Weapon[i];
				IMPL_SNPRNINTF(tempBuffer, sizeof(tempBuffer), "Insignia.Weapon%d.%s", i + 1, "%s");
				Data.Shapes.Read(exINI, pSection, tempBuffer);

				IMPL_SNPRNINTF(tempBuffer, sizeof(tempBuffer), "InsigniaFrame.Weapon%d.%s", i + 1, "%s");
				Data.Frame.Read(exINI, pSection, tempBuffer);

				IMPL_SNPRNINTF(tempBuffer, sizeof(tempBuffer), "InsigniaFrames.Weapon%d", i + 1);
				Data.Frames.Read(exINI, pSection, tempBuffer);
			}
		}

		this->AttachedEffect.Read(exINI);
		this->NoAmmoEffectAnim.Read(exINI, pSection, "NoAmmoEffectAnim", true);
		this->AttackFriendlies_WeaponIdx.Read(exINI, pSection, "AttackFriendlies.WeaponIdx");
		this->PipScaleIndex.Read(exINI, pSection, "PipScaleIndex");
		this->AmmoPip.Read(exINI, pSection, "AmmoPip");
		this->AmmoPip_Offset.Read(exINI, pSection, "AmmoPipOffset");
		this->AmmoPip_Palette.Read(exINI, pSection, "AmmoPipPalette");

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


		this->Promote_Elite_Flash.Read(exINI, pSection, "Promote.EliteFlash");
		this->Promote_Vet_Flash.Read(exINI, pSection, "Promote.VeteranFlash");

		this->Promote_Elite_Sound.Read(exINI, pSection, "Promote.EliteSound");
		this->Promote_Vet_Sound.Read(exINI, pSection, "Promote.VeteranSound");

		this->Promote_Vet_Type.Read(exINI, pSection, "Promote.VeteranType");
		this->Promote_Elite_Type.Read(exINI, pSection, "Promote.EliteType");

		this->Promote_Vet_Anim.Read(exINI, pSection, "Promote.VeteranAnim");
		this->Promote_Elite_Anim.Read(exINI, pSection, "Promote.EliteAnim");

		this->Promote_Vet_Exp.Read(exINI, pSection, "Promote.VeteranExperience");
		this->Promote_Elite_Exp.Read(exINI, pSection, "Promote.EliteExperience");

		this->DeployDir.Read(exINI, pSection, "DeployDir");

		this->PassengersWhitelist.Read(exINI, pSection, "Passengers.Allowed");
		this->PassengersBlacklist.Read(exINI, pSection, "Passengers.Disallowed");

		this->NoManualUnload.Read(exINI, pSection, "NoManualUnload");
		this->NoSelfGuardArea.Read(exINI, pSection, "NoSelfGuardArea");
		this->NoManualFire.Read(exINI, pSection, "NoManualFire");
		this->NoManualEnter.Read(exINI, pSection, "NoManualEnter");
		this->NoManualEject.Read(exINI, pSection, "NoManualEject");

		//this->Crashable.Read(exINI, pSection, "Crashable");

		this->Passengers_BySize.Read(exINI, pSection, "Passengers.BySize");
		this->Convert_Deploy.Read(exINI, pSection, "Convert.Deploy");
		this->Convert_Script.Read(exINI, pSection, "Convert.Script");
		this->Harvester_LongScan.Read(exINI, pSection, "Harvester.LongScan");
		this->Harvester_ShortScan.Read(exINI, pSection, "Harvester.ShortScan");
		this->Harvester_ScanCorrection.Read(exINI, pSection, "Harvester.ScanCorrection");

		this->Harvester_TooFarDistance.Read(exINI, pSection, "Harvester.TooFarDistance");
		this->Harvester_KickDelay.Read(exINI, pSection, "Harvester.KickDelay");

		this->TurretRot.Read(exINI, pSection, "TurretROT");

		this->FallRate_Parachute.Read(exINI, pSection, "FallRate.Parachute");
		this->FallRate_NoParachute.Read(exINI, pSection, "FallRate.NoParachute");
		this->FallRate_ParachuteMax.Read(exINI, pSection, "FallRate.ParachuteMax");
		this->FallRate_NoParachuteMax.Read(exINI, pSection, "FallRate.NoParachuteMax");

		// Dont need to be SL ?
		//const bool Eligible = pThis->TurretCount >= TechnoTypeClass::MaxWeapons;
		//TechnoTypeExt::InitImageData(this->BarrelImageData, Eligible ? (pThis->TurretCount - TechnoTypeClass::MaxWeapons) + 1 : 0);
		//TechnoTypeExt::InitImageData(this->TurretImageData, Eligible ? (pThis->TurretCount - TechnoTypeClass::MaxWeapons) + 1 : 0);

		this->NoShadowSpawnAlt.Read(exINI, pSection, "NoShadowSpawnAlt");

		this->OmniCrusher_Aggressive.Read(exINI, pSection, "OmniCrusher.Aggressive");
		this->CrusherDecloak.Read(exINI, pSection, "Crusher.Decloak");
		this->Crusher_SupressLostEva.Read(exINI, pSection, "Crusher.SuppressLostEVA");
		this->CrushRange.Read(exINI, pSection, "Crusher.Range.%s");

		this->CrushFireDeathWeapon.Read(exINI, pSection, "CrushFireDeathWeaponChance.%s");
		this->CrushDamage.Read(exINI, pSection, "CrushDamage.%s");
		this->CrushDamageWarhead.Read(exINI, pSection, "CrushDamage.Warhead");

		this->DigInSound.Read(exINI, pSection, "DigInSound");
		this->DigOutSound.Read(exINI, pSection, "DigOutSound");
		this->DigInAnim.Read(exINI, pSection, "DigInAnim");
		this->DigOutAnim.Read(exINI, pSection, "DigOutAnim");

		this->EVA_UnitLost.Read(exINI, pSection, "EVA.Lost");

		this->BuildTime_Speed.Read(exINI, pSection, "BuildTime.Speed");
		this->BuildTime_Cost.Read(exINI, pSection, "BuildTime.Cost");
		this->BuildTime_LowPowerPenalty.Read(exINI, pSection, "BuildTime.LowPowerPenalty");
		this->BuildTime_MinLowPower.Read(exINI, pSection, "BuildTime.MinLowPower");
		this->BuildTime_MaxLowPower.Read(exINI, pSection, "BuildTime.MaxLowPower");
		this->BuildTime_MultipleFactory.Read(exINI, pSection, "BuildTime.MultipleFactory");

		this->CloakStages.Read(exINI, pSection, "Cloakable.Stages");
		this->DamageSparks.Read(exINI, pSection, "DamageSparks");

		this->ParticleSystems_DamageSmoke.Read(exINI, pSection, "DamageSmokeParticleSystems");
		this->ParticleSystems_DamageSparks.Read(exINI, pSection, "DamageSparksParticleSystems");
		// issue #896235: cyclic gattling
		this->GattlingCyclic.Read(exINI, pSection, "Gattling.Cycle");
		this->CloakSound.Read(exINI, pSection, "CloakSound");
		this->DecloakSound.Read(exINI, pSection, "DecloakSound");
		this->VoiceRepair.Read(exINI, pSection, "VoiceIFVRepair");
		this->ReloadAmount.Read(exINI, pSection, "ReloadAmount");
		this->EmptyReloadAmount.Read(exINI, pSection, "EmptyReloadAmount");

		this->TiberiumProof.Read(exINI, pSection, "TiberiumProof");
		this->TiberiumSpill.Read(exINI, pSection, "TiberiumSpill");
		this->TiberiumRemains.Read(exINI, pSection, "TiberiumRemains");
		this->TiberiumTransmogrify.Read(exINI, pSection, "TiberiumTransmogrify");

		// sensors
		this->SensorArray_Warn.Read(exINI, pSection, "SensorArray.Warn");

		this->IronCurtain_Modifier.Read(exINI, pSection, "IronCurtain.Modifier");
		this->ForceShield_Modifier.Read(exINI, pSection, "ForceShield.Modifier");
		this->Survivors_PilotCount.Read(exINI, pSection, "Survivor.Pilots");
		// berserking options
		this->BerserkROFMultiplier.Read(exINI, pSection, "Berserk.ROFMultiplier");

		// refinery and storage
		this->Refinery_UseStorage.Read(exINI, pSection, "Refinery.UseStorage");
		this->VHPscan_Value.Read(exINI, pSection, "VHPScan.Value");

		this->SelfHealing_Rate.Read(exINI, pSection, "SelfHealing.Rate");
		this->SelfHealing_Amount.Read(exINI, pSection, "SelfHealing.%sAmount");
		this->SelfHealing_Max.Read(exINI, pSection, "SelfHealing.%sMax");
		this->SelfHealing_CombatDelay.Read(exINI, pSection, "SelfHealing.%sCombatDelay");

		this->CloakAllowed.Read(exINI, pSection, "Cloakable.Allowed");

		this->InitialPayload_Types.Read(exINI, pSection, "InitialPayload.Types");
		this->InitialPayload_Nums.Read(exINI, pSection, "InitialPayload.Nums");
		this->InitialPayload_Vet.Read(exINI, pSection, "InitialPayload.Ranks");
		this->InitialPayload_AddToTransportTeam.Read(exINI, pSection, "InitialPayload.AddToTransportTeam");

		this->HasSpotlight.Read(exINI, pSection, "HasSpotlight");
		this->Crew_TechnicianChance.Read(exINI, pSection, "Crew.TechnicianChance");
		this->Crew_EngineerChance.Read(exINI, pSection, "Crew.EngineerChance");
		this->Saboteur.Read(exINI, pSection, "Saboteur");

		this->RadialIndicatorRadius.Read(exINI, pSection, "RadialIndicatorRadius");
		this->RadialIndicatorColor.Read(exINI, pSection, "RadialIndicatorColor");

		this->GapRadiusInCells.Read(exINI, pSection, "GapRadiusInCells");
		this->SuperGapRadiusInCells.Read(exINI, pSection, "SuperGapRadiusInCells");

		this->Survivors_PilotChance.Read(exINI, pSection, "Survivor.%sPilotChance");
		this->HijackerOneTime.Read(exINI, pSection, "VehicleThief.OneTime");
		this->HijackerKillPilots.Read(exINI, pSection, "VehicleThief.KillPilots");
		this->HijackerEnterSound.Read(exINI, pSection, "VehicleThief.EnterSound");
		this->HijackerLeaveSound.Read(exINI, pSection, "VehicleThief.LeaveSound");
		this->HijackerBreakMindControl.Read(exINI, pSection, "VehicleThief.BreakMindControl");
		this->HijackerAllowed.Read(exINI, pSection, "VehicleThief.Allowed");
		this->Cursor_Deploy.Read(exINI, pSection, "Cursor.Deploy");
		this->Cursor_NoDeploy.Read(exINI, pSection, "Cursor.NoDeploy");
		this->Cursor_Enter.Read(exINI, pSection, "Cursor.Enter");
		this->Cursor_NoEnter.Read(exINI, pSection, "Cursor.NoEnter");
		this->Cursor_Move.Read(exINI, pSection, "Cursor.Move");
		this->Cursor_NoMove.Read(exINI, pSection, "Cursor.NoMove");

		// #680, 1362
		this->ImmuneToAbduction.Read(exINI, pSection, "ImmuneToAbduction");
		this->UseROFAsBurstDelays.Read(exINI, pSection, "UseROFAsBurstDelays");
		this->Chronoshift_Crushable.Read(exINI, pSection, "Chronoshift.Crushable");

		this->CanBeReversed.Read(exINI, pSection, "CanBeReversed");
		this->ReversedAs.Read(exINI, pSection, "ReversedAs");
		this->AssaulterLevel.Read(exINI, pSection, "Assaulter.Level");

		// smoke when damaged
		this->SmokeAnim.Read(exINI, pSection, "Smoke.Anim");
		this->SmokeChanceRed.Read(exINI, pSection, "Smoke.ChanceRed");
		this->SmokeChanceDead.Read(exINI, pSection, "Smoke.ChanceDead");

		this->CarryallAllowed.Read(exINI, pSection, "Carryall.Allowed");
		this->CarryallSizeLimit.Read(exINI, pSection, "Carryall.SizeLimit");

		this->VoiceAirstrikeAttack.Read(exINI, pSection, "VoiceAirstrikeAttack");
		this->VoiceAirstrikeAbort.Read(exINI, pSection, "VoiceAirstrikeAbort");

		// note the wrong spelling of the tag for consistency
		this->CanPassiveAcquire_Guard.Read(exINI, pSection, "CanPassiveAquire.Guard");
		this->CanPassiveAcquire_Cloak.Read(exINI, pSection, "CanPassiveAquire.Cloak");

		this->CrashSpin.Read(exINI, pSection, "CrashSpin");
		this->AirRate.Read(exINI, pSection, "AirRate");
		this->Unsellable.Read(exINI, pSection, "Unsellable");
		this->CreateSound_afect.Read(exINI, pSection, "CreateSound.AffectOwner");

		this->Chronoshift_Allow.Read(exINI, pSection, "Chronoshift.Allow");
		this->Chronoshift_IsVehicle.Read(exINI, pSection, "Chronoshift.IsVehicle");

		this->SuppressorRange.Read(exINI, pSection, "SuppressorRange");
		this->AttractorRange.Read(exINI, pSection, "AttractorRange");
		this->FactoryPlant_Multiplier.Read(exINI, pSection, "FactoryPlant.Multiplier");
		this->MassSelectable.Read(exINI, pSection, "MassSelectable");

		this->TiltsWhenCrushes_Vehicles.Read(exINI, pSection, "TiltsWhenCrushes.Vehicles");
		this->TiltsWhenCrushes_Overlays.Read(exINI, pSection, "TiltsWhenCrushes.Overlays");
		this->CrushForwardTiltPerFrame.Read(exINI, pSection, "CrushForwardTiltPerFrame");
		this->CrushOverlayExtraForwardTilt.Read(exINI, pSection, "CrushOverlayExtraForwardTilt");
		this->CrushSlowdownMultiplier.Read(exINI, pSection, "CrushSlowdownMultiplier");

		this->AIIonCannonValue.Read(exINI, pSection, "AIIonCannonValue");
		this->ExtraPower_Amount.Read(exINI, pSection, "ExtraPower.Amount");
		this->CanDrive.Read(exINI, pSection, "CanDrive");

		if (exINI.ReadString(pSection, "Operator"))
		{ // try to read the flag
			this->Operators.clear();
			this->Operator_Any = (IS_SAME_STR_N(exINI.value(), "_ANY_")); // set whether this type accepts all operators
			if (!this->Operator_Any)
			{ // if not, find the specific operator it allows
				detail::parse_values(this->Operators, exINI, pSection, "Operator");
			}
		}

		this->AlwayDrawRadialIndicator.Read(exINI, pSection, "RadialIndicator.AlwaysDraw");
		this->ReloadRate.Read(exINI, pSection, "ReloadRate");
		this->CloakAnim.Read(exINI, pSection, "CloakAnim");
		this->DecloakAnim.Read(exINI, pSection, "DecloakAnim");
		this->Cloak_KickOutParasite.Read(exINI, pSection, "Cloak.KickOutParasite");

		// killer tags
		this->Bounty.Read(exINI, pSection, "Bounty");
		this->Bounty_Display.Read(exINI, pSection, "Bounty.Display");
		this->BountyAllow.Read(exINI, pSection, "Bounty.AffectTypes");
		this->BountyDissallow.Read(exINI, pSection, "Bounty.IgnoreTypes");
		this->BountyBonusmult.Read(exINI, pSection, "Bounty.%sBonusMult");
		this->Bounty_IgnoreEnablers.Read(exINI, pSection, "Bounty.IgnoreEnablers");

		// victim tags
		this->Bounty_Value.Read(exINI, pSection, "Bounty.%sValue");
		this->Bounty_Value_Option.Read(exINI, pSection, "Bounty.RewardOption");
		this->Bounty_Value_mult.Read(exINI, pSection, "Bounty.%sValueMult");

		this->DeathWeapon_CheckAmmo.Read(exINI, pSection, "DeathWeapon.CheckAmmo");
		this->Initial_DriverKilled.Read(exINI, pSection, "Initial.DriverKilled");

		this->VoiceCantDeploy.Read(exINI, pSection, "VoiceCantDeploy");
		this->DigitalDisplay_Disable.Read(exINI, pSection, "DigitalDisplay.Disable");
		this->DigitalDisplayTypes.Read(exINI, pSection, "DigitalDisplayTypes");

		//not fully working atm , disabled
		//this->DeployAnims.Read(exINI, pSection, "DeployingAnim");

		this->AmmoPip.Read(exINI, pSection, "AmmoPip");
		this->EmptyAmmoPip.Read(exINI, pSection, "EmptyAmmoPip");
		this->PipWrapAmmoPip.Read(exINI, pSection, "PipWrapAmmoPip");
		this->AmmoPipSize.Read(exINI, pSection, "AmmoPipSize");
		this->ProduceCashDisplay.Read(exINI, pSection, "ProduceCashDisplay");

		// drain settings
		this->Drain_Local.Read(exINI, pSection, "Drain.Local");
		this->Drain_Amount.Read(exINI, pSection, "Drain.Amount");

		this->FactoryOwners.Read(exINI, pSection, "FactoryOwners");
		this->FactoryOwners_Forbidden.Read(exINI, pSection, "FactoryOwners.Forbidden");
		this->FactoryOwners_HaveAllPlans.Read(exINI, pSection, "FactoryOwners.HaveAllPlans");
		this->FactoryOwners_HaveAllPlans.Read(exINI, pSection, "FactoryOwners.Permanent");
		this->FactoryOwners_HasAllPlans.Read(exINI, pSection, "FactoryOwners.HasAllPlans");

		this->HealthBar_Sections.Read(exINI, pSection, "HealthBar.Sections");
		this->HealthBar_Border.Read(exINI, pSection, "HealthBar.Border");
		this->HealthBar_BorderFrame.Read(exINI, pSection, "HealthBar.BorderFrame");
		this->HealthBar_BorderAdjust.Read(exINI, pSection, "HealthBar.BorderAdjust");

#pragma region AircraftOnly
		if (Is_AircraftType(pThis))
		{
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
			this->CustomMissilePreLauchAnim.Read(exINI, pSection, "Missile.PreLaunchAnim");
			this->CustomMissileTrailerAnim.Read(exINI, pSection, "Missile.TrailerAnim");
			this->CustomMissileTrailerSeparation.Read(exINI, pSection, "Missile.TrailerSeparation");
			this->CustomMissileWeapon.Read(exINI, pSection, "Missile.Weapon");
			this->CustomMissileEliteWeapon.Read(exINI, pSection, "Missile.EliteWeapon");

			this->AttackingAircraftSightRange.Read(exINI, pSection, "AttackingAircraftSightRange");
			this->CrashWeapon_s.Read(exINI, pSection, "Crash.Weapon", true);
			this->CrashWeapon.Read(exINI, pSection, "Crash.%sWeapon",nullptr , true);

			this->NoAirportBound_DisableRadioContact.Read(exINI, pSection, "NoAirportBound.DisableRadioContact");

			this->TakeOff_Anim.Read(exINI, pSection, "TakeOff.Anim");
			this->PoseDir.Read(exINI, pSection, GameStrings::PoseDir());
			this->Firing_IgnoreGravity.Read(exINI, pSection, "Firing.IgnoreGravity");
			this->CurleyShuffle.Read(exINI, pSection, "CurleyShuffle");

			//No code
			this->Aircraft_DecreaseAmmo.Read(exINI, pSection, "Firing.ReplaceFiringMode");

			// hunter seeker
			this->HunterSeekerDetonateProximity.Read(exINI, pSection, "HunterSeeker.DetonateProximity");
			this->HunterSeekerDescendProximity.Read(exINI, pSection, "HunterSeeker.DescendProximity");
			this->HunterSeekerAscentSpeed.Read(exINI, pSection, "HunterSeeker.AscentSpeed");
			this->HunterSeekerDescentSpeed.Read(exINI, pSection, "HunterSeeker.DescentSpeed");
			this->HunterSeekerEmergeSpeed.Read(exINI, pSection, "HunterSeeker.EmergeSpeed");
			this->HunterSeekerIgnore.Read(exINI, pSection, "HunterSeeker.Ignore");

			this->MissileHoming.Read(exINI, pSection, "Missile.Homing");
			this->MyDiveData.Read(exINI, pSection);
			this->MyPutData.Read(exINI, pSection);
			this->MyFighterData.Read(exINI, pSection, pThis);
		}
#pragma endregion

		//this->ShadowIndices.Read(exINI, pSection, "ShadowIndices");

	}

	// Art tags
	if (pArtIni && pArtIni->GetSection(pArtSection))
	{
		INI_EX exArtINI(pArtIni);

		this->TurretOffset.Read(exArtINI, pArtSection, GameStrings::TurretOffset());

		if (!this->TurretOffset.isset())
		{
			//put ddedfault single value inside
			PartialVector3D<int> nRes { pThis->TurretOffset , 0 ,0 , 1 };
			this->TurretOffset = nRes;
		}

		this->TurretShadow.Read(exArtINI, pArtSection, "TurretShadow");
		this->ShadowIndices.Read(exArtINI, pArtSection, "ShadowIndices");

		char tempBuffer[0x40];
		char HitCoord_tempBuffer[0x20];
		char alternateFLHbuffer[0x40];

		for (size_t i = 0; ; ++i)
		{
			NullableIdx<LaserTrailTypeClass> trail;
			IMPL_SNPRNINTF(tempBuffer, sizeof(tempBuffer), "LaserTrail%d.Type", i);
			trail.Read(exArtINI, pArtSection, tempBuffer);

			if (!trail.isset())
				break;

			Valueable<CoordStruct> flh;
			IMPL_SNPRNINTF(tempBuffer, sizeof(tempBuffer), "LaserTrail%d.FLH", i);
			flh.Read(exArtINI, pArtSection, tempBuffer);

			Valueable<bool> isOnTurret;
			IMPL_SNPRNINTF(tempBuffer, sizeof(tempBuffer), "LaserTrail%d.IsOnTurret", i);
			isOnTurret.Read(exArtINI, pArtSection, tempBuffer);

			this->LaserTrailData.emplace_back(trail.Get(), flh.Get(), isOnTurret.Get());
		}

		for (size_t i = 5; ; ++i)
		{
			Nullable<CoordStruct> alternateFLH;
			IMPL_SNPRNINTF(alternateFLHbuffer, sizeof(alternateFLHbuffer), "AlternateFLH%u", i);
			alternateFLH.Read(exArtINI, pArtSection, alternateFLHbuffer);

			if (!alternateFLH.isset())
				break;

			this->AlternateFLHs.push_back(alternateFLH.Get());
		}

		for (size_t i = 0; ; ++i)
		{
			Nullable<CoordStruct> nHitBuff;
			IMPL_SNPRNINTF(HitCoord_tempBuffer, sizeof(HitCoord_tempBuffer), "HitCoordOffset%d", i);
			nHitBuff.Read(exArtINI, pArtSection, HitCoord_tempBuffer);

			if (!nHitBuff.isset())
				break;

			this->HitCoordOffset.push_back(nHitBuff);
		}

		this->HitCoordOffset_Random.Read(exArtINI, pArtSection, "HitCoordOffset.Random");

		this->Spawner_SpawnOffsets.Read(exArtINI, pArtSection, "SpawnOffset");
		this->Spawner_SpawnOffsets_OverrideWeaponFLH.Read(exArtINI, pArtSection, "SpawnOffsetOverrideFLH");
		this->ShadowScale.Read(exArtINI, pArtSection, "ShadowScale");

		//LineTrailData::LoadFromINI(this->LineTrailData, exArtINI, pArtSection);

		TechnoTypeExt::GetBurstFLHs(pThis, exArtINI, pArtSection, WeaponBurstFLHs, EliteWeaponBurstFLHs, "");
		TechnoTypeExt::GetBurstFLHs(pThis, exArtINI, pArtSection, DeployedWeaponBurstFLHs, EliteDeployedWeaponBurstFLHs, "Deployed");
		TechnoTypeExt::GetBurstFLHs(pThis, exArtINI, pArtSection, CrouchedWeaponBurstFLHs, EliteCrouchedWeaponBurstFLHs, "Prone");

		TechnoTypeExt::GetFLH(exArtINI, pArtSection, PronePrimaryFireFLH, E_PronePrimaryFireFLH, "PronePrimaryFire");
		TechnoTypeExt::GetFLH(exArtINI, pArtSection, ProneSecondaryFireFLH, E_ProneSecondaryFireFLH, "ProneSecondaryFire");
		TechnoTypeExt::GetFLH(exArtINI, pArtSection, DeployedPrimaryFireFLH, E_DeployedPrimaryFireFLH, "DeployedPrimaryFire");
		TechnoTypeExt::GetFLH(exArtINI, pArtSection, DeployedSecondaryFireFLH, E_DeployedSecondaryFireFLH, "DeployedSecondaryFire");

		TechnoTypeExt::GetFLH(exArtINI, pArtSection, PrimaryCrawlFLH, Elite_PrimaryCrawlFLH, "PrimaryCrawling");
		TechnoTypeExt::GetFLH(exArtINI, pArtSection, SecondaryCrawlFLH, Elite_SecondaryCrawlFLH, "SecondaryCrawling");

		this->MyExtraFireData.ReadArt(exArtINI, pArtSection);
		this->MySpawnSupportFLH.Read(exArtINI, pArtSection);
		this->Trails.Read(exArtINI, pArtSection, true);

	}
}

void TechnoTypeExt::ExtData::LoadFromINIFile_Aircraft(CCINIClass* pINI)
{
	//auto pThis = Get();
	//const char* pSection = pThis->ID;
	//INI_EX exINI(pINI);
}

void TechnoTypeExt::ExtData::LoadFromINIFile_EvaluateSomeVariables(CCINIClass* pINI)
{
	//auto pThis = Get();
	//const char* pSection = pThis->ID;
	//INI_EX exINI(pINI);

}

void ImageStatusses::ReadVoxel(ImageStatusses& arg0, const char* const nKey, bool a4)
{
	char buffer[0x60];
	IMPL_SNPRNINTF(buffer, sizeof(buffer), "%s.VXL", nKey);
	CCFileClass CCFileV { buffer };

	if (CCFileV.Exists())
	{
		MotLib* pLoadedHVA = nullptr;
		VoxLib* pLoadedVXL = GameCreate<VoxLib>(&CCFileV, false);
		IMPL_SNPRNINTF(buffer, sizeof(buffer), "%s.HVA", nKey);
		CCFileClass  CCFileH { buffer };

		if (CCFileH.Open(FileAccessMode::Read))
		{
			pLoadedHVA = GameCreate<MotLib>(&CCFileH);
		}

		CCFileH.Close();
		CCFileH.~CCFileClass();
		if (!pLoadedHVA || pLoadedVXL->LoadFailed || pLoadedHVA->LoadedFailed)
		{
			arg0 = { {nullptr , nullptr} , false };

			GameDelete<true, true>(pLoadedHVA);
			GameDelete<true, false>(pLoadedVXL);
		}
		else
		{
			pLoadedHVA->Scale(pLoadedVXL->TailerData[pLoadedVXL->HeaderData->limb_number].HVAMultiplier);
			arg0 = { {pLoadedVXL , pLoadedHVA }, true };
		}
	}
	else
	{
		arg0 = { {nullptr , nullptr} , a4 };
	}

	CCFileV.Close();
	CCFileV.~CCFileClass();
}

void TechnoTypeExt::InitImageData(ImageVector& nVec, size_t size)
{
	if (size <= 0)
		return;

	nVec.resize(size);
}

void TechnoTypeExt::ClearImageData(ImageVector& nVec, size_t pos)
{
	if (nVec.empty())
		return;

	if (pos >= 0 && pos < nVec.size())
	{
		for (auto i = (nVec.begin() + pos); i != nVec.end(); ++i)
		{
			GameDelete<true, true>((*i).HVA);
			(*i).HVA = nullptr;
			GameDelete<true, true>((*i).VXL);
			(*i).VXL = nullptr;
		}
	}
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

bool TechnoTypeExt::PassangersAllowed(TechnoTypeClass* pThis, TechnoTypeClass* pPassanger)
{
	const auto pExt = TechnoTypeExt::ExtMap.Find(pThis);

	if (!pExt->PassengersWhitelist.Eligible(pPassanger))
		return false;

	if (!pExt->PassengersBlacklist.empty() && pExt->PassengersBlacklist.Contains(pPassanger))
		return false;

	return true;
}

template <typename T>
void TechnoTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Initialized)
		.Process(this->HealthBar_Hide)
		.Process(this->UIDescription)
		.Process(this->LowSelectionPriority)
		.Process(this->MindControlRangeLimit)
		.Process(this->Phobos_EliteAbilities)
		.Process(this->Phobos_VeteranAbilities)
		.Process(this->E_ImmuneToType)
		.Process(this->V_ImmuneToType)
		.Process(this->R_ImmuneToType)
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
		.Process(this->TurretShadow)
		.Process(this->ShadowIndices)
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

		.Process(this->DrawInsignia)
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
		.Process(this->CustomMissilePreLauchAnim)
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
		.Process(this->DeathWeapon_CheckAmmo)
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
		.Process(this->Prerequisite_Lists)
		.Process(this->Prerequisite_Negative)
		.Process(this->ConsideredNaval)
		.Process(this->ConsideredVehicle)


		.Process(this->VirtualUnit)

		.Process(this->PrimaryCrawlFLH)
		.Process(this->Elite_PrimaryCrawlFLH)
		.Process(this->SecondaryCrawlFLH)
		.Process(this->Elite_SecondaryCrawlFLH)

		.Process(this->BountyAllow)
		.Process(this->BountyDissallow)
		.Process(this->BountyBonusmult)
		.Process(this->Bounty_IgnoreEnablers)
		.Process(this->MissileHoming)

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
		.Process(this->Promote_Vet_Anim)
		.Process(this->Promote_Elite_Anim)
		.Process(this->Promote_Vet_Exp)
		.Process(this->Promote_Elite_Exp)
		.Process(this->DeployDir)
		.Process(this->PassengersWhitelist)
		.Process(this->PassengersBlacklist)
		.Process(this->NoManualUnload)
		.Process(this->NoSelfGuardArea)
		.Process(this->NoManualFire)
		.Process(this->NoManualEnter)
		.Process(this->NoManualEject)
		.Process(this->Passengers_BySize)
		//.Process(this->Crashable)
		.Process(this->Convert_Deploy)
		.Process(this->Convert_Script)
		.Process(this->Harvester_LongScan)
		.Process(this->Harvester_ShortScan)
		.Process(this->Harvester_ScanCorrection)

		.Process(this->Harvester_TooFarDistance)
		.Process(this->Harvester_KickDelay)

		.Process(this->TurretRot)

		.Process(this->WaterImage)

		.Process(this->FallRate_Parachute)
		.Process(this->FallRate_NoParachute)
		.Process(this->FallRate_ParachuteMax)
		.Process(this->FallRate_NoParachuteMax)

		.Process(this->BarrelImageData)
		.Process(this->TurretImageData)
		.Process(this->SpawnAltData)
		.Process(this->WeaponUINameX)
		.Process(this->NoShadowSpawnAlt)

		.Process(this->AdditionalWeaponDatas)
		.Process(this->AdditionalEliteWeaponDatas)
		.Process(this->AdditionalTurrentWeapon)
		.Process(this->OmniCrusher_Aggressive)
		.Process(this->CrusherDecloak)
		.Process(this->Crusher_SupressLostEva)
		.Process(this->CrushFireDeathWeapon)
		.Process(this->CrushDamage)
		.Process(this->CrushDamageWarhead)
		.Process(this->CrushRange)
		.Process(this->DigInSound)
		.Process(this->DigOutSound)
		.Process(this->DigInAnim)
		.Process(this->DigOutAnim)
		.Process(this->EVA_UnitLost)

		.Process(this->BuildTime_Speed)
		.Process(this->BuildTime_Cost)
		.Process(this->BuildTime_LowPowerPenalty)
		.Process(this->BuildTime_MinLowPower)
		.Process(this->BuildTime_MaxLowPower)
		.Process(this->BuildTime_MultipleFactory)
		.Process(this->CloakStages)
		.Process(this->DamageSparks)
		.Process(this->ParticleSystems_DamageSmoke)
		.Process(this->ParticleSystems_DamageSparks)
		.Process(this->GattlingCyclic)
		.Process(this->CloakSound)
		.Process(this->DecloakSound)
		.Process(this->VoiceRepair)
		.Process(this->ReloadAmount)
		.Process(this->EmptyReloadAmount)
		.Process(this->TiberiumProof)
		.Process(this->TiberiumSpill)
		.Process(this->TiberiumRemains)
		.Process(this->TiberiumTransmogrify)
		.Process(this->SensorArray_Warn)
		.Process(this->IronCurtain_Modifier)
		.Process(this->ForceShield_Modifier)
		.Process(this->Survivors_PilotCount)
		.Process(this->BerserkROFMultiplier)
		.Process(this->Refinery_UseStorage)
		.Process(this->VHPscan_Value)

		.Process(this->SelfHealing_Rate)
		.Process(this->SelfHealing_Amount)
		.Process(this->SelfHealing_Max)
		.Process(this->SelfHealing_CombatDelay)
		.Process(this->Bounty)
		.Process(this->HasSpotlight)
		.Process(this->CloakAllowed)
		.Process(this->InitialPayload_Types)
		.Process(this->InitialPayload_Nums)
		.Process(this->InitialPayload_Vet)
		.Process(this->InitialPayload_AddToTransportTeam)

		.Process(this->AlternateTheaterArt)

		.Process(this->HijackerOneTime)
		.Process(this->HijackerKillPilots)
		.Process(this->HijackerEnterSound)
		.Process(this->HijackerLeaveSound)
		.Process(this->HijackerBreakMindControl)
		.Process(this->HijackerAllowed)
		.Process(this->Survivors_PilotChance)
		.Process(this->Crew_TechnicianChance)
		.Process(this->Crew_EngineerChance)

		.Process(this->Saboteur)

		.Process(this->Cursor_Deploy)
		.Process(this->Cursor_NoDeploy)
		.Process(this->Cursor_Enter)
		.Process(this->Cursor_NoEnter)
		.Process(this->Cursor_Move)
		.Process(this->Cursor_NoMove)

		.Process(this->ImmuneToAbduction)
		.Process(this->UseROFAsBurstDelays)
		.Process(this->Chronoshift_Crushable)
		.Process(this->CanBeReversed)
		.Process(this->ReversedAs)
		.Process(this->AssaulterLevel)
		.Process(this->RadialIndicatorRadius)
		.Process(this->RadialIndicatorColor)
		.Process(this->GapRadiusInCells)
		.Process(this->SuperGapRadiusInCells)
		.Process(this->SmokeChanceRed)
		.Process(this->SmokeChanceDead)
		.Process(this->SmokeAnim)
		.Process(this->CarryallAllowed)
		.Process(this->CarryallSizeLimit)
		.Process(this->VoiceAirstrikeAttack)
		.Process(this->VoiceAirstrikeAbort)
		.Process(this->HunterSeekerDetonateProximity)
		.Process(this->HunterSeekerDescendProximity)
		.Process(this->HunterSeekerAscentSpeed)
		.Process(this->HunterSeekerDescentSpeed)
		.Process(this->HunterSeekerEmergeSpeed)
		.Process(this->HunterSeekerIgnore)

		.Process(this->Bounty_Display)
		.Process(this->Bounty_Value)
		.Process(this->Bounty_Value_Option)
		.Process(this->Bounty_Value_mult)

		.Process(this->CanPassiveAcquire_Guard)
		.Process(this->CanPassiveAcquire_Cloak)
		.Process(this->CrashSpin)
		.Process(this->AirRate)
		.Process(this->Unsellable)
		.Process(this->CreateSound_afect)
		.Process(this->Chronoshift_Allow)
		.Process(this->Chronoshift_IsVehicle)
		.Process(this->SuppressorRange)
		.Process(this->AttractorRange)
		.Process(this->FactoryPlant_Multiplier)
		.Process(this->MassSelectable)
		.Process(this->TiltsWhenCrushes_Vehicles)
		.Process(this->TiltsWhenCrushes_Overlays)
		.Process(this->CrushForwardTiltPerFrame)
		.Process(this->CrushOverlayExtraForwardTilt)
		.Process(this->CrushSlowdownMultiplier)
		.Process(this->AIIonCannonValue)
		.Process(this->GenericPrerequisite)
		.Process(this->ExtraPower_Amount)
		.Process(this->RecheckTechTreeWhenDie)
		.Process(this->Linked_SW)
		.Process(this->CanDrive)
		.Process(this->Operators)
		.Process(this->Operator_Any)
		.Process(this->AlwayDrawRadialIndicator)
		.Process(this->ReloadRate)
		.Process(this->CloakAnim)
		.Process(this->DecloakAnim)
		.Process(this->Cloak_KickOutParasite)
		.Process(this->DeployAnims)
		.Process(this->SpecificExpFactor)
		.Process(this->Initial_DriverKilled)
		.Process(this->VoiceCantDeploy)
		.Process(this->DigitalDisplay_Disable)
		.Process(this->DigitalDisplayTypes)
		.Process(this->AmmoPip)
		.Process(this->EmptyAmmoPip)
		.Process(this->PipWrapAmmoPip)
		.Process(this->AmmoPipSize)
		.Process(this->ProduceCashDisplay)

		.Process(this->Drain_Local)
		.Process(this->Drain_Amount)

		.Process(this->HealthBar_Sections)
		.Process(this->HealthBar_Border)
		.Process(this->HealthBar_BorderFrame)
		.Process(this->HealthBar_BorderAdjust)
#pragma endregion
		;

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


	Stm.Process(this->AttachedEffect)
		.Process(this->NoAmmoEffectAnim)
		.Process(this->AttackFriendlies_WeaponIdx)
		.Process(this->PipScaleIndex)
		.Process(this->AmmoPip)
		.Process(this->AmmoPip_Offset)
		.Process(this->AmmoPip_Palette)

		.Process(this->Insignia_Weapon)
		;
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

double TechnoTypeExt::TurretMultiOffsetDefaultMult = 1.0;
double TechnoTypeExt::TurretMultiOffsetOneByEightMult = 0.125;

// =============================
// container
TechnoTypeExt::ExtContainer TechnoTypeExt::ExtMap;

TechnoTypeExt::ExtContainer::ExtContainer() : Container("TechnoTypeClass") { }
TechnoTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x711835, TechnoTypeClass_CTOR, 0x5)
{
	GET(TechnoTypeClass* , pItem, ESI);
	TechnoTypeExt::ExtMap.Allocate(pItem);
	return 0;
}

DEFINE_HOOK(0x711AE0, TechnoTypeClass_DTOR, 0x5)
{
	GET(TechnoTypeClass*, pItem, ECX);

	const auto pExt = TechnoTypeExt::ExtMap.Find(pItem);
	TechnoTypeExt::ClearImageData(pExt->BarrelImageData);
	TechnoTypeExt::ClearImageData(pExt->TurretImageData);

	GameDelete<true, true>(pExt->SpawnAltData.VXL);
	GameDelete<true, true>(pExt->SpawnAltData.HVA);

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

	TechnoTypeExt::ExtMap.LoadFromINI(pItem, pINI, R->Origin() == 0x716132);

	return 0;
}

////hook before stuffs got pop-ed to remove crash possibility
//DEFINE_HOOK(0x41CD74, AircraftTypeClass_LoadFromINI, 0x6)
//{
//	GET(AircraftTypeClass*, pItem, ESI);
//	GET(CCINIClass* const, pINI, EBX);
//
//	R->AL(pINI->ReadBool(pItem->ID, GameStrings::FlyBack(), R->CL()));
//
//	if (auto pExt = TechnoTypeExt::ExtMap.Find(pItem))
//		pExt->LoadFromINIFile_Aircraft(pINI);
//
//	return 0x41CD82;
//}