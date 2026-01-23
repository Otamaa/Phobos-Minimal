#pragma once

#include <Phobos.h>
#include <map>
#include <set>
#include <atomic>
#include <mutex>

//// Configuration constants
//#define MEMORY_ALIGNMENT 16          // Ensure 16-byte alignment for SSE/optimized code
//#define SAFETY_PADDING 32            // Safety padding at end of allocations
//#define SMALL_ALLOC_THRESHOLD 4096   // Use heap for allocations < 4KB
//
//// Properly aligned header structure - exactly 16 bytes
//#pragma pack(push, 1)
//struct AllocHeader
//{
//	uint32_t magic;       // 4 bytes
//	uint32_t size;        // 4 bytes - actual requested size
//	uint32_t allocSize;   // 4 bytes - total allocation size including padding
//	uint32_t reserved;    // 4 bytes - second magic signature
//};
//#pragma pack(pop)
//
//static_assert(sizeof(AllocHeader) == 16, "AllocHeader must be exactly 16 bytes");
//
//class CustomMemoryManager
//{
//private:
//	static uint32_t MAGIC_SIGNATURE;
//	static uint32_t MAGIC_SIGNATURE2;
//	static bool s_signaturesInitialized;
//	static constexpr size_t MAX_REASONABLE_SIZE = 0x10000000; // 256MB limit
//
//	// Thread-local storage for strtok
//	static __declspec(thread) char* s_strtok_next;
//
//	// Allocation tracking for proper cleanup
//	static std::mutex s_allocMutex;
//	static std::map<void*, void*> s_userToBase;  // Maps user ptr -> actual allocation base
//	static std::map<void*, bool> s_isVirtualAlloc;  // Track allocation type
//
//	struct ModuleSignatureInfo
//	{
//		std::string moduleName;
//		HMODULE moduleHandle;
//		std::map<uint32_t, int> signatures;
//		size_t moduleSize;
//	};
//
//	// Alignment helper functions
//	static constexpr inline size_t AlignUp(size_t size, size_t alignment)
//	{
//		return (size + alignment - 1) & ~(alignment - 1);
//	}
//
//	static constexpr inline bool IsAligned(void* ptr, size_t alignment)
//	{
//		return ((uintptr_t)ptr & (alignment - 1)) == 0;
//	}
//
//	// Internal helper functions
//	static bool IsValidHeader(AllocHeader* header, void* userPtr);
//	static void* AllocateWithHeader(size_t size, bool preferHeap = false);
//	static AllocHeader* GetHeader(void* ptr);
//	static void* GetBasePtr(void* userPtr);
//
//	// Signature scanning functions
//	static std::vector<ModuleSignatureInfo> ScanAllModulesForSignatures();
//	static std::vector<ModuleSignatureInfo> ScanModulesAlternative();
//	static std::set<uint32_t> GetAllConflictingSignatures();
//	static std::pair<uint32_t, uint32_t> GenerateSafeSignatures();
//	static uint32_t HashCombine(uint32_t a, uint32_t b, uint32_t c);
//	static void InitializeSignatures();
//	static bool TestSignatureCollisions();
//
//public:
//	// Recreated original game functions - made __cdecl for easier use
//	static void* __cdecl RecreatedHeapAlloc(size_t size);
//	static void* __cdecl RecreatedNHMalloc(size_t size, int nhFlag);
//	static void __cdecl RecreatedFree(void* ptr);
//	static void* __cdecl RecreatedCalloc(size_t num, size_t size);
//	static void* __cdecl RecreatedRealloc(void* pBlock, size_t newSize);
//	static size_t __cdecl RecreatedMSize(void* ptr);
//
//	// Memory allocation interface
//	static void* __cdecl Malloc(size_t size, int nhFlag);
//	static void __cdecl Free(void* ptr);
//	static void* __cdecl Realloc(void* pBlock, size_t newSize);
//	static void* __cdecl Calloc(size_t num, size_t size);
//	static size_t __cdecl MSize(void* ptr);
//	static char* __cdecl StrDup(char* str);
//	static char* __cdecl StrTok(char* str, const char* delims);
//
//	// Utility functions
//	static bool IsOurAllocation(void* ptr);
//	static size_t GetAllocationSize(void* ptr);
//	static void GetMemoryStats(size_t* totalAllocated, size_t* allocationCount);
//
//	// Public interface for signature scanning and testing
//	static bool RunSignatureSafetyCheck();
//	static void GetSignatureInfo(uint32_t* signature1, uint32_t* signature2);
//	static void RegenerateSignatures();
//};
//
//// Initialization functions
//void InitializeCustomMemorySystem();