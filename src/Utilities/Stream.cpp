#include "Stream.h"
#include "Debug.h"

#include <Utilities/Swizzle.h>

#include <Objidl.h>

const char* RCFunc = "PhobosStreamReader::RegisterChange()";

bool PhobosByteStream::Write(const void* buffer, size_t size)
{
	if (!buffer && size > 0) return false;

	size_t newSize = position + size;
	if (newSize > data.size())
	{
		data.resize(newSize);
	}

	if (size > 0)
	{
		std::memcpy(data.data() + position, buffer, size);
	}
	position += size;
	return true;
}

bool PhobosByteStream::Read(void* buffer, size_t size)
{
	if (!buffer && size > 0) return false;
	if (position + size > data.size()) return false;

	if (size > 0)
	{
		std::memcpy(buffer, data.data() + position, size);
	}
	position += size;
	return true;
}

bool PhobosByteStream::WriteToStream(LPSTREAM stream) const
{
	if (!stream) return false;

	ULONG written = 0;

	// 1. Write start marker
	size_t startMarkerLen = START_MARKER_LEN;
	HRESULT hr = stream->Write(START_MARKER, startMarkerLen, &written);
	if (FAILED(hr) || written != startMarkerLen)
	{
		return false;
	}

	// 2. Write data size
	DWORD dataSize = static_cast<DWORD>(data.size());
	hr = stream->Write(&dataSize, sizeof(DWORD), &written);
	if (FAILED(hr) || written != sizeof(DWORD))
	{
		return false;
	}

	// 3. Write actual data
	if (dataSize > 0)
	{
		hr = stream->Write(data.data(), dataSize, &written);
		if (FAILED(hr) || written != dataSize)
		{
			return false;
		}
	}

	// 4. Write end marker
	size_t endMarkerLen = END_MARKER_LEN;
	hr = stream->Write(END_MARKER, endMarkerLen, &written);
	if (FAILED(hr) || written != endMarkerLen)
	{
		return false;
	}

	return true;
}

bool PhobosByteStream::ReadFromStream(LPSTREAM stream)
{
	if (!stream) return false;

	ULONG read = 0;

	// 1. Read and validate start marker
	size_t startMarkerLen = START_MARKER_LEN;
	std::vector<char> startBuffer(startMarkerLen + 1, 0);
	HRESULT hr = stream->Read(startBuffer.data(), startMarkerLen, &read);
	if (FAILED(hr) || read != startMarkerLen || strcmp(startBuffer.data(), START_MARKER) != 0)
	{
		return false;
	}

	// 2. Read data size
	DWORD dataSize = 0;
	hr = stream->Read(&dataSize, sizeof(DWORD), &read);
	if (FAILED(hr) || read != sizeof(DWORD))
	{
		return false;
	}

	// 3. Validate size is reasonable
	if (dataSize > 100 * 1024 * 1024)
	{ // 100MB sanity limit
		Debug::FatalErrorAndExit("Data Size of the PhobosByteStream is exceeding it size limit of 100mb");
		return false;
	}

	// 4. Read actual data
	data.resize(dataSize);
	position = 0;

	if (dataSize > 0)
	{
		hr = stream->Read(data.data(), dataSize, &read);
		if (FAILED(hr) || read != dataSize)
		{
			data.clear();
			return false;
		}
	}

	// 5. Read and validate end marker
	size_t endMarkerLen = END_MARKER_LEN;
	std::vector<char> endBuffer(endMarkerLen + 1, 0);
	hr = stream->Read(endBuffer.data(), endMarkerLen, &read);
	if (FAILED(hr) || read != endMarkerLen || strcmp(endBuffer.data(), END_MARKER) != 0)
	{
		data.clear();
		return false;
	}

	return true;
}

void PhobosByteStream::LogStreamInfo() const
{
	char msg[512];
	sprintf_s(msg, "[PhobosByteStream] START:'%s' SIZE:%zu END:'%s' TOTAL:%zu bytes\n",
			 START_MARKER, data.size(), END_MARKER, GetStreamSize());
	OutputDebugStringA(msg);
}

bool PhobosStreamReader::RegisterChange(void* newPtr)
{
	static_assert(sizeof(long) == sizeof(void*), "long and void* need to be of same size.");

	long oldPtr = 0;
	if (this->Load(oldPtr)) {
		if (SUCCEEDED(PhobosSwizzleManager.Here_I_Am_Dbg(oldPtr, newPtr, __FILE__, __LINE__, RCFunc, "Unkown")))
			return true;
		//GameDebugLog::Log("[PhobosStreamReader] Failed To RegisterChange for [%p] to [%p]", oldPtr, newPtr);
	}

	return false;
}
