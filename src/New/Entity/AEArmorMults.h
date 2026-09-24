#pragma once

#include <Utilities/Template.h>
#include <Utilities/EnumFunctions.h>

#include <WarheadTypeClass.h>
#include <New/Entity/AEEligible.h>

struct AEArmorMults
{
	struct MultData
	{
		double Mult { 1.0 };
		double Chance { 1.0 };
		int delay {};
		ValueableVector<AnimTypeClass*>* HitAnims { nullptr };
		ValueableVector<WarheadTypeClass*>* allow { nullptr };
		ValueableVector<WarheadTypeClass*>* disallow { nullptr };
		AffectedHouse  allowhouse { AffectedHouse::All };
		CDTimerClass ArmorMultiplierTimer {};

	public:

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
				.Process(this->Chance)
				.Process(this->delay)
				.Process(this->HitAnims)
				.Process(this->allow)
				.Process(this->disallow)
				.Process(this->allowhouse)
				.Process(this->ArmorMultiplierTimer)
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

	double Get(double initial, WarheadTypeClass* who, TechnoClass* pOwner, HouseClass* pInvoker, bool playHitAnim, bool isReallyHit);


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
