#pragma once
#include <Phobos.version.h>

#pragma region Release build version numbering

// Indicates project maturity and completeness
#define KRATOS_VERSION_MAJOR 0

// Indicates major changes and significant additions, like new logics
#define KRATOS_VERSION_MINOR 1

// Indicates minor changes, like vanilla bugfixes, unhardcodings or hacks
#define KRATOS_VERSION_REVISION 13

// Indicates Kratos-related bugfixes only
#define KRATOS_VERSION_PATCH 0

#pragma endregion

// Build number. Incremented on each released build.
#define KRATOS_BUILD_NUMBER 1

// version infomation
#define KRATOS_PRODUCT_NAME "Kratos"
#define KRATOS_COMPANY_NAME "ChrisLv_CN (https://space.bilibili.com/276838)"
#define KRATOS_LEGAL_COPYRIGHT "© The ChrisLv_CN 🐼 Contributors 2023"
#define KRATOS_FILE_DESCRIPTION "Kratos, Ares-like YR engine extension"
#define KRATOS_FILE_VERSION_STR _STR(VERSION_MAJOR) "." _STR(VERSION_MINOR) "." _STR(VERSION_REVISION) "." _STR(VERSION_PATCH)
#define KRATOS_FILE_VERSION VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION, VERSION_PATCH
#define KRATOS_SAVEGAME_ID ((VERSION_MAJOR << 24) | (VERSION_MINOR << 16) | (VERSION_REVISION << 8) | VERSION_PATCH)
#define KRATOS_PRODUCT_VERSION _STR(VERSION_MAJOR) "." _STR(VERSION_MINOR)
#define KRATOS_VERSION_SHORT_STR "Ver." _STR(VERSION_MAJOR) "." _STR(VERSION_MINOR) "." _STR(VERSION_REVISION)
#define KRATOS_VERSION_SHORT_WSTR L"Ver." _WSTR(VERSION_MAJOR) L"." _WSTR(VERSION_MINOR) L"." _WSTR(VERSION_REVISION)

