#pragma once

#include "INIParser.h"
#include "Stream.h"
#include <GeneralDefinitions.h>

class TranslucencyLevel
{
public:
	constexpr TranslucencyLevel() noexcept = default;

	constexpr TranslucencyLevel(int nInt)
	{
		*this = nInt;
	}

	constexpr TranslucencyLevel(const TranslucencyLevel& other) = default;
	constexpr TranslucencyLevel& operator=(const TranslucencyLevel& other) = default;
	constexpr ~TranslucencyLevel() = default;

	constexpr TranslucencyLevel& operator = (int nInt)
	{
		switch (nInt)
		{
		default:
		case 0:
			this->value = BlitterFlags::None;
			break;
		case 25:
			this->value = BlitterFlags::TransLucent25;
			break;
		case 50:
			this->value = BlitterFlags::TransLucent50;
			break;
		case 75:
			this->value = BlitterFlags::TransLucent75;
			break;
		}

		return *this;
	}

	constexpr operator BlitterFlags() const
	{
		return this->value;
	}

	constexpr BlitterFlags GetBlitterFlags() const
	{
		return *this;
	}

	bool Read(INI_EX& parser, const char* pSection, const char* pKey)
	{
		int buf;
		if (parser.ReadInteger(pSection, pKey, &buf))
		{
			*this = buf;
			return true;
		}

		return false;
	}

	inline bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		Stm.Load(this->value);
		return true;
	}

	inline bool Save(PhobosStreamWriter& Stm) const
	{
		Stm.Save(this->value);
		return true;
	}

private:
	BlitterFlags value { BlitterFlags::None };
};