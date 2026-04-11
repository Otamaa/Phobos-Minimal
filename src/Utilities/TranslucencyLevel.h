#pragma once

#include <YRMath.h>
#include <Base/Always.h>
#include <Utilities/Enum.h>

class INI_EX;
class PhobosStreamReader;
class PhobosStreamWriter;
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

	bool Read(INI_EX& parser, const char* pSection, const char* pKey);
	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

private:
	BYTE value;
};