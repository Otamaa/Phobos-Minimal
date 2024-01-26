#ifndef VERSION_H
#define VERSION_H

#define _WSTR(x) _WSTR_(x)
#define _WSTR_(x) L ## #x
#define _STR(x) _STR_(x)
#define _STR_(x) #x

#pragma region Release build version numbering

// Indicates project maturity and completeness
#define VERSION_MAJOR 0

// Indicates major changes and significant additions, like new logics
#define VERSION_MINOR 100

// Indicates minor changes, like vanilla bugfixes, unhardcodings or hacks
#define VERSION_REVISION 50

// Indicates Phobos-related bugfixes only
#define VERSION_PATCH 150

#pragma endregion

// Build number. Incremented on each released build.
#define BUILD_NUMBER 28

#define PHOBOSSAVEGAME_ID ((BUILD_NUMBER << 24) | (BUILD_NUMBER << 12) | (BUILD_NUMBER))
#define FILE_DESCRIPTION "Unofficial Development build of Phobos - Minimal engine extension"
#define FILE_VERSION_STR "Unofficial Build #" _STR(BUILD_NUMBER)
#define FILE_VERSION 0,0,0,BUILD_NUMBER
#define PRODUCT_VERSION "Unofficial Development Build #" _STR(BUILD_NUMBER)

#endif // VERSION_H
