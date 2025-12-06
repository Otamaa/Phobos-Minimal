#pragma once

#include <YRMath.h>
#include <Utilities/SavegameDef.h>
#include <Utilities/Macro.h>
#include <Utilities/INIParser.h>

class TranslucencyLevel
{
public:
	COMPILETIMEEVAL TranslucencyLevel() noexcept = default;

	COMPILETIMEEVAL TranslucencyLevel(int nInt, bool clamp = false) :
		value(clamp ? (BYTE)Math::StepSnapClamped(nInt, 25, 75, 25) : (BYTE)nInt)
	{ }

	COMPILETIMEEVAL TranslucencyLevel(BlitterFlags nInt) : value {}
	{ this->operator=(Math::StepSnapClamped((int)nInt, 25, 75, 25)); }

	COMPILETIMEEVAL TranslucencyLevel(const TranslucencyLevel& other) = default;
	COMPILETIMEEVAL TranslucencyLevel& operator=(const TranslucencyLevel& other) = default;
	COMPILETIMEEVAL ~TranslucencyLevel() = default;

	COMPILETIMEEVAL int GetIntValue() const {
		return this->value;
	}

	COMPILETIMEEVAL TranslucencyLevel& operator = (int nInt)
	{
		switch (nInt)
		{
		default:
		case 0:
			this->value = (BYTE)BlitterFlags::None;
			break;
		case 25:
			this->value = (BYTE)BlitterFlags::TransLucent25;
			break;
		case 50:
			this->value = (BYTE)BlitterFlags::TransLucent50;
			break;
		case 75:
			this->value = (BYTE)BlitterFlags::TransLucent75;
			break;
		}

		return *this;
	}

	COMPILETIMEEVAL operator BlitterFlags() const
	{
		return (BlitterFlags)this->value;
	}

	COMPILETIMEEVAL BlitterFlags GetBlitterFlags() const
	{
		return (BlitterFlags)this->value;
	}

	bool Read(INI_EX& parser, const char* pSection, const char* pKey)
	{
		int buf;
		if (parser.ReadInteger(pSection, pKey, &buf)) {
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
	BYTE value;
};