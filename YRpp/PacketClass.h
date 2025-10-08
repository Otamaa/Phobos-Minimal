#pragma once

#include <Base/Always.h>
#include <Memory.h>
#pragma pack(push, 1)

class FieldClass
{
public:
	FieldClass() = default;

	template <size_t N>
	FieldClass(const char(&str)[N], CHAR data)
	{
		static_assert(N - 1 <= 4, "String too long! (max 4 chars)");
		JMP_THIS(0x4CB580);
	}

	template <size_t N>
	FieldClass(const char(&str)[N], BYTE data)
	{ static_assert(N - 1 <= 4, "String too long! (max 4 chars)"); JMP_THIS(0x4CB5E0); }

	template <size_t N>
	FieldClass(const char(&str)[N], SHORT data)
	{ static_assert(N - 1 <= 4, "String too long! (max 4 chars)"); JMP_THIS(0x4CB640); }

	template <size_t N>
	FieldClass(const char(&str)[N], WORD data)
	{ static_assert(N - 1 <= 4, "String too long! (max 4 chars)"); JMP_THIS(0x4CB6A0); }

	template <size_t N>
	FieldClass(const char(&str)[N], LONG data)
	{ static_assert(N - 1 <= 4, "String too long! (max 4 chars)"); JMP_THIS(0x4CB700); }

	template <size_t N>
	FieldClass(const char(&str)[N], DWORD data)
	{ static_assert(N - 1 <= 4, "String too long! (max 4 chars)"); JMP_THIS(0x4CB760); }

	template <size_t N>
	FieldClass(const char(&str)[N], char* data)
	{ static_assert(N - 1 <= 4, "String too long! (max 4 chars)"); JMP_THIS(0x4CB7C0); }

	template <size_t N>
	FieldClass(const char(&str)[N], void* data, int length)
	{ static_assert(N - 1 <= 4, "String too long! (max 4 chars)"); JMP_THIS(0x4CB830); }

	~FieldClass()
	{ JMP_THIS(0x4CB890); }

	void HostToNet()
	{ JMP_THIS(0x4CB8B0); }

	void NetToHost()
	{ JMP_THIS(0x4CB920); }

	// Properties
	char ID[4];
	WORD DataType;
	WORD Size;
	void* Data;
	FieldClass* Next;
};
static_assert(sizeof(FieldClass) == 0x10);

class PacketClass
{
public:
	void COMPILETIMEEVAL OPTIONALINLINE AddField(FieldClass* pField)
	{
		pField->Next = this->Head;
		this->Head = pField;
	}

	template<typename T, size_t N, typename... Args>
	void OPTIONALINLINE AddField(const char(&id)[N], T data, Args... args)
	{
		static_assert(N - 1 <= 4, "String too long! (max 4 chars)");

		auto pField = GameCreate<FieldClass>(id, data, args...);
		this->AddField(pField);
	}

	// Properties
	WORD Size;
	WORD ID;
	FieldClass* Head;
	FieldClass* Current;
};
static_assert(sizeof(PacketClass) == 0x0C);

#pragma pack(pop)