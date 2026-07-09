#include "INISection.h"

#include <CRC.h>
#include <INIComment.h>

INIEntry* INISection::FindEntry(const char* entry) const
{
	if (!entry) {
		return nullptr;
	}

	const int crc = CRCEngine()(entry, std::strlen(entry));

	// IsPresent: checks Archive cache, sorts if needed, binary-searches,
	// sets Archive on hit. VERIFY: confirm YRpp's IsPresent sets Archive
	// before returning — the asm shows Archive written before the final
	// return, so this assumption should hold.
	if (!this->EntryIndex.IsPresent(crc)) {
		return nullptr;
	}

	return this->EntryIndex.Archive->Data;
}

//INISection::~INISection()
//{
//	YRMemory::free(this->Name);
//	this->Entries.List<INIEntry*>::~List<INIEntry*>();
//	this->EntryIndex.EntryIndexType::~EntryIndexType();
//	if (this->Comments) {
//
//		do {
//			YRMemory::free(this->Comments->Value);
//			INIComment* next = this->Comments->Next;
//			YRMemory::free(this->Comments);
//			this->Comments = next;
//		}
//		while (this->Comments);
//	}
//
//	this->AsNode::~AsNode();
//}