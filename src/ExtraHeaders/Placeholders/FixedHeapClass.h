#pragma once

#include "RVectorClass.h"
#include <Pipes.h>
#include <Straws.h>

class FixedHeapClass
{
public:
	FixedHeapClass(int size) :
		IsAllocated(false),
		Size(size),
		TotalCount(0),
		ActiveCount(0),
		Buffer(0)
	{
	}

	virtual ~FixedHeapClass(void)
	{
		FixedHeapClass::Clear();
	}

	int Count(void) const { return ActiveCount; };
	int Length(void) const { return TotalCount; };
	int Avail(void) const { return TotalCount - ActiveCount; };

	virtual int ID(void const* pointer) const
	{
		if (pointer && Size)
		{
			return((int)(((char*)pointer - (char*)Buffer) / Size));
		}
		return(-1);
	}

	virtual int Set_Heap(int count, void* buffer = 0)
	{
		/*
		**	Clear out the old heap data.
		*/
		Clear();

		/*
		**	If there is no size to the objects in the heap, then this block memory
		**	handler can NEVER function. Return with a failure condition.
		*/
		if (!Size) return(false);

		/*
		**	If there is no count specified, then this indicates that the heap should
		**	be disabled.
		*/
		if (!count) return(true);

		/*
		**	Initialize the free boolean vector and the buffer for the actual
		**	allocation objects.
		*/
		if (FreeFlag.Resize(count))
		{
			if (!buffer)
			{
				buffer = new char[count * Size];
				if (!buffer)
				{
					FreeFlag.Clear();
					return(false);
				}
				IsAllocated = true;
			}
			Buffer = buffer;
			TotalCount = count;
			return(true);
		}
		return(false);
	}
	virtual void* Allocate(void)
	{
		if (ActiveCount < TotalCount)
		{
			int index = FreeFlag.First_False();

			if (index != -1)
			{
				ActiveCount++;
				FreeFlag[index] = true;
				return((*this)[index]);
			}
		}
		return(0);
	}
	virtual void Clear(void)
	{
		/*
		**	Free the old buffer (if present).
		*/
		if (Buffer && IsAllocated)
		{
			delete[] Buffer;
		}
		Buffer = 0;
		IsAllocated = false;
		ActiveCount = 0;
		TotalCount = 0;
		FreeFlag.Clear();
	}
	virtual int Free(void* pointer)
	{
		if (pointer && ActiveCount)
		{
			int index = ID(pointer);

			if (index < TotalCount)
			{
				if (FreeFlag[index])
				{
					ActiveCount--;
					FreeFlag[index] = false;
					return(true);
				}
			}
		}
		return(false);
	}
	virtual int Free_All(void)
	{
		ActiveCount = 0;
		FreeFlag.Reset();
		return(true);
	}

	void* operator[](int index) { return ((char*)Buffer) + (index * Size); };
	void const* operator[](int index) const { return ((char*)Buffer) + (index * Size); };

protected:
	/*
	**	If the memory block buffer was allocated by this class, then this flag
	**	will be true. The block must be deallocated by this class if true.
	*/
	unsigned IsAllocated : 1;

	/*
	**	This is the size of each sub-block within the buffer.
	*/
	int Size;

	/*
	**	This records the absolute number of sub-blocks in the buffer.
	*/
	int TotalCount;

	/*
	**	This is the total blocks allocated out of the heap. This number
	**	will never exceed Count.
	*/
	int ActiveCount;

	/*
	**	Pointer to the heap's memory buffer.
	*/
	void* Buffer;

	/*
	**	This is a boolean vector array of allocation flag bits.
	*/
	BooleanVectorClass FreeFlag;

private:
	// The assignment operator is not supported.
	FixedHeapClass& operator = (FixedHeapClass const&);

	// The copy constructor is not supported.
	FixedHeapClass(FixedHeapClass const&);
};


/**************************************************************************
**	This template serves only as an interface to the heap manager class. By
**	using this template, the object pointers are automatically converted
**	to the correct type without any code overhead.
*/
template<class T>
class TFixedHeapClass : public FixedHeapClass
{
public:
	TFixedHeapClass(void) : FixedHeapClass(sizeof(T)) { };
	virtual ~TFixedHeapClass(void) { };


	virtual int ID(T const* pointer) const { return FixedHeapClass::ID(pointer); };
	virtual T* Alloc(void) { return (T*)FixedHeapClass::Allocate(); };
	virtual int Free(T* pointer) { return(FixedHeapClass::Free(pointer)); };

	T& operator[](int index) { return *(T*)(((char*)Buffer) + (index * Size)); };
	T const& operator[](int index) const { return *(T*)(((char*)Buffer) + (index * Size)); };
};

/**************************************************************************
**	This is a derivative of the fixed heap class. This class adds the
**	ability to quickly iterate through the active (allocated) objects. Since the
**	active array is a sequence of pointers, the overhead of this class
**	is 4 bytes per potential allocated object (be warned).
*/
class FixedIHeapClass : public FixedHeapClass
{
public:
	FixedIHeapClass(int size) : FixedHeapClass(size) { };
	virtual ~FixedIHeapClass(void) { };

	virtual int Set_Heap(int count, void* buffer = 0)
	{
		Clear();
		if (FixedHeapClass::Set_Heap(count, buffer))
		{
			ActivePointers.Resize(count);
			return(true);
		}
		return(false);
	}

	virtual void* Allocate(void)
	{
		void* ptr = FixedHeapClass::Allocate();
		if (ptr)
		{
			ActivePointers.Add(ptr);
			memset(ptr, 0, Size);
		}
		return(ptr);
	}
	virtual void Clear(void)
	{
		FixedHeapClass::Clear();
		ActivePointers.Clear();
	}
	virtual int Free(void* pointer)
	{
		if (FixedHeapClass::Free(pointer))
		{
			ActivePointers.Delete(pointer);
		}
		return(false);
	}
	virtual int Free_All(void)
	{
		ActivePointers.Delete_All();
		return(FixedHeapClass::Free_All());
	}
	virtual int Logical_ID(void const* pointer) const
	{
		if (pointer != NULL)
		{
			for (int index = 0; index < Count(); index++)
			{
				if (Active_Ptr(index) == pointer)
				{
					return(index);
				}
			}
		}
		return(-1);
	}
	virtual int Logical_ID(int id) const { return(Logical_ID((*this)[id])); }

	virtual void* Active_Ptr(int index) { return ActivePointers[index]; };
	virtual void const* Active_Ptr(int index) const { return ActivePointers[index]; };

	/*
	**	This is an array of pointers to allocated objects. Using this array
	**	to control iteration through the objects ensures a minimum of processing.
	**	It also allows access to this array so that custom sorting can be
	**	performed.
	*/
	RDynamicVectorClass<void*> ActivePointers;
};

/**************************************************************************
**	This template serves only as an interface to the iteratable heap manager
**	class. By using this template, the object pointers are automatically converted
**	to the correct type without any code overhead.
*/
class FileClass;
template<class T>
class TFixedIHeapClass : public FixedIHeapClass
{
public:
	TFixedIHeapClass(void) : FixedIHeapClass(sizeof(T)) { };
	virtual ~TFixedIHeapClass(void) { };

	virtual int ID(T const* pointer) const { return FixedIHeapClass::ID(pointer); };
	virtual int Logical_ID(T const* pointer) const { return(FixedIHeapClass::Logical_ID(pointer)); }
	virtual int Logical_ID(int id) const { return(FixedIHeapClass::Logical_ID(id)); }
	virtual T* Alloc(void) { return (T*)FixedIHeapClass::Allocate(); };
	virtual int Free(T* pointer) { return FixedIHeapClass::Free(pointer); };
	virtual int Free(void* pointer) { return FixedIHeapClass::Free(pointer); };
	virtual int Save(Pipe& file) const
	{
		/*
		** Save the number of instances of this class
		*/
		file.Put(&ActiveCount, sizeof(ActiveCount));

		/*
		** Save each instance of this class
		*/
		for (int i = 0; i < ActiveCount; i++)
		{

			/*
			** Save the array index of the object, so it can be loaded back into the
			** same array location (so TARGET translations will work)
			*/
			int idx = ID(Ptr(i));
			file.Put(&idx, sizeof(idx));

			/*
			** Save the object itself
			*/
			file.Put(Ptr(i), sizeof(T));
		}

		return(true);
	}
	virtual int Load(Straw& file)
	{
		int i;			// loop counter
		int idx;			// object index
		T* ptr;			// object pointer
		int a_count;

		/*
		** Read the number of instances of this class
		*/
		if (file.Get(&a_count, sizeof(a_count)) != sizeof(a_count))
		{
			return(false);
		}

		/*
		** Error if more objects than we can hold
		*/
		if (a_count > TotalCount)
		{
			return(false);
		}

		/*
		** Read each class instance
		*/
		for (i = 0; i < a_count; i++)
		{
			/*
			** Read the object's array index
			*/
			if (file.Get(&idx, sizeof(idx)) != sizeof(idx))
			{
				return(false);
			}

			/*
			** Get a pointer to the object, activate that object
			*/
			ptr = (T*)(*this)[idx];
			FreeFlag[idx] = true;
			ActiveCount++;
			ActivePointers.Add(ptr);

			/*
			** Load the object
			*/
			file.Get(ptr, sizeof(T));
			new(ptr) T(NoInitClass());
			//		if (!ptr->Load(file)) {
			//			return(false);
			//		}
		}

		return(true);
	}
	virtual void Code_Pointers(void)
	{
		int i;

		for (i = 0; i < ActiveCount; i++)
		{
			Ptr(i)->Code_Pointers();
		}
	}
	virtual void Decode_Pointers(void)
	{
		int i;

		for (i = 0; i < ActiveCount; i++)
		{
			Ptr(i)->Decode_Pointers();
		}
	}

	virtual T* Ptr(int index) const { return (T*)FixedIHeapClass::ActivePointers[index]; };
	virtual T* Raw_Ptr(int index) { return (index >= 0 && index < Length()) ? (T*)((*this)[index]) : NULL; };
};
