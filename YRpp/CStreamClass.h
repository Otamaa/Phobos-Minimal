#pragma once

#include <ASMMacros.h>
#include <YRPPCore.h>
#include <Base/Always.h>

#include <Interface/IDontKnow.h>
#include <Interface/ILinkStream.h>

#include <comip.h>
#include <comdef.h>

_COM_SMARTPTR_TYPEDEF(IStream, __uuidof(IStream));
_COM_SMARTPTR_TYPEDEF(ILinkStream, __uuidof(ILinkStream));

class DECLSPEC_UUID("B48FA168-646F-11D2-9B74-00104B972FE8")
NOVTABLE CStreamClass : public IStream, public ILinkStream
{
public:
	/**
	 *  IUnknown
	 */
	IFACEMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj) override JMP_STD(0x4A2990);
	IFACEMETHOD_(ULONG, AddRef)() override JMP_STD(0x4A2930);
	IFACEMETHOD_(ULONG, Release)() override JMP_STD(0x4A2950);

	/**
	 *  ISequentialStream
	 */
	IFACEMETHOD(Read)(void* pv, ULONG cb, ULONG* pcbRead) override JMP_STD(0x4A2B60);
	IFACEMETHOD(Write)(const void* pv, ULONG cb, ULONG* pcbWritten) override JMP_STD(0x4A2CD0);

	/**
	 *  IStream
	 */
	IFACEMETHOD(Seek)(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER* plibNewPosition) override JMP_STD(0x4A2E00);
	IFACEMETHOD(SetSize)(ULARGE_INTEGER libNewSize) override JMP_STD(0x4A2E50);
	IFACEMETHOD(CopyTo)(IStream* pstm, ULARGE_INTEGER cb, ULARGE_INTEGER* pcbRead, ULARGE_INTEGER* pcbWritten) override JMP_STD(0x4A2EA0);
	IFACEMETHOD(Commit)(DWORD grfCommitFlags) override JMP_STD(0x4A2EE0);
	IFACEMETHOD(Revert)() override JMP_STD(0x4A2F10);
	IFACEMETHOD(LockRegion)(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType) override JMP_STD(0x4A2F40);
	IFACEMETHOD(UnlockRegion)(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType) override JMP_STD(0x4A2F80);
	IFACEMETHOD(Stat)(STATSTG* pstatstg, DWORD grfStatFlag) override JMP_STD(0x4A2FC0);
	IFACEMETHOD(Clone)(IStream** ppstm) override JMP_STD(0x4A2FF0);

	/**
	 *  ILinkStream
	 */
	IFACEMETHOD(Link_Stream)(IUnknown* stream) override JMP_STD(0x4A2A20);
	IFACEMETHOD(Unlink_Stream)(IUnknown** stream) override JMP_STD(0x4A2AB0);

public:
	CStreamClass() JMP_THIS(0x4A2820);
	virtual ~CStreamClass() JMP_THIS(0x4A2880);

	HRESULT Compress(void* in_buffer, ULONG length) { JMP_THIS(0x4A3020); }
	HRESULT Compress() { JMP_THIS(0x4A30E0); }

private:
	IStreamPtr field_8;
	LONG RefCount;
	bool field_10;
	bool field_11;
	int Counter;
	unsigned char* Buffer;
	unsigned char* Buffer2;
	void* Dictionary;
	struct
	{
		unsigned int CompCount;
		unsigned int UncompCount;
	} Header;
};
