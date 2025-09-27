#include "CustomMemoryManager.h"

#include <psapi.h>
#include <tlhelp32.h>

#include <Utilities/Debug.h>

// Static member definitions
uint32_t CustomMemoryManager::MAGIC_SIGNATURE = 0;
uint32_t CustomMemoryManager::MAGIC_SIGNATURE2 = 0;
bool CustomMemoryManager::s_signaturesInitialized = false;
__declspec(thread) char* CustomMemoryManager::s_strtok_next = nullptr;
std::mutex CustomMemoryManager::s_allocMutex;
std::map<void*, void*> CustomMemoryManager::s_userToBase;
std::map<void*, bool> CustomMemoryManager::s_isVirtualAlloc;

// Recreated original game functions - now __cdecl
void* __cdecl CustomMemoryManager::RecreatedHeapAlloc(size_t size)
{
	size_t threshold = CRT::_sbh_threshold();

	if (size <= threshold)
	{
		CRT::_lock(9);
		void* sbhResult = CRT::_sbh_alloc_block((int)size);
		CRT::_unlock(9);

		if (sbhResult)
		{
			return sbhResult;
		}
	}

	if (size == 0) size = 1;
	size_t alignedSize = (size + 15) & 0xFFFFFFF0;

	return HeapAlloc(CRT::Heap(), 0, alignedSize);
}

void* __cdecl CustomMemoryManager::RecreatedNHMalloc(size_t size, int nhFlag)
{
	if (size > 0x7FFFFFFF)
	{
		return nullptr;
	}

	unsigned int adjustedSize = (unsigned int)size;
	void* result = nullptr;

	while (true)
	{
		result = RecreatedHeapAlloc(adjustedSize);

		if (result || !nhFlag)
		{
			break;
		}

		if (!CRT::_callnewh(adjustedSize))
		{
			return nullptr;
		}
	}

	return result;
}

void __cdecl CustomMemoryManager::RecreatedFree(void* ptr)
{
	if (!ptr) return;

	CRT::_lock(9);
	struct tagHeader* block = CRT::_sbh_find_block(ptr);
	if (block)
	{
		CRT::_sbh_free_block(block, ptr);
		CRT::_unlock(9);
	}
	else
	{
		CRT::_unlock(9);
		HeapFree(CRT::Heap(), 0, ptr);
	}
}

void* __cdecl CustomMemoryManager::RecreatedCalloc(size_t num, size_t size)
{
	if (num != 0 && size > SIZE_MAX / num)
	{
		return nullptr;
	}

	size_t totalSize = num * size;
	if (totalSize > 0x7FFFFFFF)
	{
		return nullptr;
	}

	if (totalSize == 0) totalSize = 1;
	size_t alignedSize = (totalSize + 15) & 0xFFFFFFF0;

	void* result = nullptr;
	size_t threshold = CRT::_sbh_threshold();

	do
	{
		if (totalSize <= threshold)
		{
			CRT::_lock(9);
			result = CRT::_sbh_alloc_block((int)totalSize);
			CRT::_unlock(9);

			if (result)
			{
				memset(result, 0, totalSize);
				return result;
			}
		}

		result = HeapAlloc(CRT::Heap(), HEAP_ZERO_MEMORY, alignedSize);

		if (result)
		{
			return result;
		}

		if (!CRT::_callnewh(totalSize))
		{
			return nullptr;
		}

	}
	while (true);
}

void* __cdecl CustomMemoryManager::RecreatedRealloc(void* pBlock, size_t newSize)
{
	if (!pBlock)
	{
		return RecreatedNHMalloc(newSize, 1);
	}

	if (newSize == 0)
	{
		RecreatedFree(pBlock);
		return nullptr;
	}

	if (newSize == 0) newSize = 1;
	size_t alignedSize = (newSize + 15) & 0xFFFFFFF0;

	void* result = HeapReAlloc(CRT::Heap(), 0, pBlock, alignedSize);

	if (!result)
	{
		result = HeapAlloc(CRT::Heap(), 0, alignedSize);
		if (result)
		{
			memcpy(result, pBlock, newSize);
			HeapFree(CRT::Heap(), 0, pBlock);
		}
	}

	return result;
}

size_t __cdecl CustomMemoryManager::RecreatedMSize(void* ptr)
{
	if (!ptr) return 0;

	CRT::_lock(9);
	if (CRT::_sbh_find_block(ptr))
	{
		size_t size = *((size_t*)((char*)ptr - 4)) - 9;
		CRT::_unlock(9);
		return size;
	}
	else
	{
		CRT::_unlock(9);
		return HeapSize(CRT::Heap(), 0, ptr);
	}
}

// Header validation and allocation helpers with ALIGNMENT FIXES
bool CustomMemoryManager::IsValidHeader(AllocHeader* header, void* userPtr)
{
	if (!header || !s_signaturesInitialized) return false;

	// Check magic signatures
	if (header->magic != MAGIC_SIGNATURE) return false;
	if (header->reserved != MAGIC_SIGNATURE2) return false;

	// Sanity check the size
	if (header->size == 0 || header->size > MAX_REASONABLE_SIZE) return false;

	// Verify alignment
	if (!IsAligned(userPtr, MEMORY_ALIGNMENT)) return false;

	return true;
}

void* CustomMemoryManager::AllocateWithHeader(size_t size, bool preferHeap)
{
	InitializeSignatures();

	if (size == 0) size = 1;
	if (size > MAX_REASONABLE_SIZE) return nullptr;

	// Decide allocation strategy
	bool useHeap = preferHeap || (size < SMALL_ALLOC_THRESHOLD);

	if (useHeap)
	{
		// Use heap for small allocations - more efficient and compatible
		size_t totalSize = sizeof(AllocHeader) + size + SAFETY_PADDING;
		totalSize = AlignUp(totalSize, MEMORY_ALIGNMENT);

		void* basePtr = RecreatedNHMalloc(totalSize, 1);
		if (!basePtr) return nullptr;

		// Ensure user pointer will be aligned
		uintptr_t basePtrInt = (uintptr_t)basePtr;
		uintptr_t userPtrInt = basePtrInt + sizeof(AllocHeader);

		// Align user pointer if needed
		if (!IsAligned((void*)userPtrInt, MEMORY_ALIGNMENT))
		{
			userPtrInt = AlignUp(userPtrInt, MEMORY_ALIGNMENT);
		}

		// Place header right before aligned user pointer
		AllocHeader* header = (AllocHeader*)(userPtrInt - sizeof(AllocHeader));
		header->magic = MAGIC_SIGNATURE;
		header->size = (uint32_t)size;
		header->allocSize = (uint32_t)totalSize;
		header->reserved = MAGIC_SIGNATURE2;

		void* userPtr = (void*)userPtrInt;

		// Track allocation
		{
			std::lock_guard<std::mutex> lock(s_allocMutex);
			s_userToBase[userPtr] = basePtr;
			s_isVirtualAlloc[userPtr] = false;
		}

		// Zero safety padding
		memset((char*)userPtr + size, 0, SAFETY_PADDING);

		return userPtr;
	}
	else
	{
		// Use VirtualAlloc for large allocations
		size_t headerSize = sizeof(AllocHeader);
		size_t totalSize = headerSize + size + SAFETY_PADDING;

		// VirtualAlloc is page-aligned, ensure enough space
		totalSize = AlignUp(totalSize, 4096);

		void* basePtr = VirtualAlloc(nullptr, totalSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		if (!basePtr) return nullptr;

		// Setup header
		AllocHeader* header = (AllocHeader*)basePtr;
		header->magic = MAGIC_SIGNATURE;
		header->size = (uint32_t)size;
		header->allocSize = (uint32_t)totalSize;
		header->reserved = MAGIC_SIGNATURE2;

		// User pointer right after header, already aligned due to 16-byte header
		void* userPtr = (char*)basePtr + sizeof(AllocHeader);

		// Track allocation
		{
			std::lock_guard<std::mutex> lock(s_allocMutex);
			s_userToBase[userPtr] = basePtr;
			s_isVirtualAlloc[userPtr] = true;
		}

		// Zero safety padding
		memset((char*)userPtr + size, 0, SAFETY_PADDING);

		return userPtr;
	}
}

AllocHeader* CustomMemoryManager::GetHeader(void* ptr)
{
	if (!ptr || !s_signaturesInitialized) return nullptr;

	// Find the header location from our tracking
	{
		std::lock_guard<std::mutex> lock(s_allocMutex);
		if (s_userToBase.find(ptr) == s_userToBase.end())
		{
			return nullptr;  // Not our allocation
		}
	}

	AllocHeader* header = (AllocHeader*)((char*)ptr - sizeof(AllocHeader));
	return IsValidHeader(header, ptr) ? header : nullptr;
}

void* CustomMemoryManager::GetBasePtr(void* userPtr)
{
	std::lock_guard<std::mutex> lock(s_allocMutex);
	auto it = s_userToBase.find(userPtr);
	return (it != s_userToBase.end()) ? it->second : nullptr;
}

// Public memory allocation interface with FIXES
void* CustomMemoryManager::Malloc(size_t size, int nhFlag)
{
	// Prefer heap for small allocations
	void* ptr = AllocateWithHeader(size, size < SMALL_ALLOC_THRESHOLD);

	if (ptr)
	{
		return ptr;
	}

	// Our allocation failed - try new handler and fallback to recreated original
	if (nhFlag)
	{
		if (CRT::_callnewh(size))
		{
			ptr = AllocateWithHeader(size, size < SMALL_ALLOC_THRESHOLD);
			if (ptr) return ptr;
		}
	}

	// Still failed - use recreated original nh_malloc logic
	Debug::LogDeferred("Custom allocation failed for size %u, using recreated nh_malloc\n", (unsigned int)size);
	return RecreatedNHMalloc(size, nhFlag);
}

void CustomMemoryManager::Free(void* ptr)
{
	if (!ptr) return;

	void* basePtr = nullptr;
	bool isVirtual = false;

	// Look up allocation info
	{
		std::lock_guard<std::mutex> lock(s_allocMutex);
		auto it = s_userToBase.find(ptr);
		if (it != s_userToBase.end())
		{
			basePtr = it->second;
			isVirtual = s_isVirtualAlloc[ptr];
			s_userToBase.erase(it);
			s_isVirtualAlloc.erase(ptr);
		}
	}

	if (basePtr)
	{
		// Our allocation - free appropriately
		if (isVirtual)
		{
			VirtualFree(basePtr, 0, MEM_RELEASE);
		}
		else
		{
			RecreatedFree(basePtr);
		}
	}
	else
	{
		// Not our allocation - use recreated original free logic
		Debug::LogDeferred("Freeing non-custom allocation %p using recreated free\n", ptr);
		RecreatedFree(ptr);
	}
}

void* CustomMemoryManager::Realloc(void* pBlock, size_t newSize)
{
	// If pBlock is null, act like malloc
	if (!pBlock)
	{
		return Malloc(newSize, 1);
	}

	// If newSize is 0, act like free and return null
	if (newSize == 0)
	{
		Free(pBlock);
		return nullptr;
	}

	AllocHeader* header = GetHeader(pBlock);
	if (!header)
	{
		// Not our allocation - use recreated original realloc logic
		Debug::LogDeferred("Reallocating non-custom allocation %p using recreated realloc\n", pBlock);
		return RecreatedRealloc(pBlock, newSize);
	}

	size_t oldSize = header->size;

	// If new size is same or smaller, just update size and return same pointer
	if (newSize <= oldSize)
	{
		header->size = (uint32_t)newSize;
		return pBlock;
	}

	// Need to allocate new block
	void* newPtr = Malloc(newSize, 1);
	if (newPtr)
	{
		// Copy old data and free old block
		memcpy(newPtr, pBlock, oldSize);
		Free(pBlock);
		return newPtr;
	}

	// Our allocation failed - use recreated original realloc logic
	Debug::LogDeferred("Custom realloc failed for size %u, using recreated realloc\n", (unsigned int)newSize);
	return RecreatedRealloc(pBlock, newSize);
}

void* CustomMemoryManager::Calloc(size_t num, size_t size)
{
	// Check for overflow
	if (num != 0 && size > SIZE_MAX / num)
	{
		return nullptr;
	}

	size_t totalSize = num * size;

	// Check for known CRT allocation sizes - bypass custom allocator
	static const size_t crtSizes[] = { 0x74, 0x214, 0x58, 0x428, 0x10 };
	for (size_t crtSize : crtSizes)
	{
		if (totalSize == crtSize)
		{
			Debug::LogDeferred("Bypassing CRT allocation of size 0x%X\n", (unsigned int)totalSize);
			return RecreatedCalloc(num, size);
		}
	}

	// Prefer heap for small allocations
	void* ptr = AllocateWithHeader(totalSize, totalSize < SMALL_ALLOC_THRESHOLD);

	if (ptr)
	{
		// Our allocation succeeded - zero the memory
		memset(ptr, 0, totalSize);
		return ptr;
	}

	// Our allocation failed - use recreated original calloc logic
	Debug::LogDeferred("Custom calloc failed for %u x %u, using recreated calloc\n", (unsigned int)num, (unsigned int)size);
	return RecreatedCalloc(num, size);
}

size_t CustomMemoryManager::MSize(void* ptr)
{
	if (!ptr) return 0;

	AllocHeader* header = GetHeader(ptr);
	if (header)
	{
		// Our allocation - return size from header
		return header->size;
	}
	else
	{
		// Not our allocation - use recreated original _msize logic
		return RecreatedMSize(ptr);
	}
}

char* CustomMemoryManager::StrDup(char* str)
{
	if (!str) return nullptr;

	int len = 0;

	while (str[len])
	{
		len++;
	}
	char*  c_str = (char*)RecreatedNHMalloc(len + 1, 1);
	char*  p = c_str;
	while (*str)
	{
		*p++ = *str++;
	}
	*p = '\0';
	return c_str;
}

char* CustomMemoryManager::StrTok(char* str, const char* delims)
{
	char delim_table[32]; // 256 bits = 32 bytes
	memset(delim_table, 0, sizeof(delim_table));

	// Build delimiter lookup table
	const char* d = delims;
	while (*d)
	{
		unsigned char c = *d;
		delim_table[c >> 3] |= (1 << (c & 7));
		d++;
	}

	char* current;
	if (str)
	{
		// First call - use provided string
		current = str;
		s_strtok_next = str;
	}
	else
	{
		// Subsequent call - use saved position
		current = s_strtok_next;
	}

	if (!current) return nullptr;

	// Skip leading delimiters
	while (*current && (delim_table[*current >> 3] & (1 << (*current & 7))))
	{
		current++;
	}

	if (!*current)
	{
		s_strtok_next = nullptr;
		return nullptr;
	}

	char* token_start = current;

	// Find end of token
	while (*current && !(delim_table[*current >> 3] & (1 << (*current & 7))))
	{
		current++;
	}

	if (*current)
	{
		// Found delimiter - null terminate and advance
		*current = '\0';
		s_strtok_next = current + 1;
	}
	else
	{
		// End of string
		s_strtok_next = nullptr;
	}

	return token_start;
}

// Utility functions
bool CustomMemoryManager::IsOurAllocation(void* ptr)
{
	if (!ptr) return false;

	std::lock_guard<std::mutex> lock(s_allocMutex);
	return s_userToBase.find(ptr) != s_userToBase.end();
}

size_t CustomMemoryManager::GetAllocationSize(void* ptr)
{
	AllocHeader* header = GetHeader(ptr);
	return header ? header->size : 0;
}

void CustomMemoryManager::GetMemoryStats(size_t* totalAllocated, size_t* allocationCount)
{
	std::lock_guard<std::mutex> lock(s_allocMutex);

	size_t total = 0;
	size_t count = s_userToBase.size();

	for (const auto& pair : s_userToBase)
	{
		AllocHeader* header = (AllocHeader*)((char*)pair.first - sizeof(AllocHeader));
		if (header->magic == MAGIC_SIGNATURE && header->reserved == MAGIC_SIGNATURE2)
		{
			total += header->size;
		}
	}

	if (totalAllocated) *totalAllocated = total;
	if (allocationCount) *allocationCount = count;
}

// Signature scanning implementation (keeping original)
//std::vector<CustomMemoryManager::ModuleSignatureInfo> CustomMemoryManager::ScanModulesAlternative()
//{
//	// ... keeping your original implementation ...
//	std::vector<ModuleSignatureInfo> moduleInfos;
//
//	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, GetCurrentProcessId());
//	if (hSnapshot == INVALID_HANDLE_VALUE)
//	{
//		Debug::LogDeferred("Error: CreateToolhelp32Snapshot failed (Error: %lu)\n", GetLastError());
//		return moduleInfos;
//	}
//
//	MODULEENTRY32 me32 = {};
//	me32.dwSize = sizeof(MODULEENTRY32);
//
//	if (Module32First(hSnapshot, &me32))
//	{
//		do
//		{
//			ModuleSignatureInfo moduleInfo;
//			moduleInfo.moduleHandle = me32.hModule;
//			moduleInfo.moduleName = me32.szModule;
//			moduleInfo.moduleSize = me32.modBaseSize;
//
//			// Scan the module
//			__try
//			{
//				BYTE* baseAddr = me32.modBaseAddr;
//				SIZE_T imageSize = me32.modBaseSize;
//
//				for (SIZE_T j = 0; j < imageSize - 4; j += 4)
//				{
//					uint32_t value = *(uint32_t*)(baseAddr + j);
//
//					if (value == 0x00000000 || value == 0xFFFFFFFF ||
//						value == 0xCCCCCCCC || value == 0xCDCDCDCD)
//					{
//						continue;
//					}
//
//					moduleInfo.signatures[value]++;
//				}
//			}
//			__except (EXCEPTION_EXECUTE_HANDLER)
//			{
//				Debug::LogDeferred("Warning: Access violation scanning module %s (alternative method)\n", moduleInfo.moduleName.c_str());
//			}
//
//			moduleInfos.push_back(moduleInfo);
//
//		}
//		while (Module32Next(hSnapshot, &me32));
//	}
//	else
//	{
//		Debug::LogDeferred("Error: Module32First failed (Error: %lu)\n", GetLastError());
//	}
//
//	CloseHandle(hSnapshot);
//	return moduleInfos;
//}
//
//std::vector<CustomMemoryManager::ModuleSignatureInfo> CustomMemoryManager::ScanAllModulesForSignatures()
//{
//	// ... keeping your original implementation ...
//	std::vector<ModuleSignatureInfo> moduleInfos;
//
//	HANDLE hProcess = GetCurrentProcess();
//	HMODULE hModules[1024];
//	DWORD cbNeeded;
//
//	if (EnumProcessModules(hProcess, hModules, sizeof(hModules), &cbNeeded))
//	{
//		DWORD moduleCount = cbNeeded / sizeof(HMODULE);
//
//		for (DWORD i = 0; i < moduleCount; i++)
//		{
//			ModuleSignatureInfo moduleInfo;
//			moduleInfo.moduleHandle = hModules[i];
//
//			char modulePath[MAX_PATH];
//			if (GetModuleFileNameA(hModules[i], modulePath, MAX_PATH))
//			{
//				const char* moduleName = strrchr(modulePath, '\\');
//				moduleInfo.moduleName = moduleName ? (moduleName + 1) : modulePath;
//			}
//			else
//			{
//				moduleInfo.moduleName = "Unknown";
//			}
//
//			MODULEINFO modInfo = {};
//			if (GetModuleInformation(hProcess, hModules[i], &modInfo, sizeof(MODULEINFO)))
//			{
//				BYTE* baseAddr = (BYTE*)modInfo.lpBaseOfDll;
//				SIZE_T imageSize = modInfo.SizeOfImage;
//				moduleInfo.moduleSize = imageSize;
//
//				__try
//				{
//					for (SIZE_T j = 0; j < imageSize - 4; j += 4)
//					{
//						uint32_t value = *(uint32_t*)(baseAddr + j);
//
//						if (value == 0x00000000 || value == 0xFFFFFFFF ||
//							value == 0xCCCCCCCC || value == 0xCDCDCDCD)
//						{
//							continue;
//						}
//
//						moduleInfo.signatures[value]++;
//					}
//				}
//				__except (EXCEPTION_EXECUTE_HANDLER)
//				{
//					Debug::LogDeferred("Warning: Access violation scanning module %s\n", moduleInfo.moduleName.c_str());
//				}
//			}
//			else
//			{
//				MEMORY_BASIC_INFORMATION mbi;
//				if (VirtualQuery(hModules[i], &mbi, sizeof(mbi)))
//				{
//					moduleInfo.moduleSize = mbi.RegionSize;
//					Debug::LogDeferred("Warning: GetModuleInformation failed for %s, using VirtualQuery\n", moduleInfo.moduleName.c_str());
//				}
//				else
//				{
//					Debug::LogDeferred("Warning: Could not get size for module %s\n", moduleInfo.moduleName.c_str());
//					moduleInfo.moduleSize = 0;
//				}
//			}
//
//			moduleInfos.push_back(moduleInfo);
//		}
//	}
//	else
//	{
//		Debug::LogDeferred("Error: EnumProcessModules failed (Error: %lu), trying alternative method\n", GetLastError());
//		return ScanModulesAlternative();
//	}
//
//	return moduleInfos;
//}
//
//std::set<uint32_t> CustomMemoryManager::GetAllConflictingSignatures()
//{
//	std::set<uint32_t> conflictingSignatures;
//	auto moduleInfos = ScanAllModulesForSignatures();
//
//	Debug::LogDeferred("=== Signature Scan Results ===\n");
//
//	for (const auto& moduleInfo : moduleInfos)
//	{
//		Debug::LogDeferred("Module: %s (Size: %u bytes)",
//			   moduleInfo.moduleName.c_str(), (unsigned int)moduleInfo.moduleSize);
//
//		std::vector<std::pair<uint32_t, int>> frequentSignatures;
//		for (const auto& sigPair : moduleInfo.signatures)
//		{
//			if (sigPair.second >= 3)
//			{
//				frequentSignatures.push_back(sigPair);
//				conflictingSignatures.insert(sigPair.first);
//			}
//		}
//
//		std::sort(frequentSignatures.begin(), frequentSignatures.end(),
//				 [](const auto& a, const auto& b) { return a.second > b.second; });
//
//		int shown = 0;
//		for (const auto& sigPair : frequentSignatures)
//		{
//			char extraInfo[64] = "";
//
//			if (sigPair.first == 0xDEADBEEF) strcpy(extraInfo, " (DEADBEEF - common debug)\n");
//			else if (sigPair.first == 0xCAFEBABE) strcpy(extraInfo, " (CAFEBABE - Java/common)\n");
//			else if (sigPair.first == 0xFEEDFACE) strcpy(extraInfo, " (FEEDFACE - Mach-O)\n");
//			else if (sigPair.first == 0xBAADF00D) strcpy(extraInfo, " (BADFOOD - MS debug)\n");
//			else if (sigPair.first == 0xFDFDFDFD) strcpy(extraInfo, " (FDFDFDFD - freed memory)\n");
//
//			Debug::LogDeferred("  0x%08X: %d occurrences%s", sigPair.first, sigPair.second, extraInfo);
//
//			if (++shown >= 10) break;
//		}
//
//		if (frequentSignatures.empty())
//		{
//			Debug::LogDeferred("  No frequent signatures found\n");
//		}
//	}
//
//	Debug::LogDeferred("Total conflicting signatures found: %u", (unsigned int)conflictingSignatures.size());
//	return conflictingSignatures;
//}
//
//std::pair<uint32_t, uint32_t> CustomMemoryManager::GenerateSafeSignatures()
//{
//	Debug::LogDeferred("Generating safe signatures...\n");
//
//	std::set<uint32_t> conflicting = GetAllConflictingSignatures();
//
//	HMODULE gameModule = GetModuleHandle(nullptr);
//	uint32_t baseAddr = (uint32_t)gameModule;
//	uint32_t timestamp = (uint32_t)time(nullptr);
//	uint32_t pid = GetCurrentProcessId();
//
//	uint32_t signature1, signature2;
//	int attempts = 0;
//
//	do
//	{
//		signature1 = HashCombine(baseAddr, timestamp, pid + attempts);
//		signature2 = HashCombine(baseAddr ^ 0x12345678, timestamp ^ 0x87654321, pid ^ 0xABCDEF01);
//
//		if (signature1 == 0x00000000 || signature1 == 0xFFFFFFFF ||
//			signature2 == 0x00000000 || signature2 == 0xFFFFFFFF ||
//			signature1 == signature2)
//		{
//			attempts++;
//			continue;
//		}
//
//		attempts++;
//
//	}
//	while ((conflicting.find(signature1) != conflicting.end() ||
//		conflicting.find(signature2) != conflicting.end()) &&
//		  attempts < 1000);
//
//	if (attempts >= 1000)
//	{
//		Debug::LogDeferred("Warning: Could not generate completely safe signatures after 1000 attempts\n");
//		Debug::LogDeferred("Using fallback signatures...\n");
//		signature1 = 0x4D454D41;  // "MEMA"
//		signature2 = 0x4D454D42;  // "MEMB"
//	}
//
//	Debug::LogDeferred("Generated signatures: 0x%08X, 0x%08X (attempts: %d)",
//		   signature1, signature2, attempts);
//
//	bool sig1_safe = conflicting.find(signature1) == conflicting.end();
//	bool sig2_safe = conflicting.find(signature2) == conflicting.end();
//
//	Debug::LogDeferred("Signature safety: 0x%08X=%s, 0x%08X=%s",
//		   signature1, sig1_safe ? "SAFE" : "CONFLICT",
//		   signature2, sig2_safe ? "SAFE" : "CONFLICT\n");
//
//	return { signature1, signature2 };
//}

uint32_t CustomMemoryManager::HashCombine(uint32_t a, uint32_t b, uint32_t c)
{
	uint32_t result = a;
	result ^= b + 0x9e3779b9 + (result << 6) + (result >> 2);
	result ^= c + 0x9e3779b9 + (result << 6) + (result >> 2);
	return result;
}

void CustomMemoryManager::InitializeSignatures()
{
	if (s_signaturesInitialized) return;

	Debug::LogDeferred("Initializing CustomMemoryManager signatures...\n");

	// Use simple signatures for early init
	MAGIC_SIGNATURE = 0x4D454D41;   // "MEMA"
	MAGIC_SIGNATURE2 = 0x4D454D42;  // "MEMB"
	s_signaturesInitialized = true;

	Debug::LogDeferred("CustomMemoryManager initialized with signatures: 0x%08X, 0x%08X\n",
		   MAGIC_SIGNATURE, MAGIC_SIGNATURE2);
}

//bool CustomMemoryManager::TestSignatureCollisions()
//{
//	Debug::LogDeferred("Testing for runtime signature collisions...\n");
//
//	std::vector<void*> testPtrs;
//
//	for (int i = 0; i < 200; i++)
//	{
//		size_t size = 32 + (i % 256);
//		void* ptr = malloc(size);
//		if (ptr)
//		{
//			testPtrs.push_back(ptr);
//		}
//	}
//
//	int collisions = 0;
//	for (void* ptr : testPtrs)
//	{
//		for (int offset = -32; offset <= 0; offset += 4)
//		{
//			uint32_t* checkAddr = (uint32_t*)((char*)ptr + offset);
//
//			__try
//			{
//				if (*checkAddr == MAGIC_SIGNATURE || *checkAddr == MAGIC_SIGNATURE2)
//				{
//					collisions++;
//					Debug::LogDeferred("COLLISION: Found signature 0x%08X at %p (offset %d from %p)\n",
//						   *checkAddr, checkAddr, offset, ptr);
//				}
//			}
//			__except (EXCEPTION_EXECUTE_HANDLER)
//			{
//				// Access violation - skip
//			}
//		}
//	}
//
//	for (void* ptr : testPtrs)
//	{
//		free(ptr);
//	}
//
//	Debug::LogDeferred("Runtime collision test complete: %d collisions found\n", collisions);
//	return collisions == 0;
//}

// Public interface for signature scanning and testing
//bool CustomMemoryManager::RunSignatureSafetyCheck()
//{
//	Debug::LogDeferred("=== CustomMemoryManager Signature Safety Check ===\n");
//
//	InitializeSignatures();
//
//	bool safe = TestSignatureCollisions();
//
//	Debug::LogDeferred("=== Signature Safety Check Complete ===\n");
//	Debug::LogDeferred("Result: %s", safe ? "SAFE" : "POTENTIAL CONFLICTS DETECTED\n");
//
//	return safe;
//}

void CustomMemoryManager::GetSignatureInfo(uint32_t* signature1, uint32_t* signature2)
{
	InitializeSignatures();
	if (signature1) *signature1 = MAGIC_SIGNATURE;
	if (signature2) *signature2 = MAGIC_SIGNATURE2;
}

void CustomMemoryManager::RegenerateSignatures()
{
	s_signaturesInitialized = false;
	InitializeSignatures();
}