#include "TheaterTypeClass.h"

#include <Phobos.h>

Enumerable<TheaterTypeClass>::container_t Enumerable<TheaterTypeClass>::Array;

const char* Enumerable<TheaterTypeClass>::GetMainSection()
{
	return "TheaterTypes";
}

void TheaterTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* pSection = this->Name.data();

	if (!pINI->GetSection(pSection))
		return;

	INI_EX exIni(pINI);
	UIName.Read(exIni, pSection, "UIName");
	ControlFileName.Read(pINI, pSection, "ControlFileName");
	ArtFileName.Read(pINI, pSection, "ArtFileName");
	PaletteFileName.Read(pINI, pSection, "PaletteFileName");
	Extension.Read(pINI, pSection, "Extension");
	MMExtension.Read(pINI, pSection, "MMExtension");
	Letter.Read(pINI, pSection, "ImageLetter");
	IsArctic.Read(exIni, pSection, "IsArtic");
	IsAllowedInMapGenerator.Read(exIni, pSection, "IsAllowedInMapGenerator");
	LowRadarBrightness1.Read(exIni, pSection, "LowRadarBrightness");
}

bool TheaterTypeClass::IsDefaultTheater()
{
	return !_strcmpi(Name.data(), "TEMPERATE")
		|| !_strcmpi(Name.data(), "SNOW")
		|| !_strcmpi(Name.data(), "URBAN")
		|| !_strcmpi(Name.data(), "DESERT")
		|| !_strcmpi(Name.data(), "NEWURBAN")
		|| !_strcmpi(Name.data(), "LUNAR");
}

CCINIClass* TheaterTypeClass::GetConfigINI()
{
	auto nFileName = "THEATERS.INI";

	if (CCINIClass* pINI = GameCreate<CCINIClass>())
	{
		if (auto pTheaterIni = GameCreate<CCFileClass>(nFileName))
		{
			if (pTheaterIni->Exists())
			{
				pINI->ReadCCFile(pTheaterIni);
				return pINI;
			}
			else
			{
				Debug::Log("%s not found!\n", nFileName);
			}
		}
	}

	return nullptr;
}

void TheaterTypeClass::LoadConfiguration(CCINIClass* pINI)
{
	if (pINI)
	{
		const char* pSection = TheaterTypeClass::GetMainSection();
		if (pINI->GetSection(pSection))
		{
			if (pINI->ReadString(pSection, this->Name.data(), "", Phobos::readBuffer))
			{
				LoadFromINI(pINI);
				Debug::Log("Trying to Read %s \"%s\".\n", pSection, Phobos::readBuffer);
			}
		}
	}
}

void TheaterTypeClass::LoadAllTheatersToArray()
{
	TheaterTypeClass::AddDefaults();

	if (auto pINI = GetConfigINI())
	{
		const char* pSection = TheaterTypeClass::GetMainSection();
		if (pINI->GetSection(pSection))
		{
			for (int i = 0; i < pINI->GetKeyCount(pSection); ++i)
			{
				if (pINI->ReadString(pSection, pINI->GetKeyName(pSection, i), "", Phobos::readBuffer))
				{
					if (auto pTheater = FindOrAllocate(Phobos::readBuffer))
						pTheater->LoadFromINI(pINI);
					else
						Debug::Log("Error Reading %s \"%s\".\n", pSection, Phobos::readBuffer);
				}
			}
		}

		GameDelete(pINI);
		pINI = nullptr;
	}
}

void TheaterTypeClass::AddDefaults()
{
	if (auto pTem = FindOrAllocate("TEMPERATE"))
	{
		pTem->UIName = "Name:Temperate";
		pTem->ControlFileName = "TEMPERAT";
		pTem->ArtFileName = "ISOTEMP";
		pTem->PaletteFileName = "ISOTEM";
		pTem->Extension = "TEM";
		pTem->MMExtension = "MMT";
		pTem->Letter = "T";
		pTem->IsArctic = false;
		pTem->IsAllowedInMapGenerator = true;
		pTem->LowRadarBrightness1 = 1.0f;
	}

	if (auto pSno = FindOrAllocate("SNOW"))
	{
		pSno->UIName = "Name:Snow";
		pSno->ControlFileName = "SNOW";
		pSno->ArtFileName = "ISOSNOW";
		pSno->PaletteFileName = "ISOSNO";
		pSno->Extension = "SNO";
		pSno->MMExtension, "MMS";
		pSno->Letter = "A";
		pSno->IsArctic = true;
		pSno->IsAllowedInMapGenerator = true;
		pSno->LowRadarBrightness1 = 0.8f;
	}

	if (auto pUrb = FindOrAllocate("URBAN"))
	{
		pUrb->UIName = "Name:Urban";
		pUrb->ControlFileName = "URBAN";
		pUrb->ArtFileName = "ISOURB";
		pUrb->PaletteFileName = "ISOURB";
		pUrb->Extension = "URB";
		pUrb->MMExtension = "MMU";
		pUrb->Letter = "U";
		pUrb->IsArctic = false;
		pUrb->IsAllowedInMapGenerator = true;
		pUrb->LowRadarBrightness1 = 1.0f;
	}
	if (auto pDes = FindOrAllocate("DESERT"))
	{
		pDes->UIName = "Name:Desert";
		pDes->ControlFileName = "DESERT";
		pDes->ArtFileName = "ISODES";
		pDes->PaletteFileName = "ISODES";
		pDes->Extension = "DES";
		pDes->MMExtension = "MMD";
		pDes->Letter = "D";
		pDes->IsArctic = false;
		pDes->IsAllowedInMapGenerator = true;
		pDes->LowRadarBrightness1 = 1.0f;
	}

	if (auto pNewUrb = FindOrAllocate("NEWURBAN"))
	{
		pNewUrb->UIName = "Name:New Urban";
		pNewUrb->ControlFileName = "URBANN";
		pNewUrb->ArtFileName = "ISOUBN";
		pNewUrb->PaletteFileName = "ISOUBN";
		pNewUrb->Extension = "UBN";
		pNewUrb->MMExtension = "MMT";
		pNewUrb->Letter = "N";
		pNewUrb->IsArctic = false;
		pNewUrb->IsAllowedInMapGenerator = false;
		pNewUrb->LowRadarBrightness1 = 1.0f;
	}

	if (auto pLun = FindOrAllocate("LUNAR"))
	{
		pLun->UIName = "Name:Lunar";
		pLun->ControlFileName = "LUNAR";
		pLun->ArtFileName = "ISOLUN";
		pLun->PaletteFileName = "ISOLUN";
		pLun->Extension = "LUN";
		pLun->MMExtension = "MML";
		pLun->Letter = "L";
		pLun->IsArctic = false;
		pLun->IsAllowedInMapGenerator = false;
		pLun->LowRadarBrightness1 = 1.0f;
	}
}


const TheaterTypeClass& TheaterTypeClass::As_Reference(TheaterType nType)
{
	static const TheaterTypeClass _x;
	return nType == TheaterType::None || (size_t)nType > Array.size() ? _x : *Array[(int)nType];
}

const TheaterTypeClass* TheaterTypeClass::As_Pointer(TheaterType nType)
{
	return nType != TheaterType::None && (size_t)nType < Array.size() ? Array[(int)nType].get() : nullptr;
}

TheaterTypeClass* TheaterTypeClass::As_Pointer_(TheaterType nType)
{
	return nType != TheaterType::None && (size_t)nType < Array.size() ? Array[(int)nType].get() : nullptr;
}

const TheaterTypeClass& TheaterTypeClass::As_Reference(const char* pName)
{
	return As_Reference(From_Name(pName));
}

const TheaterTypeClass* TheaterTypeClass::As_Pointer(const char* pName)
{
	return As_Pointer(From_Name(pName));
}

TheaterType TheaterTypeClass::From_Name(const char* pName)
{
	if (CCINIClass::IsBlank(pName))
		return TheaterType::None;

	if (pName != nullptr && std::strlen(pName))
		return (TheaterType)FindIndex(pName);

	return TheaterType::None;
}

const char* TheaterTypeClass::GetIdentifier(TheaterType type)
{
	return (type != TheaterType::None && (size_t)type < Array.size() ? As_Reference(type).Name.data() : "<none>");
}

const char* TheaterTypeClass::GetUIName(TheaterType type)
{
	return (type != TheaterType::None && (size_t)type < Array.size() ? As_Reference(type).UIName.Get().Label.data() : "Name:<none>");
}

const char* TheaterTypeClass::GetControlFileName(TheaterType type)
{
	return As_Reference(type).ControlFileName.data();
}

const char* TheaterTypeClass::GetArtFileName(TheaterType type) { return As_Reference(type).ArtFileName.c_str(); }
const char* TheaterTypeClass::GetPaletteFileName(TheaterType type) { return As_Reference(type).PaletteFileName.c_str(); }
const char* TheaterTypeClass::GetExtension(TheaterType type) { return As_Reference(type).Extension.c_str(); }
char* TheaterTypeClass::GetCharExtension(TheaterType type) { return _strdup(As_Reference(type).Extension.data()); }
const char* TheaterTypeClass::GetMMExtension(TheaterType type) { return As_Reference(type).MMExtension.c_str(); }
const char* TheaterTypeClass::GetLetter(TheaterType type) { return As_Reference(type).Letter.c_str(); }

bool TheaterTypeClass::GetIsArtic(TheaterType type) { return As_Reference(type).IsArctic.Get(); }
bool TheaterTypeClass::GetAllowMapGen(TheaterType type) { return As_Reference(type).IsAllowedInMapGenerator.Get(); }

float TheaterTypeClass::GetLowRadarBrightness(TheaterType type, bool bSecond)
{ return As_Reference(type).LowRadarBrightness1.Get(); }