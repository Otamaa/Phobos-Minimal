// This file is only provided for compatibility and may be removed in future
// versions. Use fmt/base.h if you don't need fmt::format and fmt/format.h
// otherwise.

#define FMT_ASSERT(condition, message) ((void)0)
#define FMT_NO_UNREACHABLE_WARNING
#if !FMT_EXCEPTIONS && defined(FMT_NO_UNREACHABLE_WARNING)
#  define FMT_UNREACHABLE() ((void)0)
#else
#  define FMT_UNREACHABLE() FMT_ASSERT(false, "unreachable")
#endif
#pragma warning(push)
#pragma warning(disable : 4702)  // Unreachable code
#include "format.h"
#pragma warning(pop)
