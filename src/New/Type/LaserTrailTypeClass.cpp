#include "LaserTrailTypeClass.h"

#include <Utilities/TemplateDef.h>
#include <HouseClass.h>

Enumerable<LaserTrailTypeClass>::container_t Enumerable<LaserTrailTypeClass>::Array;

const char* Enumerable<LaserTrailTypeClass>::GetMainSection()
{
	return "LaserTrailTypes";
}

void LaserTrailTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* section = this->Name.c_str();

	INI_EX exINI(pINI);

	this->IsHouseColor.Read(exINI, section, "IsHouseColor");
	this->Color.Read(exINI, section, "Color");

	this->FadeDuration.Read(exINI, section, "FadeDuration");
	this->Thickness.Read(exINI, section, "Thickness");
	this->SegmentLength.Read(exINI, section, "SegmentLength");
	this->IgnoreVertical.Read(exINI, section, "IgnoreVertical");
	this->IsIntense.Read(exINI, section, "IsIntense");
	this->InitialDelay.Read(exINI, section, "InitialDelay");
	this->CloakVisible.Read(exINI, section, "CloakVisible");
	this->CloakVisible_Houses.Read(exINI, section, "CloakVisible.DetectedOnly");
	this->DroppodOnly.Read(exINI, section, "DropPodOnly");
	this->Permanent.Read(exINI, section, "Permanent");

	this->DrawType.Read(exINI, section, "DrawType");

	this->IsAlternateColor.Read(exINI, section, "IsAlternateColor");

	char tempBuffer[0x40];
	for (int idx = 0; idx < 3; ++idx)
	{
		_snprintf_s(tempBuffer, _TRUNCATE, "Bolt.Color%d", idx + 1);
		this->Bolt_Color[idx].Read(exINI, section, tempBuffer);

		_snprintf_s(tempBuffer, _TRUNCATE, "Bolt.Disable%d", idx + 1);
		this->Bolt_Disable[idx].Read(exINI, section, tempBuffer);
	}

	this->Bolt_Arcs.Read(exINI, section, "Bolt.Arcs");

	this->Beam_Color.Read(exINI, section, "Beam.Color");
	this->Beam_Amplitude.Read(exINI, section, "Beam.Amplitude");

}

template <typename T>
void LaserTrailTypeClass::Serialize(T& Stm)
{
	//Debug::LogInfo("Processing Element From LaserTrailTypeClass ! ");
	Stm
		.Process(this->IsHouseColor)
		.Process(this->Color)
		.Process(this->FadeDuration)
		.Process(this->Thickness)
		.Process(this->SegmentLength)
		.Process(this->IgnoreVertical)
		.Process(this->IsIntense)
		.Process(this->InitialDelay)
		.Process(this->CloakVisible)
		.Process(this->CloakVisible_Houses)
		.Process(this->DroppodOnly)
		.Process(this->Permanent)

		.Process(this->DrawType)
		.Process(this->IsAlternateColor)
		.Process(this->Bolt_Color)
		.Process(this->Bolt_Disable)
		.Process(this->Bolt_Arcs)
		.Process(this->Beam_Color)
		.Process(this->Beam_Amplitude)
		;
}

void LaserTrailTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void LaserTrailTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}
