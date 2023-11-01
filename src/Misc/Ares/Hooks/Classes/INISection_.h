#pragma once
#include "INIComment.h"
#include "INIEntry_.h"

class GenericListOfINISection;
class GenericNodeOfINISection
{
public:
	GenericNodeOfINISection() : NextNode(nullptr), PrevNode(nullptr) { }
	virtual ~GenericNodeOfINISection() { Unlink(); }
	GenericNodeOfINISection(GenericNodeOfINISection& node) { node.Link(this); }
	GenericNodeOfINISection& operator = (GenericNodeOfINISection& node)
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

	GenericListOfINISection* MainList() const
	{
		GenericNodeOfINISection const* node = this;

		while (node->PrevNode)
			node = this->PrevNode;

		return (GenericListOfINISection*)this;
	}

	void Link(GenericNodeOfINISection* pNode)
	{
		pNode->Unlink();
		pNode->NextNode = this->NextNode;
		pNode->PrevNode = this;
		if (this->NextNode) this->NextNode->PrevNode = pNode;
		this->NextNode = pNode;
	}

	GenericNodeOfINISection* Next() const { return this->NextNode; }
	GenericNodeOfINISection* Prev() const { return this->PrevNode; }
	bool IsValid() const { return this && this->NextNode && this->PrevNode; }

protected:
	GenericNodeOfINISection* NextNode;
	GenericNodeOfINISection* PrevNode;
};

class GenericListOfINISection
{
public:
	GenericListOfINISection()
	{
		FirstNode.Link(&LastNode);
	}

	GenericListOfINISection(GenericListOfINISection& list) = default;
	GenericListOfINISection& operator = (GenericListOfINISection const&) = default;

	virtual ~GenericListOfINISection()
	{
		this->UnlinkAll();
	}

	GenericNodeOfINISection* First() const { return FirstNode.Next(); }
	GenericNodeOfINISection* Last() const { return LastNode.Prev(); }
	GenericNodeOfINISection* GetFirst() { return (&FirstNode); };
	GenericNodeOfINISection* GetLast() { return (&LastNode); };

	bool IsEmpty() const { return !FirstNode.Next()->IsValid(); }
	void AddHead(GenericNodeOfINISection* pNode) { FirstNode.Link(pNode); }
	void AddTail(GenericNodeOfINISection* pNode) { LastNode.Prev()->Link(pNode); }
	void Delete() { while (this->FirstNode.Next()->IsValid()) GameDelete(this->FirstNode.Next()); }
	void __forceinline UnlinkAll()
	{
		this->FirstNode.Unlink();
		this->LastNode.Unlink();
	}
protected:
	GenericNodeOfINISection FirstNode;
	GenericNodeOfINISection LastNode;

};

class INISection;
class ListOfINISection : public GenericListOfINISection
{
public:

	ListOfINISection() = default;
	virtual ~ListOfINISection() override {
		JMP_THIS(0x40E4C0);
	}

	INISection* First() const { return (INISection*)GenericListOfINISection::First(); }
	INISection* Last() const { return (INISection*)GenericListOfINISection::Last(); }
};

class NodeOfINISection : public GenericNodeOfINISection
{
public:
	ListOfINISection* MainList() const { return (ListOfINISection*)GenericNodeOfINISection::MainList(); }
	INISection* Next() const { return (INISection*)GenericNodeOfINISection::Next(); }
	INISection* Prev() const { return (INISection*)GenericNodeOfINISection::Prev(); }
	bool IsValid() const { return GenericNodeOfINISection::IsValid(); }
};

class INISection : public NodeOfINISection
{
public:

	char* Name;
	ListOfINIEntryPtr Entries;
	IndexClassOfINIEntry EntryIndex;
	INIComment* Comments;

	//DTOR 
	void DeallocINISection() {
		JMP_THIS(0x52AB80);
	}

	void VectorDealloc(char args) {
		JMP_THIS(0x52AE00);
	}
};

static_assert(sizeof(INISection) == 0x44, "Invalid size.");

struct NodeElementOfINISection
{
	NodeElementOfINISection& operator=(const NodeElementOfINISection& node) { ID = node.ID; Data = node.Data; return *this; }
	bool operator<(const NodeElementOfINISection& another) const { return ID < another.ID; }
	bool operator==(const NodeElementOfINISection& another) const { return ID == another.ID; }

	int ID;
	INISection* Data;
};

class IndexClassOfINISection
{
public:
	// ranged for support
	NodeElementOfINISection* begin() const { return IndexTable; }
	NodeElementOfINISection* end() const { return IndexTable + IndexCount; }
	NodeElementOfINISection* begin() { return IndexTable; }
	NodeElementOfINISection* end() { return IndexTable + IndexCount; }

public:
	IndexClassOfINISection() :
		IndexTable(0),
		IndexCount(0),
		IndexSize(0),
		IsSorted(false),
		Archive(0)
	{
	}

	~IndexClassOfINISection()
	{
		this->Clear();
	}

	bool IsPresent(int id, bool Lowerbound = false) const
	{
		if (!this->IndexCount)
			return false;

		if (this->IsArchiveSame(id))
			return true;

		if (NodeElementOfINISection const* nodeptr = SearchForNode(id, Lowerbound))
		{
			const_cast<IndexClassOfINISection*>(this)->SetArchive(nodeptr);
			return true;
		}

		return false;
	}

	int Count() const
	{
		return this->IndexCount;
	}

	NodeElementOfINISection* FetchItem(int id, bool Lowerbound) const
	{
		if (!this->IsArchiveSame(id))
		{
			if (NodeElementOfINISection const* nodeptr = const_cast<IndexClassOfINISection*>(this)->SearchForNode(id, Lowerbound))
			{
				const_cast<IndexClassOfINISection*>(this)->SetArchive(nodeptr);
				return this->Archive;
			}

			return (NodeElementOfINISection*)nullptr;
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
		const NodeElementOfINISection* n1 = static_cast<const NodeElementOfINISection*>(ptr);
		const NodeElementOfINISection* n2 = static_cast<const NodeElementOfINISection*>(ptr2);

		if (n1->ID == n2->ID)
			return 0;
		if (n1->ID < n2->ID)
			return -1;

		return 1;
	}

	NodeElementOfINISection* IndexTable;
	int IndexCount;
	int IndexSize;
	bool IsSorted;
	PROTECTED_PROPERTY(char, padding[3]);
	NodeElementOfINISection* Archive;

private:

	bool IsArchiveSame(int id) const
	{
		return this->Archive != 0 && this->Archive->ID == id;
	}

	void InvalidateArchive()
	{
		this->Archive = nullptr;
	}

	void SetArchive(NodeElementOfINISection const* pNode)
	{
		this->Archive = const_cast<NodeElementOfINISection*>(pNode);
	}

	NodeElementOfINISection const* SearchForNode(int id, bool Lowerbound = false) const
	{
		if (!this->IndexCount)
			return 0;

		const_cast<IndexClassOfINISection*>(this)->Sort();

		NodeElementOfINISection seatch { id , nullptr };

		if (Lowerbound)
		{
			const NodeElementOfINISection* find = static_cast<const NodeElementOfINISection*>(std::lower_bound(begin(), end(), seatch));
			return (find == end() || id < find->ID) ? nullptr : find;
		}

		return static_cast<const NodeElementOfINISection*>(std::bsearch(&seatch, &IndexTable[0], IndexCount, sizeof(IndexTable[0]), Comparator));
	}
};
