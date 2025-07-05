#include "GenericList.h"
#include <INISection.h>
#include <INIEntry.h>

GenericNode::~GenericNode() { this->Unlink(); }
GenericNode::GenericNode(GenericNode& node) { node.Link(this); }

GenericNode& GenericNode::operator = (GenericNode& node)
{
	if (&node != this)
		node.Link(this);

	return *this;
}

GenericList* GenericNode::MainList() const
{
	GenericNode const* node = this;

	while (node->PrevNode)
		node = this->PrevNode;

	return (GenericList*)this;
}

void GenericNode::Link(GenericNode* pNode)
{
	pNode->Unlink();
	pNode->NextNode = this->NextNode;
	pNode->PrevNode = this;
	if (this->NextNode) this->NextNode->PrevNode = pNode;
	this->NextNode = pNode;
}

void GenericNode::Unlink()
{
	if (this->IsValid())
	{
		this->PrevNode->NextNode = this->NextNode;
		this->NextNode->PrevNode = this->PrevNode;
		this->PrevNode = nullptr;
		this->NextNode = nullptr;
	}
}

GenericList::GenericList()
{
	FirstNode.Link(&LastNode);
}

GenericList::~GenericList()
{
	this->UnlinkAll();
}

void GenericList::UnlinkAll()
{
	this->FirstNode.Unlink();
	this->LastNode.Unlink();
}

template class Node<INISection>;
template class List<INISection>;
template class List<INIEntry*>;