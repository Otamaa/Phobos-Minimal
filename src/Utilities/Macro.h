#pragma once
#include <Helpers/Macro.h>
#include <ASMMacros.h>
#include <YRPPCore.h>
#include "Patch.h"

template<class T, size_t offs>
struct PointerOffset
{
	inline constexpr auto Get()
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
	volatile T& Memory(const uintptr_t ptr) {
		return *reinterpret_cast<T*>(ptr);
	}

	volatile uint8_t& Memory(const uint16_t ptr) {
		return *reinterpret_cast<uint8_t*>(ptr);
	}
};

#define NAKED __declspec(naked)

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

	constexpr
		_LJMP(DWORD offset, DWORD pointer) :
		command(LJMP_LETTER),
		pointer(pointer - offset - 5)
	{
	};

	constexpr FORCEINLINE size_t size() const{
		return sizeof(*this);
	}
};

typedef JumpType CALL;
struct _CALL
{
	BYTE command;
	DWORD pointer;

	constexpr
		_CALL(DWORD offset, DWORD pointer) :
		command(CALL_LETTER),
		pointer(pointer - offset - 5)
	{
	};

	constexpr FORCEINLINE size_t size() const {
		return sizeof(*this);
	}
};

typedef JumpType CALL6;
struct _CALL6
{
	BYTE command;
	DWORD pointer;
	BYTE nop;

	constexpr
		_CALL6(DWORD offset, DWORD pointer) :
		command(CALL_LETTER),
		pointer(pointer - offset - 5),
		nop(NOP_LETTER)
	{
	};

	constexpr FORCEINLINE size_t size() const {
		return sizeof(*this);
	}
};

typedef JumpType VTABLE;
struct _VTABLE
{
	DWORD pointer;

	constexpr
		_VTABLE(DWORD offset, DWORD pointer) :
		pointer(pointer)
	{
	};

	constexpr FORCEINLINE size_t size() const {
		return sizeof(*this);
	}
};

#pragma warning(pop)
#pragma pack(pop)
#pragma endregion Patch Structs

#pragma region Macros
#define GET_OFFSET(pointer) reinterpret_cast<DWORD>(pointer)

#pragma region Static Patch
#define _ALLOCATE_STATIC_PATCH(offset, size, data)                \
	namespace STATIC_PATCH##offset                                \
	{                                                             \
		__declspec(allocate(PATCH_SECTION_NAME))                  \
		Patch patch = {offset, size, (BYTE*)data};                \
	}

#define DEFINE_PATCH(offset, ...)                                 \
	namespace STATIC_PATCH##offset                                \
	{                                                             \
		const BYTE data[] = {__VA_ARGS__};                        \
	}                                                             \
	_ALLOCATE_STATIC_PATCH(offset, sizeof(data), data);

/*
	Be aware that LJMP is only recomended to use on 5 bytes address
	using it on other than that , possibly causing stack corruption
*/
#define DEFINE_JUMP(jumpType, offset, pointer)                    \
	namespace STATIC_PATCH##offset                                \
	{                                                             \
		const _##jumpType data (offset, pointer);                 \
	}                                                             \
	_ALLOCATE_STATIC_PATCH(offset, sizeof(data), &data);

#define DEFINE_NAKED_HOOK(hook, funcname)                         \
	void funcname();                                              \
	DEFINE_JUMP(LJMP, hook, GET_OFFSET(funcname))                 \
	void NAKED funcname()

#pragma endregion Static Patch

#pragma region Dynamic Patch
#define _ALLOCATE_DYNAMIC_PATCH(name, offset, size, data)         \
	namespace DYNAMIC_PATCH_##name                                \
	{                                                             \
		Patch patch = {offset, size, (BYTE*)data};                \
	}                                                             \
	Patch* const name = &DYNAMIC_PATCH_##name::patch;

#define DEFINE_DYNAMIC_PATCH(name, offset, ...)                   \
	namespace DYNAMIC_PATCH_##name                                \
	{                                                             \
		const BYTE data[] = {__VA_ARGS__};                        \
	}                                                             \
	_ALLOCATE_DYNAMIC_PATCH(name, offset, sizeof(data), data);

#define ALLOCATE_LOCAL_PATCH(name , offset , ...)                 \
	BYTE name##_data[] = {__VA_ARGS__};                           \
	PatchWrapper name { offset , sizeof(##name##_data) , name##_data };

#define DEFINE_DYNAMIC_JUMP(jumpType, name, offset, pointer)      \
	namespace DYNAMIC_PATCH_##name                                \
	{                                                             \
		const _##jumpType data (offset, pointer);                 \
	}                                                             \
	_ALLOCATE_DYNAMIC_PATCH(name, offset, sizeof(data), &data);
#pragma endregion Dynamic Patch

#pragma endregion Macros
#pragma endregion Patch Macros