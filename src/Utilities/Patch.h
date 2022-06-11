#pragma once

#include <vector>
#include <Lib/MapViewOfFileClass.h>

struct module_export
{
	void* address;
	const char* name;
	unsigned short ordinal;
};

struct patch_decl;

struct Patch final
{
	static void Apply();
	static void Apply(const patch_decl* pItem);
	static void Apply(HANDLE process, const patch_decl* pItem);

	template<typename TFrom, typename To>
	static inline void Apply(uintptr_t addrFrom, To toImpl , DWORD& protect_flag , size_t size = 4u) {
		if (VirtualProtect((LPVOID)addrFrom, size, PAGE_READWRITE, &protect_flag) == TRUE) {
			*reinterpret_cast<TFrom*>(addrFrom) = toImpl;
			VirtualProtect((LPVOID)addrFrom, size, protect_flag, NULL);
		}
	}

	/**
	 *  Get the memory address of a function.
	 */
	template<typename T>
	static inline uintptr_t Get_Func_Address(T func) {
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
	static __forceinline T Adjust_Ptr(T ptr, int amount) {
		uintptr_t rawptr = reinterpret_cast<uintptr_t>(ptr);
		return reinterpret_cast<T>(rawptr + amount);
	}

	static std::vector<module_export> enumerate_module_exports(HMODULE handle);
	static uintptr_t GetModuleBaseAddress(const char* modName);
	static DWORD GetDebuggerProcessId(DWORD dwSelfProcessId);

};