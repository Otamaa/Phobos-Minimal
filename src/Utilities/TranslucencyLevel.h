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

	COMPILETIMEEVAL TranslucencyLevel(int nInt, bool clamp)
	{
		if (clamp)
		{
			if (nInt >= 75)
				nInt = 75;
			else if (nInt >= 50)
				nInt = 50;
			else if (nInt >= 25)
				nInt = 25;
			else
				nInt = 0;
		}

		*this = nInt;
	}

	COMPILETIMEEVAL TranslucencyLevel(const TranslucencyLevel& other) = default;
	COMPILETIMEEVAL TranslucencyLevel& operator=(const TranslucencyLevel& other) = default;
	COMPILETIMEEVAL ~TranslucencyLevel() = default;

	COMPILETIMEEVAL int GetIntValue() const
	{
		int _result = 0;

		switch (this->value)
		{
		case BlitterFlags::TransLucent75:
			_result = 75;
			break;
		case BlitterFlags::TransLucent50:
			_result = 50;
			break;
		case BlitterFlags::TransLucent25:
			_result = 25;
			break;
		default:
			break;
		}

		return _result;
	}

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
