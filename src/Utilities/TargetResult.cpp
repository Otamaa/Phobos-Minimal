#include "TargetResult.h"

#include <Utilities/SavegameDef.h>

bool TargetResult::load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Stm
		.Process(Target, RegisterForChange)
		.Process(Flags, RegisterForChange)
		.Success();
}

bool TargetResult::save(PhobosStreamWriter& Stm) const
{
	return Stm
		.Process(Target)
		.Process(Flags)
		.Success();
}