#pragma once

#include "ASMMacros.h"

struct Base64
{
	static int __fastcall Encode(const void* source, int slen, void* dest, int dlen)
	{ JMP_STD(0x42FD30); }

	static int __fastcall Decode(const void* source, int slen, void* dest, int dlen)
	{ JMP_STD(0x42FE50); }
};
