#pragma once

#include "Intrinsics.h"
#include "Macros.h"
#include <inttypes.h>

#include <windows.h>
#define NAME_MAX FILENAME_MAX

#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif


/**
 *  Enable inline recursion.
 */
#pragma inline_recursion(on)
#pragma inline_depth(255) // Allow lots of inlining.

 /**
  *  Alias the ICU unicode functions when not building against it.
  */
#define u_strlen wcslen
#define u_strcpy wcscpy
#define u_strcat wcscat
#define u_vsnprintf_u vswprintf
#define u_strcmp wcscmp
#define u_strcasecmp(x, y, z) _wcsicmp(x, y)
#define u_isspace iswspace
#define u_tolower towlower
#define U_COMPARE_CODE_POINT_ORDER 0x8000


  /**
   *  Define some stuff here for cross platform consistency.
   */
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
