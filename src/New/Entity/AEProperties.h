#pragma once

#include <Utilities/SavegameDef.h>
#include <Utilities/VectorSet.h>

class TechnoClass;
class WarheadTypeClass;
struct AEProperties
{
	double FirepowerMultiplier { 1.0 };
	double ArmorMultiplier { 1.0 };
	double SpeedMultiplier { 1.0 };
	double ROFMultiplier { 1.0 };
	double ReceiveRelativeDamageMult { 1.0 };

	double SpeedBonus { 0.0 };
	int FirepowerBonus { 0 };
	int ArmorBonus { 0 };
	int ROFBonus { 0 };

	MinMaxValue<int> ReceivedDamage { INT32_MIN , INT32_MAX };
	MinMaxValue<double> Speed { 0.0 ,  INT32_MAX };

	union {
		struct {
			bool Cloakable : 1;
			bool ForceDecloak : 1;
			bool DisableWeapons : 1;
			bool DisableSelfHeal : 1;
			bool HasRangeModifier : 1;
			bool HasTint : 1;
			bool HasOnFireDiscardables : 1;
			bool HasExtraWarheads : 1;
			bool HasFeedbackWeapon : 1;
			bool ReflectDamage : 1;
			bool Untrackable : 1;
			bool DisableRadar : 1;
			bool DisableSpySat : 1;
			bool Unkillable : 1;
			uint16_t _padding : 2;  // Unused bits
		};
		uint16_t AllFlags {};  // Access all at once
	};

public:

	static void Recalculate(TechnoClass* pTechno);

public:

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		return this->Serialize(Stm);
	}

	bool Save(PhobosStreamWriter& Stm) const
	{
		return const_cast<AEProperties*>(this)->Serialize(Stm);
	}

protected:
	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(this->FirepowerMultiplier)
			.Process(this->ArmorMultiplier)
			.Process(this->SpeedMultiplier)
			.Process(this->ROFMultiplier)
			.Process(this->ReceiveRelativeDamageMult)
			.Process(this->FirepowerBonus)
			.Process(this->ArmorBonus)
			.Process(this->SpeedBonus)
			.Process(this->ROFBonus)
			.Process(this->ReceivedDamage)
			.Process(this->Speed)

			.Process(this->AllFlags)
			.Success()
			;
	}
};

struct AEPropertiesExtraRange
{
	struct RangeData
	{
		double rangeMult { 1.0 };
		double extraRange { 0.0 };
		VectorSet<WeaponTypeClass*> allow {};
		VectorSet<WeaponTypeClass*> disallow {};

		bool FORCEDINLINE Load(PhobosStreamReader& Stm, bool RegisterForChange)
		{
			return this->Serialize(Stm);
		}

		bool FORCEDINLINE Save(PhobosStreamWriter& Stm) const
		{
			return const_cast<RangeData*>(this)->Serialize(Stm);
		}

		bool Eligible(WeaponTypeClass* who);

	private:

		template <typename T>
		bool FORCEDINLINE Serialize(T& Stm)
		{
			return Stm
				.Process(this->rangeMult)
				.Process(this->extraRange)
				.Process(this->allow)
				.Process(this->disallow)
				.Success()
				;
		}
	};

	struct RangeDataOut
	{
		double rangeMult { 1.0 };
		double extraRange { 0.0 };
	};

	HelperedVector<RangeData> ranges { };

	bool FORCEDINLINE Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		return this->Serialize(Stm);
	}

	bool FORCEDINLINE Save(PhobosStreamWriter& Stm) const
	{
		return const_cast<AEPropertiesExtraRange*>(this)->Serialize(Stm);
	}

	COMPILETIMEEVAL void Clear()
	{
		ranges.clear();
	}

	COMPILETIMEEVAL bool Enabled()
	{
		return !ranges.empty();
	}

	COMPILETIMEEVAL int Get(int initial, WeaponTypeClass* who)
	{
		int add = 0;
		for (auto& ex_range : ranges)
		{

			if (!ex_range.Eligible(who))
				continue;

			initial = static_cast<int>(initial * MaxImpl(ex_range.rangeMult, 0.0));
			add += static_cast<int>(ex_range.extraRange);
		}

		return initial + add;
	}

	COMPILETIMEEVAL void FillEligible(WeaponTypeClass* who, std::vector<RangeDataOut>& eligible)
	{
		for (auto& ex_range : this->ranges)
		{
			if (ex_range.Eligible(who))
			{
				eligible.emplace_back(ex_range.rangeMult, ex_range.extraRange);
			}
		}
	}

	static COMPILETIMEEVAL int Count(int initial, std::vector<RangeDataOut>& eligible)
	{
		int add = 0;
		for (auto& ex_range : eligible)
		{
			initial = static_cast<int>(initial * MaxImpl(ex_range.rangeMult, 0.0));
			add += static_cast<int>(ex_range.extraRange);
		}

		return initial + add;
	}

private:

	template <typename T>
	bool FORCEDINLINE Serialize(T& Stm)
	{
		return Stm
			.Process(this->ranges)
			.Success()
			;
	}
};

struct AEPropertiesExtraCrit
{
	struct CritData
	{
		double Mult { 1.0 };
		double extra { 0.0 };
		VectorSet<WarheadTypeClass*> allow {};
		VectorSet<WarheadTypeClass*> disallow {};

		bool FORCEDINLINE Load(PhobosStreamReader& Stm, bool RegisterForChange)
		{
			return this->Serialize(Stm);
		}

		bool FORCEDINLINE Save(PhobosStreamWriter& Stm) const
		{
			return const_cast<CritData*>(this)->Serialize(Stm);
		}

		bool Eligible(WarheadTypeClass* who);

	private:

		template <typename T>
		bool FORCEDINLINE Serialize(T& Stm)
		{
			return Stm
				.Process(this->Mult)
				.Process(this->extra)
				.Process(this->allow)
				.Process(this->disallow)
				.Success()
				;
		}
	};

	struct CritDataOut
	{
		double Mult { 1.0 };
		double extra { 0.0 };
	};

	HelperedVector<CritData> ranges { };

	bool FORCEDINLINE Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		return this->Serialize(Stm);
	}

	bool FORCEDINLINE Save(PhobosStreamWriter& Stm) const
	{
		return const_cast<AEPropertiesExtraCrit*>(this)->Serialize(Stm);
	}

	COMPILETIMEEVAL void Clear()
	{
		ranges.clear();
	}

	COMPILETIMEEVAL bool Enabled()
	{
		return !ranges.empty();
	}

	COMPILETIMEEVAL double Get(double initial, WarheadTypeClass* who)
	{
		double add = 0.0;
		for (auto& ex_range : ranges)
		{

			if (!ex_range.Eligible(who))
				continue;

			initial = initial * ex_range.Mult;
			add += ex_range.extra;
		}

		return initial + add;
	}

	COMPILETIMEEVAL void FillEligible(WarheadTypeClass* who, std::vector<CritDataOut>& eligible)
	{
		for (auto& ex_range : this->ranges)
		{
			if (ex_range.Eligible(who))
			{
				eligible.emplace_back(ex_range.Mult, ex_range.extra);
			}
		}
	}

	static COMPILETIMEEVAL double Count(double initial, std::vector<CritDataOut>& eligible)
	{
		double add = 0.0;
		for (auto& ex_range : eligible)
		{
			initial *= MaxImpl(ex_range.Mult, 0.0);
			add += ex_range.extra;
		}

		return initial + add;
	}

private:

	template <typename T>
	bool FORCEDINLINE Serialize(T& Stm)
	{
		return Stm
			.Process(this->ranges)
			.Success()
			;
	}
};

struct AEPropertiesArmorMult
{
	struct MultData
	{
		double Mult { 1.0 };
		VectorSet<WarheadTypeClass*> allow {};
		VectorSet<WarheadTypeClass*> disallow {};

		bool FORCEDINLINE Load(PhobosStreamReader& Stm, bool RegisterForChange)
		{
			return this->Serialize(Stm);
		}

		bool FORCEDINLINE Save(PhobosStreamWriter& Stm) const
		{
			return const_cast<MultData*>(this)->Serialize(Stm);
		}

		bool Eligible(WarheadTypeClass* who);

	private:

		template <typename T>
		bool FORCEDINLINE Serialize(T& Stm)
		{
			return Stm
				.Process(this->Mult)
				.Process(this->allow)
				.Process(this->disallow)
				.Success()
				;
		}
	};

	HelperedVector<MultData> mults { };

	bool FORCEDINLINE Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		return this->Serialize(Stm);
	}

	bool FORCEDINLINE Save(PhobosStreamWriter& Stm) const
	{
		return const_cast<AEPropertiesArmorMult*>(this)->Serialize(Stm);
	}

	COMPILETIMEEVAL void Clear()
	{
		mults.clear();
	}

	COMPILETIMEEVAL bool Enabled()
	{
		return !mults.empty();
	}

	COMPILETIMEEVAL double Get(double initial, WarheadTypeClass* who)
	{
		for (auto& ex_range : mults)
		{

			if (!ex_range.Eligible(who))
				continue;

			initial *= ex_range.Mult;
		}

		return initial;
	}

	COMPILETIMEEVAL void FillEligible(WarheadTypeClass* who, std::vector<double>& eligible)
	{
		for (auto& ex_range : this->mults)
		{
			if (ex_range.Eligible(who))
			{
				eligible.emplace_back(ex_range.Mult);
			}
		}
	}

	static COMPILETIMEEVAL double Apply(double initial, std::vector<double>& eligible)
	{
		for (auto& ex_range : eligible)
		{
			initial *= ex_range;
		}

		return initial;
	}
private:

	template <typename T>
	bool FORCEDINLINE Serialize(T& Stm)
	{
		return Stm
			.Process(this->mults)
			.Success()
			;
	}
};
