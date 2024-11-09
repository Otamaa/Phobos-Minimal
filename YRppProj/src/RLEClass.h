#pragma once

#include <YRAllocator.h>
#include <MemoryBuffer.h>

class RLEClass
{
public:
	RLEClass() {}

	int Compress(void *source, void *dest, int length); // JMP_THIS(0x661590);
	int Uncompress(void *source, void *dest); // JMP_THIS(0x661640);
};


inline int RLE_Compress(const MemoryBuffer &inbuff, const MemoryBuffer&outbuff)
{
	RLEClass rle;
	return rle.Compress(inbuff, outbuff, inbuff.Get_Size());
}


inline int RLE_Uncompress(const MemoryBuffer&inbuff, const MemoryBuffer&outbuff)
{
	RLEClass rle;
	return rle.Uncompress(inbuff, outbuff);
}