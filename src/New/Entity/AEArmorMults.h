#pragma once

#include <Utilities/Template.h>

#include <WarheadTypeClass.h>
#include <New/Entity/AEEligible.h>

struct AEArmorMults
{
	struct MultData
	{
		double Mult { 1.0 };
		const ValueableVector<WarheadTypeClass*>* allow { nullptr };
		const ValueableVector<WarheadTypeClass*>* disallow { nullptr };

		COMPILETIMEEVAL bool Eligible(WarheadTypeClass* who) const
		{
			return AEIsEligible(who, allow, disallow);
		}
	};

	std::vector<MultData> mults {};

	COMPILETIMEEVAL void Clear()
	{
		mults.clear();
	}

	COMPILETIMEEVAL bool Enabled() const
	{
		return !mults.empty();
	}

	COMPILETIMEEVAL double Get(double initial, WarheadTypeClass* who) const
	{
		for (const auto& entry : mults)
		{
			if (!entry.Eligible(who))
				continue;

			initial *= entry.Mult;
		}

		return initial;
	}

	COMPILETIMEEVAL void FillEligible(WarheadTypeClass* who, std::vector<double>& eligible) const
	{
		for (const auto& entry : mults)
		{
			if (entry.Eligible(who))
				eligible.emplace_back(entry.Mult);
		}
	}

	static COMPILETIMEEVAL double Apply(double initial, const std::vector<double>& eligible)
	{
		for (const auto& mult : eligible)
			initial *= mult;

		return initial;
	}
};
