#pragma once
#include <Base/Always.h>
#include <Unsorted.h>

template<class T, int size>
class QueueClass
{
public:
	QueueClass();
	~QueueClass() { }

	T& operator[](int index);

	T& First();
	void Init();
	int Next();

	bool Add(const T& q);
	bool Add(const T* q);

	int GetHead();
	int GetTail();
	T* GetArray();

public:
	int Count;

private:
	int Head;
	int Tail;
	T Array[size];
	int Timings[size];
};


template<class T, int size>
OPTIONALINLINE QueueClass<T, size>::QueueClass() :
	Count(0)
{
	Init();
}


template<class T, int size>
OPTIONALINLINE void QueueClass<T, size>::Init()
{
	Count = 0;
	Head = 0;
	Tail = 0;
}


template<class T, int size>
OPTIONALINLINE bool QueueClass<T, size>::Add(const T& q)
{
	if (Count < size)
	{
		Array[Tail] = q;
		Timings[Tail] = static_cast<int>(Imports::TimeGetTime().invoke());
		Tail = (Tail + 1) & (size - 1);
		Count = Count + 1;
		return true;
	}
	return false;
}

template<class T, int size>
OPTIONALINLINE bool QueueClass<T, size>::Add(const T* q)
{
	if (Count < size)
	{
		memcpy((Array + Tail), q, sizeof(T));
		Timings[Tail] = static_cast<int>(Imports::TimeGetTime.invoke()());
		Tail = (Tail + 1) & (size - 1);
		Count = Count + 1;
		return true;
	}
	return false;
}

template<class T, int size>
OPTIONALINLINE int QueueClass<T, size>::Next()
{
	if (Count)
	{
		Head = (Head + 1) & (size - 1);
		Count = Count - 1;
	}
	return Count;
}


template<class T, int size>
OPTIONALINLINE T& QueueClass<T, size>::operator[](int index)
{
	return Array[(Head + index) & (size - 1)];
}


template<class T, int size>
OPTIONALINLINE T& QueueClass<T, size>::First()
{
	return Array[Head];
}


template<class T, int size>
OPTIONALINLINE int QueueClass<T, size>::GetHead()
{
	return Head;
}


template<class T, int size>
OPTIONALINLINE int QueueClass<T, size>::GetTail()
{
	return Tail;
}


template<class T, int size>
OPTIONALINLINE T* QueueClass<T, size>::GetArray()
{
	return Array;
}