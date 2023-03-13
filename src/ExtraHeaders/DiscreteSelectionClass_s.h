#pragma once


#include <Randomizer.h>

#include <vector>

template <typename T>
class DiscreteSelectionClass_s
{
public:
	DiscreteSelectionClass() = default;
	explicit DiscreteSelectionClass(int initial) : Rating(initial) { }

	void Add(T value, int rating)
	{
		if (this->Rating > rating)
		{
			return;
		}

		if (rating > this->Rating)
		{
			this->Clear();
			this->Rating = rating;
		}

		this->Items.push_back(std::move(value));
	}

	void Clear()
	{
		this->Items.clear();
	}

	int GetRating() const
	{
		return this->Rating;
	}

	int GetCount() const
	{
		return (int)this->Items.size();
	}

	bool IsValid() const
	{
		return this->GetCount() > 0;
	}

	bool Select(int index, T* pOut) const
	{
		if (ValidIndex(index))
		{
			if (pOut)
			{
				*pOut = this->Items[index];
			}
			return true;
		}

		return false;
	}

	bool Select(Random2Class& random, T* pOut) const
	{
		if (!this->IsValid())
		{
			return false;
		}

		int value = random.RandomRanged(0, this->GetCount() - 1);
		return this->Select(value, pOut);
	}

	T Select(int index, T nDefault = T()) const
	{
		this->Select(index, &nDefault);
		return nDefault;
	}

	T Select(Random2Class& random, T nDefault = T()) const
	{
		this->Select(random, &nDefault);
		return nDefault;
	}
protected: 
	bool ValidIndex(int nIdx) const
	{
		return nIdx > 0 && nIdx < (int)Items.size();
	}

private:
	std::vector<T> Items {};
	int Rating { 0 };
};
