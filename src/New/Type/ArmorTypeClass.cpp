#include "ArmorTypeClass.h"

#include <Utilities/TemplateDef.h>
#include <Utilities/GeneralUtils.h>

#include <Ext/WarheadType/Body.h>

#include <Conversions.h>

Enumerable<ArmorTypeClass>::container_t Enumerable<ArmorTypeClass>::Array;

const char* Enumerable<ArmorTypeClass>::GetMainSection()
{
	return "ArmorTypes";
}

void ArmorTypeClass::AddDefaults()
{
	if (!Array.empty())
		return;

	for (auto const& nDefault : Unsorted::ArmorNameArray)
	{
		AllocateNoCheck(nDefault);
	}
}

bool ArmorTypeClass::IsDefault(const char* pName)
{
	for (auto const& nDefault : Unsorted::ArmorNameArray)
	{
		if (IS_SAME_STR_(pName, nDefault))
			return true;
	}

	return false;
}

void ArmorTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* pName = this->Name.data();

	//Dont need to load these for default !
	if (!IsDefault(pName))
	{
		INI_EX exINI(pINI);

		char buffer[0x64];
		pINI->ReadString(Enumerable<ArmorTypeClass>::GetMainSection(), pName, "", buffer);

		if (!this->DefaultVerses.Parse(buffer))
		{
			this->DefaultString = buffer;
		}
	}
}

void ArmorTypeClass::EvaluateDefault()
{
	if (!IsDefault(Name.data()) && DefaultTo == -1)
	{
		DefaultTo = FindIndexById(DefaultString.data());
		DefaultString = "\0";
	}
}

void ArmorTypeClass::LoadFromINIList_New(CCINIClass* pINI, bool bDebug)
{
	ArmorTypeClass::AddDefaults();

	if (!pINI)
		return;

	const char* pSection = GetMainSection();

	if (!pINI->GetSection(pSection))
		return;

	auto const pkeyCount = pINI->GetKeyCount(pSection);

	if (!pkeyCount)
		return;

	if (pkeyCount > (int)Array.size())
		Array.reserve(pkeyCount);

	for (int i = 0; i < pkeyCount; ++i)
	{
		const auto pKeyHere = pINI->GetKeyName(pSection, i);
		if (auto const pAlloc = Find(pKeyHere))
			pAlloc->LoadFromINI(pINI);
		else
			Allocate(pKeyHere)->LoadFromINI(pINI);
	}
}

void ArmorTypeClass::LoadForWarhead(CCINIClass* pINI, WarheadTypeClass* pWH)
{
	auto pWHExt = WarheadTypeExt::ExtMap.Find(pWH);

	if (!pWHExt) {
		return;
	}

	if (pWHExt->Verses.size() < Array.size())
	{
		pWHExt->Verses.reserve(Array.size());

		while (pWHExt->Verses.size() < Array.size())
		{
			auto pArmor = Array[pWHExt->Verses.size()].get();
			const int nDefaultIdx = pArmor->DefaultTo;
			pWHExt->Verses.push_back(
				nDefaultIdx == -1
					? pArmor->DefaultVerses
					: pWHExt->Verses[nDefaultIdx]
			);
		}
	}

	char ret[0x64];
	const char* section = pWH->get_ID();

	for (size_t i = 0; i < Array.size(); ++i)
	{
		auto nVers = &pWHExt->Verses[i];
		if (pINI->ReadString(section, Array[i]->BaseTag.c_str(), Phobos::readDefval, ret)) {
			nVers->Parse_NoCheck(ret);
		}

		nVers->Flags.ForceFire = pINI->ReadBool(section, Array[i]->FF_Tag.c_str(), nVers->Flags.ForceFire);
		nVers->Flags.Retaliate = pINI->ReadBool(section, Array[i]->RT_Tag.c_str(), nVers->Flags.Retaliate);
		nVers->Flags.PassiveAcquire = pINI->ReadBool(section, Array[i]->PA_Tag.c_str(), nVers->Flags.PassiveAcquire);
	}
}

template <typename T>
void ArmorTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->DefaultTo)
		.Process(this->DefaultString)
		.Process(this->DefaultVerses)
		.Process(this->BaseTag)
		.Process(this->FF_Tag)
		.Process(this->RT_Tag)
		.Process(this->PA_Tag)
		;
}

void ArmorTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void ArmorTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}
