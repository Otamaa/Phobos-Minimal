#include "ImmunityTypeClass.h"

Enumerable<ImmunityTypeClass>::container_t Enumerable<ImmunityTypeClass>::Array;

const char* Enumerable<ImmunityTypeClass>::GetMainSection()
{
	return "ImmunityTypes";
}

void ImmunityTypeClass::LoadFromINIList(CCINIClass * pINI, bool bDebug)
{
	if (!pINI)
		return;

	const char* section = ImmunityTypeClass::GetMainSection();

	if (!pINI->GetSection(section))
		return;

	auto const pKeyCount = pINI->GetKeyCount(section);

	if (!pKeyCount)
		return;

	if (pKeyCount > (int)Array.size())
	{
		Array.reserve(pKeyCount);
	}

	for (int i = 0; i < pKeyCount; ++i)
	{
		if (pINI->ReadString(section, pINI->GetKeyName(section, i),
			Phobos::readDefval, Phobos::readBuffer) > 0)
		{
			FindOrAllocate(Phobos::readBuffer);
		}
	}
}