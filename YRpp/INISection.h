#pragma once

#include <GenericList.h>
#include <IndexClass.h>

class INIEntry;
struct INIComment;
class INISection : public Node<INISection>
{
public:
	using EntryIndexType = IndexClass<int, INIEntry*>;
	using AsNode = Node<INISection>;

public:

	char* Name;
	List<INIEntry*> Entries;
	EntryIndexType EntryIndex;
	INIComment* Comments;

public:

	INISection() :Name(), Entries(), EntryIndex(), Comments()
	{}

	virtual ~INISection() JMP_THIS(0x52AE00);

	INIEntry* FindEntry(const char* entry) const;
private:
	INISection(INISection const& rvalue) = delete;
	INISection operator=(INISection const& that) = delete;
};
static_assert(sizeof(INISection) == 0x44, "Invalid size.");