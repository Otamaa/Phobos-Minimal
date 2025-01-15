#pragma once

#include "INIParser.h"
#include "Stream.h"
#include <GeneralDefinitions.h>

class TranslucencyLevel
{
public:
	COMPILETIMEEVAL TranslucencyLevel() noexcept = default;

	COMPILETIMEEVAL TranslucencyLevel(int nInt)
	{
		*this = nInt;
	}

	COMPILETIMEEVAL TranslucencyLevel(const TranslucencyLevel& other) = default;
	COMPILETIMEEVAL TranslucencyLevel& operator=(const TranslucencyLevel& other) = default;
	COMPILETIMEEVAL ~TranslucencyLevel() = default;

	COMPILETIMEEVAL TranslucencyLevel& operator = (int nInt)
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

	COMPILETIMEEVAL operator BlitterFlags() const
	{
		return this->value;
	}

	COMPILETIMEEVAL BlitterFlags GetBlitterFlags() const
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

	OPTIONALINLINE bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		Stm.Load(this->value);
		return true;
	}

	OPTIONALINLINE bool Save(PhobosStreamWriter& Stm) const
	{
		Stm.Save(this->value);
		return true;
	}

private:
	BlitterFlags value { BlitterFlags::None };
};