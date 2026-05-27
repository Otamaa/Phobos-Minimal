#pragma once

// https://github.com/electronicarts/CnC_Renegade/blob/main/Code/Launcher/dictionary.h
// Westwood Hashmap

#include <cstdint>

#include <Memory.h>

template<class K, class V>
class Dictionary
{
	struct Node
	{
		Node(const K& key, const V& value, Node* next) : Key(key), Value(value), Next(next) {}

		K Key;
		V Value;
		Node* Next;
	};

	using HashFunctionProc = uint32_t(__fastcall*)(const K& key);
public:
	Dictionary(HashFunctionProc hashfn) :
		ShrinkThreshold { 0.2 },
		ExpandThreshold { 0.8 },
		MinTableSize { 32 }
	{
		Log2Size = MinTableSize;
		TableSize = MinTableSize;
		TableBits = 0;
		while (Log2Size)
		{
			++TableBits;
			Log2Size >>= 1;
		}
		--TableBits;
		TableSize = 1 << TableBits;
		Count = 0;
		KeepSize = false;
		Table = GameCreateArray<Node*>(TableSize);
		HashFunction = hashfn;
	}

	~Dictionary()
	{
		clear();
		GameDelete(Table);
	}

	struct iterator
	{
		iterator() :
			Dict { nullptr },
			Index { 0 },
			Current { nullptr }
		{}

		iterator(Dictionary* dict, uint32_t index, Node* current) :
			Dict { dict },
			Index { index },
			Current { current }
		{}

		Node& operator*() const
		{
			return *Current;
		}

		Node* operator->() const
		{
			return Current;
		}

		iterator& operator++()
		{
			if (!Dict || !Current)
				return *this;

			if (Current->Next)
			{
				Current = Current->Next;
				return *this;
			}

			while (++Index < Dict->TableSize)
			{
				Current = Dict->Table[Index];
				if (Current)
					return *this;
			}

			Current = nullptr;
			return *this;
		}

		iterator operator++(int)
		{
			auto temp = *this;
			++(*this);
			return temp;
		}

		bool operator==(const iterator& other) const
		{
			return Current == other.Current;
		}

		bool operator!=(const iterator& other) const
		{
			return !(*this == other);
		}

	private:
		Dictionary* Dict;
		uint32_t Index;
		Node* Current;
	};

	// Provide a set of modern C++ container interface for convienience
	iterator begin()
	{
		for (uint32_t i = 0; i < TableSize; ++i)
			if (Table[i])
				return iterator { this, i, Table[i] };
		return end();
	}

	iterator end()
	{
		return iterator { this, TableSize, nullptr };
	}

	size_t size() const
	{
		return Count;
	}

	V& operator[](const K& key)
	{
		auto offset = key_hash(key);
		auto node = Table[offset];
		while (node)
		{
			if (node->Key == key)
				return node->Value;
			node = node->Next;
		}
		insert(key, V {});
		return Table[offset]->Value;
	}

	void clear()
	{
		for (uint32_t i = 0; i < TableSize; ++i)
		{
			auto node = Table[i];
			while (node)
			{
				auto next = node->Next;
				GameDelete(node);
				node = next;
			}
			Table[i] = nullptr;
		}
		Count = 0;
		while (TableSize > static_cast<uint32_t>(MinTableSize) && !KeepSize)
			shrink();
	}

	bool insert(const K& key, const V& value)
	{
		auto item = GameCreate<Node>(key, value, nullptr);
		erase(key);

		auto offset = key_hash(key);
		auto node = Table[offset];
		if (!node)
			Table[offset] = item;
		else
		{
			auto temp = node;
			Table[offset] = item;
			item->Next = temp;
		}

		++Count;
		float percent = static_cast<float>(Count) / TableSize;
		if (percent >= ExpandThreshold)
			expand();

		return true;
	}

	V* try_get(const K& key)
	{
		auto offset = key_hash(key);
		auto node = Table[offset];
		if (!node)
			return nullptr;

		while (node)
		{
			if (node->Key == key)
				return &node->Value;
			node = node->Next;
		}

		return nullptr;
	}

	bool erase(const K& key)
	{
		if (!Count)
			return false;

		float percent = static_cast<float>(Count) / TableSize;

		auto offset = key_hash(key);
		auto node = Table[offset];

		if (!node)
			return false;
		auto last = node;

		//special case table points to thing to delete
		if (node->Key == key)
		{
			auto temp = node->Next;
			GameDelete(node);
			Table[offset] = temp;
			--Count;
			if (percent <= ShrinkThreshold)
				shrink();

			return true;
		}
		node = node->Next;

		// Now the case if the thing to delete is not the first
		while (node)
		{
			if (node->Key == key)
			{
				last->Next = node->Next;
				--Count;
				GameDelete(node);
				break;
			}
			last = node;
			node = node->Next;
		}

		if (percent <= ShrinkThreshold)
			shrink();

		return true;
	}

private:
	uint32_t key_hash(const K& key)
	{
		return HashFunction(key) & ((1 << TableBits) - 1);
	}

	void shrink()
	{
		if (TableSize <= static_cast<uint32_t>(MinTableSize) || KeepSize)
			return;

		auto oldTable = Table;
		auto oldTableSize = TableSize;
		TableSize /= 2;
		--TableBits;

		Table = GameCreateArray<Node*>(TableSize);
		for (uint32_t i = 0; i < oldTableSize; ++i)
		{
			for (auto node = oldTable[i]; node; )
			{
				auto offset = key_hash(node->Key);
				auto first = Table[offset];
				Table[offset] = node;
				auto next = node->Next;
				node->Next = first;
				node = next;
			}
		}
		GameDelete(oldTable);
	}

	void expand()
	{
		if (KeepSize)
			return;

		auto oldTable = Table;
		auto oldTableSize = TableSize;

		TableSize *= 2;
		++TableBits;

		Table = GameCreateArray<Node*>(TableSize);
		for (uint32_t i = 0; i < oldTableSize; ++i)
		{
			for (auto node = oldTable[i]; node; )
			{
				auto offset = key_hash(node->Key);
				auto first = Table[offset];
				Table[offset] = node;
				auto next = node->Next;
				node->Next = first;
				node = next;
			}
		}
		GameDelete(oldTable);
	}

	Node** Table;
	uint32_t Count;
	uint32_t TableSize;
	uint32_t TableBits;
	uint32_t Log2Size;
	bool KeepSize;
	HashFunctionProc HashFunction;
	double ShrinkThreshold;
	double ExpandThreshold;
	int MinTableSize;
};
static_assert(sizeof(Dictionary<void*, void*>) == 0x38, "Dictionary size mismatch");