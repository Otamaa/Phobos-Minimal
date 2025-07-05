//General linked list class

#pragma once

#include <Memory.h>

class GenericList;
class GenericNode
{
public:
	GenericNode() : NextNode(nullptr), PrevNode(nullptr) { }
	virtual ~GenericNode();
	GenericNode(GenericNode& node);
	GenericNode& operator = (GenericNode& node);


	GenericList* MainList() const;
	GenericNode* Next() const { return this->NextNode; }
	GenericNode* Prev() const { return this->PrevNode; }

	bool IsValid() const { return this && this->NextNode && this->PrevNode; }
	void Link(GenericNode* pNode);
	void Unlink();

protected:
	GenericNode* NextNode;
	GenericNode* PrevNode;
};

class GenericList
{
public:
	GenericList();
	GenericList(GenericList& list) = default;
	GenericList& operator = (GenericList const&) = default;

	virtual ~GenericList();

	GenericNode* First() const { return FirstNode.Next(); }
	GenericNode* Last() const { return LastNode.Prev(); }
	GenericNode* GetFirst() { return (&FirstNode); };
	GenericNode* GetLast() { return (&LastNode); };

	bool IsEmpty() const { return !FirstNode.Next()->IsValid(); }
	void AddHead(GenericNode* pNode) { FirstNode.Link(pNode); }
	void AddTail(GenericNode* pNode) { LastNode.Prev()->Link(pNode); }
	void Delete() { while (this->FirstNode.Next()->IsValid()) GameDelete(this->FirstNode.Next()); }
	void UnlinkAll();

protected:
	GenericNode FirstNode;
	GenericNode LastNode;
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

	virtual ~Node() override;
};

template<typename T>
Node<T>::~Node()
{
	this->Unlink();
}

template<class T>
class List : public GenericList
{
public:

	virtual ~List() override;

	T* First() const { return (T*)GenericList::First(); }
	T* Last() const { return (T*)GenericList::Last(); }
};

template<typename T>
List<T>::~List()
{
	this->UnlinkAll();
}