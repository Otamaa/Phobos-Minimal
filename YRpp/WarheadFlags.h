#pragma once

struct WarheadFlags
{
	bool ForceFire;
	bool Retaliate;
	bool PassiveAcquire;

public:

	constexpr bool operator==(WarheadFlags const& nThat) const
	{ return ForceFire == nThat.ForceFire && Retaliate == nThat.Retaliate && PassiveAcquire == nThat.PassiveAcquire; }

	constexpr bool operator!=(WarheadFlags const& nThat) const
	{ return !((*this) == nThat); }

};