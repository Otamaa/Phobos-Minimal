#pragma once

#include <Base/Always.h>
#include <YRPPCore.h>
#include <Memory.h>

struct noinit_t;
/**
 *  Forward-declaration of DynamicVectorClass.
 */
template<typename T>
class ViDynamicVectorClass;


/*******************************************************************************
 *  @class   VectorCursor
 *
 *  @brief   An iterator for DynamicVectorClass (similar to std::iterator).
 */
template<typename T, class V /*= DynamicVectorClass<T>*/>
class VectorCursor
{
public:
	VectorCursor(V* vector, int pos = 0);
	VectorCursor(const V* vector, int pos = 0);
	virtual ~VectorCursor();

	VectorCursor& operator=(const VectorCursor& that);

	bool operator==(const VectorCursor& that) const;
	bool operator!=(const VectorCursor& that) const;

	T& operator*() { return *Get_Element(); }
	const T& operator*() const { return *Get_Element(); }

	T* operator->() { return Get_Element(); }
	const T* operator->() const { return Get_Element(); }

	operator bool() { return Can_Loop(); }

	VectorCursor& operator++() { Next(); return *this; }
	VectorCursor& operator--() { Prev(); return *this; }

	VectorCursor operator++(int) { VectorCursor tmp; tmp.Next(); return tmp; }
	VectorCursor operator--(int) { VectorCursor tmp; tmp.Prev(); return tmp; }

protected:
	T* Get_Element();
	const T* Get_Element() const;

	const V& Get_Vector() const { return *VectorPtr; }
	int Get_Position() const { return Position; }

	virtual bool Can_Loop() const;

	virtual void Next() { ++Position; }
	virtual void Prev() { --Position; }

	bool Next_Match(T* that_elem);
	bool Next_Match(const T* that_elem);
	void Next_Valid();

protected:
	const V* VectorPtr;
	int Position;
};


template<typename T, class V>
VectorCursor<T, V>::VectorCursor(V* vector, int pos) :
	VectorPtr(vector),
	Position(pos)
{
}


template<typename T, class V>
VectorCursor<T, V>::VectorCursor(const V* vector, int pos) :
	VectorPtr(vector),
	Position(pos)
{
}


template<typename T, class V>
VectorCursor<T, V>::~VectorCursor()
{
}


template<typename T, class V>
VectorCursor<T, V>& VectorCursor<T, V>::operator=(const VectorCursor<T, V>& that)
{
	if (this != &that)
	{
		VectorPtr = that.VectorPtr;
		Position = that.Position;
	}
	return *this;
}


template<typename T, class V>
bool VectorCursor<T, V>::operator==(const VectorCursor<T, V>& that) const
{
	return VectorPtr == that.VectorPtr
		&& Position == that.Position;
}


template<typename T, class V>
bool VectorCursor<T, V>::operator!=(const VectorCursor<T, V>& that) const
{
	return VectorPtr != that.VectorPtr
		&& Position != that.Position;
}


template<typename T, class V>
T* VectorCursor<T, V>::Get_Element()
{
	if (!VectorCursor::Can_Loop())
	{
		return nullptr;
	}

	if (!VectorPtr)
	{
		return false;
	}

	return &const_cast<T&>((*VectorPtr)[Position]);
}


template<typename T, class V>
const T* VectorCursor<T, V>::Get_Element() const
{
	if (!VectorCursor::Can_Loop())
	{
		return nullptr;
	}

	if (!VectorPtr)
	{
		return false;
	}

	return &(*VectorPtr)[Position];
}


template<typename T, class V>
bool VectorCursor<T, V>::Can_Loop() const
{
	if (!VectorPtr)
	{
		return false;
	}

	return Position < VectorPtr->Count();
}


template<typename T, class V>
bool VectorCursor<T, V>::Next_Match(T* that_elem)
{
	while (Can_Loop())
	{
		const T* elem = &(*VectorPtr)[Position];
		if (elem == that_elem)
		{
			break;
		}
		Next();
	}
}


template<typename T, class V>
bool VectorCursor<T, V>::Next_Match(const T* that_elem)
{
	while (Can_Loop())
	{
		const T* elem = &(*VectorPtr)[Position];
		if (elem == that_elem)
		{
			break;
		}
		Next();
	}
}


template<typename T, class V>
void VectorCursor<T, V>::Next_Valid()
{
	while (Can_Loop())
	{
		const T* v3 = &(*VectorPtr)[Position];
		if (v3 != nullptr)
		{
			break;
		}
		Next();
	}
}

template<typename T>
class ViVectorClass
{
public:
	ViVectorClass(unsigned size = 0, T* arr = nullptr);
	ViVectorClass(const noinit_t& noinit) { }
	ViVectorClass(const ViVectorClass<T>& that);
	virtual ~ViVectorClass();

	T& operator[](int index);
	const T& operator[](int index) const;

	ViVectorClass<T>& operator=(const ViVectorClass<T>& that);

	virtual bool operator==(const ViVectorClass<T>& that) const;

	virtual bool Resize(int newsize, T* arr = nullptr);
	virtual void Clear();
	virtual int ID(const T* ptr);
	virtual int ID(const T& ptr);

	int Length() const { return VectorMax; }

protected:
	T* Vector;
	int VectorMax;
	bool IsValid;
	bool IsAllocated;
	bool ViVectorClassPad[2];
};

template<typename T>
T& ViVectorClass<T>::operator[](int index)
{
	return Vector[index];
}

template<typename T>
const T& ViVectorClass<T>::operator[](int index) const
{
	return Vector[index];
}

template<typename T>
ViVectorClass<T>::ViVectorClass(unsigned size, T* arr) :
	Vector(nullptr),
	VectorMax(size),
	IsValid(true),
	IsAllocated(false)
{
	if (size > 0)
	{
		if (arr != nullptr)
		{
			Vector = GameConstructArray<T>(arr, size);
		}
		else
		{
			Vector = GameCreateArray<T>(size);
			IsAllocated = true;
		}
	}
}

template<typename T>
ViVectorClass<T>::~ViVectorClass()
{
	ViVectorClass<T>::Clear();
}

template<typename T>
ViVectorClass<T>::ViVectorClass(const ViVectorClass<T>& that) :
	Vector(nullptr),
	VectorMax(0),
	IsAllocated(false)
{
	*this = that;
}

template<typename T>
ViVectorClass<T>& ViVectorClass<T>::operator=(const ViVectorClass<T>& that)
{
	if (this != &that)
	{
		Clear();
		VectorMax = that.Length();

		if (VectorMax > 0)
		{
			Vector = GameCreateArray<T>(VectorMax);

			if (Vector != nullptr)
			{
				IsAllocated = true;

				for (int i = 0; i < VectorMax; ++i)
				{
					(*this)[i] = that[i];
				}
			}
		}
		else
		{
			Vector = nullptr;
			IsAllocated = false;
		}
	}

	return *this;
}

template<typename T>
bool ViVectorClass<T>::operator==(const ViVectorClass<T>& that) const
{
	if (VectorMax != that.Length())
	{
		return false;
	}

	for (int i = 0; i < VectorMax; ++i)
	{
		if ((*this)[i] != that[i])
		{
			return false;
		}
	}

	return true;
}

template<typename T>
int ViVectorClass<T>::ID(const T* ptr)
{
	if (!IsValid)
	{
		return 0;
	}

	return ((uintptr_t)ptr - (uintptr_t)Vector) / sizeof(T);
}

template<typename T>
int ViVectorClass<T>::ID(const T& object)
{
	if (!IsValid)
	{
		return 0;
	}

	for (int i = 0; i < VectorMax; ++i)
	{
		if ((*this)[i] == object)
		{
			return i;
		}
	}

	return -1;
}

template<typename T>
void ViVectorClass<T>::Clear()
{
	if (Vector != nullptr && IsAllocated)
	{
		Deallocate(Vector);
		Vector = nullptr;
	}

	IsAllocated = false;
	VectorMax = 0;
}

template<typename T>
bool ViVectorClass<T>::Resize(int newsize, T* arr)
{
	if (newsize > 0)
	{
		T* newptr = nullptr;
		IsValid = false;

		if (arr == nullptr)
		{
			newptr = GameCreateArray<T>(newsize);
		}
		else
		{
			newptr = GameConstructArray<T>(arr, newsize);
		}

		IsValid = true;

		if (newptr == nullptr)
		{
			return false;
		}

		if (Vector != nullptr)
		{
			int copy_count = (newsize < VectorMax) ? newsize : VectorMax;

			for (int i = 0; i < copy_count; ++i)
			{
				newptr[i] = (*this)[i];
			}

			if (IsAllocated)
			{
				Deallocate(Vector);
				Vector = nullptr;
			}
		}

		Vector = newptr;
		VectorMax = newsize;
		IsAllocated = (Vector != nullptr && arr == nullptr);
	}
	else
	{
		Clear();
	}

	return true;
}

template<typename T>
class ViDynamicVectorClass : public ViVectorClass<T>
{
	using ViVectorClass<T>::Vector;
	using ViVectorClass<T>::VectorMax;
	using ViVectorClass<T>::IsAllocated;
public:
	ViDynamicVectorClass(unsigned size = 0, T* arr = nullptr);
	ViDynamicVectorClass(const noinit_t& noinit) : ViVectorClass(noinit) { }
	ViDynamicVectorClass(const ViDynamicVectorClass& that);
	virtual ~ViDynamicVectorClass() { }

	T& operator[](int index);
	const T& operator[](int index) const;

	bool operator==(const ViDynamicVectorClass& src) { return false; }
	bool operator!=(const ViDynamicVectorClass& src) { return true; }

	ViDynamicVectorClass& operator=(const ViDynamicVectorClass& that);

	virtual bool Resize(int newsize, T* array = nullptr) override;
	virtual void Clear() override;
	virtual int ID(const T* ptr) override { return ViVectorClass::ID(ptr); }
	virtual int ID(const T& ptr) override;

	bool Add(const T& object);
	bool Add_Head(const T& object);

	T* Uninitialized_Add();
	bool Delete(const T& object);
	bool Delete(int index);
	void Delete_All();

	bool Insert(int index, const T& object);

	const T& Fetch_Head() const { return (*this)[0]; }
	const T& Fetch_Tail() const { return (*this)[ActiveCount - 1]; }

	void Reset_Active() { ActiveCount = 0; }
	void Set_Active(int count) { ActiveCount = count; }

	int Count() const { return ActiveCount; }

	bool Empty() const { return ActiveCount <= 0; }

	int Set_Growth_Step(int step) { return GrowthStep = step; }

	int Growth_Step() { return GrowthStep; }

	int Calculate_Growth(int new_size);

	typedef VectorCursor<T, ViDynamicVectorClass<T>> Iterator;
	typedef const VectorCursor<T, ViDynamicVectorClass<T>> ConstIterator;

	/**
	*  Adds support for C++ 11 range-based for loops using VectorCursor.
	*/
	Iterator begin() { return Iterator(this, 0); }
	Iterator end() { return Iterator(this, ActiveCount); }

	Iterator begin() const { return Iterator(this, 0); }
	Iterator end() const { return Iterator(this, ActiveCount); }

	ConstIterator cbegin() { return ConstIterator(this, 0); }
	ConstIterator cend() { return ConstIterator(this, ActiveCount); }

	ConstIterator cbegin() const { return ConstIterator(this, 0); }
	ConstIterator cend() const { return ConstIterator(this, ActiveCount); }

protected:
	int ActiveCount;
	int GrowthStep;
};

/**
 *  Given old and new_size, calculate geometric a new growth.
 *
 *  @author: secsome
 */
template<typename T>
int ViDynamicVectorClass<T>::Calculate_Growth(int new_size)
{
	int old = VectorMax;

	int geometric = old + old / 2;

	if (geometric < new_size)
	{

		/**
		 *  Geometric growth would be insufficient.
		 */
		return new_size;
	}

	/**
	 *  Geometric growth is sufficient.
	 */
	return geometric;
}


template<typename T>
ViDynamicVectorClass<T>::ViDynamicVectorClass(unsigned size, T* array) :
	ViVectorClass<T>(size, array),
	GrowthStep(10),
	ActiveCount(0)
{
}

template<typename T>
ViDynamicVectorClass<T>::ViDynamicVectorClass(const ViDynamicVectorClass<T>& that) :
	ViVectorClass<T>(that),
	GrowthStep(10),
	ActiveCount(0)
{
	*this = that;
}

template<typename T>
T& ViDynamicVectorClass<T>::operator[](int index)
{
	return Vector[index];
}

template<typename T>
const T& ViDynamicVectorClass<T>::operator[](int index) const
{
	return Vector[index];
}

template<typename T>
void ViDynamicVectorClass<T>::Clear()
{
	ActiveCount = 0;
	ViVectorClass<T>::Clear();
}

template<typename T>
ViDynamicVectorClass<T>& ViDynamicVectorClass<T>::operator=(const ViDynamicVectorClass<T>& that)
{
	ViVectorClass<T>::operator=(that);
	ActiveCount = that.ActiveCount;
	GrowthStep = that.GrowthStep;

	return *this;
}

template<typename T>
bool ViDynamicVectorClass<T>::Resize(int newsize, T* array)
{
	if (!ViVectorClass<T>::Resize(newsize, array))
	{
		return false;
	}

	if (VectorMax < ActiveCount)
	{
		ActiveCount = VectorMax;
	}

	return true;
}

template<typename T>
int ViDynamicVectorClass<T>::ID(const T& object)
{
	for (int i = 0; i < Count(); ++i)
	{
		if ((*this)[i] == object)
		{
			return i;
		}
	}

	return -1;
}

template<typename T>
bool ViDynamicVectorClass<T>::Add(const T& object)
{
	if (ActiveCount >= VectorMax)
	{
		if ((IsAllocated || !VectorMax) && GrowthStep > 0)
		{
			if (!Resize(VectorMax + GrowthStep))
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}

	(*this)[ActiveCount++] = object;

	return true;
}

template<typename T>
bool ViDynamicVectorClass<T>::Add_Head(const T& object)
{
	if (ActiveCount >= VectorMax)
	{
		if ((IsAllocated || !VectorMax) && GrowthStep > 0)
		{
			if (!Resize(VectorMax + GrowthStep))
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}

	if (ActiveCount > 0)
	{
		std::memmove(&(*this)[1], &(*this)[0], ActiveCount * sizeof(T));
	}

	(*this)[0] = object;
	++ActiveCount;

	return true;
}

template<typename T>
bool ViDynamicVectorClass<T>::Insert(int index, const T& object)
{
	if (index < 0 || index > ActiveCount)
	{
		return false;
	}

	if (ActiveCount >= VectorMax)
	{
		if ((IsAllocated || !VectorMax) && GrowthStep > 0)
		{
			if (!Resize(VectorMax + GrowthStep))
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}

	if (index < ActiveCount)
	{
		std::memmove(&(*this)[index + 1], &(*this)[index], (ActiveCount - index) * sizeof(T));
	}

	(*this)[index] = object;
	++ActiveCount;

	return true;
}

template<typename T>
bool ViDynamicVectorClass<T>::Delete(const T& object)
{
	int id = ID(object);

	if (id != -1)
	{
		return Delete(id);
	}

	return false;
}

template<typename T>
bool ViDynamicVectorClass<T>::Delete(int index)
{
	if (index < ActiveCount)
	{
		--ActiveCount;
		for (int i = index; i < ActiveCount; ++i)
		{
			(*this)[i] = (*this)[i + 1];
		}

		return true;
	}

	return false;
}

template<typename T>
void ViDynamicVectorClass<T>::Delete_All()
{
	int len = VectorMax;
	Clear();
	Resize(len);
}

template<typename T>
T* ViDynamicVectorClass<T>::Uninitialized_Add()
{
	if (ActiveCount >= VectorMax)
	{
		if (GrowthStep > 0)
		{
			if (!Resize(VectorMax + GrowthStep))
			{
				return nullptr;
			}
		}
		else
		{
			return nullptr;
		}
	}

	return &((*this)[ActiveCount++]);
}

template<class T>
int Pointer_Vector_Add(T* ptr, ViVectorClass<T*>& vec)
{
	int id = 0;
	bool foundspot = false;
	for (int i = 0; i < vec.Length(); ++i)
	{
		if (vec[i] == nullptr)
		{
			id = i;
			foundspot = true;
			break;
		}
	}
	if (!foundspot)
	{
		id = vec.Length();
		vec.Resize((vec.Length() + 1) * 2);
		for (int i = id; i < vec.Length(); ++i)
		{
			vec[i] = nullptr;
		}
	}
	vec[id] = ptr;
	return id;
}

template<class T>
bool Pointer_Vector_Remove(const T* ptr, ViVectorClass<T*>& vec)
{
	int id = vec.ID((T*)ptr);
	if (id != -1)
	{
		vec[id] = nullptr;
		return true;
	}
	return false;
}

template<class T>
class SimpleVecClass
{
public:
	SimpleVecClass(unsigned size = 0);
	virtual ~SimpleVecClass();

	virtual bool Resize(int newsize);
	virtual bool Uninitialised_Grow(int newsize);

	T& operator[](int index)
	{
		return Vector[index];
	}

	T const& operator[](int index) const
	{
		return Vector[index];
	}

	int Length() const { return VectorMax; }

	void Zero_Memory()
	{
		if (Vector != nullptr)
		{
			std::memset(Vector, 0, VectorMax * sizeof(T));
		}
	}

protected:
	T* Vector;
	int VectorMax;
};

template<class T>
SimpleVecClass<T>::SimpleVecClass(unsigned size) :
	Vector(nullptr),
	VectorMax(0)
{
	if (size > 0)
	{
		Resize(size);
	}
}

template<class T>
SimpleVecClass<T>::~SimpleVecClass()
{
	if (Vector != nullptr)
	{
		delete[] Vector;
		Vector = nullptr;
		VectorMax = 0;
	}
}

template<class T>
bool SimpleVecClass<T>::Resize(int newsize)
{
	if (newsize == VectorMax)
	{
		return true;
	}

	if (newsize > 0)
	{
		T* newptr = GameCreateArray<T>(newsize);

		if (Vector != nullptr)
		{
			int copycount = (newsize < VectorMax) ? newsize : VectorMax;
			std::memcpy(newptr, Vector, copycount * sizeof(T));

			Deallocate(Vector);
			Vector = nullptr;
		}

		Vector = newptr;
		VectorMax = newsize;
	}
	else
	{
		VectorMax = 0;
		if (Vector != nullptr)
		{
			Deallocate(Vector);
			Vector = nullptr;
		}
	}
	return true;
}

template<class T>
bool SimpleVecClass<T>::Uninitialised_Grow(int newsize)
{
	if (newsize <= VectorMax)
	{
		return true;
	}

	if (newsize > 0)
	{
		Deallocate(Vector);
		Vector = GameCreateArray<T>(newsize);
		VectorMax = newsize;
	}
	return true;
}

template<class T>
class SimpleDynVecClass : public SimpleVecClass<T>
{
	using SimpleVecClass<T>::Vector;
	using SimpleVecClass<T>::VectorMax;
	using SimpleVecClass<T>::Length;

public:
	SimpleDynVecClass(unsigned size = 0);
	virtual ~SimpleDynVecClass();

	virtual bool Resize(int newsize);

	T& operator[](int index)
	{
		return Vector[index];
	}

	const T& operator[](int index) const
	{
		return Vector[index];
	}

	int Count() const { return (ActiveCount); }

	bool Add(const T& object, int new_size_hint = 0);
	T* Add_Multiple(int number_to_add);

	bool Delete(int index, bool allow_shrink = true);
	bool Delete(const T& object, bool allow_shrink = true);
	bool Delete_Range(int start, int count, bool allow_shrink = true);
	void Delete_All(bool allow_shrink = true);

	typedef VectorCursor<T, SimpleDynVecClass<T>> Iterator;
	typedef const VectorCursor<T, SimpleDynVecClass<T>> ConstIterator;

	/**
	*  Adds support for C++ 11 range-based for loops using VectorCursor.
	*/
	Iterator begin() { return Iterator(this, 0); }
	Iterator end() { return Iterator(this, ActiveCount); }

	Iterator begin() const { return Iterator(this, 0); }
	Iterator end() const { return Iterator(this, ActiveCount); }

	ConstIterator cbegin() { return ConstIterator(this, 0); }
	ConstIterator cend() { return ConstIterator(this, ActiveCount); }

	ConstIterator cbegin() const { return ConstIterator(this, 0); }
	ConstIterator cend() const { return ConstIterator(this, ActiveCount); }

protected:
	bool Grow(int new_size_hint);
	bool Shrink();

	int Find_Index(const T& object);

protected:
	int ActiveCount;
};

template<class T>
SimpleDynVecClass<T>::SimpleDynVecClass(unsigned size) :
	SimpleVecClass<T>(size),
	ActiveCount(0)
{
}

template<class T>
SimpleDynVecClass<T>::~SimpleDynVecClass()
{
	if (Vector != nullptr)
	{
		Deallocate(Vector);
		Vector = nullptr;
	}
}

template<class T>
bool SimpleDynVecClass<T>::Resize(int newsize)
{
	if (SimpleVecClass<T>::Resize(newsize))
	{
		if (Length() < ActiveCount)
		{
			ActiveCount = Length();
		}
		return true;
	}
	return false;
}

template<class T>
bool SimpleDynVecClass<T>::Add(const T& object, int new_size_hint)
{
	if (ActiveCount >= VectorMax)
	{
		if (!Grow(new_size_hint))
		{
			return false;
		}
	}

	(*this)[ActiveCount++] = object;
	return true;
}

template<class T>
T* SimpleDynVecClass<T>::Add_Multiple(int number_to_add)
{
	int i = ActiveCount;
	ActiveCount += number_to_add;

	if (ActiveCount >= VectorMax)
	{
		Grow(ActiveCount);
	}

	return &Vector[i];
}

template<class T>
bool SimpleDynVecClass<T>::Delete(int index, bool allow_shrink)
{
	TSPP_ASSERT(index < ActiveCount);

	if (index < ActiveCount - 1)
	{
		std::memmove(&(Vector[index]), &(Vector[index + 1]), (ActiveCount - index - 1) * sizeof(T));
	}
	--ActiveCount;

	if (allow_shrink)
	{
		Shrink();
	}

	return true;
}

template<class T>
bool SimpleDynVecClass<T>::Delete(const T& object, bool allow_shrink)
{
	int id = Find_Index(object);
	if (id != -1)
	{
		return Delete(id, allow_shrink);
	}
	return false;
}

template<class T>
bool SimpleDynVecClass<T>::Delete_Range(int start, int count, bool allow_shrink)
{
	if (start < ActiveCount - count)
	{
		std::memmove(&(Vector[start]), &(Vector[start + count]), (ActiveCount - start - count) * sizeof(T));
	}

	ActiveCount -= count;

	if (allow_shrink)
	{
		Shrink();
	}

	return true;
}

template<class T>
void SimpleDynVecClass<T>::Delete_All(bool allow_shrink)
{
	ActiveCount = 0;
	if (allow_shrink)
	{
		Shrink();
	}
}

template<class T>
bool SimpleDynVecClass<T>::Grow(int new_size_hint)
{
	const int new_size = MaxImpl(Length() + Length() / 4, Length() + 4);
	new_size = MaxImpl(new_size, new_size_hint);

	return Resize(new_size);
}

template<class T>
bool SimpleDynVecClass<T>::Shrink()
{
	// Shrink the array if it is wasting more than 25%.
	if (ActiveCount < VectorMax / 4)
	{
		return Resize(ActiveCount);
	}
	return true;
}

template<class T>
int SimpleDynVecClass<T>::Find_Index(const T& object)
{
	for (int i = 0; i < Count(); ++i)
	{
		if ((*this)[i] == object)
		{
			return i;
		}
	}
	return -1;
}

/**
 *  A fixed-size array of vectors.
 */
template<class T, int COUNT>
class VectorArrayClass
{
public:
	static const int CollectionCount = COUNT;

public:
	VectorArrayClass() : ActiveContext(0) { }
	virtual ~VectorArrayClass() { }

	T& operator[](unsigned index)
	{
		return Collection[ActiveContext][index];
	}

	T const& operator[](unsigned index) const
	{
		return Collection[ActiveContext][index];
	}

	ViVectorClass<T>& Raw()
	{
		return Collection[ActiveContext];
	}

	ViVectorClass<T>& Raw(int context)
	{
		return Collection[context];
	}

	void Set_Active_Context(int active)
	{
		ActiveContext = active;
	}

	int Active_Context_Length() const
	{
		return Collection[ActiveContext].Length();
	}

	void Clear_All()
	{
		for (int i = 0; i < CollectionCount; ++i)
		{
			Collection[i].Clear()
		}
	}

	void Clear()
	{
		Collection[ActiveContext].Clear();
	}

	void Clear(int context)
	{
		Collection[context].Clear();
	}

	int Length(int context) const
	{
		return Collection[context].Length();
	}

private:
	ViVectorClass<T> Collection[COUNT];
	int ActiveContext;
};

/**
 *  A fixed-size array of dynamic vectors.
 */
template<class T, int COUNT, int FIRST = 0, int DEFAULT = FIRST>
class DynamicVectorArrayClass
{
public:
	static const int CollectionCount = COUNT;

public:
	DynamicVectorArrayClass() : ActiveContext(DEFAULT) { }
	virtual ~DynamicVectorArrayClass() { }

	T& operator[](unsigned index)
	{
		return Collection[ActiveContext][index];
	}

	T const& operator[](unsigned index) const
	{
		return Collection[ActiveContext][index];
	}

	ViDynamicVectorClass<T>& Raw()
	{
		return Collection[ActiveContext];
	}

	ViDynamicVectorClass<T>& Raw(int context)
	{
		return Collection[context];
	}

	void Set_Active_Context(int active)
	{
		ActiveContext = active;
	}

	int Active_Context_Count() const
	{
		return Count(ActiveContext);
	}

	int Add_To_Active(T const& object)
	{
		return Add(ActiveContext, object);
	}

	int Add_To_Active_Head(T const& object)
	{
		return Add_Head(ActiveContext, object);
	}

	int Delete_From_Active(T const& object)
	{
		return Delete(ActiveContext, object);
	}

	int Delete_From_Active(int index)
	{
		return Delete(ActiveContext, index);
	}

	int Delete_All(T const& object)
	{
		int count = 0;
		for (int i = FIRST; i < CollectionCount; ++i)
		{
			count += Delete(i, object);
		}
		return count;
	}

	int Delete_All_Except(T const& object, int except)
	{
		int count = 0;
		for (int i = FIRST; i < CollectionCount; ++i)
		{
			if (except != i)
			{
				count += Delete(i, object);
			}
		}
		return count;
	}

	void Clear_All()
	{
		for (int i = FIRST; i < CollectionCount; ++i)
		{
			Collection[i].Clear();
		}
	}

	void Clear()
	{
		Clear(Active);
	}

	void Clear(int context)
	{
		Collection[context].Clear();
	}

	int Count(int context) const
	{
		return Collection[context].Count();
	}

	int Add(int context, T const& object)
	{
		return Collection[context].Add(object);
	}

	int Add_Head(int context, T const& object)
	{
		return Collection[context].Add(object);
	}

	int Delete(int context, T const& object)
	{
		return Collection[context].Delete(object);
	}

	int Delete(int context, int index)
	{
		return Collection[context].Delete(index);
	}

private:
	ViDynamicVectorClass<T> Collection[COUNT];
	int ActiveContext;
	int ActiveContextCount;
};
