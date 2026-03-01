#pragma once

#include <atlcomcli.h>
#include <CStreamClass.h>

struct CompressedStream
{
	ATL::CComPtr<IStream> Raw;
	CStreamClass* Compressor = nullptr;
	ATL::CComPtr<IStream> Stream;

	HRESULT Create(IStorage* storage, const OLECHAR* name)
	{
		HRESULT hr = storage->CreateStream(name,
			STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
			0, 0, &Raw);
		if (FAILED(hr)) return hr;

		Compressor = new CStreamClass();
		if (!Compressor) return E_OUTOFMEMORY;

		hr = Compressor->Link_Stream(Raw);
		if (FAILED(hr)) return hr;

		return Compressor->QueryInterface(__uuidof(IStream), (void**)&Stream);
	}

	HRESULT Open(IStorage* storage, const OLECHAR* name)
	{
		HRESULT hr = storage->OpenStream(name, nullptr,
			STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &Raw);
		if (FAILED(hr)) return hr;

		Compressor = new CStreamClass();
		if (!Compressor) return E_OUTOFMEMORY;

		hr = Compressor->Link_Stream(Raw);
		if (FAILED(hr)) return hr;

		return Compressor->QueryInterface(__uuidof(IStream), (void**)&Stream);
	}

	void Close()
	{
		if (Compressor)
		{
			Compressor->Unlink_Stream(nullptr);
		}
		Stream.Release();
		if (Compressor)
		{
			Compressor->Release();
			Compressor = nullptr;
		}
		Raw.Release();
	}
};
