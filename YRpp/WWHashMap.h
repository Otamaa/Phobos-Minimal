#pragma once
// Westwood's GENIUS hash map
// Finally we got a standard implementation, thank you Westwood

#include <bit>
#include <Memory.h>

template<typename TKey, typename TValue>
class WWHashMap
{
	struct WWHashNode
	{
		TKey Key;
		TValue Value;
		WWHashNode* Next;
	};

	using HashFunctionProc = int(__fastcall)(const TKey& key);
public:
	WWHashMap(HashFunctionProc* hasher = DefaultHashFunction)
	{
		MinLoadFactor = 0.2;
		MaxLoadFactor = 0.8;
		DefaultBucketCount = 128;
		BucketBits = DefaultBucketCount ? std::bit_width(DefaultBucketCount - 1) : 0;
		Block = nullptr;
		Count = 0;
		IsFixedSize = false;
		BucketCount = 1 << BucketBits;
		Buckets = GameCreateArray<WWHashNode*>(BucketCount);
		HashFunction = hasher;
	}

	~WWHashMap()
	{
		Clear();
		if (Buckets)
		{
			YRMemory::Deallocate(Buckets);
			Buckets = nullptr;
		}
		BucketCount = 0;
		BucketBits = 0;
	}

	void Clear()
	{
		const unsigned int originalBucketCount = BucketCount;

		for (int i = 0; i < BucketCount; ++i)
		{
			WWHashNode* node = Buckets[i];

			while (node)
			{
				WWHashNode* current = node;
				node = node->Next;
				GameDelete(current);
			}
			Buckets[i] = nullptr;
		}

		Count = 0;

		if (originalBucketCount > static_cast<unsigned int>(DefaultBucketCount))
			ShrinkToDefaultBucketCount();
	}

	bool Insert(const TKey& key, const TValue& value)
	{
		if (!Buckets)
		{
			BucketBits = DefaultBucketCount ? std::bit_width(static_cast<unsigned int>(DefaultBucketCount - 1)) : 0;
			BucketCount = 1 << BucketBits;
			Buckets = GameCreateArray<WWHashNode*>(BucketCount);
		}

		const int bucket = GetBucketIndex(key);

		for (WWHashNode* node = Buckets[bucket]; node; node = node->Next)
		{
			if (node->Key == key)
			{
				node->Value = value;
				return false;
			}
		}

		WWHashNode* node = static_cast<WWHashNode*>(operator new(sizeof(WWHashNode)));
		new (node) WWHashNode { key, value, Buckets[bucket] };

		Buckets[bucket] = node;
		++Count;

		GrowIfNeeded();

		return true;
	}

	bool Remove(const TKey& key)
	{
		if (!Buckets || BucketCount <= 0)
			return false;

		const int bucket = GetBucketIndex(key);

		WWHashNode* previous = nullptr;
		WWHashNode* node = Buckets[bucket];

		while (node)
		{
			if (node->Key == key)
			{
				if (previous)
					previous->Next = node->Next;
				else
					Buckets[bucket] = node->Next;

				GameDelete(node);
				--Count;
				ShrinkIfNeeded();
				return true;
			}

			previous = node;
			node = node->Next;
		}

		return false;
	}

	TValue* Find(const TKey& key)
	{
		WWHashNode* node = FindNode(key);
		return node ? &node->Value : nullptr;
	}

	const TValue* Find(const TKey& key) const
	{
		const WWHashNode* node = FindNode(key);
		return node ? &node->Value : nullptr;
	}

	bool Contains(const TKey& key) const
	{
		return FindNode(key) != nullptr;
	}

	int GetCount() const
	{
		return Count;
	}

	int GetBucketCount() const
	{
		return BucketCount;
	}

	bool IsEmpty() const
	{
		return Count == 0;
	}

	void SetFixedSize(bool fixed)
	{
		IsFixedSize = fixed;
	}

private:
	static int __fastcall DefaultHashFunction(const TKey& key)
	{
		return *reinterpret_cast<int*>(&key);
	}

	int GetBucketMask()
	{
		return (1 << BucketBits) - 1;
	}

	int GetBucketIndex(const TKey& key)
	{
		return HashFunction(key) & GetBucketMask();
	}

	WWHashNode* FindNode(const TKey& key)
	{
		if (!Count)
			return nullptr;
		auto pNode = Buckets[GetBucketIndex(key)];
		while (pNode)
		{
			if (pNode->Key == key)
				return pNode;
			pNode = pNode->Next;
		}
		return nullptr;
	}

	WWHashNode* FindNode(const TKey& key) const
	{
		if (!Count)
			return nullptr;
		auto pNode = Buckets[GetBucketIndex(key)];
		while (pNode)
		{
			if (pNode->Key == key)
				return pNode;
			pNode = pNode->Next;
		}
		return nullptr;
	}

	void GrowIfNeeded()
	{
		if (IsFixedSize || BucketCount <= 0)
			return;

		const double loadFactor = static_cast<double>(Count) / static_cast<double>(BucketCount);

		if (loadFactor > MaxLoadFactor)
			Resize(BucketCount << 1);
	}

	void ShrinkIfNeeded()
	{
		if (IsFixedSize || BucketCount <= DefaultBucketCount)
			return;

		const double loadFactor = static_cast<double>(Count) / static_cast<double>(BucketCount);

		if (loadFactor < MinLoadFactor)
		{
			int newBucketCount = BucketCount >> 1;
			if (newBucketCount < DefaultBucketCount)
				newBucketCount = DefaultBucketCount;
			Resize(newBucketCount);
		}
	}

	void ShrinkToDefaultBucketCount()
	{
		while (BucketCount > DefaultBucketCount)
		{
			if (IsFixedSize || BucketCount <= DefaultBucketCount)
				break;
			Resize(BucketCount >> 1);
		}
	}

	void Resize(int newBucketCount)
	{
		if (newBucketCount <= 0 || newBucketCount == BucketCount)
			return;

		WWHashNode** oldBuckets = Buckets;
		const int oldBucketCount = BucketCount;

		BucketCount = newBucketCount;
		BucketBits = std::bit_width(static_cast<unsigned int>(BucketCount - 1));

		Buckets = GameCreateArray<WWHashNode*>(BucketCount);

		for (int i = 0; i < oldBucketCount; ++i)
		{
			WWHashNode* node = oldBuckets[i];
			while (node)
			{
				WWHashNode* next = node->Next;
				const int bucket = GetBucketIndex(node->Key);
				node->Next = Buckets[bucket];
				Buckets[bucket] = node;
				node = next;
			}
		}

		GameDelete(oldBuckets);
	}

	WWHashNode** Buckets;
	int Count;
	int BucketCount;
	int BucketBits;
	void* Block;
	bool IsFixedSize;
	HashFunctionProc* HashFunction;
	void* Reserved;
	double MinLoadFactor;
	double MaxLoadFactor;
	int DefaultBucketCount;
};
static_assert(sizeof(WWHashMap<int, int>) == 0x38, "WWHashMap size mismatch");