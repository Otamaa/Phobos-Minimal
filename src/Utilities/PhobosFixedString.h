#pragma once
#include <Helpers/String.h>
#include <CCINIClass.h>

// fixed string with read method
template <std::size_t Capacity>
class PhobosFixedString : public FixedString<Capacity>
{
public:
	PhobosFixedString() = default;
	explicit PhobosFixedString(nullptr_t) noexcept { };
	explicit PhobosFixedString(const char* value) noexcept : FixedString<Capacity>(value) { }

	using FixedString<Capacity>::operator=;

	bool Read(INIClass* pINI, const char* pSection, const char* pKey, const char* pDefault = "")
	{
		if (pINI->ReadString(pSection, pKey, pDefault, Phobos::readBuffer, FixedString<Capacity>::Size))
		{
			if (!INIClass::IsBlank(Phobos::readBuffer))
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

	inline const char* c_str() const
	{ return ((*this).data()); }
};