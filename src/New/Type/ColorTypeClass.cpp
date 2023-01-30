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
	nColors.X = Math::clamp(nColors.X, 0, 255);
	nColors.Y = Math::clamp(nColors.Y, 0, 255);
	nColors.Z = Math::clamp(nColors.Z, 0, 255);
	this->Colors = nColors;
}

void ColorTypeClass::LoadFromINIList_New(CCINIClass* pINI, bool bDebug)
{
	const char* pSection = GetMainSection();
	for (int i = 0; i < pINI->GetKeyCount(pSection); ++i)
	{
		const char* pKey = pINI->GetKeyName(pSection, i);
		if (auto const pAlloc = FindOrAllocate(pKey))
			pAlloc->LoadFromINI(pINI);

		if (bDebug)
			Debug::Log("Allocating ColorTypeClass with Name[%s] at [%d] \n", pKey, i);
	}


	if (bDebug)
		Debug::Log("ColorTypeClass Array count currently [%d]\n", Array.size());
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