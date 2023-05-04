#pragma once

#include <WarheadTypeClass.h>
#include <Helpers/Macro.h>

struct VersesData
{
	double Verses;
	WarheadFlags Flags;

	VersesData() : Verses { 1.0 }
		, Flags { true, true, true } 
	{ };

	VersesData(double VS, bool FF, bool Retal, bool Acquire) : Verses { VS }
		, Flags { FF, Retal, Acquire }
	{ };

	~VersesData() noexcept = default;

	bool operator ==(const VersesData& RHS) const
	{
		return (CLOSE_ENOUGH(this->Verses, RHS.Verses));
	}

	bool Parse(const char* str);
	void Parse_NoCheck(const char* str);
};

struct WarheadVersesData : public VersesData
{
	bool IsReadedAlready;
	double Verses;
	WarheadFlags Flags;

	bool operator ==(const WarheadVersesData& RHS) const
	{
		return (CLOSE_ENOUGH(this->Verses, RHS.Verses));
	}
};