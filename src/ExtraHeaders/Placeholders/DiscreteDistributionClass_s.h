#pragma once

#include <Randomizer.h>

#include <vector>

template <typename T>
class DiscreteDistributionClass_s
{
public:

	template <typename T>
	struct DistributionObject
	{
		DistributionObject() = default;
		explicit DistributionObject(T value, unsigned int weight = 1u) : Value(std::move(value)), Weight(weight) { }

		bool operator ==(const DistributionObject<T>& rhs) const { return false; }
		bool operator !=(const DistributionObject<T>& rhs) const { return true; }

		T Value {};
		unsigned int Weight { 0u };
	};

	DiscreteDistributionClass_s() = default;

	void Add(DistributionObject<T> item)
	{
		this->TotalWeight += item.Weight;
		this->Items.push_back(std::move(item));
	}

	void Add(T value, unsigned int weight = 1u)
	{
		DistributionObject<T> item(std::move(value), weight);
		this->Add(std::move(item));
	}

	void Clear()
	{
		this->TotalWeight = 0u;
		this->Items.clear();
	}

	unsigned int GetTotalWeight() const
	{
		return this->TotalWeight;
	}

	int GetCount() const
	{
		return (int)this->Items.size();
	}

	bool IsValid() const
	{
		return this->TotalWeight > 0U && (int)this->Items.size() > 0;
	}

	bool Select(unsigned int value, T* pOut) const
	{
		if (this->IsValid() && value && value <= this->TotalWeight)
		{
			unsigned int acc = 0u;
			for (auto i = this->Items.begin(); i < this->Items.end(); ++i)
			{
				acc += i->Weight;

				if (acc >= value)
				{
					if (pOut)
					{
						*pOut = i->Value;
					}
					return true;
				}
			}
		}

		return false;
	}

	bool Select(Random2Class& random, T* pOut) const
	{
		if (!this->TotalWeight)
		{
			return false;
		}

		int value = random.RandomRanged(1, static_cast<int>(this->TotalWeight));
		return this->Select(static_cast<unsigned int>(value), pOut);
	}

	T Select(unsigned int index, T nDefault = T()) const
	{
		this->Select(index, &nDefault);
		return nDefault;
	}

	T Select(Random2Class& random, T nDefault = T()) const
	{
		this->Select(random, &nDefault);
		return nDefault;
	}

private:
	std::vector<DistributionObject<T>> Items {};
	unsigned int TotalWeight { 0u };
};
