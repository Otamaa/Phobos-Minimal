#pragma once
#include "INIComment.h"

class GenericListOfINIEntry;
class GenericNodeOfINIEntry
{
public:
	GenericNodeOfINIEntry() : NextNode(nullptr), PrevNode(nullptr) { }
	virtual ~GenericNodeOfINIEntry() { Unlink(); }
	GenericNodeOfINIEntry(GenericNodeOfINIEntry& node) { node.Link(this); }
	GenericNodeOfINIEntry& operator = (GenericNodeOfINIEntry& node)
	{
		if (&node != this)
			node.Link(this);

		return *this;
	}

	void __forceinline Unlink()
	{
		if (this->IsValid())
		{
			this->PrevNode->NextNode = this->NextNode;
			this->NextNode->PrevNode = this->PrevNode;
			this->PrevNode = nullptr;
			this->NextNode = nullptr;
		}
	}

	GenericListOfINIEntry* MainList() const
	{
		GenericNodeOfINIEntry const* node = this;

		while (node->PrevNode)
			node = this->PrevNode;

		return (GenericListOfINIEntry*)this;
	}

	void Link(GenericNodeOfINIEntry* pNode)
	{
		pNode->Unlink();
		pNode->NextNode = this->NextNode;
		pNode->PrevNode = this;
		if (this->NextNode) this->NextNode->PrevNode = pNode;
		this->NextNode = pNode;
	}

	GenericNodeOfINIEntry* Next() const { return this->NextNode; }
	GenericNodeOfINIEntry* Prev() const { return this->PrevNode; }
	bool IsValid() const { return this && this->NextNode && this->PrevNode; }

protected:
	GenericNodeOfINIEntry* NextNode;
	GenericNodeOfINIEntry* PrevNode;
};

class GenericListOfINIEntry
{
public:
	GenericListOfINIEntry()
	{
		FirstNode.Link(&LastNode);
	}

	GenericListOfINIEntry(GenericListOfINIEntry& list) = default;
	GenericListOfINIEntry& operator = (GenericListOfINIEntry const&) = default;

	virtual ~GenericListOfINIEntry()
	{
		this->UnlinkAll();
	}

	GenericNodeOfINIEntry* First() const { return FirstNode.Next(); }
	GenericNodeOfINIEntry* Last() const { return LastNode.Prev(); }
	GenericNodeOfINIEntry* GetFirst() { return (&FirstNode); }
	GenericNodeOfINIEntry* GetLast() { return (&LastNode); }

	bool IsEmpty() const { return !FirstNode.Next()->IsValid(); }
	void AddHead(GenericNodeOfINIEntry* pNode) { FirstNode.Link(pNode); }
	void AddTail(GenericNodeOfINIEntry* pNode) { LastNode.Prev()->Link(pNode); }
	void Delete() { while (this->FirstNode.Next()->IsValid()) GameDelete(this->FirstNode.Next()); }
	void __forceinline UnlinkAll()
	{
		this->FirstNode.Unlink();
		this->LastNode.Unlink();
	}
protected:
	GenericNodeOfINIEntry FirstNode;
	GenericNodeOfINIEntry LastNode;

};

class INIEntry;
class ListOfINIEntry : public GenericListOfINIEntry
{
public:

	ListOfINIEntry() = default;
	virtual ~ListOfINIEntry() override { this->UnlinkAll(); }

	INIEntry* First() const { return (INIEntry*)GenericListOfINIEntry::First(); }
	INIEntry* Last() const { return (INIEntry*)GenericListOfINIEntry::Last(); }
};

class NodeOfINIEntry : public GenericNodeOfINIEntry
{
public:
	ListOfINIEntry* MainList() const { return (ListOfINIEntry*)GenericNodeOfINIEntry::MainList(); }
	INIEntry* Next() const { return (INIEntry*)GenericNodeOfINIEntry::Next(); }
	INIEntry* Prev() const { return (INIEntry*)GenericNodeOfINIEntry::Prev(); }
	bool IsValid() const { return GenericNodeOfINIEntry::IsValid(); }
};

class INIEntry : public NodeOfINIEntry
{
public:
	char* Key;
	char* Value;
	INIComment* Comments;
	char* CommentString;
	int PreIndentCursor;
	int PostIndentCursor;
	int CommentCursor;
};
static_assert(sizeof(INIEntry) == 0x28, "Invalid size.");

struct NodeElementOfINIEntry
{
	NodeElementOfINIEntry& operator=(const NodeElementOfINIEntry& node) { ID = node.ID; Data = node.Data; return *this; }
	bool operator<(const NodeElementOfINIEntry& another) const { return ID < another.ID; }
	bool operator==(const NodeElementOfINIEntry& another) const { return ID == another.ID; }

	int ID;
	INIEntry* Data;
};

class IndexClassOfINIEntry
{
public:
	// ranged for support
	NodeElementOfINIEntry* begin() const { return &IndexTable[0]; }
	NodeElementOfINIEntry* end() const { return &IndexTable[IndexCount]; }
	NodeElementOfINIEntry* begin() { return &IndexTable[0]; }
	NodeElementOfINIEntry* end() { return &IndexTable[IndexCount]; }

public:
	IndexClassOfINIEntry() :
		IndexTable(0),
		IndexCount(0),
		IndexSize(0),
		IsSorted(false),
		Archive(0)
	{
		this->InvalidateArchive();
	}

	~IndexClassOfINIEntry()
	{
		this->Clear();
	}

	bool IsPresent(int id, bool Lowerbound = false) const
	{
		if (!this->IndexCount)
			return false;

		if (this->IsArchiveSame(id))
			return true;

		if (NodeElementOfINIEntry const* nodeptr = SearchForNode(id, Lowerbound))
		{
			const_cast<IndexClassOfINIEntry*>(this)->SetArchive(nodeptr);
			return true;
		}

		return false;
	}

	int Count() const
	{
		return this->IndexCount;
	}

	NodeElementOfINIEntry* FetchItem(int id, bool Lowerbound) const
	{
		if (!this->IsArchiveSame(id))
		{
			if (NodeElementOfINIEntry const* nodeptr = const_cast<IndexClassOfINIEntry*>(this)->SearchForNode(id, Lowerbound))
			{
				const_cast<IndexClassOfINIEntry*>(this)->SetArchive(nodeptr);
				return this->Archive;
			}

			return (NodeElementOfINIEntry*)nullptr;
		}

		return this->Archive;
	}

	void Clear()
	{
		GameDelete(this->IndexTable);
		this->IndexTable = 0;
		this->IndexCount = 0;
		this->IndexSize = 0;
		this->IsSorted = false;
		this->InvalidateArchive();
	}

	inline void Sort()
	{
		if (!IsSorted)
		{
			std::sort(begin(), end());
			InvalidateArchive();
			IsSorted = true;
		}
	}

	static int __cdecl Comparator(void const* ptr, void const* ptr2)
	{
		const NodeElementOfINIEntry* n1 = static_cast<const NodeElementOfINIEntry*>(ptr);
		const NodeElementOfINIEntry* n2 = static_cast<const NodeElementOfINIEntry*>(ptr2);

		if (n1->ID == n2->ID)
			return 0;
		if (n1->ID < n2->ID)
			return -1;

		return 1;
	}

	NodeElementOfINIEntry* IndexTable;
	int IndexCount;
	int IndexSize;
	bool IsSorted;
	PROTECTED_PROPERTY(char, padding[3]);
	NodeElementOfINIEntry* Archive;

private:

	bool IsArchiveSame(int id) const
	{
		return this->Archive != 0 && this->Archive->ID == id;
	}

	void InvalidateArchive()
	{
		this->Archive = nullptr;
	}

	void SetArchive(NodeElementOfINIEntry const* pNode)
	{
		this->Archive = const_cast<NodeElementOfINIEntry*>(pNode);
	}

	NodeElementOfINIEntry const* SearchForNode(int id, bool Lowerbound = false) const
	{
		if (!this->IndexCount)
			return 0;

		const_cast<IndexClassOfINIEntry*>(this)->Sort();

		NodeElementOfINIEntry seatch { id , nullptr };

		if (Lowerbound)
		{
			const NodeElementOfINIEntry* find = static_cast<const NodeElementOfINIEntry*>(std::lower_bound(begin(), end(), seatch));
			return (find == end() || id < find->ID) ? nullptr : find;
		}

		return static_cast<const NodeElementOfINIEntry*>(std::bsearch(&seatch, &IndexTable[0], IndexCount, sizeof(IndexTable[0]), Comparator));
	}
};

class GenericListOfINIEntryPtr;
class GenericNodeOfINIEntryPtr
{
public:
	GenericNodeOfINIEntryPtr() : NextNode(nullptr), PrevNode(nullptr) { }
	virtual ~GenericNodeOfINIEntryPtr() { Unlink(); }
	GenericNodeOfINIEntryPtr(GenericNodeOfINIEntryPtr& node) { node.Link(this); }
	GenericNodeOfINIEntryPtr& operator = (GenericNodeOfINIEntryPtr& node)
	{
		if (&node != this)
			node.Link(this);

		return *this;
	}

	void __forceinline Unlink()
	{
		if (this->IsValid())
		{
			this->PrevNode->NextNode = this->NextNode;
			this->NextNode->PrevNode = this->PrevNode;
			this->PrevNode = nullptr;
			this->NextNode = nullptr;
		}
	}

	GenericListOfINIEntryPtr* MainList() const
	{
		GenericNodeOfINIEntryPtr const* node = this;

		while (node->PrevNode)
			node = this->PrevNode;

		return (GenericListOfINIEntryPtr*)this;
	}

	void Link(GenericNodeOfINIEntryPtr* pNode)
	{
		pNode->Unlink();
		pNode->NextNode = this->NextNode;
		pNode->PrevNode = this;
		if (this->NextNode) this->NextNode->PrevNode = pNode;
		this->NextNode = pNode;
	}

	GenericNodeOfINIEntryPtr* Next() const { return this->NextNode; }
	GenericNodeOfINIEntryPtr* Prev() const { return this->PrevNode; }
	bool IsValid() const { return this && this->NextNode && this->PrevNode; }

protected:
	GenericNodeOfINIEntryPtr* NextNode;
	GenericNodeOfINIEntryPtr* PrevNode;
};

class GenericListOfINIEntryPtr
{
public:
	GenericListOfINIEntryPtr()
	{
		FirstNode.Link(&LastNode);
	}

	GenericListOfINIEntryPtr(GenericListOfINIEntryPtr& list) = default;
	GenericListOfINIEntryPtr& operator = (GenericListOfINIEntryPtr const&) = default;

	virtual ~GenericListOfINIEntryPtr()
	{
		this->UnlinkAll();
	}

	GenericNodeOfINIEntryPtr* First() const { return FirstNode.Next(); }
	GenericNodeOfINIEntryPtr* Last() const { return LastNode.Prev(); }
	GenericNodeOfINIEntryPtr* GetFirst() { return (&FirstNode); };
	GenericNodeOfINIEntryPtr* GetLast() { return (&LastNode); };

	bool IsEmpty() const { return !FirstNode.Next()->IsValid(); }
	void AddHead(GenericNodeOfINIEntryPtr* pNode) { FirstNode.Link(pNode); }
	void AddTail(GenericNodeOfINIEntryPtr* pNode) { LastNode.Prev()->Link(pNode); }
	void Delete() { while (this->FirstNode.Next()->IsValid()) GameDelete(this->FirstNode.Next()); }
	void __forceinline UnlinkAll()
	{
		this->FirstNode.Unlink();
		this->LastNode.Unlink();
	}
protected:
	GenericNodeOfINIEntryPtr FirstNode;
	GenericNodeOfINIEntryPtr LastNode;

};

class ListOfINIEntryPtr : public GenericListOfINIEntryPtr
{
public:

	ListOfINIEntryPtr() = default;
	virtual ~ListOfINIEntryPtr() override { this->UnlinkAll(); }

	INIEntry** First() const { return (INIEntry**)GenericListOfINIEntryPtr::First(); }
	INIEntry** Last() const { return (INIEntry**)GenericListOfINIEntryPtr::Last(); }
};
