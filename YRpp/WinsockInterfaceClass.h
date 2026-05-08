#pragma once
#include <ArrayClasses.h>

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

typedef enum tProtocolEnum
{
	PROTOCOL_NONE,
	PROTOCOL_IPX,
	PROTOCOL_UDP
} ProtocolEnum;


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
	virtual ~WinsockInterfaceClass() { JMP_THIS(0x7B1AB0); }

	// Properties
public:
#ifndef _VTBLE
	virtual void Close_Socket() RX;
	virtual int Read(void* buffer, int& buffer_len, void* address, int& address_len) R0;
	virtual void WriteTo(void* buffer, int buffer_len, void* address) RX;
	virtual void Broadcast(void* buffer, int buffer_len) RX;
	virtual void Discard_In_Buffers() RX;
	virtual void Discard_Out_Buffers() RX;
	virtual bool Start_Listening() R0;
	virtual void Stop_Listening() RX;
	virtual void Clear_Socket_Error(SOCKET socket) RX;
	virtual bool Set_Socket_Options() R0;
	virtual void Set_Broadcast_Address(void* addr) RX;
	virtual void Clear_Broadcast_Addresses() RX;
	virtual ProtocolEnum Get_Protocol() RT(ProtocolEnum);
	virtual int Protocol_Event_Message() R0;
	virtual bool Open_Socket(SOCKET socket) R0;
	virtual LRESULT Message_Handler(HWND hWnd, UINT uMsg, UINT wParam, LONG lParam) RT(LRESULT);
	virtual bool Get_Host_Name(char* name, int name_len) const R0;
	virtual int Local_Addresses_Count() const R0;
	virtual unsigned char* Get_Local_Address(int i) const R0;
	virtual void Set_NetCard(int netcard) RX;
#else 
	void* vftable;
#endif

public:
	DWORD MaxPacketSize;
	DECLARE_PROPERTY(DynamicVectorClass<WinsockBufferType*>, InBuffers);
	DECLARE_PROPERTY(DynamicVectorClass<WinsockBufferType*>, OutBuffers);
	DECLARE_PROPERTY(DynamicVectorClass<WinsockBufferType*>, AltOutBuffers);
	std::array<WinsockBufferType, 128> StaticInBuffer;
	std::array<WinsockBufferType, 128> StaticOutBuffer;
	std::array<WinsockBufferType, 128> StaticAltOutBuffer;
	DWORD StaticInBufferPos;
	DWORD StaticOutBufferPos;
	int StaticAltOutBufferPos;
	DWORD StaticInBuffersInUse;
	DWORD StaticOutBuffersInUse;
	DWORD StaticAltOutBuffersInUse;
	BYTE WinsockInitialised;
	int Socket;
	DWORD AlternateSocket;
	std::array<char, 640> ReceiveBuffer;
	int field_3F2F4;
	int NetCard;
};
static_assert(sizeof(WinsockInterfaceClass) == 0x3F2FC);