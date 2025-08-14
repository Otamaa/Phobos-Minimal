#pragma once

#include <WarheadFlags.h>
#include <Helpers/Macro.h>

struct VersesData
{
	double Verses;
	WarheadFlags Flags;
	bool LastParseIsValid;

	VersesData() : Verses { 1.0 }
		, Flags { true, true, true } , LastParseIsValid { false }
	{ };

	VersesData(double VS, bool FF, bool Retal, bool Acquire) : Verses { VS }
		, Flags { FF, Retal, Acquire }, LastParseIsValid { false }
	{ };

	~VersesData() noexcept = default;

	bool operator ==(const VersesData& RHS) const
	{
		return (CLOSE_ENOUGH(this->Verses, RHS.Verses));
	}

	bool Parse(const char* str);
	void Parse_NoCheck(const char* str);

	VersesData(const VersesData& other) = default;
	VersesData& operator=(const VersesData& other) = default;
};
