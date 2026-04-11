#pragma once
#include <GenericList.h>

struct INIComment;
class INIEntry : public Node<INIEntry>
{
public:
	char* Key;
	char* Value;
	INIComment* Comments;
	char* CommentString;
	int PreIndentCursor;
	int PostIndentCursor;
	int CommentCursor;

	virtual ~INIEntry() = default;

private:
	INIEntry(INIEntry const& rvalue) = delete;
	INIEntry operator=(INIEntry const& that) = delete;
};
static_assert(sizeof(INIEntry) == 0x28, "Invalid size.");