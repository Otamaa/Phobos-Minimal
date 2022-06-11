#pragma once


template<typename T, int N>
class Array
{
public:
	inline Array() { }

	inline Array(T val)
	{
		for (int i = 0; i < N; ++i)
			Data[i] = val;
	}

	inline const int Size() const
	{
		return N;
	}

	inline T& operator [](int idx)
	{
		return Data[idx];
	}

	inline T operator [](int idx) const
	{
		return Data[idx];
	}

private:
	T Data[N];
};