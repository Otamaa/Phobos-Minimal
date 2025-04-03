#pragma once

#include <Base/Always.h>
#include <ArrayClasses.h>
// Used with color schemes and what not.

class HashString
{
public:
	char Name[256];
};

template<typename key_type, typename value_type>
class HashObject
{
public:
	key_type Key;
	value_type Value;
};

template<typename key_type, typename value_type>
struct HashTable
{
public:
	DynamicVectorClass<HashObject<key_type,value_type>>* Buckets;
	DWORD (*BucketHashFunction)(const key_type&);
	int BucketCount;
	int BucketGrowthStep;
};

struct HashIterator
{
	int BucketIndex;
	int InBucketIndex;
	bool OutOfBuckets;
};
