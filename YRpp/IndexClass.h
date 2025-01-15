#pragma once

#include <YRPPCore.h>
#include <GenericList.h>

/*
* IndexClass, most impl from CCR
* --secsome
*/

template<typename TKey, typename TValue>
struct NodeElement
{
	//NodeElement() = default;
	//~NodeElement() = default;

	NodeElement& operator=(const NodeElement& node) { ID = node.ID; Data = node.Data; return *this; }
	bool operator<(const NodeElement& another) const { return ID < another.ID; }
	bool operator==(const NodeElement& another) const { return ID == another.ID; }

	TKey ID;
	TValue Data;
};

// TKey is always int in WW's code
template<typename TKey, typename TValue>
class IndexClass
{
public:
	IndexClass(void);
	~IndexClass(void);
	using NodeType = NodeElement<TKey, TValue>;

	COMPILETIMEEVAL bool AddIndex(TKey id, const TValue& data);
	bool AddIndex(TKey id, TValue&& data);
	bool RemoveIndex(TKey id);
	bool IsPresent(TKey id, bool Lowerbound = false) const;
	int Count() const;
	const TValue& FetchIndex(TKey id) const;
	TValue& FetchIndex(TKey id);
	void Clear();
	bool Reverse(int nAmount);
	OPTIONALINLINE void Sort();
	static int __cdecl Comparator(void const* ptr, void const* ptr2);


	//static_assert(sizeof(NodeElement) == 0x8);//

	NodeType* IndexTable;
	int IndexCount;
	int IndexSize;
	bool IsSorted;
	PROTECTED_PROPERTY(char, padding[3]);
	NodeType* Archive;

	// ranged for support
	auto begin() const { return &IndexTable[0]; }
	auto end() const { return &IndexTable[IndexCount]; }
	auto begin() { return &IndexTable[0]; }
	auto end() { return &IndexTable[IndexCount]; }
	auto FetchItem(TKey id, bool Lowerbound = false) const;

private:
	bool IncreaseTableSize(int nAmount);
	bool IsArchiveSame(TKey id) const;
	void InvalidateArchive();
	void SetArchive(NodeType const* pNode);
	NodeType const* SearchForNode(TKey id, bool Lowerbound = false) const;

	//static int __cdecl search_compfunc(void const * ptr, void const * ptr2);
};

// See RA1 code
template<typename TKey, typename TValue>
int __cdecl IndexClass<TKey, TValue>::Comparator(void const* ptr1, void const* ptr2)
{
	const NodeType* n1 = static_cast<const NodeType*>(ptr1);
	const NodeType* n2 = static_cast<const NodeType*>(ptr2);

	if (n1->ID == n2->ID)
		return 0;
	if (n1->ID < n2->ID)
		return -1;

	return 1;
}

template<typename TKey, typename TValue>
IndexClass<TKey, TValue>::IndexClass() :
	IndexTable(0),
	IndexCount(0),
	IndexSize(0),
	IsSorted(false),
	Archive(0)
{
	this->InvalidateArchive();
}

template<typename TKey, typename TValue>
IndexClass<TKey, TValue>::~IndexClass()
{
	this->Clear();
}

template<typename TKey, typename TValue>
void IndexClass<TKey, TValue>::Clear()
{
	GameDelete(this->IndexTable);
	this->IndexTable = 0;
	this->IndexCount = 0;
	this->IndexSize = 0;
	this->IsSorted = false;
	this->InvalidateArchive();
}

template<typename TKey, typename TValue>
bool IndexClass<TKey, TValue>::IncreaseTableSize(int amount)
{
	if (amount < 0) return false;

	NodeType* table = GameCreateArray<NodeType>(this->IndexSize + amount);
	if (table != nullptr)
	{
		for (int i = 0; i < this->IndexCount; ++i)
			table[i] = this->IndexTable[i];

		GameDelete(this->IndexTable);
		this->IndexTable = table;
		this->IndexSize += amount;
		this->InvalidateArchive();

		return true;
	}
	return false;
}

template<typename TKey, typename TValue>
bool IndexClass<TKey, TValue>::Reverse(int amount)
{
	Clear();
	return IncreaseTableSize(amount);
}

template<typename TKey, typename TValue>
OPTIONALINLINE void IndexClass<TKey, TValue>::Sort()
{
	if (!IsSorted)
	{
		std::sort(begin(), end());
		InvalidateArchive();
		IsSorted = true;
	}
}

template<typename TKey, typename TValue>
int IndexClass<TKey, TValue>::Count() const
{
	return this->IndexCount;
}

template<typename TKey, typename TValue>
bool IndexClass<TKey, TValue>::IsPresent(TKey id, bool Lowerbound) const
{
	if (!this->IndexCount)
		return false;

	if (this->IsArchiveSame(id))
		return true;

	if (NodeType const* nodeptr = SearchForNode(id, Lowerbound))
	{
		const_cast<IndexClass<TKey, TValue>*>(this)->SetArchive(nodeptr);
		return true;
	}

	return false;
}

template<typename TKey, typename TValue>
auto IndexClass<TKey, TValue>::FetchItem(TKey id, bool Lowerbound) const
{
	if (!this->IsArchiveSame(id))
	{
		if (NodeType const* nodeptr = const_cast<IndexClass<TKey, TValue>*>(this)->SearchForNode(id , Lowerbound)) {
			const_cast<IndexClass<TKey, TValue>*>(this)->SetArchive(nodeptr);
			return this->Archive;
		} 

		return (NodeType*)nullptr;
	}

	return this->Archive;
}

template<typename TKey, typename TValue>
const TValue& IndexClass<TKey, TValue>::FetchIndex(TKey id) const
{
	return this->IsPresent(id) ? this->Archive->Data : TValue();
}

template<typename TKey, typename TValue>
TValue& IndexClass<TKey, TValue>::FetchIndex(TKey id)
{
	if (!this->IsPresent(id))
	{
		this->AddIndex(id, TValue());
		this->IsPresent(id);
	}

	return this->Archive->Data;
}

template<typename TKey, typename TValue>
bool IndexClass<TKey, TValue>::IsArchiveSame(TKey id) const
{
	return this->Archive != 0 && this->Archive->ID == id;
}

template<typename TKey, typename TValue>
void IndexClass<TKey, TValue>::InvalidateArchive()
{
	this->Archive = nullptr;
}

template<typename TKey, typename TValue>
void IndexClass<TKey, TValue>::SetArchive(NodeType const* node)
{
	this->Archive = const_cast<NodeType*>(node);
}

template<typename TKey, typename TValue>
COMPILETIMEEVAL bool IndexClass<TKey, TValue>::AddIndex(TKey id, const TValue& data)
{
	if (this->IndexCount + 1 > this->IndexSize)
		if (!this->IncreaseTableSize(this->IndexSize == 0 ? 10 : this->IndexSize))
			return false;

	this->IndexTable[IndexCount].ID = std::move(id);
	this->IndexTable[IndexCount].Data = std::move(data);
	++this->IndexCount;
	this->IsSorted = false;

	return true;
}

template<typename TKey, typename TValue>
bool IndexClass<TKey, TValue>::AddIndex(TKey id, TValue&& data)
{
	if (this->IndexCount + 1 > this->IndexSize)
		if (!this->IncreaseTableSize(this->IndexSize == 0 ? 10 : this->IndexSize))
			return false;

	this->IndexTable[IndexCount].ID = std::move(id);
	this->IndexTable[IndexCount].Data = std::move(data);
	++this->IndexCount;
	this->IsSorted = false;

	return true;
}

template<typename TKey, typename TValue>
bool IndexClass<TKey, TValue>::RemoveIndex(TKey id)
{
	int found_index = -1;
	for (int i = 0; i < this->IndexCount; ++i)
		if (this->IndexTable[i].ID == id)
		{
			found_index = i;
			break;
		}

	if (found_index != -1)
	{
		for (int i = found_index + 1; i < this->IndexCount; ++i)
			IndexTable[i - 1] = IndexTable[i];
		--IndexCount;

		IndexTable[IndexCount] = std::move(NodeType(TKey(), TValue()));		// zap last (now unused) element

		this->InvalidateArchive();
		return true;
	}

	return false;
}

//template<typename TKey, typename TValue>
//int __cdecl IndexClass<TKey, TValue>::search_compfunc(void const* ptr1, void const* ptr2)
//{
//	if (*(int const*)ptr1 == *(int const*)ptr2)
//	{
//		return 0;
//	}
//	if (*(int const*)ptr1 < *(int const*)ptr2)
//	{
//		return -1;
//	}
//	return 1;
//}

template<typename TKey, typename TValue>
typename IndexClass<TKey, TValue>::NodeType const* IndexClass<TKey, TValue>::SearchForNode(TKey id , bool Lowerbound) const
{
	if (!this->IndexCount)
		return 0;

	const_cast<IndexClass<TKey, TValue>*>(this)->Sort();

	if(Lowerbound) {
		const NodeType* find = static_cast<const NodeType*>(std::lower_bound(begin(), end(), NodeType{ id , {} }));
		return (find == end() || id < find->ID) ? nullptr : find;
	}

	NodeType nodeToSearch { id , {} };
	return static_cast<const NodeType*>(std::bsearch(&nodeToSearch, &IndexTable[0], IndexCount, sizeof(IndexTable[0]), Comparator));
}
