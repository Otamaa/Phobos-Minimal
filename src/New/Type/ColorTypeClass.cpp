#include "ColorTypeClass.h"

#include <Utilities/TemplateDef.h>
#include <Utilities/GeneralUtils.h>

Enumerable<ColorTypeClass>::container_t Enumerable<ColorTypeClass>::Array;

void ColorTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* pName = this->Name.c_str();

	INI_EX exINI(pINI);
	this->Colors.Read(exINI, ColorTypeClass::MainSection, pName, false);

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

	const char* pSection = ColorTypeClass::MainSection;

	if (!pINI->GetSection(pSection))
		return;

	auto const pkeyCount = pINI->GetKeyCount(pSection);

	if (!pkeyCount)
		return;

	if (pkeyCount > (int)Array.size())
		Array.reserve(pkeyCount);

	for (int i = 0; i < pkeyCount; ++i)	{
		const auto pKeyHere = pINI->GetKeyName(pSection, i);
		if (auto const pAlloc = FindOrAllocate(pKeyHere))
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