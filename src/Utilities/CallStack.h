#pragma once

#include <unordered_map>
#include <chrono>
#include <string>
#include <vector>

class CallStackTracker
{
private:
    static thread_local std::unordered_map<DWORD, int> CallCounts;
    static thread_local std::unordered_map<DWORD, std::chrono::steady_clock::time_point> CallTimes;
    static thread_local std::vector<DWORD> CallStack;
    
    static constexpr int MAX_RECURSIVE_CALLS = 1000;
    static constexpr int MAX_CALL_TIME_MS = 5000;

public:
    static void OnHookEnter(DWORD address, const char* funcName);
    static void OnHookExit(DWORD address, const char* funcName);
    
    static bool IsRecursiveCall(DWORD address);
    static void ResetCallCount(DWORD address);
    
    // Untuk debugging
    static void DumpCallStack();
    static int GetCallCount(DWORD address);
};

// Macro untuk auto-inject ke semua hooks
#define AUTO_RECURSIVE_GUARD(address, funcName) \
    CallStackTracker::OnHookEnter(address, funcName); \
    struct AutoGuard { \
        DWORD addr; \
        const char* name; \
        AutoGuard(DWORD a, const char* n) : addr(a), name(n) {} \
        ~AutoGuard() { CallStackTracker::OnHookExit(addr, name); } \
    } __guard(address, funcName); 