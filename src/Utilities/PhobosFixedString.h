#pragma once
#include <Helpers/String.h>
#include <CCINIClass.h>
#include <Phobos.h>

// fixed string with read method
template <std::size_t Capacity>
class PhobosFixedString : public FixedString<Capacity, char>
{
public:
	PhobosFixedString() = default;
	explicit PhobosFixedString(nullptr_t) noexcept { };
	explicit PhobosFixedString(const char* value) noexcept : FixedString<Capacity, char>(value) { }

	using FixedString<Capacity, char>::operator=;

	bool Read(INIClass* pINI, const char* pSection, const char* pKey, const char* pDefault = "")
	{
		if (pINI->ReadString(pSection, pKey, pDefault, Phobos::readBuffer, FixedString<Capacity>::Size) > 0)
		{
			if (!GameStrings::IsBlank(Phobos::readBuffer))
			{
				*this = Phobos::readBuffer;
			}
			else
			{
				*this = nullptr;
			}
		}
		return Phobos::readBuffer[0] != 0;
	}

	OPTIONALINLINE const char* c_str() const
	{ return ((*this).data()); }
};