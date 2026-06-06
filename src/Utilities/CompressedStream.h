#pragma once

#include <atlcomcli.h>
#include <CStreamClass.h>

template<bool disableCompressor>
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

		if constexpr (!disableCompressor)
		{
			Compressor = new CStreamClass();
			if (!Compressor) return E_OUTOFMEMORY;

			hr = Compressor->Link_Stream(Raw);
			if (FAILED(hr)) return hr;

			return Compressor->QueryInterface(__uuidof(IStream), (void**)&Stream);
		}
		else
		{
			Stream = Raw;
			return S_OK;
		}
	}

	HRESULT Open(IStorage* storage, const OLECHAR* name)
	{
		HRESULT hr = storage->OpenStream(name, nullptr,
			STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &Raw);
		if (FAILED(hr)) return hr;

		if constexpr (!disableCompressor)
		{
			Compressor = new CStreamClass();
			if (!Compressor) return E_OUTOFMEMORY;

			hr = Compressor->Link_Stream(Raw);
			if (FAILED(hr)) return hr;

			return Compressor->QueryInterface(__uuidof(IStream), (void**)&Stream);
		}
		else
		{
			Stream = Raw;
			return S_OK;
		}
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