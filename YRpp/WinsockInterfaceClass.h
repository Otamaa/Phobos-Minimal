#pragma once
#include <YRPPCore.h>
#include <ArrayClasses.h>
#include <Helpers/CompileTime.h>

struct ALIGN(4) WinsockBufferType
{
	char Header[16];
	int BufferLen;
	char IsBroadcast;
	BYTE InUse;
	BYTE Allocated;
	char Bool1;
	char Bool2;
	int16_t SrcPort;
	int CRC;
	char Buffer[640];
};
static_assert(sizeof(WinsockBufferType) == 0x2A0);

class ALIGN(4) WinsockInterfaceClass
{
public:
	// Static
	static COMPILETIMEEVAL reference<WinsockInterfaceClass*, 0x887628u> const Instance {};

	bool Init()
	{ JMP_THIS(0x7B1DE0) }

	bool StartListening()
	{ JMP_THIS(0x7B1BC0) }

	void DiscardInBuffers()
	{ JMP_THIS(0x7B1CA0) }

	void DiscardOutBuffers()
	{ JMP_THIS(0x7B1D10) }

	WinsockInterfaceClass() { JMP_THIS(0x7B19C0); }
	~WinsockInterfaceClass() { JMP_THIS(0x7B1AB0); }

	// Properties
private:
	void* vftable;
public:
	DWORD MaxPacketSize;
	DECLARE_PROPERTY(DynamicVectorClass<WinsockBufferType*>, InBuffers);
	DECLARE_PROPERTY(DynamicVectorClass<WinsockBufferType*>, OutBuffers);
	DECLARE_PROPERTY(DynamicVectorClass<WinsockBufferType*>, AltOutBuffers);
	WinsockBufferType StaticInBuffer[128];
	WinsockBufferType StaticOutBuffer[128];
	WinsockBufferType StaticAltOutBuffer[128];
	DWORD StaticInBufferPos;
	DWORD StaticOutBufferPos;
	int StaticAltOutBufferPos;
	DWORD StaticInBuffersInUse;
	DWORD StaticOutBuffersInUse;
	DWORD StaticAltOutBuffersInUse;
	BYTE WinsockInitialised;
	int Socket;
	DWORD AlternateSocket;
	char ReceiveBuffer[640];
	int field_3F2F4;
	int NetCard;
};
static_assert(sizeof(WinsockInterfaceClass) == 0x3F2FC);