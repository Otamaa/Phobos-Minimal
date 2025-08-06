#include "SpyAffects .h"

#include <CCINIClass.h>

void SpyAffectTypes::Read(CCINIClass* pINI)
{
	if (!this->Owner)
		Debug::FatalError("SpyAffectTypes Has no Owner ! ");

	INI_EX exINI(pINI);
	auto pSection = this->Owner->ID;

	this->SpyEffect_Custom.Read(exINI, pSection, "SpyEffect.Custom");
	this->SpyEffect_VictimSuperWeapon.Read(exINI, pSection, "SpyEffect.VictimSuperWeapon");
	if (this->SpyEffect_VictimSuperWeapon.isset())
		this->SpyEffect_VictimSW_RealLaunch.Read(exINI, pSection, "SpyEffect.VictimSuperWeapon.RealLaunch");

	this->SpyEffect_InfiltratorSuperWeapon.Read(exINI, pSection, "SpyEffect.InfiltratorSuperWeapon");
	if (this->SpyEffect_InfiltratorSuperWeapon.isset())
		this->SpyEffect_InfiltratorSW_JustGrant.Read(exINI, pSection, "SpyEffect.InfiltratorSuperWeapon.JustGrant");

	this->SpyEffect_RevealProduction.Read(exINI, pSection, "SpyEffect.RevealProduction");
	this->SpyEffect_ResetSW.Read(exINI, pSection, "SpyEffect.ResetSuperweapons");
	this->SpyEffect_ResetRadar.Read(exINI, pSection, "SpyEffect.ResetRadar");
	this->SpyEffect_RevealRadar.Read(exINI, pSection, "SpyEffect.RevealRadar");
	this->SpyEffect_RevealRadarPersist.Read(exINI, pSection, "SpyEffect.KeepRadar");
	this->SpyEffect_GainVeterancy.Read(exINI, pSection, "SpyEffect.UnitVeterancy");

	ValueableVector<int> SpyEffect_StolenTechIndex;
	SpyEffect_StolenTechIndex.Read(exINI, pSection, "SpyEffect.StolenTechIndex");

	auto pos = SpyEffect_StolenTechIndex.begin();
	const auto end = SpyEffect_StolenTechIndex.end();

	if (pos != end)
	{
		this->SpyEffect_StolenTechIndex_result.reset();
		do
		{
			if ((*pos) > -1 && (*pos) < MaxHouseCount)
			{
				this->SpyEffect_StolenTechIndex_result.set((*pos));
			}
			else if ((*pos) != -1)
			{
				Debug::LogInfo("BuildingType {} has a SpyEffect.StolenTechIndex of {}. The value has to be less than 32.", pSection, (*pos));
				Debug::RegisterParserError();
			}

		}
		while (++pos != end);
	}

	this->SpyEffect_PowerOutageDuration.Read(exINI, pSection, "SpyEffect.PowerOutageDuration");
	this->SpyEffect_StolenMoneyAmount.Read(exINI, pSection, "SpyEffect.StolenMoneyAmount");
	this->SpyEffect_StolenMoneyPercentage.Read(exINI, pSection, "SpyEffect.StolenMoneyPercentage");
	this->SpyEffect_SabotageDelay.Read(exINI, pSection, "SpyEffect.SabotageDelay");
	this->SpyEffect_SuperWeapon.Read(exINI, pSection, "SpyEffect.SuperWeapon");
	this->SpyEffect_SuperWeaponPermanent.Read(exINI, pSection, "SpyEffect.SuperWeaponPermanent");
	this->SpyEffect_UnReverseEngineer.Read(exINI, pSection, "SpyEffect.UndoReverseEngineer");

	this->SpyEffect_InfantryVeterancy.Read(exINI, pSection, "SpyEffect.InfantryVeterancy"); { }
	this->SpyEffect_VehicleVeterancy.Read(exINI, pSection, "SpyEffect.VehicleVeterancy");
	this->SpyEffect_NavalVeterancy.Read(exINI, pSection, "SpyEffect.NavalVeterancy");
	this->SpyEffect_AircraftVeterancy.Read(exINI, pSection, "SpyEffect.AircraftVeterancy");
	this->SpyEffect_BuildingVeterancy.Read(exINI, pSection, "SpyEffect.BuildingVeterancy");

	this->SpyEffect_SellDelay.Read(exINI, pSection, "SpyEffect.SellDelay");

	this->SpyEffect_Anim.Read(exINI, pSection, "SpyEffect.Anim");
	this->SpyEffect_Anim_Duration.Read(exINI, pSection, "SpyEffect.Anim.Duration");
	this->SpyEffect_Anim_DisplayHouses.Read(exINI, pSection, "SpyEffect.Anim.DisplayHouses");

	this->SpyEffect_SWTargetCenter.Read(exINI, pSection, "SpyEffect.SWTargetCenter");
}

void SpyAffectTypes::Apply(BuildingClass* entered, HouseClass* enterer)
{


}

template <typename T>
bool SpyAffectTypes::Serialize(T& Stm)
{
	return Stm
	.Process(this->Owner)
	.Process(this->SpyEffect_Custom)
	.Process(this->SpyEffect_VictimSuperWeapon)
	.Process(this->SpyEffect_InfiltratorSuperWeapon)
	.Process(this->SpyEffect_InfiltratorSW_JustGrant)
	.Process(this->SpyEffect_VictimSW_RealLaunch)
	.Process(this->SpyEffect_RevealProduction)
	.Process(this->SpyEffect_ResetSW)
	.Process(this->SpyEffect_ResetRadar)
	.Process(this->SpyEffect_RevealRadar)
	.Process(this->SpyEffect_RevealRadarPersist)
	.Process(this->SpyEffect_GainVeterancy)
	.Process(this->SpyEffect_UnReverseEngineer)
	.Process(this->SpyEffect_StolenTechIndex_result)
	.Process(this->SpyEffect_StolenMoneyAmount)
	.Process(this->SpyEffect_StolenMoneyPercentage)
	.Process(this->SpyEffect_PowerOutageDuration)
	.Process(this->SpyEffect_SabotageDelay)
	.Process(this->SpyEffect_SuperWeapon)
	.Process(this->SpyEffect_SuperWeaponPermanent)
	.Process(this->SpyEffect_InfantryVeterancy)
	.Process(this->SpyEffect_VehicleVeterancy)
	.Process(this->SpyEffect_NavalVeterancy)
	.Process(this->SpyEffect_AircraftVeterancy)
	.Process(this->SpyEffect_BuildingVeterancy)
	.Process(this->SpyEffect_SellDelay)
	.Process(this->SpyEffect_Anim)
	.Process(this->SpyEffect_Anim_Duration)
	.Process(this->SpyEffect_Anim_DisplayHouses)
	.Process(this->SpyEffect_SWTargetCenter)
	.Success();
}
