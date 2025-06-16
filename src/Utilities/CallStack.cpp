#include "CallStack.h"
#include <Utilities/Debug.h>

// Thread-local storage using __declspec(thread) for MSVC compatibility
__declspec(thread) std::unordered_map<DWORD, int>* CallStackTracker::CallCounts = nullptr;
__declspec(thread) std::unordered_map<DWORD, std::chrono::steady_clock::time_point>* CallStackTracker::CallTimes = nullptr;
__declspec(thread) std::vector<DWORD>* CallStackTracker::CallStack = nullptr;

void CallStackTracker::InitializeThreadLocal()
{
    if (!CallCounts) CallCounts = new std::unordered_map<DWORD, int>();
    if (!CallTimes) CallTimes = new std::unordered_map<DWORD, std::chrono::steady_clock::time_point>();
    if (!CallStack) CallStack = new std::vector<DWORD>();
}

void CallStackTracker::CleanupThreadLocal()
{
    delete CallCounts; CallCounts = nullptr;
    delete CallTimes; CallTimes = nullptr;
    delete CallStack; CallStack = nullptr;
}

void CallStackTracker::OnHookEnter(DWORD address, const char* funcName)
{
    InitializeThreadLocal();
    
    // Increment call count
    (*CallCounts)[address]++;
    
    // Record entry time
    (*CallTimes)[address] = std::chrono::steady_clock::now();
    
    // Add to call stack
    CallStack->push_back(address);
    
    // Check for recursive calls
    if ((*CallCounts)[address] > MAX_RECURSIVE_CALLS)
    {
        Debug::FatalErrorAndExit(
            "RECURSIVE CALL DETECTED!\n"
            "Hook: %s (0x%X)\n"
            "Call count: %d\n"
            "This usually indicates an infinite loop in hook logic.",
            funcName, address, (*CallCounts)[address]
        );
    }
    
    // Check for long-running calls (possible deadlock)
    auto now = std::chrono::steady_clock::now();
    auto it = CallTimes->find(address);
    if (it != CallTimes->end())
    {
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - it->second).count();
            
        if (duration > MAX_CALL_TIME_MS)
        {
            Debug::Log(
                "LONG RUNNING HOOK DETECTED!\n"
                "Hook: %s (0x%X)\n"
                "Duration: %lld ms\n"
                "This might indicate a performance issue or deadlock.",
                funcName, address, duration
            );
        }
    }
}

void CallStackTracker::OnHookExit(DWORD address, const char* funcName)
{
    if (!CallCounts) return;
    
    // Decrement call count
    if ((*CallCounts)[address] > 0)
        (*CallCounts)[address]--;
    
    // Remove from call stack
    if (!CallStack->empty() && CallStack->back() == address)
        CallStack->pop_back();
    
    // Clean up if no more calls
    if ((*CallCounts)[address] == 0)
    {
        CallCounts->erase(address);
        CallTimes->erase(address);
    }
}

bool CallStackTracker::IsRecursiveCall(DWORD address)
{
    if (!CallCounts) return false;
    return (*CallCounts)[address] > 1;
}

void CallStackTracker::ResetCallCount(DWORD address)
{
    if (!CallCounts) return;
    (*CallCounts)[address] = 0;
    CallTimes->erase(address);
}

void CallStackTracker::DumpCallStack()
{
    if (!CallStack) return;
    
    Debug::Log("=== CALL STACK DUMP ===");
    Debug::Log("Current stack depth: %d", CallStack->size());
    
    for (size_t i = 0; i < CallStack->size(); i++)
    {
        DWORD addr = (*CallStack)[i];
        Debug::Log("[%d] 0x%X (calls: %d)", i, addr, (*CallCounts)[addr]);
    }
    
    Debug::Log("=== END CALL STACK ===");
}

int CallStackTracker::GetCallCount(DWORD address)
{
    if (!CallCounts) return 0;
    return (*CallCounts)[address];
} 