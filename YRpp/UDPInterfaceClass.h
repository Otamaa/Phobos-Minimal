#pragma once
#include <WinsockInterfaceClass.h>

class UDPInterfaceClass : public WinsockInterfaceClass
{
public:
	// Static
	static COMPILETIMEEVAL reference<UDPInterfaceClass*, 0x887628u> const Instance {};

	bool OpenSocket(int port = 0)
	{ JMP_THIS(0x7B30B0) }

	UDPInterfaceClass()
	{ JMP_THIS(0x7B2DB0); }

	virtual ~UDPInterfaceClass()
	{ JMP_THIS(0x7B2E50); }

public:

	// Properties
	int16_t Port;
	std::array<int, 8> SpareSockets;
	std::array<int16_t, 8> SpareSocketPorts;
	DWORD NextSpareSocket;
	DWORD Socket;
	DECLARE_PROPERTY(DynamicVectorClass<void*>, BroadcastAddresses);
	std::array<int16_t, 256> words3F350;
	DECLARE_PROPERTY(DynamicVectorClass<void*>, LocalAddresses);
	DWORD NextAddressPort;
	std::array<int, 16>Addresses;
	std::array<int16_t, 16> Ports;
};
static_assert(sizeof(UDPInterfaceClass) == 0x3F5CC);