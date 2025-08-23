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
		*this = Math::StepSnapClamped(nInt, 25, 75, 25);
	}

	COMPILETIMEEVAL TranslucencyLevel(BlitterFlags nInt) : value { nInt }
	{ *this = Math::StepSnapClamped((int)nInt, 25, 75, 25); }

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
		return Stm.Process(this->value, RegisterForChange);
	}

	OPTIONALINLINE bool Save(PhobosStreamWriter& Stm) const
	{
		return Stm.Process(this->value);
	}

private:
	BlitterFlags value { BlitterFlags::None };
};