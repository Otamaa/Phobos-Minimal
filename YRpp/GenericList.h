//General linked list class

#pragma once

#include <Memory.h>

class GenericList;
class GenericNode
{
public:
	GenericNode() : NextNode(nullptr), PrevNode(nullptr) { }
	GenericNode(GenericNode& node);
	virtual ~GenericNode();

	GenericNode& operator = (GenericNode& node);


	GenericList* MainList() const;
	GenericNode* Next() const { return this->NextNode; }
	GenericNode* NextValid() const { return (NextNode != nullptr && NextNode->NextNode != nullptr) ? NextNode : nullptr; }
	GenericNode* Prev() const { return this->PrevNode; }
	GenericNode* PrevValid() const { return (PrevNode != nullptr && PrevNode->PrevNode != nullptr) ? PrevNode : nullptr; }

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
	GenericList(GenericList& list) = delete;
	GenericList& operator = (GenericList const&) = default;

	virtual ~GenericList();

	GenericNode* First() const { return FirstNode.Next(); }
	GenericNode* FirstValid() const
	{
		GenericNode* node = FirstNode.Next();
		return (node->Next() ? node : nullptr);
	}
	GenericNode* Last() const { return LastNode.Prev(); }
	GenericNode* LastValid() const
	{
		GenericNode* node = LastNode.Prev();
		return (node->Prev() ? node : nullptr);
	}
	GenericNode* GetFirst() { return (&FirstNode); };
	GenericNode* GetLast() { return (&LastNode); };

	bool IsEmpty() const { return !FirstNode.Next()->IsValid(); }
	void AddHead(GenericNode* pNode) { FirstNode.Link(pNode); }
	void AddTail(GenericNode* pNode) { LastNode.Prev()->Link(pNode); }
	void Delete() { while (this->FirstNode.Next()->IsValid()) GameDelete(this->FirstNode.Next()); }
	void UnlinkAll();

	int Get_Valid_Count() const;

protected:
	GenericNode FirstNode;
	GenericNode LastNode;
};

template<class T> class List;
template<class T>
class Node : public GenericNode
{
public:
	List<T>* MainList() const { return reinterpret_cast<List<T>*>(GenericNode::MainList()); }
	T Next() const { return reinterpret_cast<T>(GenericNode::Next()); }
	T NextValid() const { return reinterpret_cast<T>(GenericNode::NextValid()); }
	T Prev() const { return reinterpret_cast<T>(GenericNode::Prev()); }
	T PrevValid() const { return reinterpret_cast<T>(GenericNode::PrevValid()); }
	bool Is_Valid() const { return GenericNode::IsValid(); }

};

template<class T>
class List : public GenericList
{
private:
	List(const List<T>&) = delete;
	List<T> operator=(const List<T>&) = delete;

public:
	List() {}

	T First() const { return reinterpret_cast<T>(GenericList::First()); }
	T FirstValid() const { return reinterpret_cast<T>(GenericList::FirstValid()); }
	T Last() const { return reinterpret_cast<T>(GenericList::Last()); }
	T LastValid() const { return reinterpret_cast<T>(GenericList::LastValid()); }

	void Delete()
	{
		while (First()->IsValid())
		{
			delete First();
		}
	}
};

template<class T>
class DataNode : public GenericNode
{
public:
	DataNode() {}
	DataNode(T value) { Set(value); }

	void Set(T value) { Value = value; }
	T Get() const { return Value; }

	DataNode<T>* Next() const { return reinterpret_cast<DataNode<T>*>(GenericNode::Next()); }
	DataNode<T>* NextValid() const { return reinterpret_cast<DataNode<T>*>(GenericNode::NextValid()); }
	DataNode<T>* Prev() const { return reinterpret_cast<DataNode<T>*>(GenericNode::Prev()); }
	DataNode<T>* PrevValid() const { return reinterpret_cast<DataNode<T>*>(GenericNode::PrevValid()); }

private:
	T Value;
};