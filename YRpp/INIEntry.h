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
};
static_assert(sizeof(INIEntry) == 0x28, "Invalid size.");