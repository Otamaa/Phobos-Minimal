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
		ValueableVector<WarheadTypeClass*>* allow { nullptr };
		ValueableVector<WarheadTypeClass*>* disallow { nullptr };

		COMPILETIMEEVAL bool Eligible(WarheadTypeClass* who) const
		{
			return AEIsEligible(who, allow, disallow);
		}

		bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
		{
			return this->Serialize(Stm);
		}

		bool Save(PhobosStreamWriter& Stm) const
		{
			return const_cast<CritData*>(this)->Serialize(Stm);
		}

	protected:

		template <typename T>
		bool Serialize(T& Stm)
		{
			return Stm
				.Process(this->Mult)
				.Process(this->extra)
				.Process(this->allow)
				.Process(this->disallow)
				.Success() && Stm.RegisterChange(this)
				;
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

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		return this->Serialize(Stm);
	}

	bool Save(PhobosStreamWriter& Stm) const
	{
		return const_cast<AEExtraCrit*>(this)->Serialize(Stm);
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
