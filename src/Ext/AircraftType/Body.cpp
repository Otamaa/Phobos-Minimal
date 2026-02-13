#include "Body.h"
#include <Utilities/Macro.h>

#include <Phobos.SaveGame.h>
#include "Body.h"

AircraftTypeExtContainer AircraftTypeExtContainer::Instance;

bool AircraftTypeExtData::LoadFromINI(CCINIClass* pINI, bool parseFailAddr)
{
	if (!this->FootTypeExtData::LoadFromINI(pINI, parseFailAddr))
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
	this->ExtendedAircraftMissions_SmoothMoving.Read(exINI, pSection, "ExtendedAircraftMissions.SmoothMoving");
	this->ExtendedAircraftMissions_EarlyDescend.Read(exINI, pSection, "ExtendedAircraftMissions.EarlyDescend");
	this->ExtendedAircraftMissions_RearApproach.Read(exINI, pSection, "ExtendedAircraftMissions.RearApproach");
	this->ExtendedAircraftMissions_FastScramble.Read(exINI, pSection, "ExtendedAircraftMissions.FastScramble");
	this->ExtendedAircraftMissions_UnlandDamage.Read(exINI, pSection, "ExtendedAircraftMissions.UnlandDamage");

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

bool AircraftTypeExtContainer::LoadAll(const json& root)
{
	this->Clear();

	if (root.contains(AircraftTypeExtContainer::ClassName))
	{
		auto& container = root[AircraftTypeExtContainer::ClassName];

		for (auto& entry : container[AircraftTypeExtData::ClassName]) {

			uint32_t oldPtr = 0;
			if (!ExtensionSaveJson::ReadHex(entry, "OldPtr", oldPtr))
				return false;

			size_t dataSize = entry["datasize"].get<size_t>();
			std::string encoded = entry["data"].get<std::string>();
			auto buffer = this->AllocateNoInit();

			PhobosByteStream loader(dataSize);
			loader.data = std::move(Base64Handler::decodeBase64(encoded, dataSize));
			PhobosStreamReader reader(loader);

			PHOBOS_SWIZZLE_REGISTER_POINTER(oldPtr, buffer, AircraftTypeExtData::ClassName);

			buffer->LoadFromStream(reader);

			if (!reader.ExpectEndOfBlock())
				return false;
		}

		return true;
	}

	return false;
}

bool AircraftTypeExtContainer::SaveAll(json& root)
{
	auto& first_layer = root[AircraftTypeExtContainer::ClassName];

	json _extRoot = json::array();
	for (auto& _extData : AircraftTypeExtContainer::Array) {
		PhobosByteStream saver(sizeof(*_extData));
		PhobosStreamWriter writer(saver);

		_extData->SaveToStream(writer); // write all data to stream

		json entry;
		ExtensionSaveJson::WriteHex(entry, "OldPtr", (uint32_t)_extData);
		entry["datasize"] = saver.data.size();
		entry["data"] = Base64Handler::encodeBase64(saver.data);
		_extRoot.push_back(std::move(entry));
	}

	first_layer[AircraftTypeExtData::ClassName] = std::move(_extRoot);
	return true;
}

void AircraftTypeExtContainer::LoadFromINI(AircraftTypeClass* key, CCINIClass* pINI, bool parseFailAddr)
{
	if (auto ptr = this->Find(key)) {
		if (!pINI) {
			return;
		}

		//load anywhere other than rules
		ptr->LoadFromINI(pINI, parseFailAddr);
		//this function can be called again multiple time but without need to re-init the data
		ptr->SetInitState(InitState::Ruled);
	}

}

void AircraftTypeExtContainer::WriteToINI(AircraftTypeClass* key, CCINIClass* pINI)
{

	if (auto ptr = this->TryFind(key)) {
		if (!pINI) {
			return;
		}

		ptr->WriteToINI(pINI);
	}
}

ASMJIT_PATCH(0x41C91F, AircraftTypeClass_CTOR, 0x5)
{
	GET(AircraftTypeClass*, pItem, ESI);
	AircraftTypeExtContainer::Instance.Allocate(pItem);
	return 0;
}

ASMJIT_PATCH(0x41CA46, AircraftTypeClass_DTOR, 0x6)
{
	GET(AircraftTypeClass*, pItem, ESI);

	AircraftTypeExtContainer::Instance.Remove(pItem);

	return 0;
}

bool FakeAircraftTypeClass::_ReadFromINI(CCINIClass* pINI)
{
	AircraftTypeExtContainer::Instance.Find(this)->Initialize();
	bool status = this->AircraftTypeClass::LoadFromINI(pINI);
	AircraftTypeExtContainer::Instance.LoadFromINI(this, pINI, !status);
	return status;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7E28CC, FakeAircraftTypeClass::_ReadFromINI)
