#include "CallStack.h"
#include <Utilities/Debug.h>

// Thread-local storage untuk setiap thread punya stack sendiri
thread_local std::unordered_map<DWORD, int> CallStackTracker::CallCounts;
thread_local std::unordered_map<DWORD, std::chrono::steady_clock::time_point> CallStackTracker::CallTimes;
thread_local std::vector<DWORD> CallStackTracker::CallStack;

void CallStackTracker::OnHookEnter(DWORD address, const char* funcName)
{
    // Increment call count
    CallCounts[address]++;
    
    // Record entry time
    CallTimes[address] = std::chrono::steady_clock::now();
    
    // Add to call stack
    CallStack.push_back(address);
    
    // Check for recursive calls
    if (CallCounts[address] > MAX_RECURSIVE_CALLS)
    {
        Debug::FatalErrorAndExit(
            "RECURSIVE CALL DETECTED!\n"
            "Hook: %s (0x%X)\n"
            "Call count: %d\n"
            "This usually indicates an infinite loop in hook logic.",
            funcName, address, CallCounts[address]
        );
    }
    
    // Check for long-running calls (possible deadlock)
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - CallTimes[address]).count();
        
    if (duration > MAX_CALL_TIME_MS)
    {
        Debug::LogWarning(
            "LONG RUNNING HOOK DETECTED!\n"
            "Hook: %s (0x%X)\n"
            "Duration: %lld ms\n"
            "This might indicate a performance issue or deadlock.",
            funcName, address, duration
        );
    }
}

void CallStackTracker::OnHookExit(DWORD address, const char* funcName)
{
    // Decrement call count
    if (CallCounts[address] > 0)
        CallCounts[address]--;
    
    // Remove from call stack
    if (!CallStack.empty() && CallStack.back() == address)
        CallStack.pop_back();
    
    // Clean up if no more calls
    if (CallCounts[address] == 0)
    {
        CallCounts.erase(address);
        CallTimes.erase(address);
    }
}

bool CallStackTracker::IsRecursiveCall(DWORD address)
{
    return CallCounts[address] > 1;
}

void CallStackTracker::ResetCallCount(DWORD address)
{
    CallCounts[address] = 0;
    CallTimes.erase(address);
}

void CallStackTracker::DumpCallStack()
{
    Debug::LogInfo("=== CALL STACK DUMP ===");
    Debug::LogInfo("Current stack depth: %d", CallStack.size());
    
    for (size_t i = 0; i < CallStack.size(); i++)
    {
        DWORD addr = CallStack[i];
        Debug::LogInfo("[%d] 0x%X (calls: %d)", i, addr, CallCounts[addr]);
    }
    
    Debug::LogInfo("=== END CALL STACK ===");
}

int CallStackTracker::GetCallCount(DWORD address)
{
    return CallCounts[address];
} 