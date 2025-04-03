 #pragma once

#include <Base/Always.h>

//to avoid including unnessesary header

#ifndef NANOPRINTF_IMPLEMENTATION
#define IMPL_SNPRNINTF _snprintf_s
#else
#include <ExtraHeaders/nanoprintf.h>
#define IMPL_SNPRNINTF npf_snprintf
#endif

#define IMPL_STRCMPI(a ,b) _strcmpi(a,b)
#define IMPL_STRCMP(a ,b) strcmp(a,b)
#define IMPL_WSTRCMPI(a , b) _wcsicmp(a , b)

#define IS_SAME_STR_(a ,b) (IMPL_STRCMPI(a,b) == 0)
#define IS_SAME_STR_N(a ,b) (IMPL_STRCMP(a,b) == 0)
#define IS_SAME_STR_I(a,b) ( _stricmp(a,b) == 0)
#define IS_SAME_WSTR(a,b) (IMPL_WSTRCMPI(a,b) == 0)

COMPILETIMEEVAL OPTIONALINLINE const char* NULL_STR { "null" };
COMPILETIMEEVAL OPTIONALINLINE const char* NULL_STR2 { "<null>" };
COMPILETIMEEVAL OPTIONALINLINE const char* DEFAULT_STR { "default" };
COMPILETIMEEVAL OPTIONALINLINE const char* DEFAULT_STR2 { "<default>" };
COMPILETIMEEVAL OPTIONALINLINE const char* PHOBOS_STR { "Phobos" };

COMPILETIMEEVAL OPTIONALINLINE const char* GLOBALCONTROLS_SECTION { "GlobalControls" };
COMPILETIMEEVAL OPTIONALINLINE const char* SIDEBAR_SECTION_T { "Sidebar" };
COMPILETIMEEVAL OPTIONALINLINE auto UISETTINGS_SECTION { "UISettings" };

COMPILETIMEEVAL OPTIONALINLINE const wchar_t* ARES_DLL { L"Ares.dll" };
COMPILETIMEEVAL OPTIONALINLINE const char* ARES_DLL_S { "Ares.dll" };
COMPILETIMEEVAL OPTIONALINLINE const wchar_t* PHOBOS_DLL { L"Phobos.dll" };
COMPILETIMEEVAL OPTIONALINLINE const char* PHOBOS_DLL_S { "Phobos.dll" };
COMPILETIMEEVAL OPTIONALINLINE const wchar_t* GAMEMD_EXE { L"gamemd.exe" };
COMPILETIMEEVAL OPTIONALINLINE const char* UIMD_ { "uimd.ini" };
