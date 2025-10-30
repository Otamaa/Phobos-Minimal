#pragma once

#include <Base/Always.h>
#include <vector>
#include <string>
#include <Helpers/CompileTime.h>

struct module_export
{
	void* address;
	const char* name;
	WORD ordinal;
};

struct module_Import
{
	void* address;
	const char* name;
};

struct dllData
{
	std::string ModuleName;
	HMODULE Handle;
	uintptr_t BaseAddr;
	size_t Size;
	std::vector<module_Import> Impors;
	std::vector<module_export> Exports;
	//std::vector<std::string> Patches;

	COMPILETIMEEVAL dllData() = default;

	COMPILETIMEEVAL dllData(const char* name, HMODULE handle, uintptr_t baseaddr , size_t size) : ModuleName { name }
		, Handle { handle }
		, BaseAddr { baseaddr }
		, Size { size }
		, Impors {}
		, Exports {}
	//	, Patches {}
	{
	}

	COMPILETIMEEVAL ~dllData() = default;
};


#pragma pack(push, 1)
#pragma warning(push)
#pragma warning( disable : 4324)
enum class PatchType : BYTE{
	CALL_, CALL6_ , LJMP_ , VTABLE_ , PATCH_
};

struct NOVTABLE
	Patch
{
	PatchType type;
	uintptr_t offset;
	size_t size;
	const BYTE* pData;

	static void ApplyStatic();
	void Apply();

	static std::vector<dllData> ModuleDatas;

	template<typename To>
	static OPTIONALINLINE void Apply_withmemcpy(uintptr_t addrFrom, To toImpl, DWORD& protect_flag, DWORD ReadFlag = PAGE_READWRITE, size_t size = 4u)
	{
		DWORD protect_flagb {};
		if (VirtualProtect((LPVOID)addrFrom, size, ReadFlag, &protect_flag) == TRUE) {
			std::memcpy((void*)addrFrom, toImpl, size);
			VirtualProtect((LPVOID)addrFrom, size, protect_flag, &protect_flagb);
			FlushInstructionCache(Game_hInstance, (LPVOID)addrFrom, size);
		}
	}

	/**
	 *  Get the memory address of a function.
	 */
	template<typename T>
	static OPTIONALINLINE uintptr_t Get_Func_Address(T func)
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
	static FORCEDINLINE T Adjust_Ptr(T ptr, int amount)
	{
		uintptr_t rawptr = reinterpret_cast<uintptr_t>(ptr);
		return reinterpret_cast<T>(rawptr + amount);
	}

	static void Apply_RAW(uintptr_t offset, size_t sz, PatchType type, const BYTE* data);

	static FORCEDINLINE void Apply_RAW(uintptr_t offset, std::initializer_list<BYTE> data) {
		Patch::Apply_RAW(offset, data.size(), PatchType::PATCH_, const_cast<byte*>(data.begin()));
	}

	template <size_t Size>
	static FORCEDINLINE void Apply_RAW(uintptr_t offset, const char(&str)[Size]) {
		Apply_RAW(offset, Size, PatchType::PATCH_, reinterpret_cast<const BYTE*>(str));
	};

	template <typename T>
	static FORCEDINLINE void Apply_TYPED(uintptr_t offset, std::initializer_list<T> data)
	{
		Patch::Apply_RAW(offset, data.size() * sizeof(T), PatchType::PATCH_, reinterpret_cast<const BYTE*>(data.begin()));
	};

	static void Apply_LJMP(uintptr_t offset, uintptr_t pointer);
	static FORCEDINLINE void Apply_LJMP(uintptr_t offset, void* pointer)
	{
		Patch::Apply_LJMP(offset, reinterpret_cast<uintptr_t>(pointer));
	};

	static void Apply_CALL(uintptr_t offset, uintptr_t pointer);
	static FORCEDINLINE void Apply_CALL(uintptr_t offset, void* pointer)
	{
		Patch::Apply_CALL(offset, reinterpret_cast<uintptr_t>(pointer));
	};

	static void Apply_CALL6(uintptr_t offset, uintptr_t pointer);
	static FORCEDINLINE void Apply_CALL6(uintptr_t offset, void* pointer)
	{
		Patch::Apply_CALL6(offset, reinterpret_cast<DWORD>(pointer));
	};

	static void Apply_VTABLE(uintptr_t offset, uintptr_t pointer);
	static FORCEDINLINE void Apply_VTABLE(uintptr_t offset, void* pointer)
	{
		Patch::Apply_VTABLE(offset, reinterpret_cast<uintptr_t>(pointer));
	};

	static void Apply_OFFSET(uintptr_t offset, uintptr_t pointer)
	{
		Patch::Apply_TYPED<uintptr_t>(offset, { pointer });
	};
	static FORCEDINLINE void Apply_OFFSET(uintptr_t offset, uintptr_t* pointer)
	{
		Patch::Apply_OFFSET(offset, reinterpret_cast<DWORD>(pointer));
	};

	static std::vector<module_export> enumerate_module_exports(HMODULE handle);
	static uintptr_t GetModuleBaseAddress(const char* modName);
	static DWORD GetDebuggerProcessId(DWORD dwSelfProcessId);
	static void PrintAllModuleAndBaseAddr();
	static int GetSection(HANDLE hInstance, const char* sectionName, void** pVirtualAddress);
	static uintptr_t GetEATAddress(const char* moduleName, const char* funcName);
	static uintptr_t GetIATAddress(const char* moduleName, const char* funcName);
	static COMPILETIMEEVAL reference<HINSTANCE, 0xB732F0u> const Game_hInstance {};
public :
	static HANDLE CurrentProcess;
	static std::string WindowsVersion;
};

#pragma warning(pop)
#pragma pack(pop)