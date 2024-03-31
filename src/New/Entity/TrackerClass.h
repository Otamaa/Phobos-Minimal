#pragma once

#include <ArrayClasses.h>

struct TrackerClass
{
	//static u_long __stdcall htonl(u_long hostlong) JMP_STD(0x7C8962);
	//static u_long __stdcall ntohl(u_long netlong) JMP_STD(0x7C896E);

	void PopulateCounts(int count) {
		Items.resize(count);
	}

	int GetAll() const {
		int sum = 0;

		for (auto& item : this->Items) {
			if(item > 0)
				sum += item;
		}

		return sum;
	}

	int GetCounts() const {
		return (int)this->Items.size();
	}

	int GetCount(int at) const {
		if ((size_t)at > this->Items.size())
			Debug::FatalError("Trying to acces out of bound tracker !\n");

		return this->Items[at];
	}

	void Increment(int at) {
		if ((size_t)at > this->Items.size())
			Debug::FatalError("Trying to acces out of bound tracker !\n");

		++this->Items[at];
	}

	void Decrement(int at) {
		if ((size_t)at > this->Items.size())
			Debug::FatalError("Trying to acces out of bound tracker !\n");

		--this->Items[at];
	}
	void Clear() {
		this->Items.clear();
	}

	void ClearCount() {
		for (auto& item : this->Items) {
			item = 0;
		}
	}

	void ToNetwork() {
		if (!this->IsNetworkFormat) {
			for (size_t i = 0; i < this->Items.size(); ++i)
				this->Items[i] = htonl(this->Items[i]);
		}

		this->IsNetworkFormat = true;
	}

	void ToPC() {
		if (this->IsNetworkFormat)
		{
			for (size_t i = 0; i < this->Items.size(); ++i)
				this->Items[i] = ntohl(this->Items[i]);
		}

		this->IsNetworkFormat = false;
	}

	bool Load(PhobosStreamReader& stm, bool registerForChange)
	{
		return this->Serialize(stm);
	}

	bool Save(PhobosStreamWriter& stm) const
	{
		return const_cast<TrackerClass*>(this)->Serialize(stm);
	}

private:
	template <typename T>
	bool Serialize(T& stm)
	{
		return stm
			.Process(this->Items)
			.Process(this->IsNetworkFormat)
			.Success();
	}

protected:
	std::vector<int> Items;
	bool IsNetworkFormat;

};