#pragma once

#include <CellStruct.h>
#include <Utilities/Enum.h>

class PhobosStreamReader;
class PhobosStreamWriter;
struct TargetResult
{
	CellStruct Target;
	SWTargetFlags Flags;

public:

	bool load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool save(PhobosStreamWriter& Stm) const;
};
