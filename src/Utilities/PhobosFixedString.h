#pragma once
#include <Helpers/String.h>

class INIClass;

// fixed string with read method
template <std::size_t Capacity>
class PhobosFixedString : public FixedString<Capacity, char>
{
public:
	PhobosFixedString() = default;
	explicit PhobosFixedString(nullptr_t) noexcept { };
	explicit PhobosFixedString(const char* value) noexcept : FixedString<Capacity, char>(value) { }

	using FixedString<Capacity, char>::operator=;

	bool Read(INIClass* pINI, const char* pSection, const char* pKey, const char* pDefault = "");

	OPTIONALINLINE const char* c_str() const
	{ return ((*this).data()); }
};