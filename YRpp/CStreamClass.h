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

class CStreamClass : public IStream, public ILinkStream
{
public:
	/**
	 *  IUnknown
	 */
	IFACEMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj) JMP_STD(0x4A2990);
	IFACEMETHOD_(ULONG, AddRef)() JMP_STD(0x4A2930);
	IFACEMETHOD_(ULONG, Release)() JMP_STD(0x4A2950);

	/**
	 *  ISequentialStream
	 */
	IFACEMETHOD(Read)(void* pv, ULONG cb, ULONG* pcbRead) JMP_STD(0x4A2B60);
	IFACEMETHOD(Write)(const void* pv, ULONG cb, ULONG* pcbWritten) JMP_STD(0x4A2CD0);

	/**
	 *  IStream
	 */
	IFACEMETHOD(Seek)(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER* plibNewPosition) JMP_STD(0x4A2E00);
	IFACEMETHOD(SetSize)(ULARGE_INTEGER libNewSize) JMP_STD(0x4A2E50);
	IFACEMETHOD(CopyTo)(IStream* pstm, ULARGE_INTEGER cb, ULARGE_INTEGER* pcbRead, ULARGE_INTEGER* pcbWritten) JMP_STD(0x4A2EA0);
	IFACEMETHOD(Commit)(DWORD grfCommitFlags) JMP_STD(0x4A2EE0);
	IFACEMETHOD(Revert)() JMP_STD(0x4A2F10);
	IFACEMETHOD(LockRegion)(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType) JMP_STD(0x4A2F40);
	IFACEMETHOD(UnlockRegion)(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType) JMP_STD(0x4A2F80);
	IFACEMETHOD(Stat)(STATSTG* pstatstg, DWORD grfStatFlag) JMP_STD(0x4A2FC0);
	IFACEMETHOD(Clone)(IStream** ppstm) JMP_STD(0x4A2FF0);

	/**
	 *  ILinkStream
	 */
	IFACEMETHOD(Link_Stream)(IUnknown* stream) JMP_STD(0x4A2A20);
	IFACEMETHOD(Unlink_Stream)(IUnknown** stream) JMP_STD(0x4A2AB0);

public:
	CStreamClass() JMP_THIS(0x4A2820);
	virtual ~CStreamClass() JMP_THIS(0x4A2880);

	NAKEDNOINLINE HRESULT Compress(void* in_buffer, ULONG length) { JMP(0x4A3020); }
	NAKEDNOINLINE HRESULT Compress() { JMP(0x4A30E0); }

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
