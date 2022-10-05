#pragma once

#include <Base/Always.h>
#include <ASMMacros.h>

class WWCRCEngine
{
public:
	void Add(bool bIn)
	{
		JMP_THIS(0x4A1CA0);
	}

	unsigned int Add(int nVal)
	{
		JMP_THIS(0x4A1D50);
	}

protected:
	long CRC;
	int Index;
	union
	{
		long Composite;
		char Buffer[sizeof(long)];
	} StagingBuffer;
};