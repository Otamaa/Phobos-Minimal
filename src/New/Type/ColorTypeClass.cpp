#include "ColorTypeClass.h"

#include <Utilities/TemplateDef.h>
#include <Utilities/GeneralUtils.h>

Enumerable<ColorTypeClass>::container_t Enumerable<ColorTypeClass>::Array;

const char* Enumerable<ColorTypeClass>::GetMainSection()
{
	return GameStrings::Colors();
}

void ColorTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* pName = this->Name.data();

	INI_EX exINI(pINI);
	this->Colors.Read(exINI, GetMainSection(), pName, false);

	Point3D nColors = this->Colors.Get();
	nColors.X = std::clamp(nColors.X, 0, 255);
	nColors.Y = std::clamp(nColors.Y, 0, 255);
	nColors.Z = std::clamp(nColors.Z, 0, 255);
	this->Colors = nColors;
}

void ColorTypeClass::LoadFromINIList_New(CCINIClass* pINI, bool bDebug)
{
	if (!pINI)
		return;

	const char* pSection = GetMainSection();

	if (!pINI->GetSection(pSection))
		return;

	auto const pkeyCount = pINI->GetKeyCount(pSection);

	if (!pkeyCount)
		return;

	Array.reserve(pkeyCount);

	for (int i = 0; i < pkeyCount; ++i) {
		if (auto const pAlloc = FindOrAllocate(pINI->GetKeyName(pSection, i)))
			pAlloc->LoadFromINI(pINI);
	}
}

template <typename T>
void ColorTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->Colors)
		;
}

void ColorTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void ColorTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}