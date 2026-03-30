#pragma once
#include <Utilities/Template.h>

#include <WeaponTypeClass.h>
#include <New/Entity/AEEligible.h>

struct AEExtraRange
{
	// Zero-copy entry: stores pointers to the source type's allow/disallow
	// lists instead of copying VectorSets every Recalculate cycle.
	// The source AE type objects are long-lived, so pointers remain valid.
	struct RangeData
	{
		double rangeMult { 1.0 };
		double extraRange { 0.0 };
		const ValueableVector<WeaponTypeClass*>* allow { nullptr };
		const ValueableVector<WeaponTypeClass*>* disallow { nullptr };

		COMPILETIMEEVAL bool Eligible(WeaponTypeClass* who) const
		{
			return AEIsEligible(who, allow, disallow);
		}
	};

	// Pre-filtered output for external consumers that don't need eligibility
	struct RangeDataOut
	{
		double rangeMult { 1.0 };
		double extraRange { 0.0 };
	};

	std::vector<RangeData> ranges {};

	COMPILETIMEEVAL void Clear()
	{
		ranges.clear();
	}

	COMPILETIMEEVAL bool Enabled() const
	{
		return !ranges.empty();
	}

	COMPILETIMEEVAL int Get(int initial, WeaponTypeClass* who) const
	{
		int add = 0;
		for (const auto& entry : ranges)
		{
			if (!entry.Eligible(who))
				continue;

			initial = static_cast<int>(initial * MaxImpl(entry.rangeMult, 0.0));
			add += static_cast<int>(entry.extraRange);
		}

		return initial + add;
	}

	COMPILETIMEEVAL void FillEligible(WeaponTypeClass* who, std::vector<RangeDataOut>& eligible) const
	{
		for (const auto& entry : ranges)
		{
			if (entry.Eligible(who))
				eligible.emplace_back(entry.rangeMult, entry.extraRange);
		}
	}

	static COMPILETIMEEVAL int Count(int initial, const std::vector<RangeDataOut>& eligible)
	{
		int add = 0;
		for (const auto& entry : eligible)
		{
			initial = static_cast<int>(initial * MaxImpl(entry.rangeMult, 0.0));
			add += static_cast<int>(entry.extraRange);
		}

		return initial + add;
	}
};
