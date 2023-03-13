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

		auto VS = &this->DefaultVerses;

		if (!VS->Parse(buffer)) {
			this->DefaultString = buffer;
		}
	}
}

void ArmorTypeClass::EvaluateDefault() {
	if (!IsDefault(Name.data()) && CRT::strlen(DefaultString.data()) && DefaultTo == -1) {
		DefaultTo = FindIndexById(DefaultString.data());
		DefaultString = "\0";
	}
}

void ArmorTypeClass::LoadFromINIList_New(CCINIClass* pINI, bool bDebug)
{
	ArmorTypeClass::AddDefaults();

	const char* pSection = GetMainSection();
	for (int i = 0; i < pINI->GetKeyCount(pSection); ++i)
	{
		const char* pKey = pINI->GetKeyName(pSection, i);
		if (auto const pAlloc = FindOrAllocate(pKey))
			pAlloc->LoadFromINI(pINI);

		if (bDebug)
			Debug::Log("Allocating ArmorType with Name[%s] at [%d] \n", pKey, i);
	}

	if (bDebug)
		Debug::Log("ArmorType Array count currently [%d]\n", ArmorTypeClass::Array.size());
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

	//for (size_t i = Unsorted::ArmorNameArray.size() - 1;
	//	i < Array.size(); ++i)
	//{
	//	auto& nTypeDef = Array[i];
	//	pWHExt->Verses[i] =
	//	nTypeDef->DefaultTo == -1 ?
	//	nTypeDef->DefaultVerses :
	//	Array[nTypeDef->DefaultTo]->DefaultVerses;
	//}

	char buffer[0x500];
	char ret[0x64];
	const char* section = pWH->get_ID();
	constexpr const char* const nVersus = "Versus";

	//start it from default armor type amount 
	for (size_t i = 0; i < Array.size(); ++i) {

		auto nVers = &pWHExt->Verses[i];

		_snprintf_s(buffer, _TRUNCATE, "%s.%s", nVersus, Array[i]->Name.data());
		if (pINI->ReadString(section, buffer, Phobos::readDefval, ret)) {
			nVers->Parse_NoCheck(ret);
		}

		_snprintf_s(buffer, _TRUNCATE, "%s.%s.ForceFire", nVersus, Array[i]->Name.data());
		nVers->Flags.ForceFire = pINI->ReadBool(section, buffer, nVers->Flags.ForceFire);

		_snprintf_s(buffer, _TRUNCATE, "%s.%s.Retaliate", nVersus, Array[i]->Name.data());
		nVers->Flags.Retaliate = pINI->ReadBool(section, buffer, nVers->Flags.Retaliate);

		_snprintf_s(buffer, _TRUNCATE, "%s.%s.PassiveAcquire", nVersus, Array[i]->Name.data());
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
