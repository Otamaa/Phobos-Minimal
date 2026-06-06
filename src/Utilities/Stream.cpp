#include "Stream.h"
#include "Debug.h"

#include <Utilities/Swizzle.h>

#include <Objidl.h>
#include <Phobos.Defines.h>

bool PhobosStreamReader::RegisterChange(void* newPtr)
{
	static_assert(sizeof(long) == sizeof(void*), "long and void* need to be of same size.");

	long oldPtr = 0;
	if (this->Load(oldPtr))
	{
		HRESULT result = PHOBOS_SWIZZLE_REGISTER_POINTER(oldPtr, newPtr, "unknown");

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