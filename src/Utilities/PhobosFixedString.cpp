#include "PhobosFixedString.h"

#include <CCINIClass.h>
#include <Phobos.h>

#define DECLARE_READ(sz) bool PhobosFixedString<sz>::Read(INIClass* pINI, const char* pSection, const char* pKey, const char* pDefault) \
{ \
	if (pINI->ReadString(pSection, pKey, pDefault, Phobos::readBuffer, FixedString<sz>::max_size()) > 0) {  \
		if (!GameStrings::IsBlank(Phobos::readBuffer)) { *this = Phobos::readBuffer; } else { *this = nullptr; } } return Phobos::readBuffer[0] != 0; \
}

template <std::size_t Capacity>
bool PhobosFixedString<Capacity>::Read(INIClass* pINI, const char* pSection, const char* pKey, const char* pDefault)
{
	if (pINI->ReadString(pSection, pKey, pDefault, Phobos::readBuffer, FixedString<Capacity>::max_size()) > 0)
	{
		if (!GameStrings::IsBlank(Phobos::readBuffer)) {
			*this = Phobos::readBuffer;
		} else 	{
			*this = nullptr;
		}
	}
	return Phobos::readBuffer[0] != 0;
}

DECLARE_READ(2)
DECLARE_READ(4)
DECLARE_READ(10)
DECLARE_READ(16)
DECLARE_READ(24)
DECLARE_READ(25)
DECLARE_READ(32)
DECLARE_READ(64)
DECLARE_READ(100)
DECLARE_READ(128)

#undef DECLARE_READ