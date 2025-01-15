#pragma once

#include <Base/Always.h>
#include <wchar.h>
#include <Helpers/CompileTime.h>
#include <ASMMacros.h>
// contains functions that are part of the C runtime library and have been declared ingame
// just declaring them so we don't need to include our own duplicates

typedef unsigned int BITVEC;

struct tagListHead
{
	struct tagEntry
	{
		int sizeFront;
		struct tagEntry* pEntryNext;
		struct tagEntry* pEntryPrev;
	};

	tagEntry* pEntryNext;
	tagEntry* pEntryPrev;
};

struct tagGroup
{
	int cntEntries;
	tagListHead listHead[64];
};

struct tagRegion
{
	int indGroupUse;
	char cntRegionSize[64];
	BITVEC bitvGroupHi[32];
	BITVEC bitvGroupLo[32];
	tagGroup grpHeadList[32];
};


struct tagHeader
{
	BITVEC bitvEntryHi;
	BITVEC bitvEntryLo;
	BITVEC bitvCommit;
	void* pHeapData;
	tagRegion* pRegion;
};

typedef struct tagHeader* PHEADER;

namespace CRT
{
//	NO_CONSTRUCT_CLASS(CRT)
//public:

	static COMPILETIMEEVAL reference<LPCRITICAL_SECTION, 0x87C2A8u> const Critical_Sections {};
	static COMPILETIMEEVAL reference<LPCRITICAL_SECTION, 0x87C2ECu> const _87C2EC_Critical_Sections {};
	static COMPILETIMEEVAL reference<LPCRITICAL_SECTION, 0x87C2DCu> const _87C2DC_Critical_Sections {};
	static COMPILETIMEEVAL reference<LPCRITICAL_SECTION, 0x87C2CCu> const _87C2CC_Critical_Sections {};
	static COMPILETIMEEVAL reference<LPCRITICAL_SECTION, 0x87C2ACu> const _87C2AC_Critical_Sections {};

	static COMPILETIMEEVAL reference<HANDLE, 0xB78B9Cu> const Heap {};
	static COMPILETIMEEVAL reference<volatile LONG, 0xB78BA4u> const _unguarded_readlc_active {};

	static COMPILETIMEEVAL reference<int, 0xB782C4u> const AllocatorMode {};

	// unicode manipulations - "wcs" stands for "wide char string" or wchar_t equivalent of "str"

	static OPTIONALINLINE NAKED int __cdecl wcsncmp(const wchar_t* a1, const wchar_t* a2, size_t a3)
	{ JMP(0x7CB4CC); }

	static  OPTIONALINLINE NAKED wchar_t* __cdecl wcsstr(const wchar_t*, const wchar_t*)
	{ JMP(0x7CC682); }

	static  OPTIONALINLINE NAKED wchar_t* __cdecl wcscpy(wchar_t* Dest, const wchar_t* Src)
	{ JMP(0x7CA489); }

	static  OPTIONALINLINE NAKED wchar_t* __cdecl wcsncpy(wchar_t* Dest, const wchar_t* Source, size_t Count)
	{ JMP(0x7CA422); }

	static  OPTIONALINLINE NAKED wchar_t* __cdecl wcsrchr(const wchar_t* Str, wchar_t Ch)
	{ JMP(0x7CA3C5); }

	static  OPTIONALINLINE NAKED size_t __cdecl wcslen(const wchar_t* Str)
	{ JMP(0x7CA405); }

	static  OPTIONALINLINE NAKED size_t __cdecl wcscspn(const wchar_t* pFirst, const wchar_t* pSecond)
	{ JMP(0x7CD7CE); }

	static  OPTIONALINLINE NAKED wchar_t* __cdecl wcscat(wchar_t* Dest, const wchar_t* Source)
	{ JMP(0x7CA45F); }

	static  OPTIONALINLINE NAKED int __cdecl wcscmp(const wchar_t* Str1, const wchar_t* Str2)
	{ JMP(0x7CA5D3); }

	static  OPTIONALINLINE NAKED int __cdecl swprintf(wchar_t* Buffer, const wchar_t* Format, ...)
	{ JMP(0x7CA564); }

	static OPTIONALINLINE wchar_t* __fastcall wcstrim(wchar_t* Buffer)
	{ JMP_STD(0x727D60); }

	static OPTIONALINLINE NAKED wchar_t* __cdecl wcschr(const wchar_t* Str, wchar_t a2)
	{ JMP(0x7CA8C6); }

	static  OPTIONALINLINE NAKED wchar_t* __cdecl wcsncat(wchar_t* a1, const wchar_t* a2, size_t a3)
	{ JMP(0x7CB504); }

	static  OPTIONALINLINE NAKED int __cdecl wcsicmp(const wchar_t* a1, const wchar_t* a2)
	{ JMP(0x7DD0F8); }

	static  OPTIONALINLINE NAKED void __cdecl exit_noreturn(size_t reason)
	{ JMP(0x7CBDDC); }

	static  OPTIONALINLINE NAKED int __cdecl exit_returnsomething(UINT uExitCode, int a2, int a3)
	{ JMP(0x7CBE1C); }

	// memory management
	static  OPTIONALINLINE NAKED void* __cdecl malloc(size_t sz)
	{ JMP(0x7C9430); }

	static  OPTIONALINLINE NAKED void* __cdecl malloc(size_t sz, int mode)
	{ JMP(0x7C9442); }

	static  OPTIONALINLINE NAKED void __cdecl free(const void* p)
	{ JMP(0x7C93E8); }

	static  OPTIONALINLINE NAKED void* __cdecl realloc(void* pBlock, size_t sz)
	{ JMP(0x7D0F45); }

	static  OPTIONALINLINE NAKED void* __cdecl _calloc(size_t Count, size_t Size)
	{ JMP(0x7D3374); }

	static  OPTIONALINLINE NAKED void* __cdecl _new(size_t sz)
	{ JMP(0x7C8E17); }

	static  OPTIONALINLINE NAKED void __cdecl _delete(void* p)
	{ JMP(0x7C8B3D); }

	static  OPTIONALINLINE NAKED void* __cdecl _memset(void* p, int nInt, size_t sz)
	{ JMP(0x7D75E0); }

	static  OPTIONALINLINE NAKED void* __cdecl _memmove(void* dst, const void* src, size_t count)
	{ JMP(0x7CA090); }

	// strings
	static  OPTIONALINLINE NAKED int __cdecl atoi(const char* Str)
	{ JMP(0x7C9BFD); }

	static  OPTIONALINLINE NAKED long int __cdecl atol(const char* a1)
	{ JMP(0x7C9B72); }

	static  OPTIONALINLINE NAKED double __cdecl atof(const char* Str)
	{ JMP(0x7C9D66); }

	static  OPTIONALINLINE NAKED int __cdecl isspace(char* Str)
	{ JMP(0x7C99E1); }

	static  OPTIONALINLINE NAKED char* __cdecl strdup(const char* Src)
	{ JMP(0x7D5408); }

	static  OPTIONALINLINE NAKED char* __cdecl strcats(char* StrTo, char* StrFrom)
	{ JMP(0x7D4C00); }

	static  OPTIONALINLINE NAKED char* __cdecl strcat(char* StrTo, const char* StrFrom)
	{ JMP(0x7D4C00); }

	static  OPTIONALINLINE NAKED char* __cdecl strcpy(char* StrTo, const char* StrFrom)
	{ JMP(0x7D4BF0); }

	static  OPTIONALINLINE NAKED int __cdecl strcmpi(const char* lhs, const char* rhs)
	{ JMP(0x7C8D20); }

	static  OPTIONALINLINE NAKED int __cdecl strcmp(const char* lhs, const char* rhs)
	{ JMP(0x7CDA90); }

	static  OPTIONALINLINE NAKED char* __cdecl strchr(const char* Str, int Val)
	{ JMP(0x7CAF30); }

	static  OPTIONALINLINE NAKED char* __cdecl strrchr(const char* Str, int Ch)
	{ JMP(0x7C8DF0); }

	static  OPTIONALINLINE NAKED char* __cdecl strncpy(char* Dest, const char* Source, size_t Count)
	{ JMP(0x7C91D0); }

	static  OPTIONALINLINE NAKED char* __cdecl strstr(const char* Str, const char* SubStr)
	{ JMP(0x7CA4B0); }

	static  OPTIONALINLINE NAKED char* __cdecl strupr(char* pInput)
	{ JMP(0x7DCFC4); }

	static  OPTIONALINLINE NAKED int __cdecl sscanf(const char*, const char*, ...)
	{ JMP(0x7CA530); }

	static  OPTIONALINLINE NAKED int __cdecl _strnicmp(const char* a1, const char* a2, size_t a3)
	{ JMP(0x7CD680); }

	static  OPTIONALINLINE NAKED char* __cdecl strncat(char* Dest, const char* Source, size_t Count)
	{ JMP(0x7CB550); }

	static  OPTIONALINLINE NAKED char* __cdecl strtok(char* Str, const char* Delim)
	{ JMP(0x7C9CC2); }

	static  char* __cdecl strtok(char* Str, const char* Delim, char** ctx)
	{ return strtok_s(Str, Delim, ctx); }

	static  OPTIONALINLINE NAKED int __cdecl sprintf(char* Buffer, const char* Format, ...)
	{ JMP(0x7C8EF4); }

	static  OPTIONALINLINE NAKED int __cdecl vsprintf(char*, const char*, va_list)
	{ JMP(0x7CB7BA); }

	static  OPTIONALINLINE char* __fastcall strtrim(char* Buffer)
	{ JMP_STD(0x727CF0); }

	static  OPTIONALINLINE NAKED size_t __cdecl strlen(const char* input)
	{ JMP(0x7D15A0); }

	static  OPTIONALINLINE NAKED size_t __cdecl strcspn(const char* pFirst, const char* pSecond)
	{ JMP(0x7CD790); }

	// misc
	static  OPTIONALINLINE NAKED void* __cdecl memcpy(void* Dst, const void* Src, size_t Size)
	{ JMP(0x7CA090); }

	static  OPTIONALINLINE NAKED void* __cdecl memcpy_B(void* Dst, const void* Src, size_t Size)
	{ JMP(0x7D0A20); }

	static  OPTIONALINLINE NAKED void __cdecl qsort(void* buf, size_t num, size_t size, int(__cdecl* compare)(const void* lhs, const void* rhs))
	{ JMP(0x7C8B48); }

	static  OPTIONALINLINE NAKED void* __cdecl bsearch(const void*, const void*, size_t, size_t, int(__cdecl*)(const void*, const void*))
	{ JMP(0x7C8E25); }

	static  OPTIONALINLINE NAKED size_t __cdecl msize(void* nBlock)
	{ JMP(0x7D107D); }

	static  OPTIONALINLINE NAKED void __cdecl setfpmode()
	{ JMP(0x7C5EE4); }

	static  OPTIONALINLINE NAKED size_t __cdecl mbstowcs(wchar_t* lpWideCharStr, const char* lpMultiByteStr, size_t a3)
	{ JMP(0x7CC2AC); }

	// files
	static  OPTIONALINLINE NAKED FILE* __cdecl fopen(const char*, const char*)
	{ JMP(0x7CA845); }

	static  OPTIONALINLINE NAKED size_t __cdecl fread(void*, size_t, size_t, FILE*)
	{ JMP(0x7C94EB); }

	static  OPTIONALINLINE NAKED size_t __cdecl fwrite(const void*, size_t, size_t, FILE*)
	{ JMP(0x7C9602); }

	static  OPTIONALINLINE NAKED int __cdecl fprintf(FILE*, const char*, ...)
	{ JMP(0x7CA7D8); }

	static  OPTIONALINLINE NAKED int __cdecl vfprintf(FILE* File, const char* Format, va_list ArgList)
	{ JMP(0x7CB302); }

	static  OPTIONALINLINE NAKED int __cdecl fflush(FILE*)
	{ JMP(0x7CB19C); }

	static  OPTIONALINLINE NAKED int __cdecl fclose(FILE*)
	{ JMP(0x7CA75B); }

	static  OPTIONALINLINE NAKED void __cdecl _makepath(char* path, const char* drive, const char* dir, const char* fname, const char* ext)
	{ JMP(0x7C9FF0); }

	//Critical sections
	static  OPTIONALINLINE NAKED void __cdecl _lock(int ntime)
	{ JMP(0x7CD9F5); }

	static  OPTIONALINLINE NAKED void __cdecl _unlock(int ntime = 0)
	{ JMP(0x7CDA56); }

	static  OPTIONALINLINE NAKED PHEADER __cdecl _sbh_find_block(void* ptr)
	{ JMP(0x7CF7BD); }

	//
	static  OPTIONALINLINE NAKED int __cdecl isalpha(int c)
	{ JMP(0x7C990E); }

	static  OPTIONALINLINE NAKED int __cdecl toupper(int c)
	{ JMP(0x7C97D3); }

	static  OPTIONALINLINE NAKED int __cdecl tolower(int a1)
	{ JMP(0x7CAFF4); }

	static  OPTIONALINLINE NAKED char* __cdecl  itoa(int a1, char* a2, int a3)
	{ JMP(0x7D468C); }
}
