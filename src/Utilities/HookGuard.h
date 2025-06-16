#pragma once

#include "CallStack.h"

// Wrapper untuk recursive call detection di hooks
// Include file ini di hook files yang butuh AUTO_RECURSIVE_GUARD

#ifndef AUTO_RECURSIVE_GUARD
#define AUTO_RECURSIVE_GUARD(address, funcName) \
    CallStackTracker::OnHookEnter(address, funcName); \
    struct AutoGuard { \
        DWORD addr; \
        const char* name; \
        AutoGuard(DWORD a, const char* n) : addr(a), name(n) {} \
        ~AutoGuard() { CallStackTracker::OnHookExit(addr, name); } \
    } __guard(address, funcName);
#endif 