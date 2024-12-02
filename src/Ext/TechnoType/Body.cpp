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
#include <New/Type/ArmorTypeClass.h>

#include <Utilities/GeneralUtils.h>
#include <Utilities/Cast.h>
#include <Utilities/EnumFunctions.h>

void TechnoTypeExtData::Initialize()
{
	this->ShieldType = &ShieldTypeClass::Array[0];

	this->SellSound = RulesClass::Instance->SellSound;
	auto Eva_ready = GameStrings::EVA_ConstructionComplete();
	auto Eva_sold = GameStrings::EVA_StructureSold() ;
	this->AttachtoType = this->AttachedToObject->WhatAmI();

	if (this->AttachtoType != BuildingTypeClass::AbsID)
	{
		Eva_ready = GameStrings::EVA_UnitReady();
		Eva_sold = GameStrings::EVA_UnitSold();

		if (this->AttachtoType == AircraftTypeClass::AbsID)
		{
			this->CustomMissileTrailerAnim = AnimTypeClass::Find(GameStrings::V3TRAIL());
			this->CustomMissileTakeoffAnim = AnimTypeClass::Find(GameStrings::V3TAKEOFF());
		}

		this->EVA_UnitLost = VoxClass::FindIndexById(GameStrings::EVA_UnitLost());
		const auto nPromotedEva = VoxClass::FindIndexById(GameStrings::EVA_UnitPromoted());
		this->Promote_Elite_Eva = nPromotedEva;
		this->Promote_Vet_Eva = nPromotedEva;
	}

	this->Eva_Complete = VoxClass::FindIndexById(Eva_ready);
	this->EVA_Sold = VoxClass::FindIndexById(Eva_sold);
	this->EVA_Combat = VoxClass::FindIndexById("EVA_UnitsInCombat");
}

bool TechnoTypeExtData::CanBeBuiltAt(TechnoTypeClass* pProduct, BuildingTypeClass* pFactoryType)
{
	const auto pProductTypeExt = TechnoTypeExtContainer::Instance.Find(pProduct);
	const auto pBExt = BuildingTypeExtContainer::Instance.Find(pFactoryType);
	return (pProductTypeExt->BuiltAt.empty() && !pBExt->Factory_ExplicitOnly)
		|| pProductTypeExt->BuiltAt.Contains(pFactoryType);
}

void  TechnoTypeExtData::ApplyTurretOffset(Matrix3D* mtx, double factor)
{
	mtx->Translate((float)(this->TurretOffset->X * factor), (float)(this->TurretOffset->Y * factor), (float)(this->TurretOffset->Z * factor));
}

AnimTypeClass* TechnoTypeExtData::GetSinkAnim(TechnoClass* pThis)
{
	return TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType())->SinkAnim.Get(RulesClass::Instance->Wake);
}

double TechnoTypeExtData::GetTunnelSpeed(TechnoClass* pThis, RulesClass* pRules)
{
	return TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType())->Tunnel_Speed.Get(pRules->TunnelSpeed);
}

VoxelStruct* TechnoTypeExtData::GetBarrelsVoxel(TechnoTypeClass* const pThis, int const nIdx)
{
	if (nIdx == -1)/// ??
		return pThis->ChargerBarrels;

	if (nIdx < TechnoTypeClass::MaxWeapons)
		return pThis->ChargerBarrels + nIdx;

	const auto nAdditional = (nIdx - TechnoTypeClass::MaxWeapons);

	if ((size_t)nAdditional >= TechnoTypeExtContainer::Instance.Find(pThis)->BarrelImageData.size()) {
		Debug::FatalErrorAndExit(__FUNCTION__" [%s] Size[%s] Is Bigger than BarrelData ! \n", pThis->ID, nAdditional);
		return nullptr;
	}

	return TechnoTypeExtContainer::Instance.Find(pThis)->BarrelImageData.data() +
		nAdditional;
}

VoxelStruct* TechnoTypeExtData::GetTurretsVoxel(TechnoTypeClass* const pThis, int const nIdx)
{
	if (nIdx == -1)/// ??
		return pThis->ChargerTurrets;

	if (nIdx < TechnoTypeClass::MaxWeapons)
		return pThis->ChargerTurrets + nIdx;

	const auto nAdditional = (nIdx - TechnoTypeClass::MaxWeapons);
	if ((size_t)nAdditional >= TechnoTypeExtContainer::Instance.Find(pThis)->TurretImageData.size()) {
		Debug::FatalErrorAndExit(__FUNCTION__" [%s] Size[%d]  Is Bigger than TurretData ! \n", pThis->ID, nAdditional);
		return nullptr;
	}
	return TechnoTypeExtContainer::Instance.Find(pThis)->TurretImageData.data() + nAdditional;
}

// Ares 0.A source
const char* TechnoTypeExtData::GetSelectionGroupID() const
{
	return GeneralUtils::IsValidString(this->GroupAs) ? this->GroupAs : this->AttachedToObject->ID;
}

bool TechnoTypeExtData::IsGenericPrerequisite() const
{
	if(this->GenericPrerequisite.empty()) {
		auto begin = GenericPrerequisite::Array.begin();
		auto end  = GenericPrerequisite::Array.end();

		if(begin == end ){
			this->GenericPrerequisite = false;
			return false;
		}

		for(; begin != end; ++begin){
			auto alt_begin = begin->Alternates.begin();
			auto alt_end = begin->Alternates.end();

			if(alt_begin == alt_end) {
				continue;
			}

			for(; alt_begin != alt_end; ++alt_begin){
				if((*alt_begin) == this->AttachedToObject){
					this->GenericPrerequisite = true;
					return true;
				}
			}
		}
	}

	return this->GenericPrerequisite;
}

const char* TechnoTypeExtData::GetSelectionGroupID(ObjectTypeClass* pType)
{
	if (auto pTType = type_cast<TechnoTypeClass*>(pType))
		return TechnoTypeExtContainer::Instance.Find(pTType)->GetSelectionGroupID();

	return pType->ID;
}

bool TechnoTypeExtData::HasSelectionGroupID(ObjectTypeClass* pType, const std::string& pID)
{
	const auto id = TechnoTypeExtData::GetSelectionGroupID(pType);

	return (IS_SAME_STR_(id, pID.c_str()));
}

bool TechnoTypeExtData::IsCountedAsHarvester()
{
	if(!this->Harvester_Counted.isset()) {
		if(this->AttachedToObject->Enslaves){
			this->Harvester_Counted = true;
		}

		if (const auto pUnit = specific_cast<UnitTypeClass*>(this->AttachedToObject)){
			if(pUnit->Harvester || pUnit->Enslaves){
				this->Harvester_Counted = true;
			}
		}
	}

	return this->Harvester_Counted;
}

//DO NOT USE !
void TechnoTypeExtData::GetBurstFLHs(TechnoTypeClass* pThis,
	INI_EX& exArtINI,
	const char* pArtSection,
	ColletiveCoordStructVectorData& nFLH,
	ColletiveCoordStructVectorData& nEFlh,
	const char** pPrefixTag)
{
	/*char tempBuffer[0x40];
	char tempBufferFLH[0x40];

	bool parseMultiWeapons = pThis->TurretCount > 0 && pThis->WeaponCount > 0;
	auto weaponCount = parseMultiWeapons ? pThis->WeaponCount : 2;

	for (size_t g = 0; g < nFLH.size(); ++g) {
		nFLH[g]->resize(weaponCount);
		nEFlh[g]->resize(weaponCount);
	}

	for (int i = 0; i < weaponCount; i++)
	{
		for (int j = 0; j < INT_MAX; j++)
		{
			for(size_t k = 0; k < nFLH.size(); ++k){

				const auto pPrefixTagRes = *(pPrefixTag + k);

				IMPL_SNPRNINTF(tempBuffer, sizeof(tempBuffer), "%sWeapon%d", pPrefixTagRes, i + 1);
				auto prefix = parseMultiWeapons ? tempBuffer : i > 0 ? "%sSecondaryFire" : "%sPrimaryFire";
				IMPL_SNPRNINTF(tempBuffer, sizeof(tempBuffer), prefix, pPrefixTagRes);

				IMPL_SNPRNINTF(tempBufferFLH, sizeof(tempBufferFLH), "%sFLH.Burst%d", tempBuffer, j);

				CoordStruct flhFirst {};
				if(!detail::read(flhFirst, exArtINI , pArtSection, tempBufferFLH)){
					break;
				}

				(*nFLH[k])[i].emplace_back(flhFirst);
				IMPL_SNPRNINTF(tempBufferFLH, sizeof(tempBufferFLH), "Elite%sFLH.Burst%d", tempBuffer, j);
				if(!detail::read((*nEFlh[k])[i].emplace_back(), exArtINI , pArtSection, tempBufferFLH)){
					(*nEFlh[k])[i].back() = (*nFLH[k])[i].back();
				}
			}
		}
	}*/
}

void TechnoTypeExtData::GetBurstFLHs(TechnoTypeClass* pThis, INI_EX& exArtINI, const char* pArtSection,
	std::vector<BurstFLHBundle>& nFLH, const char* pPrefixTag)
{
	char tempBuffer[0x40];
	char tempBufferFLH[0x40];

	bool parseMultiWeapons = pThis->TurretCount > 0 && pThis->WeaponCount > 0;
	auto weaponCount = parseMultiWeapons ? pThis->WeaponCount : 2;
	nFLH.resize(weaponCount);

	for (int i = 0; i < weaponCount; i++)
	{
		for (int j = 0; j < INT_MAX; j++)
		{
			IMPL_SNPRNINTF(tempBuffer, sizeof(tempBuffer), "%sWeapon%d", pPrefixTag, i + 1);
			auto prefix = parseMultiWeapons ? tempBuffer : i > 0 ? "%sSecondaryFire" : "%sPrimaryFire";
			IMPL_SNPRNINTF(tempBuffer, sizeof(tempBuffer), prefix, pPrefixTag);

			IMPL_SNPRNINTF(tempBufferFLH, sizeof(tempBufferFLH), "%sFLH.Burst%d", tempBuffer, j);

			CoordStruct coord {};
			if (!detail::read(coord, exArtINI, pArtSection, tempBufferFLH))
				break;

			nFLH[i].Flh.emplace_back(coord);

			IMPL_SNPRNINTF(tempBufferFLH, sizeof(tempBufferFLH), "Elite%sFLH.Burst%d", tempBuffer, j);
			if (!detail::read(nFLH[i].EFlh.emplace_back(), exArtINI, pArtSection, tempBufferFLH))
				nFLH[i].EFlh.back() = coord;
		}
	}
};

void TechnoTypeExtData::GetFLH(INI_EX& exArtINI, const char* pArtSection, Nullable<CoordStruct>& nFlh, Nullable<CoordStruct>& nEFlh, const char* pFlag)
{
	std::string _key = "Elite";
	_key += pFlag;
	_key += "FLH";
	nFlh.Read(exArtINI, pArtSection, _key.data() + 5);
	nEFlh.Read(exArtINI, pArtSection, _key.c_str());

	if (!nEFlh.isset() && nFlh.isset())
		nEFlh = nFlh;
}

void TechnoTypeExtData::LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr)
{
	auto pThis = this->AttachedToObject;
	const auto pArtIni = &CCINIClass::INI_Art();
	const char* pSection = pThis->ID;
	const char* pArtSection = pThis->ImageFile;

	if (!parseFailAddr)
	{
		INI_EX exINI(pINI);
		// survivors
		this->Survivors_Pilots.resize(SideClass::Array->Count , nullptr);
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

		detail::read<bool>(pThis->Unbuildable, exINI, pSection, "Unbuildable", false);
		this->HumanUnbuildable.Read(exINI, pSection, "HumanUnbuildable");
		this->NoIdleSound.Read(exINI, pSection, "NoIdleSound");
		this->Soylent_Zero.Read(exINI, pSection, "Soylent.Zero");

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
		this->Spawner_AttackImmediately.Read(exINI, pSection, "Spawner.AttackImmediately");
		this->Harvester_Counted.Read(exINI, pSection, "Harvester.Counted");
		this->Promote_IncludeSpawns.Read(exINI, pSection, "Promote.IncludeSpawns");
		this->ImmuneToCrit.Read(exINI, pSection, "ImmuneToCrit");
		this->MultiMindControl_ReleaseVictim.Read(exINI, pSection, "MultiMindControl.ReleaseVictim");
		this->NoManualMove.Read(exINI, pSection, "NoManualMove");
		this->InitialStrength.Read(exINI, pSection, "InitialStrength");

		//TODO : Tag name Change
		this->Death_NoAmmo.Read(exINI, pSection, "Death.NoAmmo");
		this->Death_NoAmmo.Read(exINI, pSection, "AutoDeath.OnAmmoDepletion");
		this->Death_Countdown.Read(exINI, pSection, "Death.Countdown");
		this->Death_Countdown.Read(exINI, pSection, "AutoDeath.AfterDelay");

		this->Death_Method.Read(exINI, pSection, "Death.Method");
		this->Death_Method.Read(exINI, pSection, "AutoDeath.Behavior");

		bool Death_Peaceful;
		if(detail::read(Death_Peaceful , exINI, pSection, "Death.Peaceful"))
			this->Death_Method = Death_Peaceful ? KillMethod::Vanish : KillMethod::Explode;

		this->AutoDeath_Nonexist.Read(exINI, pSection, "AutoDeath.Nonexist");
		this->AutoDeath_Nonexist.Read(exINI, pSection, "AutoDeath.TechnosDontExist");
		this->AutoDeath_Nonexist_Any.Read(exINI, pSection, "AutoDeath.Nonexist.Any");
		this->AutoDeath_Nonexist_Any.Read(exINI, pSection, "AutoDeath.TechnosDontExist.Any");
		this->AutoDeath_Nonexist_House.Read(exINI, pSection, "AutoDeath.Nonexist.House");
		this->AutoDeath_Nonexist_House.Read(exINI, pSection, "AutoDeath.TechnosDontExist.House");
		this->AutoDeath_Nonexist_AllowLimboed.Read(exINI, pSection, "AutoDeath.Nonexist.AllowLimboed");
		this->AutoDeath_Nonexist_AllowLimboed.Read(exINI, pSection, "AutoDeath.TechnosDontExist.AllowLimboed");

		this->AutoDeath_Exist.Read(exINI, pSection, "AutoDeath.Exist");
		this->AutoDeath_Exist.Read(exINI, pSection, "AutoDeath.TechnosDontExist");
		this->AutoDeath_Exist_Any.Read(exINI, pSection, "AutoDeath.Exist.Any");
		this->AutoDeath_Exist_Any.Read(exINI, pSection, "AutoDeath.TechnosDontExist.Any");
		this->AutoDeath_Exist_House.Read(exINI, pSection, "AutoDeath.Exist.House");
		this->AutoDeath_Exist_House.Read(exINI, pSection, "AutoDeath.TechnosDontExist.House");
		this->AutoDeath_Exist_AllowLimboed.Read(exINI, pSection, "AutoDeath.Exist.AllowLimboed");
		this->AutoDeath_Exist_AllowLimboed.Read(exINI, pSection, "AutoDeath.TechnosDontExist.AllowLimboed");
		this->AutoDeath_VanishAnimation.Read(exINI, pSection, "AutoDeath.VanishAnimation");

		this->Convert_AutoDeath.Read(exINI, pSection, "Convert.AutoDeath");

		this->Death_WithMaster.Read(exINI, pSection, "Death.WithSlaveOwner");
		this->Death_WithMaster.Read(exINI, pSection, "AutoDeath.WithSlaveOwner");

		this->AutoDeath_MoneyExceed.Read(exINI, pSection, "AutoDeath.MoneyExceed");
		this->AutoDeath_MoneyBelow.Read(exINI, pSection, "AutoDeath.MoneyBelow");
		this->AutoDeath_LowPower.Read(exINI, pSection, "AutoDeath.LowPower");
		this->AutoDeath_FullPower.Read(exINI, pSection, "AutoDeath.FullPower");
		this->AutoDeath_PassengerExceed.Read(exINI, pSection, "AutoDeath.PassengersExceed");
		this->AutoDeath_PassengerBelow.Read(exINI, pSection, "AutoDeath.PassengersBelow");
		this->AutoDeath_ContentIfAnyMatch.Read(exINI, pSection, "AutoDeath.OnAnyCondition");

		this->AutoDeath_OwnedByPlayer.Read(exINI, pSection, "AutoDeath.OwnedByPlayer");
		this->AutoDeath_OwnedByAI.Read(exINI, pSection, "AutoDeath.OwnedByAI");

		this->Slaved_ReturnTo.Read(exINI, pSection, "Slaved.ReturnTo");
		this->Death_IfChangeOwnership.Read(exINI, pSection, "Death.IfChangeOwnership");

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
		ValueableVector<WarheadTypeClass*> DestroyAnimSpecificList {};
		DestroyAnimSpecificList.Read(exINI, pSection, "DestroyAnims.LinkedWarhead");

		if(!DestroyAnimSpecificList.empty()) {
			this->DestroyAnimSpecific.reserve(DestroyAnimSpecificList.size());
			for (size_t i = 0; i < DestroyAnimSpecificList.size(); ++i) {
				std::string _key = "DestroyAnims";
				_key += std::to_string(i);
				_key += ".Types";

				auto& it = this->DestroyAnimSpecific[DestroyAnimSpecificList[i]];
				detail::ReadVectors(
					it,
					exINI,
					pSection,
					_key.c_str()
				);
			}
		}

		this->NotHuman_RandomDeathSequence.Read(exINI, pSection, "NotHuman.RandomDeathSequence");

		this->PassengerDeletionType.LoadFromINI(pINI, pSection);

		this->DefaultDisguise.Read(exINI, pSection, "DefaultDisguise");

		this->OpenTopped_RangeBonus.Read(exINI, pSection, "OpenTopped.RangeBonus");
		this->OpenTopped_DamageMultiplier.Read(exINI, pSection, "OpenTopped.DamageMultiplier");
		this->OpenTopped_WarpDistance.Read(exINI, pSection, "OpenTopped.WarpDistance");
		this->OpenTopped_IgnoreRangefinding.Read(exINI, pSection, "OpenTopped.IgnoreRangefinding");
		this->OpenTopped_AllowFiringIfDeactivated.Read(exINI, pSection, "OpenTopped.AllowFiringIfDeactivated");
		this->OpenTopped_ShareTransportTarget.Read(exINI, pSection, "OpenTopped.ShareTransportTarget");
		this->OpenTopped_UseTransportRangeModifiers.Read(exINI, pSection, "OpenTopped.UseTransportRangeModifiers");
		this->OpenTopped_CheckTransportDisableWeapons.Read(exINI, pSection, "OpenTopped.CheckTransportDisableWeapons");

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
		this->MobileRefinery_TransRate.Read(exINI, pSection, "MobileRefinery.TransDelay");
		this->MobileRefinery_CashMultiplier.Read(exINI, pSection, "MobileRefinery.CashMultiplier");
		this->MobileRefinery_AmountPerCell.Read(exINI, pSection, "MobileRefinery.AmountPerCell");
		this->MobileRefinery_FrontOffset.Read(exINI, pSection, "MobileRefinery.FrontOffset");
		this->MobileRefinery_LeftOffset.Read(exINI, pSection, "MobileRefinery.LeftOffset");
		this->MobileRefinery_Display.Read(exINI, pSection, "MobileRefinery.Display");
		this->MobileRefinery_Display_House.Read(exINI, pSection, "MobileRefinery.Display.House");
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

		if (exINI.ReadString(pSection, "VoiceCreated") > 0) {
			this->VoiceCreate = VocClass::FindIndexById(exINI.c_str());
		} else {
			this->VoiceCreate.Read(exINI, pSection, "VoiceCreate");
		}

		this->VoiceCreate_Instant.Read(exINI, pSection, "VoiceCreate.Instant");

		this->SlaveFreeSound_Enable.Read(exINI, pSection, "SlaveFreeSound.Enable");
		this->SlaveFreeSound.Read(exINI, pSection, "SlaveFreeSound");

		if(Phobos::Otamaa::CompatibilityMode)
			this->SinkAnim.Read(exINI, pSection, "Wake.Sink");

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

		this->IronCurtain_KeptOnDeploy.Read(exINI, pSection, "IronCurtain.KeptOnDeploy");
		this->ForceShield_KeptOnDeploy.Read(exINI, pSection, "ForceShield.KeptOnDeploy");
		this->IronCurtain_Effect.Read(exINI, pSection, "IronCurtain.Flag");
		this->IronCurtain_KillWarhead.Read(exINI, pSection, "IronCurtain.KillWarhead", true);
		this->ForceShield_Effect.Read(exINI, pSection, "ForceShield.Effect");
		this->ForceShield_KillWarhead.Read(exINI, pSection, "ForceShield.KillWarhead", true);

		this->SellSound.Read(exINI, pSection, "SellSound");
		this->EVA_Sold.Read(exINI, pSection, "EVA.Sold");
		this->EngineerCaptureDelay.Read(exINI, pSection, "Engineer.CaptureDelay"); // code unfinished

		this->CommandLine_Move_Color.Read(exINI, pSection, "ActionLine.Move.Color");
		this->CommandLine_Attack_Color.Read(exINI, pSection, "ActionLine.Attack.Color");
		this->PassiveAcquire_AI.Read(exINI, pSection, "CanPassiveAquire.AI");
		this->CanPassiveAquire_Naval.Read(exINI, pSection, "CanPassiveAquire.Naval");

		this->TankDisguiseAsTank.Read(exINI, pSection, "Disguise.AsTank"); // code disabled , crash
		this->DisguiseDisAllowed.Read(exINI, pSection, "Disguise.Allowed");  // code disabled , crash
		this->ChronoDelay_Immune.Read(exINI, pSection, "ChronoDelay.Immune");
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
		//this->IgnoreToProtect.Read(exINI, pSection, "ToProtect.Ignore");
		this->TargetLaser_Time.Read(exINI, pSection, "TargetLaser.Time");
		this->TargetLaser_WeaponIdx.Read(exINI, pSection, "TargetLaser.WeaponIndexes");
		this->AdjustCrushProperties();

		this->ConsideredNaval.Read(exINI, pSection, "ConsideredNaval");
		this->ConsideredVehicle.Read(exINI, pSection, "ConsideredVehicle");

		this->LaserTargetColor.Read(exINI, pSection, "LaserTargetColor");
		this->VoicePickup.Read(exINI, pSection, "VoicePickup");

		this->CrateGoodie_RerollChance.Read(exINI, pSection, "CrateGoodie.RerollChance");
		this->Destroyed_CrateType.Read(exINI, pSection, "CrateGoodie.WhenDestroyed");
		this->Infantry_DimWhenEMPEd.Read(exINI, pSection, "Infantry.DimUnderEMP");
		this->Infantry_DimWhenDisabled.Read(exINI, pSection, "Infantry.DimWhenDisabled");
#pragma region Prereq

	std::string _Prerequisite_key = "Prerequisite";
	std::string _Prerequisite_ReqTheater_key = (_Prerequisite_key + ".RequiredTheaters");

	if(pINI->ReadString(pSection, _Prerequisite_ReqTheater_key.c_str(), "", Phobos::readBuffer) > 0) {
		this->Prerequisite_RequiredTheaters = 0;

		char* context = nullptr;
		for(char *cur = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context);
			cur;
			cur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			signed int idx = TheaterTypeClass::FindIndexById(cur);
			if(idx != -1) {
				this->Prerequisite_RequiredTheaters |= (1 << idx);
			} else if (!GameStrings::IsBlank(cur)) {
				Debug::INIParseFailed(pSection, _Prerequisite_ReqTheater_key.c_str(), cur);
			}
		}
	}

	// subtract the default list, get tag (not less than 0), add one back
	const auto nRead = pINI->ReadInteger(pSection, (_Prerequisite_key + ".Lists").c_str(), static_cast<int>(this->Prerequisites.size()) - 1);
	this->Prerequisites.resize(static_cast<size_t>(MaxImpl(nRead, 0) + 1));
	GenericPrerequisite::Parse(pINI, pSection, _Prerequisite_key.c_str(), this->Prerequisites[0]);

	for (size_t i = 0u; i < this->Prerequisites.size(); ++i) {
		GenericPrerequisite::Parse(pINI,
		pSection,
		(_Prerequisite_key + std::string(".List") + std::to_string(i)).c_str(),
		this->Prerequisites[i]
		);
	}

		// Prerequisite.Negative with Generic Prerequistes support
		GenericPrerequisite::Parse(pINI, pSection, (_Prerequisite_key + ".Negative").c_str(), this->Prerequisite_Negative);
		GenericPrerequisite::Parse(pINI, pSection, (_Prerequisite_key + ".Display").c_str(), this->Prerequisite_Display);
		GenericPrerequisite::Parse(pINI, pSection, (_Prerequisite_key + "Override").c_str(), pThis->PrerequisiteOverride);

		//TODO : properly Enable this
		GenericPrerequisite::Parse(pINI, pSection, "BuildLimit.Requres", this->BuildLimit_Requires);

		GenericPrerequisite::Parse(pINI, pSection, (std::string("Convert.Script.") + _Prerequisite_key).c_str(), this->Convert_Scipt_Prereq);

		this->Prerequisite_Power.Read(exINI, pSection, (_Prerequisite_key + ".Power").c_str());
		std::string _Prerequisite_StolenTechs_key = _Prerequisite_key+ ".StolenTechs";

		if (pINI->ReadString(pSection, _Prerequisite_StolenTechs_key.c_str(), Phobos::readDefval, Phobos::readBuffer) > 0)
		{
			this->RequiredStolenTech.reset();

			char* context = nullptr;
			for (char* cur = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context);
				cur;
				cur = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				signed int idx = std::atoi(cur);
				if (idx > -1 && idx < MaxHouseCount)
				{
					this->RequiredStolenTech.set(idx);
				}
				else if (idx != -1)
				{
					Debug::INIParseFailed(pSection, _Prerequisite_StolenTechs_key.c_str(), cur, "Expected a number between 0 and 31 inclusive");
				}
			}
		}

#pragma endregion Prereq

#ifndef _Handle_Old_
		this->AttachedEffect.Read(exINI);
#else
		int _AE_Dur { 0 };
		this->AttachEffect_AttachTypes.clear();
		if (detail::read(_AE_Dur, exINI, pSection, "AttachEffect.Duration") && _AE_Dur != 0) {
			auto& back = this->AttachEffect_AttachTypes.emplace_back(PhobosAttachEffectTypeClass::FindOrAllocate(pSection));
			back->Duration = _AE_Dur;
			back->Cumulative.Read(exINI, pSection, "AttachEffect.Cumulative");
			back->Animation.Read(exINI, pSection, "AttachEffect.Animation", true);
			if (!back->Animation)
				Debug::Log("Failed to find [%s] AE Anim[%s]\n", pSection, exINI.c_str());

			back->Animation_ResetOnReapply.Read(exINI, pSection, "AttachEffect.AnimResetOnReapply");

			bool AE_TemporalHidesAnim {};
			if (detail::read(AE_TemporalHidesAnim, exINI, pSection, "AttachEffect.TemporalHidesAnim") && AE_TemporalHidesAnim)
				back->Animation_TemporalAction = AttachedAnimFlag::Hides;

			back->ForceDecloak.Read(exINI, pSection, "AttachEffect.ForceDecloak");

			bool AE_DiscardOnEntry {};
			if(detail::read(AE_DiscardOnEntry, exINI, pSection, "AttachEffect.DiscardOnEntry") && AE_DiscardOnEntry)
				back->DiscardOn = DiscardCondition::Entry;

			back->FirepowerMultiplier.Read(exINI, pSection, "AttachEffect.FirepowerMultiplier");
			back->ArmorMultiplier.Read(exINI, pSection, "AttachEffect.ArmorMultiplier");
			back->SpeedMultiplier.Read(exINI, pSection, "AttachEffect.SpeedMultiplier");
			back->ROFMultiplier.Read(exINI, pSection, "AttachEffect.ROFMultiplier");
			back->ReceiveRelativeDamageMult.Read(exINI, pSection, "AttachEffect.ReceiveRelativeDamageMultiplier");
			back->Cloakable.Read(exINI, pSection, "AttachEffect.Cloakable");
			int AE_Delay {};
			detail::read(AE_Delay , exINI, pSection, "AttachEffect.Delay");
			this->AttachEffect_Delays.emplace_back(AE_Delay);

			int AE_IinitialDelay {};
			detail::read(AE_IinitialDelay, exINI, pSection, "AttachEffect.InitialDelay");
			this->AttachEffect_InitialDelays.emplace_back(AE_IinitialDelay);

			back->PenetratesIronCurtain.Read(exINI, pSection, "AttachEffect.PenetratesIronCurtain");
			back->DisableSelfHeal.Read(exINI, pSection, "AttachEffect.DisableSelfHeal");
			back->DisableWeapons.Read(exINI, pSection, "AttachEffect.DisableWeapons");
			back->Untrackable.Read(exINI, pSection, "AttachEffect.Untrackable");

			back->WeaponRange_Multiplier.Read(exINI, pSection, "AttachEffect.WeaponRange.Multiplier");
			back->WeaponRange_ExtraRange.Read(exINI, pSection, "AttachEffect.WeaponRange.ExtraRange");
			back->WeaponRange_AllowWeapons.Read(exINI, pSection, "AttachEffect.WeaponRange.AllowWeapons");
			back->WeaponRange_DisallowWeapons.Read(exINI, pSection, "AttachEffect.WeaponRange.DisallowWeapons");

			back->ROFMultiplier_ApplyOnCurrentTimer.Read(exINI, pSection, "AttachEffect.ROFMultiplier.ApplyOnCurrentTimer");
		}
#endif

		this->NoAmmoEffectAnim.Read(exINI, pSection, "NoAmmoEffectAnim", true);
		this->AttackFriendlies_WeaponIdx.Read(exINI, pSection, "AttackFriendlies.WeaponIdx");
		this->AttackFriendlies_AutoAttack.Read(exINI, pSection, "AttackFriendlies.AutoAttack");

		this->PipScaleIndex.Read(exINI, pSection, "PipScaleIndex");

		this->ShowSpawnsPips.Read(exINI, pSection, "ShowSpawnsPips");
		this->SpawnsPip.Read(exINI, pSection, "SpawnsPipFrame");
		this->EmptySpawnsPip.Read(exINI, pSection, "EmptySpawnsPipFrame");
		this->SpawnsPipSize.Read(exINI, pSection, "SpawnsPipSize");
		this->SpawnsPipOffset.Read(exINI, pSection, "SpawnsPipOffset");
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
		this->Convert_Deploy_Delay.Read(exINI, pSection, "Convert.DeployDelay");
		this->Convert_Script.Read(exINI, pSection, "Convert.Script");
		this->Convert_Water.Read(exINI, pSection, "Convert.Water");
		this->Convert_Land.Read(exINI, pSection, "Convert.Land");

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

		this->NoShadowSpawnAlt.Read(exINI, pSection, "NoShadowSpawnAlt");

		this->OmniCrusher_Aggressive.Read(exINI, pSection, "OmniCrusher.Aggressive");
		this->CrusherDecloak.Read(exINI, pSection, "Crusher.Decloak");
		this->Crusher_SupressLostEva.Read(exINI, pSection, "Crusher.SuppressLostEVA");
		this->CrushRange.Read(exINI, pSection, "Crusher.Range.%s");

		this->CrushFireDeathWeapon.Read(exINI, pSection, "CrushFireDeathWeaponChance.%s");
		this->CrushDamage.Read(exINI, pSection, "CrushDamage.%s");
		this->CrushDamageWarhead.Read(exINI, pSection, "CrushDamage.Warhead");
		this->CrushDamagePlayWHAnim.Read(exINI, pSection, "CrushDamage.PlayWarheadAnim");

		this->DigInSound.Read(exINI, pSection, "DigInSound");
		this->DigOutSound.Read(exINI, pSection, "DigOutSound");
		this->DigInAnim.Read(exINI, pSection, "DigIn");
		this->DigOutAnim.Read(exINI, pSection, "DigOut");

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
		this->Spot_Height.Read(exINI, pSection, "Spotlight.StartHeight");
		this->Spot_Distance.Read(exINI, pSection, "Spotlight.Distance");
		this->Spot_AttachedTo.Read(exINI, pSection, "Spotlight.AttachedTo");
		this->Spot_DisableR.Read(exINI, pSection, "Spotlight.DisableRed");
		this->Spot_DisableG.Read(exINI, pSection, "Spotlight.DisableGreen");
		this->Spot_DisableB.Read(exINI, pSection, "Spotlight.DisableBlue");
		this->Spot_DisableColor.Read(exINI, pSection, "Spotlight.Color");
		this->Spot_Reverse.Read(exINI, pSection, "Spotlight.IsInverted");

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

		if (exINI.ReadString(pSection, "Operator") > 0)
		{ // try to read the flag
			this->Operators.clear();
			// set whether this type accepts all operators
			this->Operator_Any = (IS_SAME_STR_N(exINI.value(), "_ANY_"));
			if (!this->Operator_Any)
			{ // if not, find the specific operator it allows
				detail::parse_values<TechnoTypeClass*, false>(this->Operators, exINI, pSection, "Operator");
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
		this->Bounty_Value_Option.Read(exINI, pSection, "Bounty.RewardOption");

		if(	this->Bounty_Value_Option ==  BountyValueOption::ValuePercentOfConst || this->Bounty_Value_Option == BountyValueOption::ValuePercentOfSoylent)
			this->Bounty_Value_PercentOf.Read(exINI, pSection, "Bounty.%sValue");
		else
			this->Bounty_Value.Read(exINI, pSection, "Bounty.%sValue");

		this->Bounty_Value_mult.Read(exINI, pSection, "Bounty.%sValueMult");

		this->Bounty_ReceiveSound.Read(exINI, pSection, "Bounty.ReceiveSound");

		this->DeathWeapon_CheckAmmo.Read(exINI, pSection, "DeathWeapon.CheckAmmo");
		this->Initial_DriverKilled.Read(exINI, pSection, "Initial.DriverKilled");

		this->VoiceCantDeploy.Read(exINI, pSection, "VoiceCantDeploy");
		this->DigitalDisplay_Disable.Read(exINI, pSection, "DigitalDisplay.Disable");
		this->DigitalDisplayTypes.Read(exINI, pSection, "DigitalDisplayTypes");

		//not fully working atm , disabled
		//this->DeployAnims.Read(exINI, pSection, "DeployingAnim");

		this->AmmoPip_Palette.Read(exINI, pSection, "AmmoPipPalette");
		this->AmmoPipOffset.Read(exINI, pSection, "AmmoPipOffset");

		this->AmmoPip.Read(exINI, pSection, "AmmoPipFrame");
		this->EmptyAmmoPip.Read(exINI, pSection, "EmptyAmmoPipFrame");
		this->PipWrapAmmoPip.Read(exINI, pSection, "AmmoPipWrapStartFrame");
		this->AmmoPipSize.Read(exINI, pSection, "AmmoPipSize");
		this->ProduceCashDisplay.Read(exINI, pSection, "ProduceCashDisplay");

		// drain settings
		this->Drain_Local.Read(exINI, pSection, "Drain.Local");
		this->Drain_Amount.Read(exINI, pSection, "Drain.Amount");

		this->FactoryOwners.Read(exINI, pSection, "FactoryOwners");
		this->FactoryOwners_Forbidden.Read(exINI, pSection, "FactoryOwners.Forbidden");
		this->Wake.Read(exINI, pSection, "Wake");

		this->UnitIdleRotateTurret.Read(exINI, pSection, "UnitIdleRotateTurret");
		this->UnitIdlePointToMouse.Read(exINI, pSection, "UnitIdlePointToMouse");

		this->FallingDownDamage.Read(exINI, pSection, "FallingDownDamage");
		this->FallingDownDamage_Water.Read(exINI, pSection, "FallingDownDamage.Water");

		this->DropCrate.Read(exINI, pSection, "DropCrate");

		this->WhenCrushed_Warhead.Read(exINI, pSection, "WhenCrushed.Warhead.%s", nullptr,  true);
		this->WhenCrushed_Weapon.Read(exINI, pSection, "WhenCrushed.Weapon.%s", nullptr, true);
		this->WhenCrushed_Damage.Read(exINI, pSection, "WhenCrushed.Damage.%s");
		this->WhenCrushed_Warhead_Full.Read(exINI, pSection, "WhenCrushed.Warhead.Full");

		this->FactoryOwners_HaveAllPlans.Read(exINI, pSection, "FactoryOwners.HaveAllPlans");
		this->FactoryOwners_HaveAllPlans.Read(exINI, pSection, "FactoryOwners.Permanent");
		this->FactoryOwners_HasAllPlans.Read(exINI, pSection, "FactoryOwners.HasAllPlans");

		this->HealthBar_Sections.Read(exINI, pSection, "HealthBar.Sections");
		this->HealthBar_Border.Read(exINI, pSection, "HealthBar.Border");
		this->HealthBar_BorderFrame.Read(exINI, pSection, "HealthBar.BorderFrame");
		this->HealthBar_BorderAdjust.Read(exINI, pSection, "HealthBar.BorderAdjust");

		this->IsBomb.Read(exINI, pSection, "IsBomb");
		this->ParachuteAnim.Read(exINI, pSection, "Parachute.Anim", true);

		this->Cloneable.Read(exINI, pSection, "Cloneable");
		this->ClonedAt.Read(exINI, pSection, "ClonedAt", true);
		this->ClonedAs.Read(exINI, pSection, "ClonedAs", true);
		this->AI_ClonedAs.Read(exINI, pSection, "AI.ClonedAs", true);
		this->BuiltAt.Read(exINI, pSection, "BuiltAt");
		this->EMP_Sparkles.Read(exINI, pSection, "EMP.Sparkles");
		this->EMP_Modifier.Read(exINI, pSection, "EMP.Modifier");

		if (pINI->ReadString(pSection, "EMP.Threshold", Phobos::readDefval, Phobos::readBuffer) > 0)
		{
			if (IS_SAME_STR_(Phobos::readBuffer, "inair")) {
				this->EMP_Threshold = -1;
			} else {

				bool ret;
				if (Parser<bool, 1>::Parse(Phobos::readBuffer, &ret)) {
					this->EMP_Threshold = (int)ret;
				} else if(!Parser<int, 1>::Parse(Phobos::readBuffer , &this->EMP_Threshold)) {
					Debug::INIParseFailed(pSection, "EMP.Treshold", Phobos::readBuffer, "[Phobos] Invalid value");
				}
			}
		}

		this->PoweredBy.Read(exINI, pSection, "PoweredBy");

		for (int i = 0; i < SideClass::Array->Count; ++i) {
			detail::read(this->Survivors_Pilots[i],
			exINI,
			pSection,
			(std::string("Survivor.Side") + std::to_string(i)).c_str()
			);
		}

		this->Ammo_AddOnDeploy.Read(exINI, pSection, "Ammo.AddOnDeploy");
		this->Ammo_AutoDeployMinimumAmount.Read(exINI, pSection, "Ammo.AutoDeployMinimumAmount");
		this->Ammo_AutoDeployMaximumAmount.Read(exINI, pSection, "Ammo.AutoDeployMaximumAmount");
		this->Ammo_DeployUnlockMinimumAmount.Read(exINI, pSection, "Ammo.DeployUnlockMinimumAmount");
		this->Ammo_DeployUnlockMaximumAmount.Read(exINI, pSection, "Ammo.DeployUnlockMaximumAmount");

		this->ImmuneToWeb.Read(exINI, pSection, "ImmuneToWeb");
		this->Webby_Anims.Read(exINI, pSection, "Webby.Anims");
		this->Webby_Modifier.Read(exINI, pSection, "Webby.Modifier");
		this->Webby_Duration_Variation.Read(exINI, pSection, "Webby.DurationVariation");

		// new secret lab
		this->Secret_RequiredHouses
			= pINI->ReadHouseTypesList(pSection, "SecretLab.RequiredHouses", this->Secret_RequiredHouses);

		this->Secret_ForbiddenHouses
			= pINI->ReadHouseTypesList(pSection, "SecretLab.ForbiddenHouses", this->Secret_ForbiddenHouses);

		this->ReloadInTransport.Read(exINI, pSection, "ReloadInTransport");
		this->Weeder_TriggerPreProductionBuildingAnim.Read(exINI, pSection, "Weeder.TriggerPreProductionBuildingAnim");
		this->Weeder_PipIndex.Read(exINI, pSection, "Weeder.PipIndex");
		this->Weeder_PipEmptyIndex.Read(exINI, pSection, "Weeder.PipEmptyIndex");
		this->CanBeDriven.Read(exINI, pSection, "CanBeDriven");

		this->CloakPowered.Read(exINI, pSection, "Cloakable.Powered");
		this->CloakDeployed.Read(exINI, pSection, "Cloakable.Deployed");
		this->ProtectedDriver.Read(exINI, pSection, "ProtectedDriver");
		this->ProtectedDriver_MinHealth.Read(exINI, pSection, "ProtectedDriver.MinHealth");
		this->KeepAlive.Read(exINI, pSection, "KeepAlive");
		this->DetectDisguise_Percent.Read(exINI, pSection, "DetectDisguise.Percent");
		this->PassengerTurret.Read(exINI, pSection, "PassengerTurret");

		this->Tint_Color.Read(exINI, pSection, "Tint.Color");
		this->Tint_Intensity.Read(exINI, pSection, "Tint.Intensity");
		this->Tint_VisibleToHouses.Read(exINI, pSection, "Tint.VisibleToHouses");

		this->PhobosAttachEffects.LoadFromINI(pINI, pSection);

		this->KeepTargetOnMove.Read(exINI, pSection, "KeepTargetOnMove");
		this->KeepTargetOnMove_ExtraDistance.Read(exINI, pSection, "KeepTargetOnMove.ExtraDistance");
		this->ForbidParallelAIQueues.Read(exINI, pSection, "ForbidParallelAIQueues");

		this->EVA_Combat.Read(exINI, pSection, "EVA.Combat");
		this->CombatAlert.Read(exINI, pSection, "CombatAlert");
		this->CombatAlert_NotBuilding.Read(exINI, pSection, "CombatAlert.NotBuilding");
		this->SubterraneanHeight.Read(exINI, pSection, "SubterraneanHeight");

		this->Spawner_RecycleRange.Read(exINI, pSection, "Spawner.RecycleRange");
		this->Spawner_RecycleFLH.Read(exINI, pSection, "Spawner.FLH");
		this->Spawner_RecycleOnTurret.Read(exINI, pSection, "Spawner.RecycleOnTurret");
		this->Spawner_RecycleAnim.Read(exINI, pSection, "Spawner.RecycleAnim");

		this->AINormalTargetingDelay.Read(exINI, pSection, "AINormalTargetingDelay");
		this->PlayerNormalTargetingDelay.Read(exINI, pSection, "PlayerNormalTargetingDelay");
		this->AIGuardAreaTargetingDelay.Read(exINI, pSection, "AIGuardAreaTargetingDelay");
		this->PlayerGuardAreaTargetingDelay.Read(exINI, pSection, "PlayerGuardAreaTargetingDelay");
		this->DistributeTargetingFrame.Read(exINI, pSection, "DistributeTargetingFrame");
		this->CanBeBuiltOn.Read(exINI, pSection, "CanBeBuiltOn");
		this->UnitBaseNormal.Read(exINI, pSection, "UnitBaseNormal");
		this->UnitBaseForAllyBuilding.Read(exINI, pSection, "UnitBaseForAllyBuilding");
		this->ChronoSpherePreDelay.Read(exINI, pSection, "ChronoSpherePreDelay");
		this->ChronoSphereDelay.Read(exINI, pSection, "ChronoSphereDelay");
		this->PassengerWeapon.Read(exINI, pSection, "PassengerWeapon");

#pragma region AircraftOnly
		if (this->AttachtoType == AircraftTypeClass::AbsID)
		{
			this->SpawnDistanceFromTarget.Read(exINI, pSection, "SpawnDistanceFromTarget");
			this->SpawnHeight.Read(exINI, pSection, "SpawnHeight");

			this->LandingDir.Read(exINI, pSection, "LandingDir");
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
			this->CustomMissileData->Type = static_cast<AircraftTypeClass*>(pThis);
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
			this->CustomMissileInaccuracy.Read(exINI, pSection, "Missile.Inaccuracy");
			this->CustomMissileTrailAppearDelay.Read(exINI, pSection, "Missile.TrailerAppearDelay");

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
			this->Crashable.Read(exINI, pSection, "Crashable");
			this->MyDiveData.Read(exINI, pSection);
			this->MyPutData.Read(exINI, pSection);
			this->MyFighterData.Read(exINI, pSection, pThis);
		}
#pragma endregion

		//this->ShadowIndices.Read(exINI, pSection, "ShadowIndices");

		this->EliteArmor.Read(exINI, pSection, "EliteArmor");
		this->VeteranArmor.Read(exINI, pSection, "VeteranArmor");
		this->DeployedArmor.Read(exINI, pSection, "DeployedArmor");

		this->Cloakable_IgnoreArmTimer.Read(exINI, pSection, "Cloakable.IgnoreROFTimer");
		this->Convert_HumanToComputer.Read(exINI, pSection, "Convert.HumanToComputer");
		this->Convert_ComputerToHuman.Read(exINI, pSection, "Convert.ComputerToHuman");

		this->ShadowSizeCharacteristicHeight.Read(exINI, pSection, "ShadowSizeCharacteristicHeight");

		if(auto pTalkBuble = FileSystem::TALKBUBL_SHP()){
			for (int i = 0; i < pTalkBuble->Frames; ++i) {
				std::string base = "TalkbubbleFrame";
				base += std::to_string(i);
				this->TalkbubbleVoices.emplace_back().Read(exINI, pSection, (base + ".Voices").c_str());
			}
		}

		this->NoExtraSelfHealOrRepair.Read(exINI, pSection, "NoExtraSelfHealOrRepair");

#pragma region BuildLimitGroup
		this->BuildLimitGroup_Types.Read(exINI, pSection, "BuildLimitGroup.Types");
		this->BuildLimitGroup_Nums.Read(exINI, pSection, "BuildLimitGroup.Nums");
		this->BuildLimitGroup_Factor.Read(exINI, pSection, "BuildLimitGroup.Factor");
		this->BuildLimitGroup_ContentIfAnyMatch.Read(exINI, pSection, "BuildLimitGroup.ContentIfAnyMatch");
		this->BuildLimitGroup_NotBuildableIfQueueMatch.Read(exINI, pSection, "BuildLimitGroup.NotBuildableIfQueueMatch");
		this->BuildLimitGroup_ExtraLimit_Types.Read(exINI, pSection, "BuildLimitGroup.ExtraLimit.Types");
		this->BuildLimitGroup_ExtraLimit_Nums.Read(exINI, pSection, "BuildLimitGroup.ExtraLimit.Nums");
		this->BuildLimitGroup_ExtraLimit_MaxCount.Read(exINI, pSection, "BuildLimitGroup.ExtraLimit.MaxCount");
		this->BuildLimitGroup_ExtraLimit_MaxNum.Read(exINI, pSection, "BuildLimitGroup.ExtraLimit.MaxNum");
#pragma endregion

		this->Tiberium_EmptyPipIdx.Read(exINI, pSection, "StorageEmptyPipIndex");
		this->Tiberium_PipIdx.Read(exINI, pSection, "StoragePipIndexes");
		this->Tiberium_PipShapes.Read(exINI, pSection, "StoragePipShapes");
		this->Tiberium_PipShapes_Palette.Read(exINI, pSection, "StoragePipShapesPalette");

		if (this->AttachtoType != AbstractType::BuildingType)
		{
			this->Untrackable.Read(exINI, pSection, "Untrackable");
			this->LargeVisceroid.Read(exINI, pSection, "Visceroid.Large");
			this->HarvesterDumpAmount.Read(exINI, pSection, "HarvesterDumpAmount");
			this->DropPodProp.Read(exINI, pSection);
		}

		this->RefinerySmokeParticleSystemOne.Read(exINI, pSection, "RefinerySmokeParticleSystemOne");
		this->RefinerySmokeParticleSystemTwo.Read(exINI, pSection, "RefinerySmokeParticleSystemTwo");
		this->RefinerySmokeParticleSystemThree.Read(exINI, pSection, "RefinerySmokeParticleSystemThree");
		this->RefinerySmokeParticleSystemFour.Read(exINI, pSection, "RefinerySmokeParticleSystemFour");

		exINI.ReadSpeed(pSection, "SubterraneanSpeed", &this->SubterraneanSpeed);

		this->ForceWeapon_InRange.Read(exINI, pSection, "ForceWeapon.InRange");
		this->ForceWeapon_InRange_Overrides.Read(exINI, pSection, "ForceWeapon.InRange.Overrides");
		this->ForceWeapon_InRange_ApplyRangeModifiers.Read(exINI, pSection, "ForceWeapon.InRange.ApplyRangeModifiers");

		if (!RefinerySmokeParticleSystemOne.isset()) {
			RefinerySmokeParticleSystemOne = this->AttachedToObject->RefinerySmokeParticleSystem;
		}

		if (!RefinerySmokeParticleSystemTwo.isset()) {
			RefinerySmokeParticleSystemTwo = this->AttachedToObject->RefinerySmokeParticleSystem;
		}

		if (!RefinerySmokeParticleSystemThree.isset()) {
			RefinerySmokeParticleSystemThree = this->AttachedToObject->RefinerySmokeParticleSystem;
		}

		if (!RefinerySmokeParticleSystemFour.isset()) {
			RefinerySmokeParticleSystemFour = this->AttachedToObject->RefinerySmokeParticleSystem;
		}

		char tempBuffer[32];

		this->Convert_ToHouseOrCountry.clear();
		Nullable<TechnoTypeClass*> technoType;
		// put all sides into the map
		this->Convert_ToHouseOrCountry.reserve(SideClass::Array->Count + HouseTypeClass::Array->Count);

		for (auto const& pSide : *SideClass::Array) {
			IMPL_SNPRNINTF(tempBuffer, sizeof(tempBuffer), "Convert.To%s", pSide->ID);
			technoType.Read(exINI, pSection, tempBuffer);
			if (technoType.isset()) {
				this->Convert_ToHouseOrCountry.insert(pSide, technoType.Get());
			}
		}

		// put all countries into the map
		for (auto const& pTHouse : *HouseTypeClass::Array) {
			IMPL_SNPRNINTF(tempBuffer, sizeof(tempBuffer), "Convert.To%s", pTHouse->ID);
			technoType.Read(exINI, pSection, tempBuffer);
			if (technoType.isset()) {
				this->Convert_ToHouseOrCountry.insert(pTHouse, technoType.Get());
			}
		}

		this->SuppressKillWeapons.Read(exINI, pSection, "SuppressKillWeapons");
		this->SuppressKillWeapons_Types.Read(exINI, pSection, "SuppressKillWeapons.Types");
	}

	// Art tags
	if (pArtIni && pArtIni->GetSection(pArtSection))
	{
		INI_EX exArtINI(pArtIni);

		this->TurretOffset.Read(exArtINI, pArtSection, GameStrings::TurretOffset());

		if (!this->TurretOffset.isset())
		{
			//put ddedfault single value inside
			this->TurretOffset = PartialVector3D<int>{ pThis->TurretOffset , 0 ,0 , 1 };
		}

		this->TurretShadow.Read(exArtINI, pArtSection, "TurretShadow");

		this->SprayOffsets.resize(8);
		//
		this->SprayOffsets[0]->X = 256;
		this->SprayOffsets[0]->Y = 0;
		this->SprayOffsets[0]->Z = 0;
		//
		this->SprayOffsets[1]->X = 180;
		this->SprayOffsets[1]->Y = 180;
		this->SprayOffsets[1]->Z = 0;
		//
		this->SprayOffsets[2]->X = 0;
		this->SprayOffsets[2]->Y = 256;
		this->SprayOffsets[2]->Z = 0;
		//
		this->SprayOffsets[3]->X = -180;
		this->SprayOffsets[3]->Y = 180;
		this->SprayOffsets[3]->Z = 0;
		//
		this->SprayOffsets[4]->X = -256;
		this->SprayOffsets[4]->Y = 0;
		this->SprayOffsets[4]->Z = 0;
		//
		this->SprayOffsets[5]->X = -180;
		this->SprayOffsets[5]->Y = -180;
		this->SprayOffsets[5]->Z = 0;
		//
		this->SprayOffsets[6]->X = 0;
		this->SprayOffsets[6]->Y = -256;
		this->SprayOffsets[6]->Z = 0;
		//
		this->SprayOffsets[7]->X = 180;
		this->SprayOffsets[7]->Y = -180;
		this->SprayOffsets[7]->Z = 0;

		for (size_t c = 0; ; ++c) {
			std::string __base_key = "SprayOffsets";
			__base_key += std::to_string(c);
			CoordStruct val {};

			if (!detail::read(val, exArtINI, pArtSection, __base_key.c_str()))
				break;
			else
			{
				if (c < this->SprayOffsets.size())
					this->SprayOffsets[c] = val;
				else
					this->SprayOffsets.emplace_back(std::move(val));
			}
		}

		std::vector<int> shadow_indices {};
		detail::ReadVectors(shadow_indices, exArtINI, pArtSection, "ShadowIndices");
		std::vector<int> shadow_indices_frame {};
		detail::ReadVectors(shadow_indices_frame, exArtINI, pArtSection, "ShadowIndices.Frame");

		if (shadow_indices_frame.size() != shadow_indices.size())
		{
			if (!shadow_indices_frame.empty())
				Debug::Log("[Developer warning] %s ShadowIndices.Frame size (%d) does not match ShadowIndices size (%d) \n"
					, pSection, shadow_indices_frame.size(), shadow_indices.size());

			shadow_indices_frame.resize(shadow_indices.size(), -1);
		}

		for (size_t i = 0; i < shadow_indices.size(); i++)
			this->ShadowIndices[shadow_indices[i]] = shadow_indices_frame[i];

		this->ShadowIndex_Frame.Read(exArtINI, pArtSection, "ShadowIndex.Frame");

		this->LaserTrailData.clear();

		for (size_t i = 0; ; ++i)
		{
			std::string _base_key = "LaserTrail";
			_base_key += std::to_string(i);

			if (exArtINI->ReadString(pArtSection, (_base_key + ".Type").c_str(), Phobos::readDefval, Phobos::readBuffer) <= 0)
				break;

			int def;
			if (!Parser<LaserTrailTypeClass,1>::TryParseIndex(Phobos::readBuffer, &def))
				break;

			auto data = &this->LaserTrailData.emplace_back();
			data->idxType = def;

			detail::read(data->FLH , exArtINI, pArtSection,  (_base_key + ".FLH").c_str());
			detail::read(data->IsOnTurret , exArtINI, pArtSection,  (_base_key + ".IsOnTurret").c_str());
		}

		this->AlternateFLHs.clear();

		for (size_t i = 5; ; ++i)
		{
			Nullable<CoordStruct> alternateFLH;
			alternateFLH.Read(exArtINI, pArtSection, (std::string("AlternateFLH") + std::to_string(i)).c_str());

			if (!alternateFLH.isset())
				break;

			this->AlternateFLHs.push_back(alternateFLH.Get());
		}

		this->HitCoordOffset.clear();

		for (size_t i = 0; ; ++i)
		{
			Nullable<CoordStruct> nHitBuff;
			nHitBuff.Read(exArtINI, pArtSection, (std::string("HitCoordOffset") + std::to_string(i)).c_str());

			if (!nHitBuff.isset())
				break;

			this->HitCoordOffset.push_back(nHitBuff);
		}

		this->HitCoordOffset_Random.Read(exArtINI, pArtSection, "HitCoordOffset.Random");

		this->Spawner_SpawnOffsets.Read(exArtINI, pArtSection, "SpawnOffset");
		this->Spawner_SpawnOffsets_OverrideWeaponFLH.Read(exArtINI, pArtSection, "SpawnOffsetOverrideFLH");
		this->ShadowScale.Read(exArtINI, pArtSection, "ShadowScale");

		//LineTrailData::LoadFromINI(this->LineTrailData, exArtINI, pArtSection);

		// change the size on techno type and add more entry
		//ColletiveCoordStructVectorData nFLH = { &WeaponBurstFLHs  , &DeployedWeaponBurstFLHs , &CrouchedWeaponBurstFLHs };
		//ColletiveCoordStructVectorData nEFLH = { &EliteWeaponBurstFLHs  , &EliteDeployedWeaponBurstFLHs , &EliteCrouchedWeaponBurstFLHs };
		//const char* tags[sizeof(ColletiveCoordStructVectorData) / sizeof(void*)] = { Phobos::readDefval  , "Deployed" , "Prone" };
		//TechnoTypeExtData::GetBurstFLHs(pThis, exArtINI, pArtSection, nFLH, nEFLH, tags);

		TechnoTypeExtData::GetBurstFLHs(pThis, exArtINI, pArtSection, WeaponBurstFLHs, "");
		TechnoTypeExtData::GetBurstFLHs(pThis, exArtINI, pArtSection, DeployedWeaponBurstFLHs, "Deployed");
		TechnoTypeExtData::GetBurstFLHs(pThis, exArtINI, pArtSection, CrouchedWeaponBurstFLHs, "Prone");

		TechnoTypeExtData::GetFLH(exArtINI, pArtSection, PronePrimaryFireFLH, E_PronePrimaryFireFLH, "PronePrimaryFire");
		TechnoTypeExtData::GetFLH(exArtINI, pArtSection, ProneSecondaryFireFLH, E_ProneSecondaryFireFLH, "ProneSecondaryFire");
		TechnoTypeExtData::GetFLH(exArtINI, pArtSection, DeployedPrimaryFireFLH, E_DeployedPrimaryFireFLH, "DeployedPrimaryFire");
		TechnoTypeExtData::GetFLH(exArtINI, pArtSection, DeployedSecondaryFireFLH, E_DeployedSecondaryFireFLH, "DeployedSecondaryFire");

		TechnoTypeExtData::GetFLH(exArtINI, pArtSection, PrimaryCrawlFLH, Elite_PrimaryCrawlFLH, "PrimaryCrawling");
		TechnoTypeExtData::GetFLH(exArtINI, pArtSection, SecondaryCrawlFLH, Elite_SecondaryCrawlFLH, "SecondaryCrawling");

		this->MyExtraFireData.ReadArt(exArtINI, pArtSection);
		this->MySpawnSupportFLH.Read(exArtINI, pArtSection);
		this->Trails.Read(exArtINI, pArtSection, true);

		this->CameoPCX.Read(exArtINI.GetINI(), pArtSection, "CameoPCX");
		this->AltCameoPCX.Read(exArtINI.GetINI(), pArtSection, "AltCameoPCX");
		this->CameoPal.Read(exArtINI, pArtSection, "CameoPalette");
	}
}

void TechnoTypeExtData::LoadFromINIFile_Aircraft(CCINIClass* pINI)
{
	//auto pThis = Get();
	//const char* pSection = pThis->ID;
	//INI_EX exINI(pINI);
}

void TechnoTypeExtData::LoadFromINIFile_EvaluateSomeVariables(CCINIClass* pINI)
{
	//auto pThis = Get();
	//const char* pSection = pThis->ID;
	//INI_EX exINI(pINI);

}

void TechnoTypeExtData::InitializeConstant()
{
	this->AttachedEffect.Owner = this->AttachedToObject;
	this->PassengerDeletionType.OwnerType = this->AttachedToObject;
}

ImageStatusses ImageStatusses::ReadVoxel(const char* const nKey, bool a4)
{
	std::string _buffer = nKey;
	const size_t key_len = _buffer.size();
	_buffer += ".VXL";
	CCFileClass CCFileV { _buffer.c_str() };

	if (CCFileV.Exists())
	{
		MotLib* pLoadedHVA = nullptr;
		VoxLib* pLoadedVXL = GameCreate<VoxLib>(&CCFileV, false);
		_buffer[key_len + 1] = 'H';
		_buffer[key_len + 2] = 'V';
		_buffer[key_len + 3] = 'A';
		CCFileClass  CCFileH { _buffer.c_str() };

		if (CCFileH.Open(FileAccessMode::Read)) {
			pLoadedHVA = GameCreate<MotLib>(&CCFileH);
		}

		if (!pLoadedHVA || pLoadedVXL->LoadFailed || pLoadedHVA->LoadedFailed)
		{
			GameDelete<true, true>(pLoadedHVA);
			GameDelete<true, true>(pLoadedVXL);

			return { {nullptr , nullptr} , false };
		}
		else
		{
			pLoadedHVA->Scale(pLoadedVXL->TailerData[pLoadedVXL->HeaderData->limb_number].HVAMultiplier);
			return { {pLoadedVXL , pLoadedHVA }, true };
		}
	}

	return { {nullptr , nullptr} , a4 };
}

void TechnoTypeExtData::AdjustCrushProperties()
{
	auto const pThis = this->AttachedToObject;

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
		this->AttachtoType == UnitTypeClass::AbsID)
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

bool TechnoTypeExtData::PassangersAllowed(TechnoTypeClass* pThis, TechnoTypeClass* pPassanger)
{
	const auto pExt = TechnoTypeExtContainer::Instance.Find(pThis);

	if (!pExt->PassengersWhitelist.Eligible(pPassanger))
		return false;

	if (!pExt->PassengersBlacklist.empty() && pExt->PassengersBlacklist.Contains(pPassanger))
		return false;

	return true;
}

template <typename T>
void TechnoTypeExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Initialized)
		.Process(this->AttachtoType)
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
		.Process(this->ShadowIndex_Frame)
		.Process(this->ShadowSizeCharacteristicHeight)
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
		.Process(this->Convert_AutoDeath)
		.Process(this->Death_WithMaster)
		.Process(this->AutoDeath_MoneyExceed)
		.Process(this->AutoDeath_MoneyBelow)
		.Process(this->AutoDeath_LowPower)
		.Process(this->AutoDeath_FullPower)
		.Process(this->AutoDeath_PassengerExceed)
		.Process(this->AutoDeath_PassengerBelow)
		.Process(this->AutoDeath_ContentIfAnyMatch)
		.Process(this->AutoDeath_OwnedByPlayer)
		.Process(this->AutoDeath_OwnedByAI)
		.Process(this->Slaved_ReturnTo)
		.Process(this->Death_IfChangeOwnership)

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
		.Process(this->DestroyAnimSpecific)
		.Process(this->NotHuman_RandomDeathSequence)
		.Process(this->DefaultDisguise)
		;

	//Debug::Log("%s AboutToLoad WeaponFLhA\n" , this->AttachedToObject->ID);
	//Stm
	//	.Process(this->WeaponBurstFLHs)
	//	;
	//Debug::Log("Done WeaponFLhA\n");

	Stm
		.Process(this->PassengerDeletionType)

		.Process(this->OpenTopped_RangeBonus)
		.Process(this->OpenTopped_DamageMultiplier)
		.Process(this->OpenTopped_WarpDistance)
		.Process(this->OpenTopped_IgnoreRangefinding)
		.Process(this->OpenTopped_AllowFiringIfDeactivated)
		.Process(this->OpenTopped_ShareTransportTarget)
		.Process(this->OpenTopped_UseTransportRangeModifiers)
		.Process(this->OpenTopped_CheckTransportDisableWeapons)
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
		.Process(this->MobileRefinery_Display_House)
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
		.Process(this->DeployedPrimaryFireFLH)
		.Process(this->DeployedSecondaryFireFLH)
		.Process(this->E_PronePrimaryFireFLH)
		.Process(this->ProneSecondaryFireFLH)
		.Process(this->E_ProneSecondaryFireFLH)
		.Process(this->E_DeployedPrimaryFireFLH)
		.Process(this->E_DeployedSecondaryFireFLH)

		.Process(this->WeaponBurstFLHs)
		.Process(this->CrouchedWeaponBurstFLHs)
		.Process(this->DeployedWeaponBurstFLHs)

		.Process(this->IronCurtain_KeptOnDeploy)
		.Process(this->ForceShield_KeptOnDeploy)
		.Process(this->IronCurtain_Effect)
		.Process(this->IronCurtain_KillWarhead)
		.Process(this->ForceShield_Effect)
		.Process(this->ForceShield_KillWarhead)
		.Process(this->SellSound)
		.Process(this->EVA_Sold);
		//Debug::Log("AboutToLoad WeaponFLhB\n");
		//Stm.Process(this->CrouchedWeaponBurstFLHs);
		//Debug::Log("Done WeaponFLhB\n");
		//Debug::Log("AboutToLoad WeaponFLhC\n");
		//Stm.Process(this->DeployedWeaponBurstFLHs);
		//Debug::Log("Done WeaponFLhC\n");
		Stm.Process(this->AlternateFLHs)
			.Process(this->Spawner_SpawnOffsets)

			.Process(this->Spawner_SpawnOffsets_OverrideWeaponFLH);

		//Debug::Log("AboutToLoad Otammaa\n");
#pragma region Otamaa
		Stm
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
		.Process(this->CustomMissileInaccuracy)
		.Process(this->CustomMissileTrailAppearDelay)
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
		.Process(this->VoiceCreate_Instant)
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
		.Process(this->PassiveAcquire_AI)
		.Process(this->CanPassiveAquire_Naval)
		.Process(this->TankDisguiseAsTank)
		.Process(this->DisguiseDisAllowed)
		.Process(this->ChronoDelay_Immune)
		.Process(this->PoseDir)
		.Process(this->Firing_IgnoreGravity)
		.Process(this->Survivors_PassengerChance)

		.Process(this->Prerequisite_RequiredTheaters)
		.Process(this->Prerequisites)
		.Process(this->Prerequisite_Lists)
		.Process(this->Prerequisite_Negative)
		.Process(this->Prerequisite_Display)
		.Process(this->BuildLimit_Requires)
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

		.Process(this->Tiberium_EmptyPipIdx)
		.Process(this->Tiberium_PipIdx)
		.Process(this->Tiberium_PipShapes)
		.Process(this->Tiberium_PipShapes_Palette)

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
		//.Process(this->IgnoreToProtect)
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
		.Process(this->Convert_Deploy_Delay)
		.Process(this->Convert_Script)
		.Process(this->Convert_Water)
		.Process(this->Convert_Land)
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

		//.Process(this->BarrelImageData)
		//.Process(this->TurretImageData)
		//.Process(this->SpawnAltData)
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
		.Process(this->CrushDamagePlayWHAnim)
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
		.Process(this->Survivors_Pilots)
		.Process(this->Ammo_AddOnDeploy)
		.Process(this->Ammo_AutoDeployMinimumAmount)
		.Process(this->Ammo_AutoDeployMaximumAmount)
		.Process(this->Ammo_DeployUnlockMinimumAmount)
		.Process(this->Ammo_DeployUnlockMaximumAmount)
		.Process(this->ImmuneToWeb)
		.Process(this->Webby_Anims)
		.Process(this->Webby_Modifier)
		.Process(this->Webby_Duration_Variation)
		.Process(this->BerserkROFMultiplier)
		.Process(this->Refinery_UseStorage)
		.Process(this->VHPscan_Value)

		.Process(this->SelfHealing_Rate)
		.Process(this->SelfHealing_Amount)
		.Process(this->SelfHealing_Max)
		.Process(this->SelfHealing_CombatDelay)
		.Process(this->Bounty)
		.Process(this->HasSpotlight)
		.Process(this->Spot_Height)
		.Process(this->Spot_Distance)
		.Process(this->Spot_AttachedTo)
		.Process(this->Spot_DisableR)
		.Process(this->Spot_DisableG)
		.Process(this->Spot_DisableB)
		.Process(this->Spot_DisableColor)
		.Process(this->Spot_Reverse)
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
		.Process(this->Bounty_Value_PercentOf)
		.Process(this->Bounty_ReceiveSound)
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
		.Process(this->ShadowScale)
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
		.Process(this->FactoryOwners)
		.Process(this->FactoryOwners_Forbidden)
		.Process(this->Wake)
		.Process(this->Spawner_AttackImmediately)
		.Process(this->FactoryOwners_HaveAllPlans)
		.Process(this->FactoryOwners_HasAllPlans)
		.Process(this->Drain_Local)
		.Process(this->Drain_Amount)

		.Process(this->HealthBar_Sections)
		.Process(this->HealthBar_Border)
		.Process(this->HealthBar_BorderFrame)
		.Process(this->HealthBar_BorderAdjust)

		.Process(this->Crashable)
		.Process(this->IsBomb)
		.Process(this->ParachuteAnim)

		.Process(this->Cloneable)
		.Process(this->ClonedAt)
		.Process(this->ClonedAs)
		.Process(this->AI_ClonedAs)
		.Process(this->BuiltAt)
		.Process(this->EMP_Sparkles)
		.Process(this->EMP_Modifier)
		.Process(this->EMP_Threshold)
		.Process(this->PoweredBy)

		.Process(this->CameoPCX)
		.Process(this->AltCameoPCX)
		.Process(this->CameoPal)
		.Process(this->LandingDir)
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
		.Process(this->AttackFriendlies_AutoAttack)
		.Process(this->PipScaleIndex)
		.Process(this->AmmoPip)
		.Process(this->AmmoPip_Palette)
		.Process(this->AmmoPipOffset)
		.Process(this->AmmoPip_Offset)
		.Process(this->AmmoPip_shape)

		.Process(this->ShowSpawnsPips)
		.Process(this->SpawnsPip)
		.Process(this->EmptySpawnsPip)
		.Process(this->SpawnsPipSize)
		.Process(this->SpawnsPipOffset)
		.Process(this->Insignia_Weapon)
		.Process(this->Secret_RequiredHouses)
		.Process(this->Secret_ForbiddenHouses)
		.Process(this->RequiredStolenTech)
		.Process(this->ReloadInTransport)
		.Process(this->Weeder_TriggerPreProductionBuildingAnim)
		.Process(this->Weeder_PipIndex)
		.Process(this->Weeder_PipEmptyIndex)

		.Process(this->CanBeDriven)
		.Process(this->CloakPowered)
		.Process(this->CloakDeployed)
		.Process(this->ProtectedDriver)
		.Process(this->ProtectedDriver_MinHealth)
		.Process(this->KeepAlive)
		.Process(this->SpawnDistanceFromTarget)
		.Process(this->SpawnHeight)
		.Process(this->HumanUnbuildable)
		.Process(this->NoIdleSound)
		.Process(this->Soylent_Zero)
		.Process(this->Prerequisite_Power)
		.Process(this->PassengerTurret)
		.Process(this->DetectDisguise_Percent)
		.Process(this->EliteArmor)
		.Process(this->VeteranArmor)
		.Process(this->DeployedArmor)
		.Process(this->Cloakable_IgnoreArmTimer)
		.Process(this->Untrackable)
		.Process(this->Convert_Scipt_Prereq)
		.Process(this->LargeVisceroid)
		.Process(this->DropPodProp)
		.Process(this->LaserTargetColor)
		.Process(this->VoicePickup)
		.Process(this->CrateGoodie_RerollChance)
		.Process(this->Destroyed_CrateType)
		.Process(this->Infantry_DimWhenEMPEd)
		.Process(this->Infantry_DimWhenDisabled)
		.Process(this->Convert_HumanToComputer)
		.Process(this->Convert_ComputerToHuman)
		.Process(this->TalkbubbleVoices)
		.Process(this->HarvesterDumpAmount)
		.Process(this->NoExtraSelfHealOrRepair)

#pragma region BuildLimitGroup
		.Process(this->BuildLimitGroup_Types)
		.Process(this->BuildLimitGroup_Nums)
		.Process(this->BuildLimitGroup_Factor)
		.Process(this->BuildLimitGroup_ContentIfAnyMatch)
		.Process(this->BuildLimitGroup_NotBuildableIfQueueMatch)
		.Process(this->BuildLimitGroup_ExtraLimit_Types)
		.Process(this->BuildLimitGroup_ExtraLimit_Nums)
		.Process(this->BuildLimitGroup_ExtraLimit_MaxCount)
		.Process(this->BuildLimitGroup_ExtraLimit_MaxNum)
#pragma endregion

		.Process(this->Tint_Color)
		.Process(this->Tint_Intensity)
		.Process(this->Tint_VisibleToHouses)

		.Process(this->PhobosAttachEffects)

		.Process(this->KeepTargetOnMove)
		.Process(this->KeepTargetOnMove_ExtraDistance)
		.Process(this->ForbidParallelAIQueues)

		.Process(this->EVA_Combat)
		.Process(this->CombatAlert)
		.Process(this->CombatAlert_NotBuilding)
		.Process(this->SubterraneanHeight)

		.Process(this->Spawner_RecycleRange)
		.Process(this->Spawner_RecycleFLH)
		.Process(this->Spawner_RecycleOnTurret)
		.Process(this->Spawner_RecycleAnim)

		.Process(this->HugeBar)
		.Process(this->HugeBar_Priority)
		.Process(this->SprayOffsets)
		.Process(this->AINormalTargetingDelay)
		.Process(this->PlayerNormalTargetingDelay)
		.Process(this->AIGuardAreaTargetingDelay)
		.Process(this->PlayerGuardAreaTargetingDelay)
		.Process(this->DistributeTargetingFrame)
		.Process(this->CanBeBuiltOn)
		.Process(this->UnitBaseNormal)
		.Process(this->UnitBaseForAllyBuilding)
		.Process(this->ChronoSpherePreDelay)
		.Process(this->ChronoSphereDelay)
		.Process(this->PassengerWeapon)


		.Process(this->RefinerySmokeParticleSystemOne)
		.Process(this->RefinerySmokeParticleSystemTwo)
		.Process(this->RefinerySmokeParticleSystemThree)
		.Process(this->RefinerySmokeParticleSystemFour)

		.Process(this->SubterraneanSpeed)

		.Process(this->ForceWeapon_InRange)
		.Process(this->ForceWeapon_InRange_Overrides)
		.Process(this->ForceWeapon_InRange_ApplyRangeModifiers)

		.Process(this->UnitIdleRotateTurret)
		.Process(this->UnitIdlePointToMouse)

		.Process(this->FallingDownDamage)
		.Process(this->FallingDownDamage_Water)
		.Process(this->DropCrate)

		.Process(this->WhenCrushed_Warhead)
		.Process(this->WhenCrushed_Weapon)
		.Process(this->WhenCrushed_Damage)
		.Process(this->WhenCrushed_Warhead_Full)
		.Process(this->Convert_ToHouseOrCountry)

		.Process(this->SuppressKillWeapons)
		.Process(this->SuppressKillWeapons_Types)
		;
}

// =============================
// container
TechnoTypeExtContainer TechnoTypeExtContainer::Instance;

bool TechnoTypeExtContainer::Load(TechnoTypeClass* key, IStream* pStm)
{
	// this really shouldn't happen
	if (!key)
	{
		//Debug::Log("[LoadKey] Attempted for a null pointer! WTF!\n");
		return false;
	}

	auto ptr = TechnoTypeExtContainer::Instance.Map.get_or_default(key);

	if (!ptr) {
		ptr = TechnoTypeExtContainer::Instance.Map.insert_unchecked(key, this->AllocateUnchecked(key));
	}

	this->ClearExtAttribute(key);
	this->SetExtAttribute(key, ptr);

	PhobosByteStream loader { 0 };
	if (loader.ReadBlockFromStream(pStm))
	{
		PhobosStreamReader reader { loader };
		if (reader.Expect(TechnoTypeExtData::Canary)
			&& reader.RegisterChange(ptr))
		{
			ptr->LoadFromStream(reader);
			if (reader.ExpectEndOfBlock())
				return true;
		}
	}

	return false;
}

// =============================
// container hooks

DEFINE_HOOK(0x711835, TechnoTypeClass_CTOR, 0x5)
{
	GET(TechnoTypeClass* , pItem, ESI);

	auto ptr = TechnoTypeExtContainer::Instance.Map.get_or_default(pItem);

	if (!ptr) {
		ptr = TechnoTypeExtContainer::Instance.Map.insert_unchecked(pItem,
			  TechnoTypeExtContainer::Instance.AllocateUnchecked(pItem));
	}

	TechnoTypeExtContainer::Instance.SetExtAttribute(pItem, ptr);

	return 0;
}

DEFINE_HOOK(0x711AE0, TechnoTypeClass_DTOR, 0x5)
{
	GET(TechnoTypeClass*, pItem, ECX);

	auto extData = TechnoTypeExtContainer::Instance.GetExtAttribute(pItem);
	TechnoTypeExtContainer::Instance.ClearExtAttribute(pItem);
	TechnoTypeExtContainer::Instance.Map.erase(pItem);
	if(extData)
		DLLCallDTOR(extData);

	return 0;
}

DEFINE_HOOK_AGAIN(0x716DC0, TechnoTypeClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x7162F0, TechnoTypeClass_SaveLoad_Prefix, 0x6)
{
	GET_STACK(TechnoTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	TechnoTypeExtContainer::Instance.PrepareStream(pItem, pStm);

	return 0;
}

// S/L very early so we properly trigger `Load3DArt` without need to reconstruct the ExtData !

DEFINE_HOOK(0x716429, TechnoTypeClass_Load_Suffix, 0x6)
{
	TechnoTypeExtContainer::Instance.LoadStatic();

	return 0;
}

DEFINE_HOOK(0x716DDE, TechnoTypeClass_Save_Suffix, 0x6)
{
	TechnoTypeExtContainer::Instance.SaveStatic();

	return 0;
}

DEFINE_HOOK_AGAIN(0x716132, TechnoTypeClass_LoadFromINI, 0x5) // this should make the techno unusable ? becase the game will return false when it
DEFINE_HOOK(0x716123, TechnoTypeClass_LoadFromINI, 0x5)
{
	GET(TechnoTypeClass*, pItem, EBP);
	GET_STACK(CCINIClass*, pINI, 0x380);

	if (R->Origin() == 0x716132) {
		if(!pItem->Strength) {
			pItem->Strength = 1;
		}
	}

	TechnoTypeExtContainer::Instance.LoadFromINI(pItem, pINI, R->Origin() == 0x716132);

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
//	if (auto pExt = TechnoTypeExtContainer::Instance.Find(pItem))
//		pExt->LoadFromINIFile_Aircraft(pINI);
//
//	return 0x41CD82;
//}