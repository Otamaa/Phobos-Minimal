#pragma once

#include <TiberiumClass.h>
#include <Utilities/SavegameDef.h>

struct NewTiberiumStorageClass
{
	NewTiberiumStorageClass::NewTiberiumStorageClass() :
		m_values {}
	{
		m_values.resize(TiberiumClass::Array->Count);
	}

	~NewTiberiumStorageClass() = default;

	std::vector<float> m_values;

	double GetStoragePercentage(int total) const
	{
		return (double)this->GetAmounts() / (double)total;
	}

	int GetHighestStorageIdx() const
	{
		int nIdx = 0;
		for (int p = 0; p < (int)m_values.size(); p++)
			nIdx += (this->m_values[nIdx] < this->m_values[p]) * (p - nIdx);

		return nIdx;
	}

	int GetTotalTiberiumValue() const
	{
		float sum = 0;
		for (size_t i = 0; i < m_values.size(); ++i)
		{
			if (m_values[i] > 0.0)
			{
				sum += TiberiumClass::Array->Items[i]->Value * m_values[i];
			}
		}

		return (int)sum;
	}

	int GetFirstSlotUsed() const {

		for (size_t i = 0; i < m_values.size(); ++i) {
			if (m_values[i] > 0.0)
				return i;
		}

		return -1;
	}

	float DecreaseLevel(float amount, int idx) {
		if (this->m_values[idx] >= amount) {
			this->m_values[idx] -= amount;
			return amount;
		} else {
			float cur = this->m_values[idx];
			this->m_values[idx] = cur - cur;
			return cur;
		}
	}

	double GetAmounts() const {
		double sum = 0.0;
		for (size_t i = 0; i < m_values.size(); ++i) {
			sum += m_values[i];
		}

		return sum;
	}

	double GetAmount(int idx) const {
		return m_values[idx];
	}

	float IncreaseAmount(float amount , int idx)
	{
		float fal = amount + m_values[idx];
		m_values[idx]= fal;
		return fal;
	}

	bool Load(PhobosStreamReader& stm, bool registerForChange)
	{
		bool load_ = this->Serialize(stm);
		Debug::LogInfo("size after Load {}", this->m_values.size());
		return load_;
	}

	bool Save(PhobosStreamWriter& stm) const
	{
		return const_cast<NewTiberiumStorageClass*>(this)->Serialize(stm);
	}

private:
	template <typename T>
	bool Serialize(T& stm)
	{
		return stm
			.Process(this->m_values)
			.Success();
	}
};