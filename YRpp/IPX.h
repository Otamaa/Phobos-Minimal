#pragma once

#include <Helpers/CompileTime.h>

#pragma pack(push, 1)

// The four-byte network number and six-byte node (MAC) address that together
// identify a machine on an IPX network. Westwood declared these as raw array
// typedefs; they are wrapped in structs here so they can be used as members
// without decaying to pointers.
struct NetNumType {
	unsigned char Value[4];
};
static_assert(sizeof(NetNumType) == 4);

struct NetNodeType {
	unsigned char Value[6];
};
static_assert(sizeof(NetNodeType) == 6);

class ALIGN(4) IPXAddressClass
{
public:

	static COMPILETIMEEVAL reference<IPXAddressClass, 0xA8D600u> const Instance {};
	static COMPILETIMEEVAL reference<bool, 0xA8D5FCu> const IsBridge {};
	static COMPILETIMEEVAL reference<int, 0x828140> const State {};

	void Assign(unsigned char* pNetworkNumber, unsigned char* pNodeAddress, short shortNetworkNumber) noexcept
	{
		// 0x53ECE7 .. 0x53ECF8 - unconditional 6-byte node copy
		std::memcpy(this->NodeAddress, pNodeAddress, sizeof(this->NodeAddress));

		// 0x53ECF8 - mov ecx, IPAddress_some_IP_State ; cmp ecx, 1
		if (State() == 1)
		{
			// 0x53ED03 - full 4-byte network number
			std::memcpy(this->NetworkNumber, pNetworkNumber, sizeof(this->NetworkNumber));
			return;
		}

		// 0x53ED0E - zero the whole dword, then overwrite only the low 16 bits
		std::memset(this->NetworkNumber, 0, sizeof(this->NetworkNumber));
		std::memcpy(this->NetworkNumber, &shortNetworkNumber, sizeof(shortNetworkNumber));
	}

	IPXAddressClass()
	{ JMP_THIS(0x53ECB0) }

	IPXAddressClass(const unsigned char* network, const unsigned char* node, unsigned short port)
	{ JMP_THIS(0x53ECE0) }

	void Set_Address(const unsigned char* network, const unsigned char* node, unsigned short port)
	{ JMP_THIS(0x53ED20) }

	void Get_Address(unsigned char* network, unsigned char* node, unsigned short* port) const
	{ JMP_THIS(0x53ED60) }

	void Get_Address(unsigned char* network, unsigned char* node) const
	{ JMP_THIS(0x53EDA0) }

	int Is_Broadcast() const
	{ JMP_THIS(0x53EE00) }

	const char* As_String(const IPXAddressClass* address = nullptr) const
	{ JMP_THIS(0x53F0E0) }

	void Set_Port(unsigned short port)
	{ JMP_THIS(0x53F200) }

	bool operator==(const IPXAddressClass& other) const
	{ JMP_THIS(0x53EE70) }

	bool operator!=(const IPXAddressClass& other) const
	{ JMP_THIS(0x53EF60) }

public:
	unsigned char NetworkNumber[4];
	unsigned char NodeAddress[6];
	// YR carries UDP/IP endpoints in this struct as well as real IPX
	// addresses, so the trailing two bytes are only meaningful in IP mode.
	unsigned char field_A[2];
};
typedef IPXAddressClass IPAddressClass;
static_assert(sizeof(IPXAddressClass) == 0x0C, "IPXAddressClass wrong size");
static_assert(sizeof(IPXAddressClass) == sizeof(IPAddressClass), "IPAddressClass wrong size");

#pragma pack(pop)