#pragma once

#include <memory>

OPTIONALINLINE void __cdecl Set_Bit(void* array, int bit, int value)
{
	__asm {
		mov	ecx, [bit]
		mov	eax, [value]
		mov	esi, [array]
		mov	ebx, ecx
		shr	ebx, 5
		and ecx, 01Fh
		btr[esi + ebx * 4], ecx
		or eax, eax
		jz	ok
		bts[esi + ebx * 4], ecx
		ok :
	}
}


OPTIONALINLINE int __cdecl Get_Bit(void const* array, int bit)
{
	__asm {
		mov	eax, [bit]
		mov	esi, [array]
		mov	ebx, eax
		shr	ebx, 5
		and eax, 01Fh
		bt[esi + ebx * 4], eax
		setc	al
	}
}

OPTIONALINLINE int __cdecl First_True_Bit(void const* array)
{
	__asm {
		mov	esi, [array]
		mov	eax, -32
		again:
		add	eax, 32
			mov	ebx, [esi]
			add	esi, 4
			bsf	ebx, ebx
			jz	again
			add	eax, ebx
	}
}


OPTIONALINLINE int __cdecl First_False_Bit(void const* array)
{
	__asm {

		mov	esi, [array]
		mov	eax, -32
		again:
		add	eax, 32
			mov	ebx, [esi]
			not ebx
			add	esi, 4
			bsf	ebx, ebx
			jz	again
			add	eax, ebx
	}
}

/**************************************************************************
**	This is a general purpose vector class. A vector is defined by this
**	class, as an array of arbitrary objects where the array can be dynamically
**	sized. Because is deals with arbitrary object types, it can handle everything.
**	As a result of this, it is not terribly efficient for integral objects (such
**	as char or int). It will function correctly, but the copy constructor and
**	equality operator could be highly optimized if the integral type were known.
**	This efficiency can be implemented by deriving an integral vector template
**	from this one in order to supply more efficient routines.
*/
template<class T>
class RVectlorClass
{
public:

	RVectlorClass(unsigned size = 0, T const* array = 0) :
		Vector(0),
		VectorMax(size),
		IsAllocated(false) {
		/*
		**	Allocate the vector. The default constructor will be called for every
		**	object in this vector.
		*/
		if (size)
		{
			if (array)
			{
				Vector = new((void*)array) T[size];
			}
			else
			{
				Vector = new T[size];
				IsAllocated = true;
			}
		}
	}

	// Copy constructor.
	RVectlorClass(RVectlorClass<T> const&) :
		Vector(0),
		VectorMax(0),
		IsAllocated(false)
	{
		*this = vector;
	}

	virtual ~RVectlorClass(void)
	{
		RVectlorClass<T>::Clear();
	}

	T& operator[](unsigned index) { return(Vector[index]); };
	T const& operator[](unsigned index) const { return(Vector[index]); };
	virtual RVectlorClass<T>& operator =(RVectlorClass<T> const&) // Assignment operator.
	{
		if (this != &vector)
		{
			Clear();
			VectorMax = vector.Length();
			if (VectorMax)
			{
				Vector = new T[VectorMax];
				if (Vector)
				{
					IsAllocated = true;
					for (int index = 0; index < (int)VectorMax; index++)
					{
						Vector[index] = vector[index];
					}
				}
			}
			else
			{
				Vector = 0;
				IsAllocated = false;
			}
		}
		return(*this);
	}
	virtual int operator == (RVectlorClass<T> const&) const	// Equality operator.
	{
		if (VectorMax == vector.Length())
		{
			for (int index = 0; index < (int)VectorMax; index++)
			{
				if (Vector[index] != vector[index])
				{
					return(false);
				}
			}
			return(true);
		}
		return(false);
	}

	virtual int Resize(unsigned newsize, T const* array = 0)
	{
		if (newsize)
		{

			/*
			**	Allocate a new vector of the size specified. The default constructor
			**	will be called for every object in this vector.
			*/
			T* newptr;
			if (!array)
			{
				newptr = new T[newsize];
			}
			else
			{
				newptr = new((void*)array) T[newsize];
			}
			if (!newptr)
			{
				return(false);
			}

			/*
			**	If there is an old vector, then it must be copied (as much as is feasible)
			**	to the new vector.
			*/
			if (Vector)
			{

				/*
				**	Copy as much of the old vector into the new vector as possible. This
				**	presumes that there is a functional assignment operator for each
				**	of the objects in the vector.
				*/
				int copycount = (newsize < VectorMax) ? newsize : VectorMax;
				for (int index = 0; index < copycount; index++)
				{
					newptr[index] = Vector[index];
				}

				/*
				**	Delete the old vector. This might cause the destructors to be called
				**	for all of the old elements. This makes the implementation of suitable
				**	assignment operator very important. The default assignment operator will
				**	only work for the simplest of objects.
				*/
				if (IsAllocated)
				{
					delete[] Vector;
					Vector = 0;
				}
			}

			/*
			**	Assign the new vector data to this class.
			*/
			Vector = newptr;
			VectorMax = newsize;
			IsAllocated = (Vector && !array);

		}
		else
		{

			/*
			**	Resizing to zero is the same as clearing the vector.
			*/
			Clear();
		}
		return(true);
	}

	virtual void Clear(void)
	{
		if (Vector && IsAllocated)
		{
			delete[] Vector;
			Vector = 0;
		}
		IsAllocated = false;
		VectorMax = 0;
	}

	unsigned Length(void) const { return VectorMax; };
	virtual int ID(T const* ptr)	// Pointer based identification.
	{
		return(((unsigned long)ptr - (unsigned long)&(*this)[0]) / sizeof(T));
	}
	virtual int ID(T const& ptr)	// Value based identification.
	{
		for (int index = 0; index < (int)VectorMax; index++)
		{
			if ((*this)[index] == object)
			{
				return(index);
			}
		}
		return(-1);
	}
protected:

	/*
	**	This is a pointer to the allocated vector array of elements.
	*/
	T* Vector;

	/*
	**	This is the maximum number of elements allowed in this vector.
	*/
	unsigned VectorMax;

	/*
	**	Does the vector data pointer refer to memory that this class has manually
	**	allocated? If so, then this class is responsible for deleting it.
	*/
	unsigned IsAllocated : 1;
};


/**************************************************************************
**	This derivative vector class adds the concept of adding and deleting
**	objects. The objects are packed to the beginning of the vector array.
**	If this is instantiated for a class object, then the assignment operator
**	and the equality operator must be supported. If the vector allocates its
**	own memory, then the vector can grow if it runs out of room adding items.
**	The growth rate is controlled by setting the growth step rate. A growth
**	step rate of zero disallows growing.
*/
template<class T>
class RDynamicVectorClass : public RVectorClass<T>
{
public:
	RDynamicVectorClass(unsigned size = 0, T const* array = 0) : RVectorClass<T>(size, array)
	{
		GrowthStep = 10;
		ActiveCount = 0;
	}

	// Change maximum size of vector.
	virtual int Resize(unsigned newsize, T const* array = 0)
	{
		if (RVectorClass<T>::Resize(newsize, array))
		{
			if (Length() < (unsigned)ActiveCount) ActiveCount = Length();
			return(true);
		}
		return(false);
	}

	// Resets and frees the vector array.
	virtual void Clear(void) { ActiveCount = 0; RVectorClass<T>::Clear(); };

	// Fetch number of "allocated" vector objects.
	int Count(void) const { return(ActiveCount); };

	// Add object to vector (growing as necessary).
	int Add(T const& object)
	{
		if ((unsigned)ActiveCount >= Length())
		{
			if ((IsAllocated || !VectorMax) && GrowthStep > 0)
			{
				if (!Resize(Length() + GrowthStep))
				{

					/*
					**	Failure to increase the size of the vector is an error condition.
					**	Return with the error flag.
					*/
					return(false);
				}
			}
			else
			{

				/*
				**	Increasing the size of this vector is not allowed! Bail this
				**	routine with the error code.
				*/
				return(false);
			}
		}

		/*
		**	There is room for the new object now. Add it to the end of the object vector.
		*/
		(*this)[ActiveCount++] = object;
		return(true);
	}

	int Add_Head(T const& object)
	{
		if (ActiveCount >= Length())
		{
			if ((IsAllocated || !VectorMax) && GrowthStep > 0)
			{
				if (!Resize(Length() + GrowthStep))
				{

					/*
					**	Failure to increase the size of the vector is an error condition.
					**	Return with the error flag.
					*/
					return(false);
				}
			}
			else
			{

				/*
				**	Increasing the size of this vector is not allowed! Bail this
				**	routine with the error code.
				*/
				return(false);
			}
		}

		/*
		**	There is room for the new object now. Add it to the end of the object vector.
		*/
		if (ActiveCount)
		{
			memmove(&(*this)[1], &(*this)[0], ActiveCount * sizeof(T));
		}
		(*this)[0] = object;
		ActiveCount++;
		//	(*this)[ActiveCount++] = object;
		return(true);
	}


	// Delete object just like this from vector.
	int Delete(T const& object)
	{
		int index = ID(object);
		if (index != -1)
		{
			return(Delete(index));
		}
		else
		{
			return (false);
		}
	}

	// Delete object at this vector index.
	int Delete(int index)
	{
		if ((unsigned)index < ActiveCount)
		{
			ActiveCount--;

			/*
			**	If there are any objects past the index that was deleted, copy those
			**	objects down in order to fill the hole. A simple memory copy is
			**	not sufficient since the vector could contain class objects that
			**	need to use the assignment operator for movement.
			*/
			for (int i = index; i < ActiveCount; i++)
			{
				(*this)[i] = (*this)[i + 1];
			}
			return(true);
		}
		return(false);
	}

	// Deletes all objects in the vector.
	void Delete_All(void) { ActiveCount = 0; };

	// Set amount that vector grows by.
	int Set_Growth_Step(int step) { return(GrowthStep = step); };

	// Fetch current growth step rate.
	int Growth_Step(void) { return GrowthStep; };

	virtual int ID(T const* ptr) { return(RVectorClass<T>::ID(ptr)); };
	virtual int ID(T const& ptr)
	{
		for (int index = 0; index < Count(); index++)
		{
			if ((*this)[index] == object) return(index);
		}
		return(-1);
	}

protected:

	/*
	**	This is a count of the number of active objects in this
	**	vector. The memory array often times is bigger than this
	**	value.
	*/
	int ActiveCount;

	/*
	**	If there is insufficient room in the vector array for a new
	**	object to be added, then the vector will grow by the number
	**	of objects specified by this value. This is controlled by
	**	the Set_Growth_Step() function.
	*/
	int GrowthStep;
};

/**************************************************************************
**	This is a derivative of a vector class that supports boolean flags. Since
**	a boolean flag can be represented by a single bit, this class packs the
**	array of boolean flags into an array of bytes containing 8 boolean values
**	each. For large boolean arrays, this results in an 87.5% savings. Although
**	the indexing "[]" operator is supported, DO NOT pass pointers to sub elements
**	of this bit vector class. A pointer derived from the indexing operator is
**	only valid until the next call. Because of this, only simple
**	direct use of the "[]" operator is allowed.
*/
class BooleanVectorClass
{
public:
	BooleanVectorClass(unsigned size = 0, unsigned char* array = 0)
	{
		BitArray.Resize(((size + (8 - 1)) / 8), array);
		LastIndex = -1;
		BitCount = size;
	}
	BooleanVectorClass(BooleanVectorClass const& vector)
	{
		LastIndex = -1;
		*this = vector;
	}
	// Assignment operator.
	BooleanVectorClass& operator =(BooleanVectorClass const& vector)
	{
		Fixup();
		Copy = vector.Copy;
		LastIndex = vector.LastIndex;
		BitArray = vector.BitArray;
		BitCount = vector.BitCount;
		return(*this);
	}

	// Equivalency operator.
	int operator == (BooleanVectorClass const& vector)
	{
		Fixup(LastIndex);
		return(BitCount == vector.BitCount && BitArray == vector.BitArray);
	}

	// Fetch number of boolean objects in vector.
	int Length(void) { return BitCount; };

	// Set all boolean values to false;
	void Reset(void)
	{
		LastIndex = -1;
		if (BitArray.Length())
		{
			memset(&BitArray[0], '\0', BitArray.Length());
		}
	}


	// Set all boolean values to true.
	void Set(void)
	{
		LastIndex = -1;
		if (BitArray.Length())
		{
			memset(&BitArray[0], '\xFF', BitArray.Length());
		}
	}

	// Resets vector to zero length (frees memory).
	void Clear(void)
	{
		Fixup();
		BitCount = 0;
		BitArray.Clear();
	}

	// Change size of this boolean vector.
	int Resize(unsigned size)
	{
		LastIndex = -1;
		if (BitArray.Length())
		{
			memset(&BitArray[0], '\0', BitArray.Length());
		}
	}

	// Fetch reference to specified index.
	bool const& operator[](int index) const
	{
		if (LastIndex != index) Fixup(index);
		return(Copy);
	};
	bool& operator[](int index)
	{
		if (LastIndex != index) Fixup(index);
		return(Copy);
	};

	// Quick check on boolean state.
	bool Is_True(int index) const
	{
		if (index == LastIndex) return(Copy);
		return(Get_Bit(&BitArray[0], index));
	};

	// Find first index that is false.
	int First_False(void) const
	{
		if (LastIndex != -1) Fixup(-1);

		int retval = First_False_Bit(&BitArray[0]);
		if (retval < BitCount) return(retval);

		/*
		**	Failure to find a false boolean value in the vector. Return this
		**	fact in the form of an invalid index number.
		*/
		return(-1);
	}

	// Find first index that is true.
	int First_True(void) const
	{
		if (LastIndex != -1) Fixup(-1);

		int retval = First_True_Bit(&BitArray[0]);
		if (retval < BitCount) return(retval);

		/*
		**	Failure to find a true boolean value in the vector. Return this
		**	fact in the form of an invalid index number.
		*/
		return(-1);
	}

private:
	void Fixup(int index = -1) const
	{
		/*
		**	If the requested index value is illegal, then force the index
		**	to be -1. This is the default non-index value.
		*/
		if (index >= BitCount)
		{
			index = -1;
		}

		/*
		**	If the new index is different than the previous index, there might
		**	be some fixing up required.
		*/
		if (index != LastIndex)
		{

			/*
			**	If the previously fetched boolean value was changed, then update
			**	the boolean array accordingly.
			*/
			if (LastIndex != -1)
			{
				Set_Bit((void*)&BitArray[0], LastIndex, Copy);
			}

			/*
			**	If this new current index is valid, then fill in the reference boolean
			**	value with the appropriate data from the bit array.
			*/
			if (index != -1)
			{
				((unsigned char&)Copy) = Get_Bit(&BitArray[0], index);
			}

			((int&)LastIndex) = index;
		}
	}

	/*
	**	This is the number of boolean values in the vector. This value is
	**	not necessarily a multiple of 8, even though the underlying character
	**	vector contains a multiple of 8 bits.
	*/
	int BitCount;

	/*
	**	This is a referential copy of an element in the bit vector. The
	**	purpose of this copy is to allow normal reference access to this
	**	object (for speed reasons). This hides the bit packing scheme from
	**	the user of this class.
	*/
	bool Copy;

	/*
	**	This records the index of the value last fetched into the reference
	**	boolean variable. This index is used to properly restore the value
	**	when the reference copy needs updating.
	*/
	int LastIndex;

	/*
	**	This points to the allocated bitfield array.
	*/
	RVectlorClass<unsigned char> BitArray;
};
