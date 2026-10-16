#include "InsigniaTypeClass.h"

Enumerable<InsigniaTypeClass>::container_t Enumerable<InsigniaTypeClass>::Array;

void InsigniaTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* section = this->Name.data();

	INI_EX exINI(pINI);

	this->Insignia.Read(exINI, section, "Insignia.%s");
	this->Palette.Read(exINI, section, "InsigniaPalette");
	this->Palette_Rookie.Read(exINI, section, "InsigniaPalette.Rookie");
	this->Palette_Veteran.Read(exINI, section, "InsigniaPalette.Veteran");
	this->Palette_Elite.Read(exINI, section, "InsigniaPalette.Elite");
	this->InsigniaFrame.Read(exINI, section, "InsigniaFrame.%s");
}

template <typename T>
void InsigniaTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->Insignia)
		.Process(this->Palette)
		.Process(this->Palette_Rookie)
		.Process(this->Palette_Veteran)
		.Process(this->Palette_Elite)
		.Process(this->InsigniaFrame)
		;
};

void InsigniaTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void InsigniaTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}


bool InsigniaDataEntry::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return this->Serialize(stm);
}

bool InsigniaDataEntry::Save(PhobosStreamWriter& stm) const
{
	return const_cast<InsigniaDataEntry*>(this)->Serialize(stm);
}

void InsigniaDataEntry::Read(INI_EX exINI, const char* pSection, bool gunner,
		size_t passengers,
		size_t weaponCount,
		InsigniaDataEntry& entry,
		std::vector<InsigniaDataEntry>& weaponsData,
		std::vector<InsigniaDataEntry>& PassengersData)
{
	entry.Type.Read(exINI, pSection, "InsigniaType");
	entry.Shape.Read(exINI, pSection, "Insignia.%s");
	entry.Palette.Read(exINI, pSection, "InsigniaPalette");
	entry.Palette_Rookie.Read(exINI, pSection, "InsigniaPalette.Rookie");
	entry.Palette_Veteran.Read(exINI, pSection, "InsigniaPalette.Veteran");
	entry.Palette_Elite.Read(exINI	, pSection, "InsigniaPalette.Elite");
	entry.Frame.Read(exINI, pSection, "InsigniaFrame.%s");
	entry.Frames.Read(exINI, pSection, "InsigniaFrames");

	char tempBuffer[Phobos::readLength];

	if (gunner) {
		weaponsData.resize(weaponCount);

		for (size_t idx = 0; idx < weaponCount; ++idx) {
			auto& data = weaponsData[idx];

			_snprintf_s(tempBuffer, sizeof(tempBuffer), "InsigniaType.Weapon%d", idx + 1);
			data.Type.Read(exINI, pSection, tempBuffer);

			_snprintf_s(tempBuffer, sizeof(tempBuffer), "Insignia.Weapon%d.%s", idx + 1, "%s");
			data.Shape.Read(exINI, pSection, tempBuffer);

			_snprintf_s(tempBuffer, sizeof(tempBuffer), "InsigniaPalette.Weapon%d", idx + 1);
			data.Palette.Read(exINI, pSection, tempBuffer);

			_snprintf_s(tempBuffer, sizeof(tempBuffer), "InsigniaPalette.Weapon%d.Rookie", idx + 1);
			data.Palette_Rookie.Read(exINI, pSection, tempBuffer);

			_snprintf_s(tempBuffer, sizeof(tempBuffer), "InsigniaPalette.Weapon%d.Veteran", idx + 1);
			data.Palette_Veteran.Read(exINI, pSection, tempBuffer);

			_snprintf_s(tempBuffer, sizeof(tempBuffer), "InsigniaPalette.Weapon%d.Elite", idx + 1);
			data.Palette_Elite.Read(exINI, pSection, tempBuffer);

			_snprintf_s(tempBuffer, sizeof(tempBuffer), "InsigniaFrame.Weapon%d.%s", idx + 1, "%s");
			data.Frame.Read(exINI, pSection, tempBuffer);

			_snprintf_s(tempBuffer, sizeof(tempBuffer), "InsigniaFrames.Weapon%d", idx + 1);
			data.Frames.Read(exINI, pSection, tempBuffer);
		}
	}

	if (((int)passengers) > 0) {

		passengers += 1;
		PassengersData.resize(passengers);

		for (size_t idx = 0; idx < passengers; ++idx) {
			auto& data = PassengersData[idx];

			_snprintf_s(tempBuffer, sizeof(tempBuffer), "InsigniaType.Passengers%d", idx);
			data.Type.Read(exINI, pSection, tempBuffer);

			_snprintf_s(tempBuffer, sizeof(tempBuffer), "Insignia.Passengers%d.%s", idx, "%s");
			data.Shape.Read(exINI, pSection, tempBuffer);

			_snprintf_s(tempBuffer, sizeof(tempBuffer), "InsigniaPalette.Passengers%d", idx);
			data.Palette.Read(exINI, pSection, tempBuffer);

			_snprintf_s(tempBuffer, sizeof(tempBuffer), "InsigniaPalette.Passengers%d.Rookie", idx);
			data.Palette_Rookie.Read(exINI, pSection, tempBuffer);

			_snprintf_s(tempBuffer, sizeof(tempBuffer), "InsigniaPalette.Passengers%d.Veteran", idx);
			data.Palette_Veteran.Read(exINI, pSection, tempBuffer);

			_snprintf_s(tempBuffer, sizeof(tempBuffer), "InsigniaPalette.Passengers%d.Elite", idx);
			data.Palette_Elite.Read(exINI, pSection, tempBuffer);

			_snprintf_s(tempBuffer, sizeof(tempBuffer), "InsigniaFrame.Passengers%d.%s", idx, "%s");
			data.Frame.Read(exINI, pSection, tempBuffer);

			_snprintf_s(tempBuffer, sizeof(tempBuffer), "InsigniaFrames.Passengers%d", idx);
			data.Frames.Read(exINI, pSection, tempBuffer);
		}
	}
}

template <typename T>
bool InsigniaDataEntry::Serialize(T& stm)
{
	return stm
		.Process(this->Type)
		.Process(this->Shape)
		.Process(this->Palette)
		.Process(this->Palette_Rookie)
		.Process(this->Palette_Veteran)
		.Process(this->Palette_Elite)
		.Process(this->Frame)
		.Process(this->Frames)
		.Success();
}
