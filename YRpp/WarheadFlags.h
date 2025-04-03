#pragma once

#include <Base/Always.h>

struct WarheadFlags
{
	bool ForceFire;
	bool Retaliate;
	bool PassiveAcquire;

public:

	COMPILETIMEEVAL bool operator==(WarheadFlags const& nThat) const
	{ return ForceFire == nThat.ForceFire && Retaliate == nThat.Retaliate && PassiveAcquire == nThat.PassiveAcquire; }

	COMPILETIMEEVAL bool operator!=(WarheadFlags const& nThat) const
	{ return !((*this) == nThat); }

};
