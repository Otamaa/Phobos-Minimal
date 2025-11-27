

#ifdef CheckForMapSaveCrash
ASMJIT_PATCH(0x50126A, HouseClass_WritetoIni0, 0x6)
{
	GET(HouseClass*, pThis, EBX);

	if (IS_SAME_STR_("USSR", pThis->Type->ID))
		Debug::LogInfo("Writing to ini TechLevel for[%s]", pThis->Type->ID);

	return 0x0;
}

ASMJIT_PATCH(0x501284, HouseClass_WritetoIni2, 0x6)
{
	GET(HouseClass*, pThis, EBX);

	if (IS_SAME_STR_("USSR", pThis->Type->ID))
		Debug::LogInfo("Writing to ini InitialCredit for[%s]", pThis->Type->ID);

	return 0x0;
}

ASMJIT_PATCH(0x5012AF, HouseClass_WritetoIni3, 0x6)
{
	GET(HouseClass*, pThis, EBX);

	if (IS_SAME_STR_("USSR", pThis->Type->ID))
		Debug::LogInfo("Writing to ini0 Control_IQ for[%s]", pThis->Type->ID);

	return 0x0;
}

ASMJIT_PATCH(0x5012C9, HouseClass_WritetoIni4, 0x6)
{
	GET(HouseClass*, pThis, EBX);

	if (IS_SAME_STR_("USSR", pThis->Type->ID))
		Debug::LogInfo("Writing to ini0 Control_Edge for[%s]", pThis->Type->ID);

	return 0x0;
}

ASMJIT_PATCH(0x5012E1, HouseClass_WritetoIni5, 0x6)
{
	GET(HouseClass*, pThis, EBX);

	if (IS_SAME_STR_("USSR", pThis->Type->ID))
		Debug::LogInfo("Writing to ini0 PlayerControl for[%s]", pThis->Type->ID);

	return 0x0;
}

ASMJIT_PATCH(0x5012F9, HouseClass_WritetoIni6, 0x6)
{
	GET(HouseClass*, pThis, EBX);

	if (IS_SAME_STR_("USSR", pThis->Type->ID))
		Debug::LogInfo("Writing to ini0 Color for[%s]", pThis->Type->ID);

	return 0x0;
}

ASMJIT_PATCH(0x50134C, HouseClass_WritetoIni7, 0x5)
{
	GET(HouseClass*, pThis, EBX);

	if (IS_SAME_STR_("USSR", pThis->Type->ID))
		Debug::LogInfo("Writing to ini0 Allies for[%s]", pThis->Type->ID);

	return 0x0;
}

ASMJIT_PATCH(0x50136E, HouseClass_WritetoIni8, 0x5)
{
	GET(HouseClass*, pThis, EBX);

	if (IS_SAME_STR_("USSR", pThis->Type->ID))
		Debug::LogInfo("Writing to ini0 UINAME for[%s]", pThis->Type->ID);

	return 0x0;
}

ASMJIT_PATCH(0x501380, HouseClass_WritetoIni9, 0xA)
{
	GET(HouseClass*, pThis, EBX);

	if (IS_SAME_STR_("USSR", pThis->Type->ID))
		Debug::LogInfo("Writing to ini0 Base for[%s]", pThis->Type->ID);

	return 0x0;
}

ASMJIT_PATCH(0x42ED72, BaseClass_WriteToINI1, 0x7)
{
	GET(BaseClass*, pThis, ESI);
	HouseClass* ptr = reinterpret_cast<HouseClass*>((DWORD)pThis - offsetof(HouseClass, Base));

	if (IS_SAME_STR_("USSR", ptr->Type->ID))
		Debug::LogInfo("Writing to ini0 Base PercentBuilt for[%s]", ptr->Type->ID);

	return 0x0;
}

ASMJIT_PATCH(0x42ED8C, BaseClass_WriteToINI2, 0x5)
{
	GET(BaseClass*, pThis, ESI);
	HouseClass* ptr = reinterpret_cast<HouseClass*>((DWORD)pThis - offsetof(HouseClass, Base));

	if (IS_SAME_STR_("USSR", ptr->Type->ID))
		Debug::LogInfo("Writing to ini0 Base NodeCount(%d) for[%s]", pThis->BaseNodes.Count, ptr->Type->ID);

	return 0x0;
}
#endif


#define HAVE_MREMAP     0
#define MORECORE_CONTIGUOUS 0
#define DLMALLOC_EXPORT /* if you want explicit exports - but we'll avoid exporting malloc/free */
#define USE_DL_PREFIX 1

#define dlmalloc  mydlmalloc
#define dlfree    mydlfree
#define dlrealloc mydlrealloc
#define dlcalloc  mydlcalloc

#include <ExtraHeaders/DougLeaMalloc.c>

template <typename T>
struct DlAllocator
{
	using value_type = T;
	DlAllocator() noexcept { }
	template <class U> DlAllocator(const DlAllocator<U>&) noexcept { }
	T* allocate(std::size_t n)
	{
		if (n == 0) return nullptr;
		size_t bytes = n * sizeof(T);
		void* p = mydlmalloc(bytes);
		if (!p) throw std::bad_alloc();
		return static_cast<T*>(p);
	}
	void deallocate(T* p, std::size_t n) noexcept
	{
		(void)n;
		if (p) mydlfree(p);
	}
	template <class U> bool operator==(DlAllocator<U> const&) const noexcept { return true; }
	template <class U> bool operator!=(DlAllocator<U> const&) const noexcept { return false; }
};

struct AllocInfo
{
	void* real_ptr;    // pointer returned by mydlmalloc (base)
	size_t requested;  // original requested size
	size_t sizeEntry;  // rounded sizeEntry used for footer/header
};

static inline void* key_empty() { return nullptr; }
static inline void* key_tomb() { return (void*)1; }

struct SimpleMapEntry
{
	void* key;      // user pointer returned to game (user-visible)
	AllocInfo val;
};

struct SimpleMap
{
	SimpleMapEntry* table;
	size_t capacity; // power of two
	size_t count;
};

static SimpleMap* g_simple_map = nullptr;
static SRWLOCK g_map_lock = SRWLOCK_INIT;
static const size_t HEADER_SIZE = 4; // sbh header bytes written before user pointer

// ---------- helper: compute SBH sizeEntry (matches decompiled) ----------
static inline size_t compute_sizeEntry(size_t requested)
{
	return (requested + 23) & ~(size_t)0xF;
}

// ---------- create SBH-style block backed by dlmalloc ----------
static void* create_sbh_block(size_t requested)
{
	if (requested == 0) requested = 1;
	size_t sizeEntry = compute_sizeEntry(requested);
	size_t allocNeeded = sizeEntry + HEADER_SIZE + 8;
	void* real = mydlmalloc(allocNeeded);
	if (!real) return nullptr;
	uint32_t stored = static_cast<uint32_t>(sizeEntry + 1);
	*(uint32_t*)real = stored;
	void* user = (void*)((char*)real + HEADER_SIZE);
	uint32_t* footer = (uint32_t*)((char*)real + (sizeEntry - 4));
	*footer = stored;
	return user;
}

static size_t simple_hash_ptr(void* p)
{
	uintptr_t x = (uintptr_t)p;
	x = x ^ (x >> 16);
	return (size_t)x;
}

static bool simple_map_init(size_t initial_pow2 = 1024)
{
	if (g_simple_map) return true;
	if (initial_pow2 < 16) initial_pow2 = 16;
	void* m = mydlmalloc(sizeof(SimpleMap));
	if (!m) return false;
	g_simple_map = (SimpleMap*)m;
	g_simple_map->capacity = initial_pow2;
	g_simple_map->count = 0;
	size_t bytes = g_simple_map->capacity * sizeof(SimpleMapEntry);
	g_simple_map->table = (SimpleMapEntry*)mydlmalloc(bytes);
	if (!g_simple_map->table)
	{
		mydlfree(g_simple_map);
		g_simple_map = nullptr;
		return false;
	}
	for (size_t i = 0; i < g_simple_map->capacity; ++i)
	{
		g_simple_map->table[i].key = key_empty();
	}
	return true;
}

static void simple_map_destroy()
{
	if (!g_simple_map) return;
	mydlfree(g_simple_map->table);
	mydlfree(g_simple_map);
	g_simple_map = nullptr;
}

static bool simple_map_resize(size_t newcap)
{
	if (!g_simple_map) return false;
	SimpleMapEntry* newtab = (SimpleMapEntry*)mydlmalloc(newcap * sizeof(SimpleMapEntry));
	if (!newtab) return false;
	for (size_t i = 0; i < newcap; ++i) newtab[i].key = key_empty();
	for (size_t i = 0; i < g_simple_map->capacity; ++i)
	{
		void* k = g_simple_map->table[i].key;
		if (k != key_empty() && k != key_tomb())
		{
			size_t h = simple_hash_ptr(k) & (newcap - 1);
			while (newtab[h].key != key_empty()) h = (h + 1) & (newcap - 1);
			newtab[h] = g_simple_map->table[i];
		}
	}
	mydlfree(g_simple_map->table);
	g_simple_map->table = newtab;
	g_simple_map->capacity = newcap;
	return true;
}

static bool simple_map_insert(void* k, const AllocInfo& v)
{
	if (!g_simple_map)
	{
		if (!simple_map_init(1024)) return false;
	}

	if ((g_simple_map->count + 1) * 10 >= g_simple_map->capacity * 6)
	{
		if (!simple_map_resize(g_simple_map->capacity * 2)) return false;
	}
	size_t idx = simple_hash_ptr(k) & (g_simple_map->capacity - 1);
	while (true)
	{
		void* cur = g_simple_map->table[idx].key;
		if (cur == key_empty() || cur == key_tomb())
		{
			g_simple_map->table[idx].key = k;
			g_simple_map->table[idx].val = v;
			g_simple_map->count++;
			return true;
		}
		if (cur == k)
		{
			g_simple_map->table[idx].val = v;
			return true;
		}
		idx = (idx + 1) & (g_simple_map->capacity - 1);
	}

	return false;
}

static AllocInfo* simple_map_find(void* k)
{
	if (!g_simple_map) return nullptr;
	size_t idx = simple_hash_ptr(k) & (g_simple_map->capacity - 1);
	size_t start = idx;
	while (true)
	{
		void* cur = g_simple_map->table[idx].key;
		if (cur == key_empty()) return nullptr;
		if (cur == k) return &g_simple_map->table[idx].val;
		idx = (idx + 1) & (g_simple_map->capacity - 1);
		if (idx == start) return nullptr;
	}
}

static bool simple_map_remove(void* k)
{
	if (!g_simple_map) return false;
	size_t idx = simple_hash_ptr(k) & (g_simple_map->capacity - 1);
	size_t start = idx;
	while (true)
	{
		void* cur = g_simple_map->table[idx].key;
		if (cur == key_empty()) return false;
		if (cur == k)
		{
			g_simple_map->table[idx].key = key_tomb();
			if (g_simple_map->count) g_simple_map->count--;
			return true;
		}
		idx = (idx + 1) & (g_simple_map->capacity - 1);
		if (idx == start) return false;
	}
}

// ---------- wrappers (no calls to original CRT or game functions) ----------
extern "C" void InitDlWrapper()
{
	simple_map_init(1024);
}

extern "C" void ShutdownDlWrapper()
{
	simple_map_destroy();
}

extern "C" void* __cdecl game_malloc_wrapper(size_t size)
{
	if (!g_simple_map) InitDlWrapper();
	void* user = create_sbh_block(size);
	if (!user) return nullptr;
	AllocInfo info;
	info.real_ptr = (void*)((char*)user - HEADER_SIZE);
	info.requested = size;
	info.sizeEntry = compute_sizeEntry(size);

	AcquireSRWLockExclusive(&g_map_lock);
	simple_map_insert(user, info);
	ReleaseSRWLockExclusive(&g_map_lock);
	return user;
}

extern "C" void __cdecl game_free_wrapper(void* user_ptr)
{
	if (!user_ptr) return;
	AcquireSRWLockExclusive(&g_map_lock);
	AllocInfo* it = simple_map_find(user_ptr);
	if (it)
	{
		void* real = it->real_ptr;
		simple_map_remove(user_ptr);
		ReleaseSRWLockExclusive(&g_map_lock);
		mydlfree(real);
		return;
	}
	ReleaseSRWLockExclusive(&g_map_lock);
}

extern "C" void* __cdecl game_realloc_wrapper(void* user_ptr, size_t newsize)
{

	if (!g_simple_map) InitDlWrapper();
	if (!user_ptr)
	{
		return game_malloc_wrapper(newsize);
	}
	AcquireSRWLockExclusive(&g_map_lock);
	AllocInfo* it = simple_map_find(user_ptr);
	if (it)
	{
		AllocInfo info = *it;
		ReleaseSRWLockExclusive(&g_map_lock);
		void* new_user = create_sbh_block(newsize);
		if (!new_user) return nullptr;
		size_t tocopy = info.requested < newsize ? info.requested : newsize;
		memcpy(new_user, user_ptr, tocopy);
		game_free_wrapper(user_ptr);
		return new_user;
	}
	ReleaseSRWLockExclusive(&g_map_lock);
	void* fresh = game_malloc_wrapper(newsize);
	return fresh;
}

extern "C" size_t __cdecl game_msize_wrapper(void* user_ptr)
{
	if (!user_ptr) return 0;
	if (!g_simple_map) return 0;
	AcquireSRWLockShared(&g_map_lock);
	AllocInfo* it = simple_map_find(user_ptr);
	if (it)
	{
		size_t req = it->requested;
		ReleaseSRWLockShared(&g_map_lock);
		return req;
	}
	ReleaseSRWLockShared(&g_map_lock);
	return 0;
}

extern "C" void* __cdecl game_calloc_wrapper(size_t num, size_t size)
{

	size_t total = num * size;
	void* user = game_malloc_wrapper(total ? total : 1);
	if (user)
	{
		// use memset
		memset(user, 0, total ? total : 1);
	}
	return user;
}

static bool allocHookInit = false;
using PVFV = void(__cdecl*)(void);
using initterm_t = void(__cdecl*)(PVFV*, PVFV*);

// Real initterm function address (unchanged)
constexpr uintptr_t REAL_INITTERM_ADDR = 0x007CBED3;

// Caller sites you provided (addresses that call REAL_INITTERM_ADDR)
constexpr uintptr_t INITTERM_CALLERS[] = {
	0x007CBDC4,
	0x007CBDD3,
	0x007CBE8B,
	0x007CBE9C
};
constexpr size_t INITTERM_CALLER_COUNT = sizeof(INITTERM_CALLERS) / sizeof(INITTERM_CALLERS[0]);

// flag flipped by wrapper after real initterm returns
static volatile LONG g_crt_ready = 0;

// The wrapper that callers will be patched to call instead of calling REAL_INITTERM_ADDR directly.
// Must match signature: void __cdecl _initterm(PVFV* first, PVFV* last)
extern "C" void __cdecl initterm_caller_wrapper(PVFV* first, PVFV* last)
{
	// Call the real initterm directly (we didn't overwrite the real function)
	initterm_t real = reinterpret_cast<initterm_t>(REAL_INITTERM_ADDR);
	// call it
	real(first, last);

	// mark ready
	InterlockedExchange(&g_crt_ready, 1);
}

// Helper: patch a near CALL (E8 rel32) at callSiteAddr to call newTarget
static bool PatchNearCall(uintptr_t callSiteAddr, void* newTarget)
{
	BYTE* p = reinterpret_cast<BYTE*>(callSiteAddr);
	// We expect a near CALL opcode 0xE8
	if (p[0] != 0xE8)
	{
		// Not a direct near call; skip (or handle other patterns manually)
		return false;
	}

	DWORD oldProtect;
	if (!VirtualProtect(p, 5, PAGE_EXECUTE_READWRITE, &oldProtect)) return false;

	// compute relative offset: target - (callSite + 5)
	intptr_t rel = (intptr_t)newTarget - (intptr_t)(callSiteAddr + 5);
	*(int32_t*)(p + 1) = (int32_t)rel;

	// restore protection and flush icache
	VirtualProtect(p, 5, oldProtect, &oldProtect);
	FlushInstructionCache(GetCurrentProcess(), (LPCVOID)callSiteAddr, 5);
	return true;
}

// Installer: patch all known callers to call our wrapper
static void InstallInittermCallerPatches()
{
	for (size_t i = 0; i < INITTERM_CALLER_COUNT; ++i)
	{
		uintptr_t site = INITTERM_CALLERS[i];
		PatchNearCall(site, (void*)&initterm_caller_wrapper);
	}
}

extern "C" void WaitAndInstallAllocHooks()
{
	// 1) Install caller patches (redirect callers -> our wrapper)
	InstallInittermCallerPatches();

	// 2) Wait for wrapper to flip the flag (timeout safety)
	const DWORD timeout_ms = 30000;
	DWORD start = GetTickCount();
	while (InterlockedCompareExchange(&g_crt_ready, 0, 0) == 0)
	{
		if ((DWORD)(GetTickCount() - start) > timeout_ms)
		{
			MessageBoxA(NULL, "Timeout waiting for CRT ready flag", "dlwrap", MB_OK | MB_ICONERROR);
			return;
		}
		Sleep(5);
	}

	if (!allocHookInit)
	{
		//  /**
		// *  C memory functions.
		// */
		Hook_Function(0x7C93E8, &game_free_wrapper);
		Hook_Function(0x7D0F45, &game_realloc_wrapper);
		Hook_Function(0x7D3374, &game_calloc_wrapper);
		Hook_Function(0x7C9430, &game_malloc_wrapper);
		Hook_Function(0x7D107D, &game_msize_wrapper);

		/**
		 *  C++ new and delete.
		 */

		allocHookInit = true;
	}

	/**
	 *  Standard functions.
	 */
	 //Hook_Function(0x7D5408, &strdup);
	 //Hook_Function(0x7C9CC2, &std::strtok);
}

DWORD WINAPI InitThreadProc(LPVOID)
{
	// DO NOT call any hooking code here for this test.
	// Minimal allocator test:
	//if (!g_simple_map) InitDlWrapper(); // or simple_map_init()

	//early stuffs that can be installed without breaking game
	Hook_Function(0x7C8E17, &std::malloc);
	Hook_Function(0x7C8B3D, &std::free);

	//WaitAndInstallAllocHooks();

	return 0;
}


#include <Misc/CustomMemoryManager.h>

void InitializeCustomMemorySystem()
{
	Debug::LogDeferred("Initializing Custom Memory System...\n");

	// Run signature safety check
	// if (!CustomMemoryManager::RunSignatureSafetyCheck())
	// {
	// 	Debug::LogDeferred("WARNING: Potential signature conflicts detected!\n");
	// 	Debug::LogDeferred("Consider using CustomMemoryManager::RegenerateSignatures() if issues occur.\n");
	// }

	/*
		Debug::LogDeferred("Custom Memory System initialization complete!\n");
	*/


}

//Imports::SetCapture = SetCapture_Hook;
//Imports::SetCursorPos = SetCursorPos_Hook;

//auto mod = GetModuleHandleW(L"ntdll.dll");

//if (auto target = (FnRtl)GetProcAddress(mod, "RtlGetAppContainerNamedObjectPath")) {
//	MH_CreateHook(target, &HookedRtlGetAppContainerNamedObjectPath, (void**)&RealFn);
//	MH_EnableHook(target);
//};


		//std::thread([]() {
		//	EnableLargeAddressAwareFlag(Patch::CurrentProcess);
		//}).detach();

	//make sure is correct executable loading the dll
	//if(IsGamemdExe((HMODULE)Patch::CurrentProcess))



using FnRtl = NTSTATUS(WINAPI*)(HANDLE, HANDLE, ULONG, PUNICODE_STRING);
FnRtl RealFn = nullptr;

NTSTATUS WINAPI HookedRtlGetAppContainerNamedObjectPath(
	HANDLE token,
	HANDLE directoryHandle,
	ULONG length,
	PUNICODE_STRING path)
{
	//OutputDebugStringA("[Phobos] RtlGetAppContainerNamedObjectPath was called!\n");
	//LogCallStack(); // Capture caller
	return RealFn(token, directoryHandle, length, path);
}

#include <ExtraHeaders/MemoryPool.h>

static CriticalSection critSec3, critSec4;
#ifdef _ReplaceAlloc
struct GameMemoryReplacer
{


	static char* _strtok_r(char* str, const char* delim, char** saveptr)
	{
		char* token;

		if (str == nullptr)
			str = *saveptr;

		// Skip leading delimiters
		str += std::strspn(str, delim);
		if (*str == '\0')
		{
			*saveptr = str;
			return nullptr;
		}

		token = str;
		// Find end of token
		str = std::strpbrk(token, delim);
		if (str == nullptr)
		{
			// This token finishes the string
			*saveptr = token + std::strlen(token);
		}
		else
		{
			*str = '\0';
			*saveptr = str + 1;
		}
		return token;
	}

	static char* _strtok(char* str, const char* delim)
	{
		static char* saveptr = nullptr;
		return _strtok_r(str, delim, &saveptr);
	}
};
#endif

HMODULE GetCurrentModule()
{
	MEMORY_BASIC_INFORMATION mbi;
	VirtualQuery((void*)GetCurrentModule, &mbi, sizeof(mbi));
	return static_cast<HMODULE>(mbi.AllocationBase);
}

void LogCallStack(int skipFrames = 0, int maxFrames = 16)
{
	void* stack[32];
	unsigned short frames = CaptureStackBackTrace(skipFrames + 1, maxFrames, stack, nullptr);

	HANDLE hProcess = GetCurrentProcess();
	static bool symInit = false;
	if (!symInit)
	{
		SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);
		symInit = SymInitialize(hProcess, nullptr, TRUE);
	}

	char out[512];
	SYMBOL_INFO* sym = (SYMBOL_INFO*)_alloca(sizeof(SYMBOL_INFO) + 256);
	sym->MaxNameLen = 255;
	sym->SizeOfStruct = sizeof(SYMBOL_INFO);

	HMODULE base = GetCurrentModule();

	for (unsigned short i = 0; i < frames; ++i)
	{
		DWORD64 addr = (DWORD64)(stack[i]);
		if (SymFromAddr(hProcess, addr, nullptr, sym))
		{
			snprintf(out, sizeof(out), "[%02d] %s - 0x%p\n", i, sym->Name, (void*)addr);
		}
		else
		{
			uintptr_t offset = (uintptr_t)addr - (uintptr_t)base;
			snprintf(out, sizeof(out), "[%02d] RetAddr: 0x%p (offset: 0x%X)\n", i, (void*)addr, (unsigned)offset);
		}
		OutputDebugStringA(out);
	}
}


TheMemoryPoolCriticalSection = &critSec4;
TheDmaCriticalSection = &critSec3;

//for (auto& datas : Patch::ModuleDatas) {
//	for (size_t i = 0; i < Patch::ModuleDatas.size(); ++i) {
//		if (i != 0 || (datas.Handle && datas.Handle != INVALID_HANDLE_VALUE)) {
//			CloseHandle(datas.Handle);
//		}
//	}
//}

		//else if (ExceptionMode != ExceptionHandlerMode::Default
		//		&& IS_SAME_STR_(dlls.ModuleName.c_str(), "kernel32.dll"))
		//{
		//	if (GetProcAddress(dlls.Handle, "AddVectoredExceptionHandler")) {
		//		pExceptionHandler = AddVectoredExceptionHandler(1, Exception::ExceptionFilter);
		//	}
		//}

//init the real logger
		//auto& logger = SafeLogger::GetInstance();
		//LogConfig config;
		//config.enabled = true;
		//config.console_output = true;
		//config.log_filename = "testings.log";
		//logger.SetConfig(config);
		//logger.Initialize();

		//LOG_INFO("DLL injection successful, logging enabled via command line");
		//LOG_INFO("Initialized Phobos " PRODUCT_VERSION ".");
		//LOG_INFO("args {}", args);

#ifdef EXPERIMENTAL_IMGUI
ASMJIT_PATCH(0x5D4E66, Windows_Message_Handler_Add, 0x7)
{
	PhobosWindowClass::Callback();
	return 0x0;
}
#endif

#pragma region DEFINES

#ifdef ENABLE_TLS
DWORD TLS_Thread::dwTlsIndex_SHPDRaw_1;
DWORD TLS_Thread::dwTlsIndex_SHPDRaw_2;
#endif

#pragma endregion


#ifdef EXPERIMENTAL_IMGUI
PhobosWindowClass::Create();
#endif

//syringe wont inject the dll unless it got atleast one hook
//so i keep this
#if !defined(NO_SYRINGE)
#endif

#if defined(NO_SYRINGE)
ApplyEarlyFuncs();
//LuaData::ApplyCoreHooks();
Phobos::ExeRun();
#endif

//HANDLE h = CreateThread(nullptr, 0, InitThreadProc, nullptr, 0, nullptr);
//if (h) CloseHandle(h);

//InitHeapPatches(HeapCreate(0, 1 << 20, 0));

			//Mem::shutdownMemoryManager();