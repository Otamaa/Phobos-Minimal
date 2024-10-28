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
, BaseTag { "Versus." }
, FF_Tag {}
, RT_Tag {}
, PA_Tag {}
, HitAnim_Tag { "HitAnim." }
{
	BaseTag += pTitle;
	FF_Tag = BaseTag + ".ForceFire";
	RT_Tag = BaseTag + ".Retaliate";
	PA_Tag = BaseTag + ".PassiveAcquire";
	HitAnim_Tag += pTitle;

}

const char* Enumerable<ArmorTypeClass>::GetMainSection()
{
	return "ArmorTypes";
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
		pINI->ReadString(Enumerable<ArmorTypeClass>::GetMainSection(), pName, Phobos::readDefval, buffer);

		if (!this->DefaultVersesValue.Parse(buffer))
		{
			this->DefaultString = buffer;
		}
	}
}

void ArmorTypeClass::EvaluateDefault()
{
	for (size_t i = 0; i < Array.size(); ++i) {
		auto pArmor = &Array[i];

		if (IsDefault(pArmor->Name.data()) || pArmor->DefaultString.empty() || !strlen(pArmor->DefaultString.c_str()))
			continue;

		if (pArmor->DefaultTo == -1)
		{
			const auto nDefault = FindIndexById(pArmor->DefaultString.c_str());
			const auto pDefault = &Array[nDefault];

			if ((int)i < nDefault)
			{
				Debug::Log("[Phobos] Armor[%d - %s] Trying to reference Armor[%d - %s] with higher array index from itself !\n",
				i, pArmor->Name.data(),
				nDefault, pDefault->Name.data());
			}

			pArmor->DefaultTo = nDefault;
		}
	}
}

void ArmorTypeClass::LoadFromINIList_New(CCINIClass* pINI, bool bDebug)
{
	if (Array.empty())
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
		if (auto const pAlloc = FindOrAllocate(pKeyHere))
			pAlloc->LoadFromINI(pINI);
	}
}

void ArmorTypeClass::LoadForWarhead(CCINIClass* pINI, WarheadTypeClass* pWH)
{
	auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWH);
	const char* section = pWH->get_ID();

	INI_EX exINI(pINI);
	//for (size_t i = pWHExt->Verses.size(); i < Array.size(); ++i)
	//{
	//	auto& pArmor = Array[i];
	//	const int nDefaultIdx = pArmor->DefaultTo;
	//	pWHExt->Verses.push_back((nDefaultIdx == -1 || nDefaultIdx > (int)i)
	//			? pArmor->DefaultVersesValue
	//			: pWHExt->Verses[nDefaultIdx]
	//	);
	//}

	for (size_t i = 0; i < pWHExt->Verses.size(); ++i)
	{
		const auto pArmor = &ArmorTypeClass::Array[i];

		if (exINI.ReadString(section, pArmor->BaseTag.c_str()) > 0)
		{
			pWHExt->Verses[i].Parse_NoCheck(exINI.value());
		}
		else
		{
			if (pArmor->DefaultTo != -1)
			{
				const auto nDefault = pArmor->DefaultTo;
				if ((int)i < nDefault)
				{
					const auto pDefault = &ArmorTypeClass::Array[nDefault];
					if (exINI.ReadString(section, pDefault->BaseTag.c_str()) > 0)
					{
						pWHExt->Verses[nDefault].Parse_NoCheck(exINI.value());
					}
				}

				pWHExt->Verses[i] = pWHExt->Verses[nDefault];
			}
		}

		pWHExt->Verses[i].Flags.ForceFire = pINI->ReadBool(section, pArmor->FF_Tag.c_str(), pWHExt->Verses[i].Flags.ForceFire);
		pWHExt->Verses[i].Flags.Retaliate = pINI->ReadBool(section, pArmor->RT_Tag.c_str(), pWHExt->Verses[i].Flags.Retaliate);
		pWHExt->Verses[i].Flags.PassiveAcquire = pINI->ReadBool(section, pArmor->PA_Tag.c_str(), pWHExt->Verses[i].Flags.PassiveAcquire);
	}
}

void ArmorTypeClass::LoadForWarhead_NoParse(WarheadTypeClass* pWH)
{
	auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWH);
	const char* section = pWH->get_ID();

	for (size_t i = 0; i < pWHExt->Verses.size(); ++i)
	{
		const auto pArmor = &ArmorTypeClass::Array[i];

		if (pArmor->DefaultTo != -1)
		{
			const size_t nDefault = (size_t)pArmor->DefaultTo;

			if (i < nDefault)
			{
				Debug::Log("Warhead - ret NoSection - [%s] - Armor [%d - %s] Trying to reference to it default [%d - %s] armor value but it not yet parsed ! \n",
					section, i, pArmor->Name.data(), nDefault, ArmorTypeClass::Array[nDefault].Name.data());

				pWHExt->Verses[i] = ArmorTypeClass::Array[nDefault].DefaultVersesValue;

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
