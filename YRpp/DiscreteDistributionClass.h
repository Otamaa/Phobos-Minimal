/*
	A helper to randomly select from many weighted items.

	Add one or more items with a weight of 0 or higher you want to select from.
	The instance is able to select values if there is at least one item with a
	with a weight of 1 or higher.

	You can select multiple items using the same instance of this class, the
	probabilities of items to be selected will not change. Valid values for
	selection are 1..GetTotalWeight(). The select function will return true if and
	only if the instance is set up correctly and an item could be selected. The
	out parameter is written to if and only if an item could be selected.
*/

#pragma once

#include <ArrayClasses.h>
#include <Randomizer.h>

#include <utility>
template <typename T>
struct DistributionObject {
	COMPILETIMEEVAL DistributionObject() = default;
	COMPILETIMEEVAL explicit DistributionObject(T value, unsigned int weight = 1u) : Value(std::move(value)), Weight(weight) {}
	COMPILETIMEEVAL FORCEDINLINE bool operator ==(const DistributionObject<T> &rhs) const { return false; }
	COMPILETIMEEVAL FORCEDINLINE bool operator !=(const DistributionObject<T> &rhs) const { return true; }
	COMPILETIMEEVAL ~DistributionObject() = default;

	T Value{};
	unsigned int Weight{ 0u };
};

template <typename T>
class DiscreteDistributionClass
{
public:
	DiscreteDistributionClass() = default;
	explicit DiscreteDistributionClass(int capacity, DistributionObject<T>* pMem = nullptr) : Items(capacity, pMem) {}

	const DynamicVectorClass<DistributionObject<T>> & GetItems() const {
		return this->Items;
	}

	void Add(DistributionObject<T> item) {
		this->TotalWeight += item.Weight;
		this->Items.AddItem(std::move(item));
	}

	void Add(T value, unsigned int weight = 1u) {
		if (!weight) {
			return;
		}

		DistributionObject<T> item(std::move(value), weight);
		this->Add(std::move(item));
	}

	void Clear() {
		this->TotalWeight = 0u;
		this->Items.Reset();
	}

	COMPILETIMEEVAL unsigned int GetTotalWeight() const {
		return this->TotalWeight;
	}

	COMPILETIMEEVAL int GetCount() const {
		return this->Items.Count;
	}

	COMPILETIMEEVAL bool IsValid() const {
		return this->TotalWeight > 0U && this->Items.Count > 0;
	}

	COMPILETIMEEVAL bool Select(unsigned int value, T* pOut) const {
		if(this->IsValid() && value && value <= this->TotalWeight) {
			unsigned int acc = 0u;
			for(auto i = this->Items.begin(); i < this->Items.end(); ++i) {
				acc += i->Weight;

				if(acc >= value) {
					if(pOut) {
						*pOut = i->Value;
					}
					return true;
				}
			}
		}

		return false;
	}

	bool Select(Random2Class &random, T* pOut) const {
		if(!this->TotalWeight) {
			return false;
		}

		int value = random.RandomRanged(1, static_cast<int>(this->TotalWeight));
		return this->Select(static_cast<unsigned int>(value), pOut);
	}

	T Select(unsigned int index, T nDefault = T()) const {
		this->Select(index, &nDefault);
		return nDefault;
	}

	T Select(Random2Class &random, T nDefault = T()) const {
		this->Select(random, &nDefault);
		return nDefault;
	}

	T SelectOrDefault(T default = T()) const {
		this->Select(ScenarioClass::Instance->Random, &default);
		return default;
	}

private:
	DynamicVectorClass<DistributionObject<T>> Items{};
	unsigned int TotalWeight{ 0u };
};