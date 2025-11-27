#pragma once
#include <Helpers/Macro.h>
#include <ASMMacros.h>
#include <YRPPCore.h>
#include "Patch.h"

#define NAKED __declspec(naked)

#define DECLARE_PATCH(name) \
    [[ noreturn ]] static NOINLINE NAKED void name() noexcept

#pragma region Patch Macros

#pragma region Patch Structs
#pragma pack(push, 1)
#pragma warning(push)
#pragma warning( disable : 4324)

#define LJMP_LETTER 0xE9
#define CALL_LETTER 0xE8
#define NOP_LETTER  0x90

typedef void JumpType;

typedef JumpType LJMP;
struct _LJMP
{
	BYTE command;
	DWORD pointer;

	COMPILETIMEEVAL
		_LJMP(DWORD offset, DWORD pointer) :
		command(LJMP_LETTER),
		pointer(pointer - offset - 5)
	{
	};


	COMPILETIMEEVAL
		_LJMP(DWORD ptr) :
		command(LJMP_LETTER),
		pointer(ptr)
	{ };


	COMPILETIMEEVAL FORCEDINLINE size_t size() const{
		return sizeof(*this);
	}
};

typedef JumpType CALL;
struct _CALL
{
	BYTE command;
	DWORD pointer;

	COMPILETIMEEVAL
		_CALL(DWORD offset, DWORD pointer) :
		command(CALL_LETTER),
		pointer(pointer - offset - 5)
	{
	};

	COMPILETIMEEVAL FORCEDINLINE size_t size() const {
		return sizeof(*this);
	}
};

typedef JumpType CALL6;
struct _CALL6
{
	BYTE command;
	DWORD pointer;
	BYTE nop;

	COMPILETIMEEVAL
		_CALL6(DWORD offset, DWORD pointer) :
		command(CALL_LETTER),
		pointer(pointer - offset - 5),
		nop(NOP_LETTER)
	{
	};

	COMPILETIMEEVAL FORCEDINLINE size_t size() const {
		return sizeof(*this);
	}
};

typedef JumpType VTABLE;
struct _VTABLE
{
	DWORD pointer;

	COMPILETIMEEVAL
		_VTABLE(DWORD offset, DWORD pointer) :
		pointer(pointer)
	{
	};

	COMPILETIMEEVAL FORCEDINLINE size_t size() const {
		return sizeof(*this);
	}
};

#pragma warning(pop)
#pragma pack(pop)
#pragma endregion Patch Structs

#pragma region Macros

#pragma region Static Patch
#define _ALLOCATE_STATIC_PATCH(offset, size, type, data)                \
	namespace STATIC_PATCH##offset                                \
	{                                                             \
		__declspec(allocate(PATCH_SECTION_NAME))                  \
		Patch patch = {type ,offset, size, (BYTE*)data};                \
	}

#define DEFINE_PATCH_TYPED(type, offset, ...)                     \
	namespace STATIC_PATCH##offset                                \
	{                                                             \
		const type data[] = {__VA_ARGS__};                        \
	}                                                             \
	_ALLOCATE_STATIC_PATCH(offset, sizeof(data), PatchType::PATCH_, data);

#define DEFINE_PATCH(offset, ...)                                 \
	DEFINE_PATCH_TYPED(byte, offset, __VA_ARGS__);

#define DEFINE_PATCH_ADDR_OFFSET(type , address , offset , ...)                      \
	namespace STATIC_PATCH##address## _ ##offset                  \
	{                                                             \
		const type data[] = {__VA_ARGS__};                        \
		__declspec(allocate(PATCH_SECTION_NAME))                  \
		Patch patch = { PatchType::PATCH_ ,address + offset, sizeof(data), (BYTE*)data};\
	}

/*
	Be aware that LJMP is only recomended to use on 5 bytes address
	using it on other than that , possibly causing stack corruption
*/
#define DEFINE_JUMP(jumpType, offset, pointer)                    \
	namespace STATIC_PATCH##offset                                \
	{                                                             \
		const _##jumpType data (offset, pointer);                 \
	}                                                             \
	_ALLOCATE_STATIC_PATCH(offset, sizeof(data),PatchType::##jumpType##_ , &data);

#define DEFINE_NAKED_HOOK(hook, funcname)                         \
	void funcname();                                              \
	DEFINE_FUNCTION_JUMP(LJMP, hook, GET_OFFSET(funcname))                 \
	void NAKED funcname()

#pragma endregion Static Patch
#define _GET_FUNCTION_ADDRESS(function, getterName)               \
	static constexpr FORCEDINLINE uintptr_t getterName()         \
	{                                                             \
		uintptr_t addr;                                           \
		{ _asm mov eax, function }                                \
		{ _asm mov addr, eax }                                    \
		return addr;                                              \
	}


//this doesnt work with CTOR and DTOR
#define DEFINE_FUNCTION_JUMP(jumpType, offset, function)		  \
	DEFINE_JUMP(jumpType, offset, MiscTools::to_DWORD(&function))
#pragma endregion

#define DEFINE_FUNCTION_JUMPB(jumpType, offset, function)		  \
	namespace NAMESPACE_THISCALL_JUMP##offset                     \
	{                                                             \
		_GET_FUNCTION_ADDRESS(function, GetAddr)                  \
		DEFINE_JUMP(jumpType, offset, GetAddr())                  \
	}
#pragma endregion

#pragma region Dynamic Patch
#define _ALLOCATE_DYNAMIC_PATCH(name, offset, size, type, data)         \
	namespace DYNAMIC_PATCH_##name                                \
	{                                                             \
		Patch patch = {type , offset, size, (BYTE*)data};                \
	}                                                             \
	Patch* const name = &DYNAMIC_PATCH_##name::patch;

#define DEFINE_DYNAMIC_PATCH(name, offset, ...)                   \
	namespace DYNAMIC_PATCH_##name                                \
	{                                                             \
		const BYTE data[] = {__VA_ARGS__};                        \
	}                                                             \
	_ALLOCATE_DYNAMIC_PATCH(name, offset, sizeof(data), PatchType::PATCH_, data);

#define ALLOCATE_LOCAL_PATCH(name , offset , ...)                 \
	BYTE name##_data[] = {__VA_ARGS__};                           \
	PatchWrapper name { offset , sizeof(##name##_data) , PatchType::PATCH_, name##_data };

#define DEFINE_DYNAMIC_JUMP(jumpType, name, offset, pointer)      \
	namespace DYNAMIC_PATCH_##name                                \
	{                                                             \
		const _##jumpType data (offset, pointer);                 \
	}                                                             \
	_ALLOCATE_DYNAMIC_PATCH(name, offset, sizeof(data), PatchType::##jumpType##_ ,&data);
#pragma endregion Dynamic Patch

#define DEFINE_VARIABLE_PATCH(offset, name, ...)                     \
namespace VARIABLE_PATCH##offset                                     \
{                                                                    \
	const byte data[] = {__VA_ARGS__};                               \
	const Patch patch = { PatchType::PATCH_, offset, sizeof(data), (byte*)data };  \
};                                                                   \
namespace VARIABLE_PATCH                                             \
{                                                                    \
	const Patch* name = &VARIABLE_PATCH##offset::patch;         \
};

#define DEFINE_VARIABLE_LJMP(from, to ,name)						\
namespace VARIABLE_PATCH##from  {									\
	const _LJMP data = { to-from-5 };				\
	const Patch patch = { PatchType::LJMP_, from, 5, (byte*)&data };				\
};																	\
namespace VARIABLE_PATCH {											\
	const Patch* name = &VARIABLE_PATCH##from::patch;			\
};

template<class T, size_t offs>
struct PointerOffset
{
	OPTIONALINLINE COMPILETIMEEVAL auto Get()
	{
		return reinterpret_cast<T>(((DWORD)this) - offs);
	}
};

struct MiscTools
{
	static DWORD __fastcall RelativeOffset(void const* pFrom, void const* pTo)
	{
		auto const from = reinterpret_cast<DWORD>(pFrom);
		auto const to = reinterpret_cast<DWORD>(pTo);

		return to - from;
	}

	template<typename T>
	static volatile T& Memory(const uintptr_t ptr)
	{
		return *reinterpret_cast<T*>(ptr);
	}

	static volatile uint8_t& Memory(const uint16_t ptr)
	{
		return *reinterpret_cast<uint8_t*>(ptr);
	}

	template<typename T>
	static COMPILETIMEEVAL FORCEDINLINE DWORD to_DWORD(T new_address)
	{
		return reinterpret_cast<DWORD>(((void*&)new_address));
	}
};

template<typename T>
void FORCEDINLINE Patch_Jump(uintptr_t address, T new_address)
{
	static_assert(sizeof(_LJMP) == 5, "Jump struct not expected size!");

	SIZE_T bytes_written { 0u };
	_LJMP cmd { address, reinterpret_cast<uintptr_t>((void*&)new_address) };
	WriteProcessMemory(Patch::CurrentProcess, (LPVOID)address, &cmd, sizeof(_LJMP), &bytes_written);
}

template<typename T>
void FORCEDINLINE Patch_Call(uintptr_t address, T new_address)
{
	static_assert(sizeof(_CALL) == 5, "Call struct not expected size!");

	SIZE_T bytes_written { 0u };
	_CALL cmd { address, reinterpret_cast<uintptr_t>((void*&)new_address) };
	WriteProcessMemory(Patch::CurrentProcess, (LPVOID)address, &cmd, sizeof(_CALL), &bytes_written);
}

template<typename T>
void FORCEDINLINE Hook_Function(uintptr_t address, T new_address) {
    Patch_Jump(address, reinterpret_cast<uintptr_t>((void *&)new_address));
}

template<typename T>
void FORCEDINLINE Patch_Call6(uintptr_t address, T new_address)
{
	static_assert(sizeof(_CALL6) == 6, "Call6 struct not expected size!");

	SIZE_T bytes_written { 0u };
	_CALL6 cmd { address, reinterpret_cast<uintptr_t>((void*&)new_address) };
	cmd.command = LJMP_LETTER;

	WriteProcessMemory(Patch::CurrentProcess, (LPVOID)address, &cmd, sizeof(_LJMP), &bytes_written);
}

template<typename T>
void FORCEDINLINE Patch_Vtable(uintptr_t address, T new_address)
{
	static_assert(sizeof(_VTABLE) == 4, "Vtable struct not expected size!");

	SIZE_T bytes_written { 0u };

	_VTABLE cmd { address  , reinterpret_cast<uintptr_t>((void*&)new_address) };
	WriteProcessMemory(Patch::CurrentProcess, (LPVOID)address, &cmd, sizeof(_VTABLE), &bytes_written);
}

#pragma endregion Macros
#pragma endregion Patch Macros