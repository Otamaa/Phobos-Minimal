#pragma once
#include <Utilities/Template.h>
#include <Utilities/GeneralUtils.h>

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
		ValueableVector<WeaponTypeClass*>* allow { nullptr };
		ValueableVector<WeaponTypeClass*>* disallow { nullptr };

		COMPILETIMEEVAL bool Eligible(WeaponTypeClass* who) const {
			return AEIsEligible(who, allow, disallow);
		}

		bool Load(PhobosStreamReader& Stm, bool RegisterForChange) {
			return this->Serialize(Stm);
		}

		bool Save(PhobosStreamWriter& Stm) const {
			return const_cast<RangeData*>(this)->Serialize(Stm);
		}

	protected:

		template <typename T>
		bool Serialize(T& Stm)
		{
			return Stm
				.Process(this->rangeMult)
				.Process(this->extraRange)
				.Process(this->allow)
				.Process(this->disallow)
				.Success() && Stm.RegisterChange(this)
				;
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

			initial = GeneralUtils::SafeMultiply(initial , MaxImpl(entry.rangeMult, 0.0));
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
			initial = GeneralUtils::SafeMultiply(initial , MaxImpl(entry.rangeMult, 0.0));
			add += static_cast<int>(entry.extraRange);
		}

		return initial + add;
	}

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		return this->Serialize(Stm);
	}

	bool Save(PhobosStreamWriter& Stm) const
	{
		return const_cast<AEExtraRange*>(this)->Serialize(Stm);
	}

protected:

	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(this->ranges)
			.Success() && Stm.RegisterChange(this)
			;
	}
};
