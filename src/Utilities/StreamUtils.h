#pragma once

#include <objbase.h>
#include <Utilities/Debug.h>

class StreamWrapperWithTracking : public IStream
{
private:
	LPSTREAM baseStream;
	ULARGE_INTEGER totalBytesProcessed;
	ULONG refCount;

public:
	StreamWrapperWithTracking(LPSTREAM pBaseStream)
		: baseStream(pBaseStream), refCount(1)
	{
		totalBytesProcessed.QuadPart = 0;
		if (baseStream)
		{
			baseStream->AddRef();
		}
	}

	virtual ~StreamWrapperWithTracking()
	{
		if (baseStream)
		{
			baseStream->Release();
		}
	}

	// IUnknown methods
	STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject) override
	{
		if (riid == IID_IUnknown || riid == IID_IStream)
		{
			*ppvObject = this;
			AddRef();
			return S_OK;
		}
		return E_NOINTERFACE;
	}

	STDMETHODIMP_(ULONG) AddRef() override
	{
		return InterlockedIncrement(&refCount);
	}

	STDMETHODIMP_(ULONG) Release() override
	{
		ULONG count = InterlockedDecrement(&refCount);
		if (count == 0)
		{
			delete this;
		}
		return count;
	}

	// IStream methods with byte counting
	STDMETHODIMP Read(void* pv, ULONG cb, ULONG* pcbRead) override
	{
		if (!baseStream) return E_FAIL;

		HRESULT hr = baseStream->Read(pv, cb, pcbRead);
		if (SUCCEEDED(hr) && pcbRead)
		{
			totalBytesProcessed.QuadPart += *pcbRead;
			//Debug::Log("StreamWrapper: Read %lu bytes, total now %llu\n",
			//			*pcbRead, totalBytesProcessed.QuadPart);
		}
		return hr;
	}

	STDMETHODIMP Write(const void* pv, ULONG cb, ULONG* pcbWritten) override
	{
		if (!baseStream) return E_FAIL;
		//Debug::Log("StreamWrapper::Write: %lu bytes\n", cb);

		HRESULT hr = baseStream->Write(pv, cb, pcbWritten);

		if (SUCCEEDED(hr))
		{
			ULONG actualBytesWritten;
			if (pcbWritten)
			{
				// Caller wants to know how many bytes were written
				actualBytesWritten = *pcbWritten;
			}
			else
			{
				// Caller doesn't care - assume all bytes were written
				actualBytesWritten = cb;
			}

			totalBytesProcessed.QuadPart += actualBytesWritten;
			//Debug::Log("StreamWrapper: Wrote %lu bytes, total now %llu\n",
			//		   actualBytesWritten, totalBytesProcessed.QuadPart);
		}
		else
		{
			Debug::Log("StreamWrapper: Write failed with hr=0x%08X\n", hr);
		}

		return hr;
	}

	STDMETHODIMP Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER* plibNewPosition) override
	{
		if (!baseStream) return E_FAIL;
		return baseStream->Seek(dlibMove, dwOrigin, plibNewPosition);
	}

	STDMETHODIMP SetSize(ULARGE_INTEGER libNewSize) override
	{
		if (!baseStream) return E_FAIL;
		return baseStream->SetSize(libNewSize);
	}

	STDMETHODIMP CopyTo(IStream* pstm, ULARGE_INTEGER cb, ULARGE_INTEGER* pcbRead, ULARGE_INTEGER* pcbWritten) override
	{
		if (!baseStream) return E_FAIL;
		return baseStream->CopyTo(pstm, cb, pcbRead, pcbWritten);
	}

	STDMETHODIMP Commit(DWORD grfCommitFlags) override
	{
		if (!baseStream) return E_FAIL;
		return baseStream->Commit(grfCommitFlags);
	}

	STDMETHODIMP Revert() override
	{
		if (!baseStream) return E_FAIL;
		return baseStream->Revert();
	}

	STDMETHODIMP LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType) override
	{
		if (!baseStream) return E_FAIL;
		return baseStream->LockRegion(libOffset, cb, dwLockType);
	}

	STDMETHODIMP UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType) override
	{
		if (!baseStream) return E_FAIL;
		return baseStream->UnlockRegion(libOffset, cb, dwLockType);
	}

	STDMETHODIMP Stat(STATSTG* pstatstg, DWORD grfStatFlag) override
	{
		if (!baseStream) return E_FAIL;
		return baseStream->Stat(pstatstg, grfStatFlag);
	}

	STDMETHODIMP Clone(IStream** ppstm) override
	{
		if (!baseStream) return E_FAIL;
		return baseStream->Clone(ppstm);
	}

	// Get current position (total bytes processed)
	ULARGE_INTEGER GetCurrentPosition() const
	{
		return totalBytesProcessed;
	}

	// Reset position counter
	void ResetPosition()
	{
		totalBytesProcessed.QuadPart = 0;
	}
};

// Universal Position Tracker for both Save and Load operations
class UniversalPositionTracker
{
private:
	struct Operation
	{
		std::string name;
		ULARGE_INTEGER startPos;
		ULARGE_INTEGER endPos;
		size_t expectedSize;
		bool success;
		std::string operation_type; // "SAVE" or "LOAD"
	};

	std::vector<Operation> operations;
	StreamWrapperWithTracking* trackedStream;
	std::string operationType;

public:
	UniversalPositionTracker(StreamWrapperWithTracking* pTrackedStream, const std::string& opType)
		: trackedStream(pTrackedStream), operationType(opType)
	{
		if (trackedStream)
		{
			trackedStream->AddRef();
		}
	}

	~UniversalPositionTracker()
	{
		if (trackedStream)
		{
			trackedStream->Release();
		}
	}

	ULARGE_INTEGER GetCurrentPosition()
	{
		if (trackedStream)
		{
			return trackedStream->GetCurrentPosition();
		}
		ULARGE_INTEGER zero = { 0 };
		return zero;
	}

	void StartOperation(const char* operationName)
	{
		Operation op;
		op.name = operationName;
		op.startPos = GetCurrentPosition();
		op.expectedSize = 0;
		op.success = false;
		op.operation_type = operationType;
		operations.push_back(op);

		Debug::Log("[%sTracker] START %s at position %llu\n",
				  operationType.c_str(), operationName, op.startPos.QuadPart);
	}

	bool EndOperation(bool operationSuccess, size_t expectedBytes = 0)
	{
		if (operations.empty()) return false;

		Operation& op = operations.back();
		op.endPos = GetCurrentPosition();
		op.success = operationSuccess;
		op.expectedSize = expectedBytes;

		size_t actualBytes = static_cast<size_t>(op.endPos.QuadPart - op.startPos.QuadPart);

		Debug::Log("[%sTracker] END %s: %s, processed %zu bytes (expected %zu), pos %llu->%llu diff=%llu \n",
				  operationType.c_str(),
				  op.name.c_str(),
				  operationSuccess ? "SUCCESS" : "FAILED",
				  actualBytes,
				  expectedBytes,
				  op.startPos.QuadPart,
				  op.endPos.QuadPart , op.endPos.QuadPart - op.startPos.QuadPart);

		if (!operationSuccess)
		{
			Debug::Log("[%sTracker] CORRUPTION: %s failed, stream may be corrupted\n",
					  operationType.c_str(), op.name.c_str());
			LogAllOperations();
			return false;
		}

		if (expectedBytes > 0 && actualBytes != expectedBytes)
		{
			Debug::Log("[%sTracker] SIZE MISMATCH: %s processed %zu bytes, expected %zu\n",
					  operationType.c_str(), op.name.c_str(), actualBytes, expectedBytes);
		}

		return true;
	}

	void LogAllOperations()
	{
		Debug::Log("[%sTracker] === OPERATION HISTORY ===\n", operationType.c_str());
		for (const auto& op : operations)
		{
			size_t actualBytes = static_cast<size_t>(op.endPos.QuadPart - op.startPos.QuadPart);
			Debug::Log("  %s: %s, %zu bytes, pos %llu->%llu\n",
					  op.name.c_str(),
					  op.success ? "OK" : "FAIL",
					  actualBytes,
					  op.startPos.QuadPart,
					  op.endPos.QuadPart);
		}
	}
};

// Typedefs for easy use
typedef UniversalPositionTracker SavePositionTracker;
typedef UniversalPositionTracker LoadPositionTracker;