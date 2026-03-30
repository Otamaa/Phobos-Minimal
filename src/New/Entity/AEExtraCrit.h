#pragma once
#include <Utilities/Template.h>

#include <WarheadTypeClass.h>
#include <New/Entity/AEEligible.h>

struct AEExtraCrit
{
	struct CritData
	{
		double Mult { 1.0 };
		double extra { 0.0 };
		const ValueableVector<WarheadTypeClass*>* allow { nullptr };
		const ValueableVector<WarheadTypeClass*>* disallow { nullptr };

		COMPILETIMEEVAL bool Eligible(WarheadTypeClass* who) const
		{
			return AEIsEligible(who, allow, disallow);
		}
	};

	struct CritDataOut
	{
		double Mult { 1.0 };
		double extra { 0.0 };
	};

	std::vector<CritData> ranges {};

	COMPILETIMEEVAL void Clear()
	{
		ranges.clear();
	}

	COMPILETIMEEVAL bool Enabled() const
	{
		return !ranges.empty();
	}

	COMPILETIMEEVAL double Get(double initial, WarheadTypeClass* who) const
	{
		double add = 0.0;
		for (const auto& entry : ranges)
		{
			if (!entry.Eligible(who))
				continue;

			initial *= entry.Mult;
			add += entry.extra;
		}

		return initial + add;
	}

	COMPILETIMEEVAL void FillEligible(WarheadTypeClass* who, std::vector<CritDataOut>& eligible) const
	{
		for (const auto& entry : ranges)
		{
			if (entry.Eligible(who))
				eligible.emplace_back(entry.Mult, entry.extra);
		}
	}

	static COMPILETIMEEVAL double Count(double initial, const std::vector<CritDataOut>& eligible)
	{
		double add = 0.0;
		for (const auto& entry : eligible)
		{
			initial *= MaxImpl(entry.Mult, 0.0);
			add += entry.extra;
		}

		return initial + add;
	}
};
