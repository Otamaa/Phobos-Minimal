#pragma once

#include <ArrayClasses.h>
#include <ocidl.h>

/**
 *  VectorClass throws a fuss as it can't find these operators for CONNECTDATA, so we need to define them.
 */
__inline bool operator==(const CONNECTDATA& a, const CONNECTDATA& b)
{
	return !std::memcmp(&a, &b, sizeof(CONNECTDATA));
}

__inline bool operator!=(const CONNECTDATA& a, const CONNECTDATA& b)
{
	return std::memcmp(&a, &b, sizeof(CONNECTDATA));
}

class ConnectionPointClass : public IConnectionPoint
{
public:
	/**
	 *  IUnknown
	 */
	IFACEMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);// JMP_STD(0x4A04B0);
	IFACEMETHOD_(ULONG, AddRef)();//JMP_STD(0x4A0520);
	IFACEMETHOD_(ULONG, Release)();// JMP_STD(0x4A0540);

	/**
	 *  IConnectionPoint
	 */
	IFACEMETHOD(GetConnectionInterface)(IID* pIID);// JMP_STD(0x4A05D0);
	IFACEMETHOD(GetConnectionPointContainer)(IConnectionPointContainer** ppCPC);// JMP_STD(0x4A0610);
	IFACEMETHOD(Advise)(IUnknown* pUnkSink, DWORD* pdwCookie);// JMP_STD(0x4A0630);
	IFACEMETHOD(Unadvise)(DWORD dwCookie);// JMP_STD(0x4A0700);
	IFACEMETHOD(EnumConnections)(IEnumConnections** ppEnum);// JMP_STD(0x4A0760);

public:
	ConnectionPointClass(REFIID riid, IUnknown* a2);//	JMP_THIS(0x4A0870);
	~ConnectionPointClass();//JMP_THIS(0x4A08D0);

private:
	CLSID field_0;
	int RefCount;
	IUnknown* field_18;
	DynamicVectorClass<CONNECTDATA> field_1C;
};


class EnumConnectionsClass : public IEnumConnections
{
public:
	/**
	 *  IUnknown
	 */
	IFACEMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);//JMP_STD(0x49FF80);
	IFACEMETHOD_(ULONG, AddRef)();//JMP_STD(0x49FFF0);
	IFACEMETHOD_(ULONG, Release)();// JMP_STD(0x4A0010);

	/**
	 *  IEnumConnection
	 */
	IFACEMETHOD(Next)(ULONG cConnections, LPCONNECTDATA rgcd, ULONG* pcFetched);// JMP_STD(0x4A00B0);
	IFACEMETHOD(Skip)(ULONG cConnections);// JMP_STD(0x4A0160);
	IFACEMETHOD(Reset)() {
		this->Current = 0;
		return 0;
	}
	IFACEMETHOD(Clone)(IEnumConnections** ppEnum);// JMP_STD(0x4A0190);

public:
	EnumConnectionsClass(const DynamicVectorClass<CONNECTDATA>& vec);//  JMP_THIS(0x4A02B0);
	EnumConnectionsClass(const EnumConnectionsClass& that);// JMP_THIS(0x4A0380);
	~EnumConnectionsClass();//JMP_THIS(0x4A0450);

private:
	DynamicVectorClass<CONNECTDATA> ConnectData;
	int Current;
	int RefCount;
};


class EnumConnectionPointsClass : public IEnumConnectionPoints
{
public:
	/**
	 *  IUnknown
	 */
	IFACEMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);// JMP_STD(0x4A0920);
	IFACEMETHOD_(ULONG, AddRef)();//JMP_STD(0x4A0990);
	IFACEMETHOD_(ULONG, Release)();//JMP_STD(0x4A09B0);

	/**
	 *  IEnumConnectionPoints
	 */
	IFACEMETHOD(Next)(ULONG cConnections, LPCONNECTIONPOINT* ppCP, ULONG* pcFetched);// JMP_STD(0x4A0A50);
	IFACEMETHOD(Skip)(ULONG cConnections);// JMP_STD(0x4A0AF0);
	IFACEMETHOD(Reset)();//JMP_STD(0x4A0B10);
	IFACEMETHOD(Clone)(IEnumConnectionPoints** ppEnum);// JMP_STD(0x4A0B20);

public:
	EnumConnectionPointsClass();//JMP_THIS(0x4A0CE0);
	EnumConnectionPointsClass(const DynamicVectorClass<LPCONNECTIONPOINT>& vec);//JMP_THIS(0x4A0C20);
	~EnumConnectionPointsClass();// JMP_THIS(0x4A0DB0);

private:
	int Current;
	int RefCount;
	DynamicVectorClass<LPCONNECTIONPOINT> ConnectionPoints;
};
