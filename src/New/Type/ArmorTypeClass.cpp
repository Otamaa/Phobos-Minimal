#include "ArmorTypeClass.h"

#include <Utilities/TemplateDef.h>
#include <Utilities/GeneralUtils.h>

#include <Ext/WarheadType/Body.h>

#include <Conversions.h>

Enumerable<ArmorTypeClass>::container_t Enumerable<ArmorTypeClass>::Array;

ArmorTypeClass::ArmorTypeClass(const char* const pTitle) : Enumerable<ArmorTypeClass>(pTitle)
, DefaultTo { -1 }
, DefaultString { }
, DefaultVersesValue { }
, IsVanillaArmor { false }
, BaseTag {}
, FF_Tag {}
, RT_Tag {}
, PA_Tag {}
, HitAnim_Tag {}
{
	char buffer[0x100];
	IMPL_SNPRNINTF(buffer, sizeof(buffer), "Versus.%s", pTitle);
	BaseTag = buffer;

	IMPL_SNPRNINTF(buffer, sizeof(buffer), "Versus.%s.ForceFire", pTitle);
	FF_Tag = buffer;

	IMPL_SNPRNINTF(buffer, sizeof(buffer), "Versus.%s.Retaliate", pTitle);
	RT_Tag = buffer;

	IMPL_SNPRNINTF(buffer, sizeof(buffer), "Versus.%s.PassiveAcquire", pTitle);
	PA_Tag = buffer;

	IMPL_SNPRNINTF(buffer, sizeof(buffer), "HitAnim.%s", pTitle);
	HitAnim_Tag = buffer;
}

const char* Enumerable<ArmorTypeClass>::GetMainSection()
{
	return "ArmorTypes";
}

void ArmorTypeClass::AddDefaults()
{
	for (auto const& nDefault : Unsorted::ArmorNameArray)
	{
		if (auto pVanillaArmor = FindOrAllocate(nDefault))
			pVanillaArmor->IsVanillaArmor = true;
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

		if (!this->DefaultVersesValue.Parse(buffer))
		{
			this->DefaultString = buffer;
		}
	}
}

void ArmorTypeClass::EvaluateDefault()
{
	if (IsDefault(this->Name.data()) || !strlen(this->DefaultString.data()))
		return;

	if (DefaultTo == -1)
	{
		const auto pArmor = Find(this->DefaultString.data());

		if (this->ArrayIndex < pArmor->ArrayIndex)
		{
			Debug::Log("[Phobos] Armor[%d - %s] Trying to reference Armor[%d - %s] with higher array index from itself !\n",
			this->ArrayIndex, this->Name.data(),
			pArmor->ArrayIndex, pArmor->Name.data());
		}

		DefaultTo = pArmor->ArrayIndex;
	}
}

void ArmorTypeClass::LoadFromINIList_New(CCINIClass* pINI, bool bDebug)
{
	if(Array.empty())
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
	char ret[0x64];
	const char* section = pWH->get_ID();

	INI_EX exINI(pINI);

	for (size_t i = 0; i < pWHExt->Verses.size(); ++i)
	{
		const auto& pArmor = ArmorTypeClass::Array[i];

		if (pINI->ReadString(section, pArmor->BaseTag.data(), Phobos::readDefval, ret))
		{

			pWHExt->Verses[i].Parse_NoCheck(ret);
		}
		else // no custom value found
		{
			if (pArmor->DefaultTo != -1)
			{ //evaluate armor with valid default index
				const auto nDefault = pArmor->DefaultTo;
				if (i < nDefault)
				{
					Debug::Log("Warhead [%s] - Armor [%d - %s] Trying to reference to it default [%d - %s] armor value but it not yet parsed ! \n",
						section, pArmor->ArrayIndex, pArmor->Name.data(), nDefault, ArmorTypeClass::Array[nDefault]->Name.data());			
				}

				pWHExt->Verses[i] = pWHExt->Verses[nDefault];
			}
		}

		pWHExt->Verses[i].Flags.ForceFire = pINI->ReadBool(section, pArmor->FF_Tag.data(), pWHExt->Verses[i].Flags.ForceFire);
		pWHExt->Verses[i].Flags.Retaliate = pINI->ReadBool(section, pArmor->RT_Tag.data(), pWHExt->Verses[i].Flags.Retaliate);
		pWHExt->Verses[i].Flags.PassiveAcquire = pINI->ReadBool(section, pArmor->PA_Tag.data(), pWHExt->Verses[i].Flags.PassiveAcquire);
	}
}

void ArmorTypeClass::LoadForWarhead_NoParse(WarheadTypeClass* pWH)
{
	auto pWHExt = WarheadTypeExt::ExtMap.Find(pWH);
	const char* section = pWH->get_ID();

	for (size_t i = 0; i < pWHExt->Verses.size(); ++i)
	{
		const auto& pArmor = ArmorTypeClass::Array[i];

		if (pArmor->DefaultTo != -1)
		{ 
			const size_t nDefault = (size_t)pArmor->DefaultTo;

			if (i < nDefault)
			{
				Debug::Log("Warhead - ret NoSection - [%s] - Armor [%d - %s] Trying to reference to it default [%d - %s] armor value but it not yet parsed ! \n",
					section, pArmor->ArrayIndex, pArmor->Name.data(), nDefault, ArmorTypeClass::Array[nDefault]->Name.data());

				pWHExt->Verses[i] = ArmorTypeClass::Array[nDefault]->DefaultVersesValue;

			}
			else
			{
				pWHExt->Verses[i] = pWHExt->Verses[nDefault];
			}
		}
		else if (!pArmor->IsVanillaArmor)
		{ //evaluate armor with not valid default index
			pWHExt->Verses[i] = pArmor->DefaultVersesValue;
		}
	}
}

void ArmorTypeClass::PrepareForWarhead(CCINIClass* pINI, WarheadTypeClass* pWH)
{

}

template <typename T>
void ArmorTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->DefaultTo)
		.Process(this->DefaultString)
		.Process(this->DefaultVersesValue)
		.Process(this->IsVanillaArmor)
		.Process(this->BaseTag)
		.Process(this->FF_Tag)
		.Process(this->RT_Tag)
		.Process(this->PA_Tag)
		.Process(this->HitAnim_Tag)
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
