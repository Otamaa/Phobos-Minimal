#include "ArmorTypeClass.h"

#include <Utilities/TemplateDef.h>
#include <Utilities/GeneralUtils.h>

#include <Ext/WarheadType/Body.h>

#include <Conversions.h>

Enumerable<ArmorTypeClass>::container_t Enumerable<ArmorTypeClass>::Array;

ArmorTypeClass::ArmorTypeClass(const char* const pTitle) : Enumerable<ArmorTypeClass>(pTitle)
, DefaultTo { -1 }
, DefaultString {}
, DefaultVersesValue {}
, IsVanillaArmor { false }
, BaseTag { std::string("Versus.") + pTitle }
, FF_Tag { BaseTag + ".ForceFire" }
, RT_Tag { BaseTag + ".Retaliate" }
, PA_Tag { BaseTag + ".PassiveAcquire" }
, HitAnim_Tag { std::string("HitAnim.") + pTitle }
{
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

	// Vanilla armors get their verses from the engine directly
	if (!IsDefault(pName))
	{
		INI_EX exINI(pINI);

		char buffer[0x64];
		pINI->ReadString(ArmorTypeClass::MainSection, pName, Phobos::readDefval, buffer, sizeof(buffer));

		if (!this->DefaultVersesValue.Parse(buffer))
		{
			this->DefaultString = buffer;
		}
	}
}

// ---------------------------------------------------------------------------
// EvaluateDefault — resolves DefaultTo indices for all custom armors
//
// Each custom armor can specify another armor as its "default" in the INI.
// This function resolves that name string to an array index.
//
// Forward references (armor i referencing armor j where j > i) are allowed
// but logged as a warning, since they require special handling during
// warhead verse loading to ensure the referenced armor's data is available.
// ---------------------------------------------------------------------------
void ArmorTypeClass::EvaluateDefault()
{
	for (size_t i = 0; i < Array.size(); ++i)
	{
		auto pArmor = Array[i].get();

		if (IsDefault(pArmor->Name.data()) || pArmor->DefaultString.empty())
			continue;

		if (pArmor->DefaultTo == -1)
		{
			const auto nDefault = FindIndexById(pArmor->DefaultString.c_str());

			if (nDefault < 0 || nDefault >= (int)Array.size())
			{
				Debug::LogInfo("[Phobos] Armor[{} - {}] references unknown default '{}', ignoring.",
					i, pArmor->Name.data(), pArmor->DefaultString.c_str());
				continue;
			}

			if ((int)i < nDefault)
			{
				const auto pDefault = Array[nDefault].get();
				Debug::LogInfo("[Phobos] Armor[{} - {}] forward-references Armor[{} - {}] (higher index). "
					"This is supported but may cause ordering issues if chained.",
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

	const char* pSection = ArmorTypeClass::MainSection;

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

// ---------------------------------------------------------------------------
// EnsureVerseParsed — helper to parse a specific armor's verse if not yet done
//
// Used during forward reference resolution to make sure the referenced
// armor's verse value (and its FF/RT/PA flags) are fully loaded before
// copying them to the referencing armor.
// ---------------------------------------------------------------------------
static void EnsureVerseParsed(
	INI_EX& exINI,
	CCINIClass* pINI,
	const char* section,
	size_t index,
	std::vector<VersesData>& Verses)
{
	if (index >= ArmorTypeClass::Array.size() || index >= Verses.size())
		return;

	// Already parsed — nothing to do
	if (Verses[index].LastParseIsValid)
		return;

	const auto pArmor = ArmorTypeClass::Array[index].get();

	if (exINI.ReadString(section, pArmor->BaseTag.c_str()) > 0)
	{
		Verses[index].Parse_NoCheck(exINI.value());
		Verses[index].LastParseIsValid = true;
	}

	// Always read flags — they may have INI values even without a verse string
	Verses[index].Flags.ForceFire =
		pINI->ReadBool(section, pArmor->FF_Tag.c_str(), Verses[index].Flags.ForceFire);
	Verses[index].Flags.Retaliate =
		pINI->ReadBool(section, pArmor->RT_Tag.c_str(), Verses[index].Flags.Retaliate);
	Verses[index].Flags.PassiveAcquire =
		pINI->ReadBool(section, pArmor->PA_Tag.c_str(), Verses[index].Flags.PassiveAcquire);
}

// ---------------------------------------------------------------------------
// LoadForWarhead — reads per-armor verse data from INI for a warhead
//
// For each armor type, attempts to read "Versus.<ArmorName>" from the
// warhead's INI section. If not found and the armor has a DefaultTo
// reference, copies the referenced armor's verse data instead.
//
// Forward references (armor i copying from armor j where j > i) are
// handled by eagerly parsing the referenced armor's data on demand.
// ---------------------------------------------------------------------------
void ArmorTypeClass::LoadForWarhead(CCINIClass* pINI, WarheadTypeClass* pWH)
{
	auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWH);
	const char* section = pWH->get_ID();

	INI_EX exINI(pINI);
	auto const armorCount = std::min(pWHExt->Verses.size(), ArmorTypeClass::Array.size());

	for (size_t i = 0; i < armorCount; ++i)
	{
		const auto pArmor = ArmorTypeClass::Array[i].get();

		if (exINI.ReadString(section, pArmor->BaseTag.c_str()) > 0)
		{
			// This armor has an explicit verse value in the INI
			pWHExt->Verses[i].Parse_NoCheck(exINI.value());
			pWHExt->Verses[i].LastParseIsValid = true;
		}
		else if (!pWHExt->Verses[i].LastParseIsValid)
		{
			// No explicit value and not yet parsed — try to inherit from default
			if (pArmor->DefaultTo != -1)
			{
				const auto nDefault = (size_t)pArmor->DefaultTo;

				if (nDefault < armorCount)
				{
					// If the default hasn't been parsed yet (forward reference),
					// parse it now so we get the correct value and flags
					EnsureVerseParsed(exINI, pINI, section, nDefault, pWHExt->Verses);

					pWHExt->Verses[i] = pWHExt->Verses[nDefault];
				}
			}
		}

		// Read per-armor flag overrides
		// These always run so modders can override flags independently of verse values
		pWHExt->Verses[i].Flags.ForceFire =
			pINI->ReadBool(section, pArmor->FF_Tag.c_str(), pWHExt->Verses[i].Flags.ForceFire);
		pWHExt->Verses[i].Flags.Retaliate =
			pINI->ReadBool(section, pArmor->RT_Tag.c_str(), pWHExt->Verses[i].Flags.Retaliate);
		pWHExt->Verses[i].Flags.PassiveAcquire =
			pINI->ReadBool(section, pArmor->PA_Tag.c_str(), pWHExt->Verses[i].Flags.PassiveAcquire);
	}
}

// ---------------------------------------------------------------------------
// LoadForWarhead_NoParse — fills verse data for warheads without INI sections
//
// When a warhead has no INI section, verse values can't be read directly.
// Instead, custom armors inherit from their DefaultTo reference or fall
// back to their DefaultVersesValue.
// ---------------------------------------------------------------------------
void ArmorTypeClass::LoadForWarhead_NoParse(WarheadTypeClass* pWH)
{
	auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWH);
	const char* section = pWH->get_ID();

	auto const armorCount = std::min(pWHExt->Verses.size(), ArmorTypeClass::Array.size());

	for (size_t i = 0; i < armorCount; ++i)
	{
		const auto pArmor = ArmorTypeClass::Array[i].get();

		if (pArmor->DefaultTo != -1)
		{
			const size_t nDefault = (size_t)pArmor->DefaultTo;

			if (nDefault >= armorCount)
				continue;

			if (i < nDefault)
			{
				// Forward reference — the default armor hasn't been visited yet
				// in this loop, so its Verses[nDefault] may not be filled.
				// Use the default armor's static DefaultVersesValue instead.
				Debug::LogInfo("[Phobos] Warhead [{}] - Armor [{} - {}] forward-references "
					"default [{} - {}], using static default value.",
					section, i, pArmor->Name.data(),
					nDefault, ArmorTypeClass::Array[nDefault]->Name.data());

				pWHExt->Verses[i] = ArmorTypeClass::Array[nDefault]->DefaultVersesValue;
			}
			else
			{
				// Back reference — default was already processed, copy it
				pWHExt->Verses[i] = pWHExt->Verses[nDefault];
			}
		}
		else if (!pArmor->IsVanillaArmor)
		{
			// No default reference — use the armor's own static default
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
		;
}

void ArmorTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
	this->RebuildTags();
}

void ArmorTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}

void ArmorTypeClass::RebuildTags()
{
	BaseTag = std::string("Versus.") + Name.data();
	FF_Tag = BaseTag + ".ForceFire";
	RT_Tag = BaseTag + ".Retaliate";
	PA_Tag = BaseTag + ".PassiveAcquire";
	HitAnim_Tag = std::string("HitAnim.") + Name.data();
}

void ArmorTypeClass::FreeTags()
{
	// swap with empty strings to actually release memory
	// clear() keeps the allocation, this doesn't
	std::string {}.swap(BaseTag);
	std::string {}.swap(FF_Tag);
	std::string {}.swap(RT_Tag);
	std::string {}.swap(PA_Tag);
	std::string {}.swap(HitAnim_Tag);
}