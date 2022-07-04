#pragma once

#include <Base/Always.h>
#include <vector>

struct module_export
{
	void* address;
	const char* name;
	unsigned short ordinal;
};

// no more than 8 characters
#define PATCH_SECTION_NAME ".patch"
#pragma section(PATCH_SECTION_NAME, read)

#pragma pack(push, 1)
#pragma warning(push)
#pragma warning( disable : 4324)
struct __declspec(novtable)
	Patch
{
	unsigned int offset;
	unsigned int size;
	BYTE* pData;

	static void ApplyStatic();
	void Apply();

	template<typename TFrom, typename To>
	static inline void Apply(uintptr_t addrFrom, To toImpl, DWORD& protect_flag, DWORD ReadFlag = PAGE_READWRITE, size_t size = 4u)
	{
		if (VirtualProtect((LPVOID)addrFrom, size, ReadFlag, &protect_flag) == TRUE)
		{
			*reinterpret_cast<TFrom*>(addrFrom) = toImpl;
			VirtualProtect((LPVOID)addrFrom, size, protect_flag, NULL);
		}
	}

	/**
	 *  Get the memory address of a function.
	 */
	template<typename T>
	static inline uintptr_t Get_Func_Address(T func)
	{
		return reinterpret_cast<uintptr_t>((void*&)func);
	}

	/**
	 *  Adjust a pointer value by the input amount.
	 *
	 *  When writing hook wrappers for stdcall interface methods, MSVC offsets
	 *  "this" by 4 bytes to try and will try to call from the wrong interface/class
	 *  table (due to dual inheritance and adjuster thunks).
	 *
	 *  So use these helpers to commit treason and adjust the pointer... I'm sorry...
	 */
	template<class T>
	static __forceinline T Adjust_Ptr(T ptr, int amount)
	{
		uintptr_t rawptr = reinterpret_cast<uintptr_t>(ptr);
		return reinterpret_cast<T>(rawptr + amount);
	}

	static std::vector<module_export> enumerate_module_exports(HMODULE handle);
	static uintptr_t GetModuleBaseAddress(const char* modName);
	static DWORD GetDebuggerProcessId(DWORD dwSelfProcessId);
};
#pragma warning(pop)
#pragma pack(pop)