#pragma once

#include <WarheadTypeClass.h>
#include <Helpers/Macro.h>

struct VersesData : public WarheadFlags
{
	double Verses;
	
	VersesData() :
		WarheadFlags { true, true, true } , Verses { 1.0 }
	{ };

	VersesData(double VS, bool FF, bool Retal, bool Acquire) :
		WarheadFlags { FF, Retal, Acquire } , Verses { VS }
	{ };

	~VersesData() noexcept = default;

	bool operator ==(const VersesData& RHS) const
	{
		return (CLOSE_ENOUGH(this->Verses, RHS.Verses));
	}

	bool Parse(const char* str);
	void Parse_NoCheck(const char* str);
};
