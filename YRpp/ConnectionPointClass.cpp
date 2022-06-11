#include "ConnectionPointClass.h"

DEFINE_IMPLEMENTATION(HRESULT STDMETHODCALLTYPE ConnectionPointClass::QueryInterface(REFIID, LPVOID*), 0x4A04B0);
DEFINE_IMPLEMENTATION(ULONG STDMETHODCALLTYPE ConnectionPointClass::AddRef(), 0x4A0520);
DEFINE_IMPLEMENTATION(ULONG STDMETHODCALLTYPE ConnectionPointClass::Release(), 0x4A0540);
DEFINE_IMPLEMENTATION(HRESULT STDMETHODCALLTYPE ConnectionPointClass::GetConnectionInterface(IID*), 0x4A05D0);
DEFINE_IMPLEMENTATION(HRESULT STDMETHODCALLTYPE ConnectionPointClass::GetConnectionPointContainer(IConnectionPointContainer**), 0x4A0610);
DEFINE_IMPLEMENTATION(HRESULT STDMETHODCALLTYPE ConnectionPointClass::Advise(IUnknown*, DWORD*), 0x4A0630);
DEFINE_IMPLEMENTATION(HRESULT STDMETHODCALLTYPE ConnectionPointClass::Unadvise(DWORD), 0x4A0700);
DEFINE_IMPLEMENTATION(HRESULT STDMETHODCALLTYPE ConnectionPointClass::EnumConnections(IEnumConnections**), 0x4A0760);

ConnectionPointClass::ConnectionPointClass(REFIID, IUnknown*)
{
	JMP_THIS(0x4A0870);
}

ConnectionPointClass::~ConnectionPointClass()
{
	JMP_THIS(0x4A08D0);
}

DEFINE_IMPLEMENTATION(HRESULT STDMETHODCALLTYPE EnumConnectionsClass::QueryInterface(REFIID, LPVOID*), 0x49FF80);
DEFINE_IMPLEMENTATION(ULONG STDMETHODCALLTYPE EnumConnectionsClass::AddRef(), 0x49FFF0);
DEFINE_IMPLEMENTATION(ULONG STDMETHODCALLTYPE EnumConnectionsClass::Release(), 0x4A0010);
DEFINE_IMPLEMENTATION(HRESULT STDMETHODCALLTYPE EnumConnectionsClass::Next(ULONG, LPCONNECTDATA, ULONG*), 0x4A00B0);
DEFINE_IMPLEMENTATION(HRESULT STDMETHODCALLTYPE EnumConnectionsClass::Skip(ULONG), 0x4A0160);
DEFINE_IMPLEMENTATION(HRESULT STDMETHODCALLTYPE EnumConnectionsClass::Clone(IEnumConnections**), 0x4A0190);

HRESULT STDMETHODCALLTYPE EnumConnectionsClass::Reset()
{
	this->Current = 0;
	return 0;
}
EnumConnectionsClass::EnumConnectionsClass(const DynamicVectorClass<CONNECTDATA>&)
{
	JMP_THIS(0x4A02B0);
}

EnumConnectionsClass::EnumConnectionsClass(const EnumConnectionsClass&)
{
	JMP_THIS(0x4A0380);
}

EnumConnectionsClass::~EnumConnectionsClass()
{
	JMP_THIS(0x4A0450);
}

DEFINE_IMPLEMENTATION(HRESULT STDMETHODCALLTYPE EnumConnectionPointsClass::QueryInterface(REFIID, LPVOID*), 0x4A0920);
DEFINE_IMPLEMENTATION(ULONG STDMETHODCALLTYPE EnumConnectionPointsClass::AddRef(), 0x4A0990);
DEFINE_IMPLEMENTATION(ULONG STDMETHODCALLTYPE EnumConnectionPointsClass::Release(), 0x4A09B0);
DEFINE_IMPLEMENTATION(HRESULT STDMETHODCALLTYPE EnumConnectionPointsClass::Next(ULONG, LPCONNECTIONPOINT*, ULONG*), 0x4A0A50);
DEFINE_IMPLEMENTATION(HRESULT STDMETHODCALLTYPE EnumConnectionPointsClass::Skip(ULONG), 0x4A0AF0);
DEFINE_IMPLEMENTATION(HRESULT STDMETHODCALLTYPE EnumConnectionPointsClass::Reset(), 0x4A0B10);
DEFINE_IMPLEMENTATION(HRESULT STDMETHODCALLTYPE EnumConnectionPointsClass::Clone(IEnumConnectionPoints**), 0x4A0B20);
EnumConnectionPointsClass::EnumConnectionPointsClass()
{
	JMP_THIS(0x4A0CE0);
}

EnumConnectionPointsClass::EnumConnectionPointsClass(const DynamicVectorClass<LPCONNECTIONPOINT>&)
{
	JMP_THIS(0x4A0C20);
}

EnumConnectionPointsClass::~EnumConnectionPointsClass()
{
	JMP_THIS(0x4A0DB0);
}