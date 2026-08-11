#pragma once

#include <IPXConnClass.h>

class NOVTABLE IPXGlobalConnClass : public IPXConnClass
{
public:
	enum GlobalConnectionEnum
	{
		GLOBAL_MAGICNUM = 0x1235,
		COMMAND_AND_CONQUER = 0xaa01,
		COMMAND_AND_CONQUER0 = 0xaa00
	};

	virtual int Send_Packet(void* buf, int buflen, int ack_req, char forwardto) override
	{ JMP_THIS(0x540610); }
	virtual int Receive_Packet(CommHeaderType* buf, int buflen) override
	{ JMP_THIS(0x540630); }
	virtual int Get_Packet(void* buf, int* buflen) override
	{ JMP_THIS(0x540650); }

	virtual int Send_Packet(void* buf, int buflen, IPXAddressClass* address, int ack_req, __int16 port, int packet_id)
	{ JMP_THIS(0x53FBD0); }

	virtual int Receive_Packet(GlobalHeaderType* buf, int buflen, IPXAddressClass* address, __int16 port)
	{ JMP_THIS(0x53FCB0); }

	virtual int Get_Packet(void* buf, int* buflen, IPXAddressClass* address, WORD* product_id)
	{ JMP_THIS(0x53FF10); }

	virtual int Peek_Packet(int index, void* buf, int* buflen, IPXAddressClass* address, WORD* product_id, int* packet_id)
	{ JMP_THIS(0x53FFA0); }

	virtual int Mark_Packet_Read(int index)
	{ JMP_THIS(0x540030); }

	virtual int Purge_Send_Queue_To(IPXAddressClass* address, __int16 port)
	{ JMP_THIS(0x540340); }

	virtual void Strip_Packets(DWORD age)
	{ JMP_THIS(0x540110); }

	void Set_Bridge(NetNumType* bridge)
	{ JMP_THIS(0x5402B0); }

protected:
	virtual int Purge_Send_Queue() override
	{ JMP_THIS(0x5402D0); }
	virtual int Service_Receive_Queue() override
	{ JMP_THIS(0x5400D0); }
	virtual int Send(CommHeaderType* buf, int buflen, void* extrabuf, int extralen, bool isGlobalConn, __int16 port) override
	{ JMP_THIS(0x540050); }
	virtual int Send_To_Address(CommHeaderType* buf, int buflen, IPXAddressClass* address, NetNodeType* nodeOverride, bool isGlobalConn, __int16 port) override
	{ JMP_THIS(0x5403F0); }

public:
	__int16 ProductID;

	NetNumType BridgeNet;
	NetNodeType BridgeNode;
	int IsBridge;
	bool field_C8;
	PROTECTED_PROPERTY(BYTE, align_C9[0x3]);
	IPXAddressClass* LastAddress;
	int* LastPacketID;
	int LastCount;
	int LastRXIndex;
};
static_assert(sizeof(IPXGlobalConnClass) == 0xDC);