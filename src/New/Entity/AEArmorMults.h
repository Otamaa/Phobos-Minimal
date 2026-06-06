#pragma once

#include <Utilities/Template.h>

#include <WarheadTypeClass.h>
#include <New/Entity/AEEligible.h>

struct AEArmorMults
{
	struct MultData
	{
		double Mult { 1.0 };
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
			return const_cast<MultData*>(this)->Serialize(Stm);
		}

	protected:

		template <typename T>
		bool Serialize(T& Stm)
		{
			return Stm
				.Process(this->Mult)
				.Process(this->allow)
				.Process(this->disallow)
				.Success() && Stm.RegisterChange(this)
				;
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

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		return this->Serialize(Stm);
	}

	bool Save(PhobosStreamWriter& Stm) const
	{
		return const_cast<AEArmorMults*>(this)->Serialize(Stm);
	}

protected:

	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(this->mults)
			.Success() && Stm.RegisterChange(this)
			;
	}
};
