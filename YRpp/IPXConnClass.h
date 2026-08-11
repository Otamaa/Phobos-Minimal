#pragma once

#include <YRPPCore.h>
#include <ConnectionClass.h>
#include <IPX.h>

class NOVTABLE IPXConnClass : public ConnectionClass
{
public:
	enum IPXConnTag
	{
		CONN_NAME_MAX = 40
	};

	DEFINE_REFERENCE(WORD, Socket, 0xAA0568)
		DEFINE_REFERENCE(int, Configured, 0xAA05A4)
		DEFINE_REFERENCE(int, SocketOpen, 0xAA05A8)
		DEFINE_REFERENCE(int, Listening, 0xAA05AC)

		virtual void Init() override
	{ JMP_THIS(0x53F4E0); }

	static int __fastcall Open_Socket(WORD socket)
	{ JMP_THIS(0x53F5F0); }
	static void __fastcall Close_Socket(WORD socket)
	{ JMP_THIS(0x53F630); }

	static int Start_Listening()
	{ JMP_STD(0x53F540); }
	static int Stop_Listening()
	{ JMP_STD(0x53F5B0); }

	static int __fastcall Broadcast(void* buf, int buflen)
	{ JMP_THIS(0x53F830); }

protected:
	virtual int Send(CommHeaderType* buf, int buflen, void* extrabuf, int extralen, bool isGlobalConn, __int16 port) override
	{ JMP_THIS(0x53F5D0); }

	virtual int Send_To_Address(CommHeaderType* buf, int buflen, IPXAddressClass* address, NetNodeType* nodeOverride, bool isGlobalConn, __int16 port)
	{ JMP_THIS(0x53F650); }

public:
	IPXAddressClass Address;
	NetNodeType ImmediateAddress;
	PROTECTED_PROPERTY(BYTE, align_5E[0x2]);

	DWORD Immed_Set;
	DWORD ID;
	wchar_t Name[CONN_NAME_MAX];
};
static_assert(sizeof(IPXConnClass) == 0xB8);