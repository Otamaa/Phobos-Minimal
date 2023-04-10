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
	for (auto const& nDefault : Unsorted::ArmorNameArray) {
		FindOrAllocate(nDefault);
	}
}

bool ArmorTypeClass::IsDefault(const char* pName)
{
	for (auto const& nDefault : Unsorted::ArmorNameArray) {
		if (IS_SAME_STR_(pName, nDefault))
			return true;
	}

	return false;
}

void ArmorTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* pName = this->Name.data();

	//Dont need to load these for default !
	if (!IsDefault(pName)) {

		INI_EX exINI(pINI);

		char buffer[0x64];
		pINI->ReadString(Enumerable<ArmorTypeClass>::GetMainSection(), pName, "", buffer);

		if (!this->DefaultVerses.Parse(buffer)) {
			this->DefaultString = buffer;
		}
	}
}

void ArmorTypeClass::EvaluateDefault() {
	if (!IsDefault(Name.data()) && DefaultTo == -1) {
		DefaultTo = FindIndexById(DefaultString.data());
		DefaultString = "\0";
	}
}

void ArmorTypeClass::LoadFromINIList_New(CCINIClass* pINI, bool bDebug)
{
	if(ArmorTypeClass::Array.empty())
		ArmorTypeClass::AddDefaults();

	if (!pINI)
		return;
	
	const char* pSection = GetMainSection();

	if (!pINI->GetSection(pSection))
		return;

	auto const pkeyCount = pINI->GetKeyCount(pSection);
	
	if (!pkeyCount)
		return;

	Array.reserve(pkeyCount + Unsorted::ArmorNameArray.size());

	for (int i = 0; i < pkeyCount; ++i) {
		if (auto const pAlloc = FindOrAllocate(pINI->GetKeyName(pSection, i)))
			pAlloc->LoadFromINI(pINI);
	}
}

void ArmorTypeClass::LoadForWarhead(CCINIClass* pINI, WarheadTypeClass* pWH)
{
	auto pWHExt = WarheadTypeExt::ExtMap.Find(pWH);

	if (!pWHExt) {
		return;
	}

	pWHExt->Verses.reserve(Array.size());

	while (pWHExt->Verses.size() < Array.size())
	{
		auto& pArmor = Array[pWHExt->Verses.size()];
		const int nDefaultIdx = pArmor->DefaultTo;
		pWHExt->Verses.push_back(
			nDefaultIdx == -1
				? pArmor->DefaultVerses
				: pWHExt->Verses[nDefaultIdx]
		);
	}

	char buffer[0x500];
	char ret[0x64];
	const char* section = pWH->get_ID();
	constexpr const char* const nVersus = "Versus";

	for (size_t i = 0; i < Array.size(); ++i) {

		auto nVers = &pWHExt->Verses[i];
		const auto pName = Array[i]->Name.data();
		
		_snprintf_s(buffer, _TRUNCATE, "%s.%s", nVersus, pName);
		if (pINI->ReadString(section, buffer, Phobos::readDefval, ret)) {
			nVers->Parse_NoCheck(ret);
		}

		_snprintf_s(buffer, _TRUNCATE, "%s.%s.ForceFire", nVersus, pName);
		nVers->Flags.ForceFire = pINI->ReadBool(section, buffer, nVers->Flags.ForceFire);

		_snprintf_s(buffer, _TRUNCATE, "%s.%s.Retaliate", nVersus, pName);
		nVers->Flags.Retaliate = pINI->ReadBool(section, buffer, nVers->Flags.Retaliate);

		_snprintf_s(buffer, _TRUNCATE, "%s.%s.PassiveAcquire", nVersus, pName);
		nVers->Flags.PassiveAcquire = pINI->ReadBool(section, buffer, nVers->Flags.PassiveAcquire);

	}
}

template <typename T>
void ArmorTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->DefaultTo)
		.Process(this->DefaultString)
		.Process(this->DefaultVerses)
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
