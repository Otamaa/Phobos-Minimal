#pragma once
#include <Base/Always.h>

template<class T, int size>
class QueueClass
{
public:
	COMPILETIMEEVAL QueueClass();
	COMPILETIMEEVAL ~QueueClass() { }

	T& operator[](int index);

	T& First();
	void Init();
	int Next();
	bool Add(const T& q);

	int Get_Head();
	int Get_Tail();
	T* Get_Array();

public:
	int Count;

private:
	int Head;
	int Tail;
	T Array[size];
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
	if (Count >= size)
	{
		return false;
	}

	if (Count < size)
	{
		Array[Tail] = q;
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
OPTIONALINLINE int QueueClass<T, size>::Get_Head()
{
	return Head;
}

template<class T, int size>
OPTIONALINLINE int QueueClass<T, size>::Get_Tail()
{
	return Tail;
}

template<class T, int size>
OPTIONALINLINE T* QueueClass<T, size>::Get_Array()
{
	return Array;
}
