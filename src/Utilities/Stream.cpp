#include "Stream.h"
#include "Debug.h"

#include <Utilities/Swizzle.h>

#include <Objidl.h>

const char* RCFunc = "PhobosStreamReader::RegisterChange()";

bool PhobosByteStream::Write(const void* buffer, size_t size)
{
	if (!ValidateParameters(buffer, size, "Write")) return false;
	if (size == 0) return true;

	// Check for potential overflow
	size_t newPosition = position + size;
	if (newPosition < position)
	{
		GameDebugLog::Log("[PhobosByteStream::Write] Position overflow detected: pos=%zu, size=%zu\n", position, size);
		DebugBreak();
		return false;
	}

	// Additional safety check for the new position
	if (newPosition > MAX_REASONABLE_SIZE)
	{
		GameDebugLog::Log("[PhobosByteStream::Write] New position %zu exceeds maximum %zu\n", newPosition, MAX_REASONABLE_SIZE);
		DebugBreak();
		return false;
	}

	// Ensure we have enough space
	if (newPosition > data.size())
	{
		try
		{
			size_t minNeeded = newPosition;
			size_t newCapacity;

			// Calculate new capacity
			if (minNeeded <= 1024)
			{
				newCapacity = 1024;
			}
			else
			{
				newCapacity = minNeeded + std::min(minNeeded / 2, size_t(4096));
			}

			// Cap the growth
			if (newCapacity > MAX_REASONABLE_SIZE)
			{
				newCapacity = std::min(MAX_REASONABLE_SIZE, minNeeded + 1024);
			}

			// Final check
			if (minNeeded > newCapacity)
			{
				GameDebugLog::Log("[PhobosByteStream::Write] Cannot satisfy allocation: needed=%zu, max_capacity=%zu\n",
					minNeeded, newCapacity);
				DebugBreak();
				return false;
			}

			// Always reserve first, then resize
			data.reserve(newCapacity);
			data.resize(minNeeded);

			// CRITICAL: Verify the resize actually worked
			if (data.size() != minNeeded)
			{
				GameDebugLog::Log("[PhobosByteStream::Write] CRITICAL ERROR: Resize failed! Expected %zu, got %zu\n",
					minNeeded, data.size());
				DebugBreak();
				return false;
			}
		}
		catch (const std::bad_alloc& e)
		{
			GameDebugLog::Log("[PhobosByteStream::Write] Memory allocation failed for %zu bytes: %s\n", newPosition, e.what());
			DebugBreak();
			return false;
		}
		catch (const std::exception& e)
		{
			GameDebugLog::Log("[PhobosByteStream::Write] Buffer resize failed: %s\n", e.what());
			DebugBreak();
			return false;
		}
	}

	// Critical safety check before memcpy
	if (position + size > data.size())
	{
		GameDebugLog::Log("[PhobosByteStream::Write] CRITICAL ERROR: Buffer too small for memcpy! pos=%zu, size=%zu, data_size=%zu\n",
			position, size, data.size());
		DebugBreak();
		return false;
	}

	// Perform the copy
	try
	{
		std::memcpy(data.data() + position, buffer, size);
		position = newPosition;

		// Update checksum if integrity checking is enabled
		if (integrity_check_enabled)
		{
			last_checksum = CalculateChecksum();
		}

		return true;
	}
	catch (...)
	{
		GameDebugLog::Log("[PhobosByteStream::Write] Memcpy failed: pos=%zu, size=%zu, data_size=%zu\n",
			position, size, data.size());
		DebugBreak();
		return false;
	}
}

bool PhobosByteStream::Read(void* buffer, size_t size)
{
	if (!ValidateParameters(buffer, size, "Read")) return false;
	if (size == 0) return true;

	// Check if data vector is empty
	if (data.size() == 0)
	{
		GameDebugLog::Log("[PhobosByteStream::Read] CRITICAL ERROR: Data vector is empty! Cannot read anything.\n");
		DebugBreak();
		return false;
	}

	// Check bounds
	if (position > data.size())
	{
		GameDebugLog::Log("[PhobosByteStream::Read] Position %zu beyond buffer size %zu\n", position, data.size());
		DebugBreak();
		return false;
	}

	if (size > data.size() - position)
	{
		GameDebugLog::Log("[PhobosByteStream::Read] Read beyond buffer bounds: pos=%zu, size=%zu, data_size=%zu, available=%zu\n",
			position, size, data.size(), data.size() - position);
		DebugBreak();
		return false;
	}

	// Additional safety check
	if (data.data() == nullptr)
	{
		GameDebugLog::Log("[PhobosByteStream::Read] CRITICAL ERROR: Data pointer is null!\n");
		DebugBreak();
		return false;
	}

	// Perform the copy
	try
	{
		std::memcpy(buffer, data.data() + position, size);
		position += size;
		return true;
	}
	catch (...)
	{
		GameDebugLog::Log("[PhobosByteStream::Read] Memcpy failed for %zu bytes at position %zu\n", size, position);
		DebugBreak();
		return false;
	}
}

bool PhobosByteStream::WriteToStream(LPSTREAM stream) const
{
	if (!stream)
	{
		GameDebugLog::Log("[PhobosByteStream::WriteToStream] Null stream pointer\n");
		DebugBreak();
		return false;
	}

	ULONG written = 0;
	HRESULT hr;

	// 1. Write start marker
	hr = stream->Write(START_MARKER, static_cast<ULONG>(START_MARKER_LEN), &written);
	if (FAILED(hr) || written != START_MARKER_LEN)
	{
		GameDebugLog::Log("[PhobosByteStream::WriteToStream] Failed to write start marker\n");
		DebugBreak();
		return false;
	}

	// 2. Write data size
	DWORD dataSize = static_cast<DWORD>(data.size());
	hr = stream->Write(&dataSize, sizeof(DWORD), &written);
	if (FAILED(hr) || written != sizeof(DWORD))
	{
		GameDebugLog::Log("[PhobosByteStream::WriteToStream] Failed to write data size\n");
		DebugBreak();
		return false;
	}

	// 3. Write checksum for integrity
	uint32_t checksum = CalculateChecksum();
	hr = stream->Write(&checksum, sizeof(uint32_t), &written);
	if (FAILED(hr) || written != sizeof(uint32_t))
	{
		GameDebugLog::Log("[PhobosByteStream::WriteToStream] Failed to write checksum\n");
		DebugBreak();
		return false;
	}

	// 4. Write actual data
	if (dataSize > 0)
	{
		hr = stream->Write(data.data(), dataSize, &written);
		if (FAILED(hr) || written != dataSize)
		{
			GameDebugLog::Log("[PhobosByteStream::WriteToStream] Failed to write data\n");
			DebugBreak();
			return false;
		}
	}

	// 5. Write end marker
	hr = stream->Write(END_MARKER, static_cast<ULONG>(END_MARKER_LEN), &written);
	if (FAILED(hr) || written != END_MARKER_LEN)
	{
		GameDebugLog::Log("[PhobosByteStream::WriteToStream] Failed to write end marker\n");
		DebugBreak();
		return false;
	}

	return true;
}

bool PhobosByteStream::ReadFromStream(LPSTREAM stream)
{
	if (!stream)
	{
		GameDebugLog::Log("[PhobosByteStream::ReadFromStream] Null stream pointer\n");
		DebugBreak();
		return false;
	}

	ULONG read = 0;
	HRESULT hr;

	// Clear existing data
	Reset();

	// 1. Read and validate start marker
	std::vector<char> startBuffer(START_MARKER_LEN + 1, 0);
	hr = stream->Read(startBuffer.data(), static_cast<ULONG>(START_MARKER_LEN), &read);
	if (FAILED(hr) || read != START_MARKER_LEN || strcmp(startBuffer.data(), START_MARKER) != 0)
	{
		GameDebugLog::Log("[PhobosByteStream::ReadFromStream] Failed to read start marker\n");
		DebugBreak();
		return false;
	}

	// 2. Read data size
	DWORD dataSize = 0;
	hr = stream->Read(&dataSize, sizeof(DWORD), &read);
	if (FAILED(hr) || read != sizeof(DWORD))
	{
		GameDebugLog::Log("[PhobosByteStream::ReadFromStream] Failed to read data size\n");
		DebugBreak();
		return false;
	}

	// 3. Validate size is reasonable
	if (dataSize > MAX_REASONABLE_SIZE)
	{
		GameDebugLog::Log("[PhobosByteStream::ReadFromStream] Data size %lu exceeds limit %zu\n",
			dataSize, MAX_REASONABLE_SIZE);
		DebugBreak();
		return false;
	}

	// 4. Read checksum
	uint32_t stored_checksum = 0;
	hr = stream->Read(&stored_checksum, sizeof(uint32_t), &read);
	if (FAILED(hr) || read != sizeof(uint32_t))
	{
		GameDebugLog::Log("[PhobosByteStream::ReadFromStream] Failed to read checksum\n");
		DebugBreak();
		return false;
	}

	// 5. Read actual data
	try
	{
		data.resize(dataSize);
		position = 0;

		if (dataSize > 0)
		{
			hr = stream->Read(data.data(), dataSize, &read);
			if (FAILED(hr) || read != dataSize)
			{
				GameDebugLog::Log("[PhobosByteStream::ReadFromStream] Failed to read data\n");
				DebugBreak();
				Reset();
				return false;
			}
		}
	}
	catch (const std::exception& e)
	{
		GameDebugLog::Log("[PhobosByteStream::ReadFromStream] Failed to allocate buffer: %s\n", e.what());
		DebugBreak();
		Reset();
		return false;
	}

	// 6. Verify checksum
	if (integrity_check_enabled)
	{
		uint32_t calculated_checksum = CalculateChecksum();
		if (calculated_checksum != stored_checksum)
		{
			GameDebugLog::Log("[PhobosByteStream::ReadFromStream] Checksum mismatch: expected 0x%x, got 0x%x\n",
				stored_checksum, calculated_checksum);
			DebugBreak();
			Reset();
			return false;
		}
		last_checksum = calculated_checksum;
	}

	// 7. Read and validate end marker
	std::vector<char> endBuffer(END_MARKER_LEN + 1, 0);
	hr = stream->Read(endBuffer.data(), static_cast<ULONG>(END_MARKER_LEN), &read);
	if (FAILED(hr) || read != END_MARKER_LEN || strcmp(endBuffer.data(), END_MARKER) != 0)
	{
		GameDebugLog::Log("[PhobosByteStream::ReadFromStream] Failed to read end marker\n");
		DebugBreak();
		Reset();
		return false;
	}

	return true;
}

void PhobosByteStream::LogStreamInfo() const
{
	char msg[1024];
	sprintf_s(msg,
		"[PhobosByteStream] START:'%s' SIZE:%zu CHECKSUM:0x%x END:'%s' TOTAL:%zu bytes INTEGRITY:%s\n",
		START_MARKER, data.size(), last_checksum, END_MARKER, GetStreamSize(),
		integrity_check_enabled ? "ENABLED" : "DISABLED");
	OutputDebugStringA(msg);

	if (data.size() > 0)
	{
		std::string hex_dump;
		size_t dump_size = std::min(data.size(), size_t(32));
		for (size_t i = 0; i < dump_size; ++i)
		{
			char buf[4];
			sprintf_s(buf, "%02X ", data[i]);
			hex_dump += buf;
		}
		sprintf_s(msg, "[PhobosByteStream] First %zu bytes: %s%s\n",
			dump_size, hex_dump.c_str(), data.size() > dump_size ? "..." : "");
		OutputDebugStringA(msg);
	}
}

bool PhobosStreamReader::RegisterChange(void* newPtr)
{
	static_assert(sizeof(long) == sizeof(void*), "long and void* need to be of same size.");

	long oldPtr = 0;
	if (this->Load(oldPtr))
	{
		HRESULT result = PhobosSwizzleManager.Here_I_Am_Dbg(oldPtr, newPtr, __FILE__, __LINE__, RCFunc, "Unknown");

		if (SUCCEEDED(result))
		{
			return true;
		}

		GameDebugLog::Log("[PhobosStreamReader] Failed to RegisterChange from %p to %p: HRESULT=0x%x\n",
			reinterpret_cast<void*>(oldPtr), newPtr, result);
		DebugBreak();
	}
	else
	{
		GameDebugLog::Log("[PhobosStreamReader] Failed to load old pointer for RegisterChange at offset %zu\n",
			stream ? stream->Offset() : 0);
		DebugBreak();
	}

	return false;
}