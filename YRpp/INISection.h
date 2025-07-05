#pragma once

#include <GenericList.h>
#include <IndexClass.h>

class INIEntry;
struct INIComment;
class INISection : public Node<INISection>
{
public:
	using EntryIndexType = IndexClass<int, INIEntry*>;

	char* Name;
	List<INIEntry*> Entries;
	EntryIndexType EntryIndex;
	INIComment* Comments;

	void DeallocINISection()
	{
		JMP_THIS(0x52AB80);
	}

	void VectorDealloc(char args)
	{
		JMP_THIS(0x52AE00);
	}

	virtual ~INISection() = default;
	//INISection() = delete; //TODO
};
static_assert(sizeof(INISection) == 0x44, "Invalid size.");