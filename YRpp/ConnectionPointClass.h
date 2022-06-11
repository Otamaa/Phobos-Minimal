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
	IFACEMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);
	IFACEMETHOD_(ULONG, AddRef)();
	IFACEMETHOD_(ULONG, Release)();

	/**
	 *  IConnectionPoint
	 */
	IFACEMETHOD(GetConnectionInterface)(IID* pIID);
	IFACEMETHOD(GetConnectionPointContainer)(IConnectionPointContainer** ppCPC);
	IFACEMETHOD(Advise)(IUnknown* pUnkSink, DWORD* pdwCookie);
	IFACEMETHOD(Unadvise)(DWORD dwCookie);
	IFACEMETHOD(EnumConnections)(IEnumConnections** ppEnum);

public:
	ConnectionPointClass(REFIID riid, IUnknown* a2);
	~ConnectionPointClass();

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
	IFACEMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);
	IFACEMETHOD_(ULONG, AddRef)();
	IFACEMETHOD_(ULONG, Release)();

	/**
	 *  IEnumConnection
	 */
	IFACEMETHOD(Next)(ULONG cConnections, LPCONNECTDATA rgcd, ULONG* pcFetched);
	IFACEMETHOD(Skip)(ULONG cConnections);
	IFACEMETHOD(Reset)();
	IFACEMETHOD(Clone)(IEnumConnections** ppEnum);

public:
	EnumConnectionsClass(const DynamicVectorClass<CONNECTDATA>& vec);
	EnumConnectionsClass(const EnumConnectionsClass& that);
	~EnumConnectionsClass();

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
	IFACEMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);
	IFACEMETHOD_(ULONG, AddRef)();
	IFACEMETHOD_(ULONG, Release)();

	/**
	 *  IEnumConnectionPoints
	 */
	IFACEMETHOD(Next)(ULONG cConnections, LPCONNECTIONPOINT* ppCP, ULONG* pcFetched);
	IFACEMETHOD(Skip)(ULONG cConnections);
	IFACEMETHOD(Reset)();
	IFACEMETHOD(Clone)(IEnumConnectionPoints** ppEnum);

public:
	EnumConnectionPointsClass();
	EnumConnectionPointsClass(const DynamicVectorClass<LPCONNECTIONPOINT>& vec);
	~EnumConnectionPointsClass();

private:
	int Current;
	int RefCount;
	DynamicVectorClass<LPCONNECTIONPOINT> ConnectionPoints;
};
