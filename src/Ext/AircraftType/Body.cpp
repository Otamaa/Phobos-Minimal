#include "Body.h"
#include <Utilities/Macro.h>

std::vector<AircraftTypeExtData*> Container<AircraftTypeExtData>::Array;
AircraftTypeExtContainer AircraftTypeExtContainer::Instance;

void Container<AircraftTypeExtData>::Clear()
{
	Array.clear();
}

bool AircraftTypeExtData::LoadFromINI(CCINIClass* pINI, bool parseFailAddr)
{
	if (!this->TechnoTypeExtData::LoadFromINI(pINI, parseFailAddr))
		return false;

	auto pThis = This();

	INI_EX exINI(pINI);

	const char* pSection = pThis->ID;
	//const char* pArtSection = pThis->ImageFile;

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
	this->CustomMissileData->Type = pThis;
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
	this->CrashWeapon.Read(exINI, pSection, "Crash.%sWeapon", nullptr, true);

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

	return true;
}

bool AircraftTypeExtContainer::LoadGlobals(PhobosStreamReader& Stm)
{
	return LoadGlobalArrayData(Stm);
}

bool AircraftTypeExtContainer::SaveGlobals(PhobosStreamWriter& Stm)
{
	return SaveGlobalArrayData(Stm);
}

ASMJIT_PATCH(0x41C91F,AircraftTypeClass_CTOR, 0x5)
{
	GET(AircraftTypeClass*, pItem, ESI);
	AircraftTypeExtContainer::Instance.Allocate(pItem);
	return 0;
}

ASMJIT_PATCH(0x41CA46,AircraftTypeClass_DTOR, 0x6)
{
	GET(AircraftTypeClass*, pItem, ESI);

	AircraftTypeExtContainer::Instance.Remove(pItem);

	return 0;
}

bool FakeAircraftTypeClass::_ReadFromINI(CCINIClass* pINI)
{
	bool status = this->AircraftTypeClass::LoadFromINI(pINI);
	AircraftTypeExtContainer::Instance.LoadFromINI(this, pINI, !status);
	return status;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7E28CC, FakeAircraftTypeClass::_ReadFromINI)
