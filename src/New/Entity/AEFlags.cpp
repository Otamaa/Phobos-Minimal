#include "AEFlags.h"

#include <Utilities/SaveGame.h>

bool AEFlags::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	for (int i = 0; i < BitCount; ++i)
	{
		bool bit = (bits >> i) & 1;

		if (!Stm.Process(bit).Success())
		{
			return false;
		}

		bits = (bits & ~(1u << i)) | (uint32_t(bit) << i);
	}

	return true;
}

bool AEFlags::Save(PhobosStreamWriter& Stm) const
{
	for (int i = 0; i < BitCount; ++i)
	{
		bool bit = (bits >> i) & 1;

		if (!Stm.Process(bit).Success())
		{
			return false;
		}
	}

	return true;
}
