#pragma once

#include <ArrayClasses.h>

class IPXConnClass;
class IPXGlobalConnClass;
class ConnectionClass;
class IPXAddressClass;
class IPXManagerClass
{
public:
	// Static
	static COMPILETIMEEVAL reference<IPXManagerClass, 0xA8E9C0u> const Instance {};

	ConnectionClass* SetTiming(int retrydelta, int maxretries, int timeout, bool a5)
	{ JMP_THIS(0x540C60) }

	int ResponseTime()
		{ JMP_THIS(0x542450) }

	// Sends an out-of-band global packet. With address == nullptr it broadcasts
	// to every global peer; otherwise it targets the given peer (use a node's
	// Address from SessionClass::Players). Returns nonzero on success.
	int Send_Global_Message(void* buf, int buflen, int ack_req = 0, IPXAddressClass* address = nullptr, unsigned short port = 0, int packet_id = 0)
		{ JMP_THIS(0x5410F0) }

	// Services the network layer (flushes queued sends / pumps receives).
	void Service()
		{ JMP_THIS(0x541820) }

	// Tears down the connection to the given peer (by connection id / house id).
	// Used when rebuilding connections after a mid-game save load.
	bool Delete_Connection(int id)
		{ JMP_THIS(0x540F00) }

	// Properties
private:
	void* vtable;
public:
	BYTE IPXStatus;
	BYTE Listening;
	DWORD Glb_MaxPacketLen;
	int Glb_NumPackets;
	DWORD Pvt_MaxPacketLen;
	DWORD Pvt_NumPackets;
	DWORD __Ext_MaxPacketLen;
	DWORD __Ext_MaxPackets;
	WORD ProductID;
	WORD Socket;
	DWORD ConnectionNum;
	IPXConnClass* Connection[7];
	DWORD NumConnections;
	IPXGlobalConnClass* __GlobalChannel;
	IPXGlobalConnClass* __IPXGlobalConn2;
	ConnectionClass* __MulticastConnection;
	DWORD CurConnection;
	DWORD RetryDelta;
	DWORD MaxRetries;
	DWORD Timeout;
	int __Global1RetryDelta;
	int __Global1Timing;
	int __Global1RetryTimeout;
	char field_70;
	char field_71;
	char field_72;
	char field_73;
	int field_74;
	char field_78;
	char field_79;
	char field_7A;
	char field_7B;
	int field_7C;
	int field_80;
	int field_84;
	int field_88;
	int field_8C;
	int field_90;
	int field_94;
	int field_98;
	int field_9C;
	DWORD SendOverflows;
	DWORD ReceiveOverflows;
	DWORD BadConnection;
};
static_assert(sizeof(IPXManagerClass) == 0xAC);