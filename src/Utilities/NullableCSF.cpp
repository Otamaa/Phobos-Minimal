#include "NullableCSF.h"

#include <CCINIClass.h>
#include <Utilities/GeneralUtils.h>

void NullableCSF::Read(CCINIClass* pINI, const char* pSection, const char* pKey)
{
	if (pINI->ReadString(pSection, pKey, GameStrings::NoneStr(), Phobos::readBuffer) > 0)
	{
		this->Text = GeneralUtils::LoadStringUnlessMissing(Phobos::readBuffer, this->Text.c_str());
	}
}