#pragma once

#include <Memory.h>

#include <algorithm>
#include <Helpers/Concepts.h>
#include <YRPPCore.h>

struct __declspec(align(4)) DummyDynamicVectorClass
{
	void* vftble;
	void** Vector_Item;
	int VectorMax;
	char IsValid;
	char IsAllocated;
	char VectorClassPad[2];
	int ActiveCount;
	int GrowthStep;
};

template<typename T>
struct TDummyDynamicVectorClass
{
	void* vftble;
	T* Vector_Item;
	int VectorMax;
	char IsValid;
	char IsAllocated;
	char VectorClassPad[2];
	int ActiveCount;
	int GrowthStep;
};

static_assert(sizeof(DummyDynamicVectorClass) == 0x18, "Invalid Size !");

#include <VectorClass.h>
#include <DynamicVectorClass.h>
#include <TypeList.h>
#include <CounterClass.h>

template<typename T, const int y, const int x>
class ArrayHelper2D
{
public:
	operator std::array<T, x>* () { return reinterpret_cast<std::array<T, x>*>(this); }
	operator const std::array<T, x>* () const { return reinterpret_cast<const std::array<T, x>*>(this); }
	std::array<T, x>* operator&() { return reinterpret_cast<std::array<T, x>*>(this); }
	const std::array<T, x>* operator&() const { return reinterpret_cast<const std::array<T, x>*>(this); }
	std::array<T, x>& operator[](int index)
	{
		return _dummy[index];
	}
	const std::array<T, x>& operator[](int index) const
	{
		return _dummy[index];
	}

	std::array<T, x>* begin() { return _dummy; }
	std::array<T, x>* end() { return _dummy + y; }

	const std::array<T, x>* begin() const { return _dummy; }
	const std::array<T, x>* end() const { return _dummy + y; }

	COMPILETIMEEVAL int size() const {
		return y;
	}

protected:
	std::array<T, x> _dummy[y];
};
