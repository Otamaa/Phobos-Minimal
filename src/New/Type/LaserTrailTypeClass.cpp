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
	auto debugProcess = [&Stm](auto& field, const char* fieldName) -> auto&
		{
			if constexpr (std::is_same_v<T, PhobosStreamWriter>)
			{
				size_t beforeSize = Stm.Getstream()->Size();
				auto& result = Stm.Process(field);
				size_t afterSize = Stm.Getstream()->Size();
				GameDebugLog::Log("[LaserTrailTypeClass] SAVE %s: size %zu -> %zu (+%zu)\n",
					fieldName, beforeSize, afterSize, afterSize - beforeSize);
				return result;
			}
			else
			{
				size_t beforeOffset = Stm.Getstream()->Offset();
				bool beforeSuccess = Stm.Success();
				auto& result = Stm.Process(field);
				size_t afterOffset = Stm.Getstream()->Offset();
				bool afterSuccess = Stm.Success();

				GameDebugLog::Log("[LaserTrailTypeClass] LOAD %s: offset %zu -> %zu (+%zu), success: %s -> %s\n",
					fieldName, beforeOffset, afterOffset, afterOffset - beforeOffset,
					beforeSuccess ? "true" : "false", afterSuccess ? "true" : "false");

				if (!afterSuccess && beforeSuccess)
				{
					GameDebugLog::Log("[LaserTrailTypeClass] ERROR: %s caused stream failure!\n", fieldName);
				}
				return result;
			}

		};
	//Debug::LogInfo("Processing Element From LaserTrailTypeClass ! ");
	debugProcess(this->IsHouseColor, "IsHouseColor");
	debugProcess(this->Color, "Color");
	debugProcess(this->FadeDuration, "FadeDuration");
	debugProcess(this->Thickness, "Thickness");
	debugProcess(this->SegmentLength, "SegmentLength");
	debugProcess(this->IgnoreVertical, "IgnoreVertical");
	debugProcess(this->IsIntense, "IsIntense");
	debugProcess(this->InitialDelay, "InitialDelay");
	debugProcess(this->CloakVisible, "CloakVisible");
	debugProcess(this->CloakVisible_Houses, "CloakVisible_Houses");
	debugProcess(this->DroppodOnly, "DroppodOnly");
	debugProcess(this->Permanent, "Permanent");
	debugProcess(this->DrawType, "DrawType");
	debugProcess(this->IsAlternateColor, "IsAlternateColor");
	debugProcess(this->Bolt_Color, "Bolt_Color");
	debugProcess(this->Bolt_Disable, "Bolt_Disable");
	debugProcess(this->Bolt_Arcs, "Bolt_Arcs");
	debugProcess(this->Beam_Color, "Beam_Color");
	debugProcess(this->Beam_Amplitude, "Beam_Amplitude");
}

void LaserTrailTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void LaserTrailTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}
