#include "INIEntry.h"
#include <INIComment.h>

 INIEntry::~INIEntry()
{
	YRMemory::free(this->Key);
	YRMemory::free(this->Value);

	if (this->Comments) {
		do {
			YRMemory::free(this->Comments->Value);
			INIComment* next = this->Comments->Next;
			YRMemory::free(this->Comments);
			this->Comments = next;
		}
		while (this->Comments);
	}


	YRMemory::free(this->CommentString);
	this->AsNode::~AsNode();
}