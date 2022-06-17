#include "ArmorTypeClass.h"

#include <Utilities/TemplateDef.h>
#include <Utilities/GeneralUtils.h>

Enumerable<ArmorTypeClass>::container_t Enumerable<ArmorTypeClass>::Array;

const char* const ArmorTypeClass::DefaultArmorName[11] =
{
	"none",
	"flak",
	"plate",
	"light",
	"medium",
	"heavy",
	"wood",
	"steel",
	"concrete",
	"special_1",
	"special_2",
};

const char* Enumerable<ArmorTypeClass>::GetMainSection()
{
	return "ArmorTypes";
}

void ArmorTypeClass::AddDefaults()
{
	for (auto const& nDefault : DefaultArmorName)
	{
		FindOrAllocate(nDefault);
	}
}

bool ArmorTypeClass::IsDefault(const char* pName)
{

	for (auto const& nDefault : DefaultArmorName)
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

		char buffer[0x40];
		pINI->ReadString(Enumerable<ArmorTypeClass>::GetMainSection(), pName, "", buffer);

		if (GeneralUtils::IsValidString(buffer))
			this->DefaultString = buffer;
	}
}

void ArmorTypeClass::EvaluateDefault()
{
	if (strlen(DefaultString) && !IsDefault(Name.data()) && DefaultTo == -1)
	{
		DefaultTo = FindIndex(DefaultString);
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

	if (!Array.empty())
	{
		for (auto const& pArmor : Array)
		{ pArmor->EvaluateDefault(); }
	}

	if (bDebug)
		Debug::Log("ArmorType Array count currently [%d]\n", ArmorTypeClass::Array.size());
}

template <typename T>
void ArmorTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->DefaultTo)
		.Process(this->DefaultString)
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
