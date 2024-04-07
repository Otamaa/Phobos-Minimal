#pragma once

struct WarheadFlags
{
	bool ForceFire;
	bool Retaliate;
	bool PassiveAcquire;

public:

	bool operator==(WarheadFlags const& nThat) const
	{ return ForceFire == nThat.ForceFire && Retaliate == nThat.Retaliate && PassiveAcquire == nThat.PassiveAcquire; }

	bool operator!=(WarheadFlags const& nThat) const
	{ return !((*this) == nThat); }

};