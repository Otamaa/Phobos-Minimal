#pragma once
#include <GenericList.h>

struct INIComment;
class INIEntry : public Node<INIEntry>
{
public:
	using AsNode = Node<INIEntry>;

public:
	char* Key;
	char* Value;
	INIComment* Comments;
	char* CommentString;
	int PreIndentCursor;
	int PostIndentCursor;
	int CommentCursor;

	virtual ~INIEntry() JMP_THIS(0x52AAF0);

	INIEntry(
		char* entry,
		char* value, 
		INIComment* comments,
		char* commentstring, 
		int preindentcursor, 
		int postindentcursor,
		int commentcursor) :
		Key(entry) , Value(value) ,
		Comments(comments) , CommentString(commentstring) ,
		PreIndentCursor(preindentcursor) , PostIndentCursor(postindentcursor) ,
		CommentCursor(commentcursor)
	{}

	INIEntry() :
		Key(),
		Value(),
		Comments(),
		CommentString(),
		PreIndentCursor(), 
		PostIndentCursor(),
		CommentCursor()
	{}

private:
	INIEntry(INIEntry const& rvalue) = delete;
	INIEntry operator=(INIEntry const& that) = delete;
};
static_assert(sizeof(INIEntry) == 0x28, "Invalid size.");