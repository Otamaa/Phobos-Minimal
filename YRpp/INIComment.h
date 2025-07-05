#pragma once

struct INIComment
{
	char* Value;
	INIComment* Next;
};
static_assert(sizeof(INIComment) == 0x8, "Invalid size.");
