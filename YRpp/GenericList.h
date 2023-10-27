//General linked list class

#pragma once

#include <Memory.h>

class GenericList;
class GenericNode
{
public:
	GenericNode() : NextNode(nullptr), PrevNode(nullptr) { }
	virtual ~GenericNode() { Unlink(); }
	GenericNode(GenericNode& node) { node.Link(this); }
	GenericNode& operator = (GenericNode& node)
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

	GenericList* MainList() const
	{
		GenericNode const* node = this;

		while (node->PrevNode)
			node = this->PrevNode;

		return (GenericList*)this;
	}
	void Link(GenericNode* pNode)
	{
		pNode->Unlink();
		pNode->NextNode = this->NextNode;
		pNode->PrevNode = this;
		if (this->NextNode) this->NextNode->PrevNode = pNode;
		this->NextNode = pNode;
	}

	GenericNode* Next() const { return this->NextNode; }
	GenericNode* Prev() const { return this->PrevNode; }
	bool IsValid() const { return this && this->NextNode && this->PrevNode; }

protected:
	GenericNode* NextNode;
	GenericNode* PrevNode;
};

class GenericList
{
public:
	GenericList()
	{
		FirstNode.Link(&LastNode);
	}

	virtual ~GenericList() {
		this->UnlinkAll();
	}

	GenericNode* First() const { return FirstNode.Next(); }
	GenericNode* Last() const { return LastNode.Prev(); }
	GenericNode* GetFirst() { return (&FirstNode); };
	GenericNode* GetLast() { return (&LastNode); };

	bool IsEmpty() const { return !FirstNode.Next()->IsValid(); }
	void AddHead(GenericNode* pNode) { FirstNode.Link(pNode); }
	void AddTail(GenericNode* pNode) { LastNode.Prev()->Link(pNode); }
	void Delete() { while (this->FirstNode.Next()->IsValid()) GameDelete(this->FirstNode.Next()); }
	void __forceinline UnlinkAll() {
		this->FirstNode.Unlink();
		this->LastNode.Unlink();
	}
protected:
	GenericNode FirstNode;
	GenericNode LastNode;

private:
	GenericList(GenericList& list);
	GenericList& operator = (GenericList const&) = delete;
};

template<class T> class List;
template<class T>
class Node : public GenericNode
{
public:
	List<T>* MainList() const { return (List<T> *)GenericNode::MainList(); }
	T* Next() const { return (T*)GenericNode::Next(); }
	T* Prev() const { return (T*)GenericNode::Prev(); }
	bool IsValid() const { return GenericNode::IsValid(); }
};

template<class T>
class List : public GenericList
{
public:

	virtual ~List<T>() override { this->UnlinkAll(); }

	T* First() const { return (T*)GenericList::First(); }
	T* Last() const { return (T*)GenericList::Last(); }
};