#pragma once

#include <Base/Always.h>
#include <Memory.h>
#pragma pack(push, 1)

class FieldClass
{
public:
	FieldClass() = default;

	FieldClass(char* id, CHAR data);
	FieldClass(char* id, BYTE data);
	FieldClass(char* id, SHORT data);
	FieldClass(char* id, WORD data);
	FieldClass(char* id, LONG data);
	FieldClass(char* id, DWORD data);
	FieldClass(char* id, char* data);
	FieldClass(char* id, void* data, int length);

	~FieldClass();

	void HostToNet()
		;//{ JMP_THIS(0x4CB8B0); }

	void NetToHost()
		;//{ JMP_THIS(0x4CB920); }

public:

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
	void AddField(FieldClass* pField)
	{
		pField->Next = this->Head;
		this->Head = pField;
	}

	template<typename T, typename... Args>
	void AddField(char* id, T data, Args... args)
	{
		auto pField = GameCreate<FieldClass>(id, data, args...);
		this->AddField(pField);
	}

public:

	// Properties
	WORD Size;
	WORD ID;
	FieldClass* Head;
	FieldClass* Current;
};
static_assert(sizeof(PacketClass) == 0x0C);

#pragma pack(pop)