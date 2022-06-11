/*
	A helper to randomly select from many equally good items with the best
	rating.

	Add one or more items you want to select from, giving each a rating value.
	The instance is able to select values if there is at least one item with
	a rating that compares equal to or better to the initial value. Items with
	a worse rating than the best are discarded.

	You can select multiple items using the same instance of this class, the
	probabilities of items to be selected will not change. Valid values for
	selection are 0..GetCount()-1. The select function will return true if and
	only if the instance is set up correctly and an item could be selected. The
	out parameter is written to if and only if an item could be selected.
*/

#pragma once

#include <ArrayClasses.h>
#include <Randomizer.h>

#include <utility>

template <typename T>
class DiscreteSelectionClass
{
public:
	DiscreteSelectionClass() = default;
	explicit DiscreteSelectionClass(int initial) : Rating(initial) {}

	void Add(T value, int rating) {
		if(this->Rating > rating) {
			return;
		}

		if(rating > this->Rating) {
			this->Clear();
			this->Rating = rating;
		}

		this->Items.AddItem(std::move(value));
	}

	void Clear() {
		this->Items.Clear();
	}

	int GetRating() const {
		return this->Rating;
	}

	int GetCount() const {
		return this->Items.Count;
	}

	bool IsValid() const {
		return this->GetCount() > 0;
	}

	bool Select(int index, T* pOut) const {
		if(this->Items.ValidIndex(index)) {
			if(pOut) {
				*pOut = this->Items[index];
			}
			return true;
		}

		return false;
	}

	bool Select(Random2Class &random, T* pOut) const {
		if(!this->IsValid()) {
			return false;
		}

		int value = random.RandomRanged(0, this->GetCount() - 1);
		return this->Select(value, pOut);
	}

	T Select(int index, T nDefault = T()) const {
		this->Select(index, &nDefault);
		return nDefault;
	}

	T Select(Random2Class &random, T nDefault = T()) const {
		this->Select(random, &nDefault);
		return nDefault;
	}

private:
	DynamicVectorClass<T> Items{};
	int Rating{ 0 };
};
