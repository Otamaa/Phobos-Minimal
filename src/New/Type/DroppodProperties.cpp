#include "DroppodProperties.h"

#include <Ext/Rules/Body.h>

void DroppodProperties::Read(INI_EX& exINI, const char* pSection)
{
	this->Droppod_PodImage_Infantry.Read(exINI, pSection, "DropPod.PodImageInfantry");
	this->Droppod_Puff.Read(exINI, pSection, "DropPod.Puff");
	this->Droppod_Angle.Read(exINI, pSection, "DropPod.Angle");

	if (this->Droppod_Angle >= Math::PI_SQRT_TWO_BY_FOUR) {
		this->Droppod_Angle = Math::PI_SQRT_TWO_BY_FOUR;
	}

	if (this->Droppod_Angle <= Math::PI_BY_EIGHT) {
		this->Droppod_Angle = Math::PI_BY_EIGHT;
	}

	this->Droppod_Speed.Read(exINI, pSection, "DropPod.Speed");
	this->Droppod_Height.Read(exINI, pSection, "DropPod.Height");
	this->Droppod_Weapon.Read(exINI, pSection, "DropPod.Weapon");
	this->Droppod_GroundPodAnim.Read(exINI, pSection, "DropPod.Pods");

	this->Droppod_Trailer.Read(exINI, pSection, "DropPod.Trailer");
	this->Droppod_Trailer_Attached.Read(exINI, pSection, "DropPod.Trailer.Attached");
	this->Droppod_Trailer_SpawnDelay.Read(exINI, pSection, "DropPod.Trailer.SpawnDelay");
	this->Droppod_AtmosphereEntry.Read(exINI, pSection, "DropPod.AtmosphereEntry");
}

void DroppodProperties::Initialize()
{
	this->Droppod_PodImage_Infantry = RulesExtData::Instance()->Droppod_ImageInfantry;
	this->Droppod_Puff = RulesClass::Instance->DropPodPuff;
	this->Droppod_Angle = RulesClass::Instance->DropPodAngle;
	this->Droppod_Speed = RulesClass::Instance->DropPodSpeed;
	this->Droppod_Height = RulesClass::Instance->DropPodHeight;
	this->Droppod_Weapon = RulesClass::Instance->DropPodWeapon;

	for (auto const Pod : RulesClass::Instance->DropPod)
		this->Droppod_GroundPodAnim.push_back(Pod);

	this->Droppod_Trailer = RulesExtData::Instance()->DropPodTrailer;
	this->Droppod_AtmosphereEntry = RulesClass::Instance->AtmosphereEntry;
	this->Droppod_Trailer_SpawnDelay = RulesExtData::Instance()->DroppodTrailerSpawnDelay;
}

void NullableDroppodProperties::Read(INI_EX& exINI, const char* pSection)
{

	this->Droppod_PodImage_Infantry.Read(exINI, pSection, "DropPod.PodImageInfantry");
	this->Droppod_Puff.Read(exINI, pSection, "DropPod.Puff");
	this->Droppod_Angle.Read(exINI, pSection, "DropPod.Angle");

	if (this->Droppod_Angle.isset() && this->Droppod_Angle >= Math::PI_SQRT_TWO_BY_FOUR) {
		this->Droppod_Angle = Math::PI_SQRT_TWO_BY_FOUR;
	}

	if (this->Droppod_Angle.isset() && this->Droppod_Angle <= Math::PI_BY_EIGHT) {
		this->Droppod_Angle = Math::PI_BY_EIGHT;
	}

	this->Droppod_Speed.Read(exINI, pSection, "DropPod.Speed");
	this->Droppod_Height.Read(exINI, pSection, "DropPod.Height");
	this->Droppod_Weapon.Read(exINI, pSection, "DropPod.Weapon");
	this->Droppod_GroundPodAnim.Read(exINI, pSection, "DropPod.Pods");

	this->Droppod_Trailer.Read(exINI, pSection, "DropPod.Trailer");
	this->Droppod_Trailer_Attached.Read(exINI, pSection, "DropPod.Trailer.Attached");
	this->Droppod_Trailer_SpawnDelay.Read(exINI, pSection, "DropPod.Trailer.SpawnDelay");
	this->Droppod_AtmosphereEntry.Read(exINI, pSection, "DropPod.AtmosphereEntry");
}