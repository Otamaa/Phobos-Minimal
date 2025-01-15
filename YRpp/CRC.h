#pragma once

#include <ASMMacros.h>
#include <Helpers/CompileTime.h>

class CRCEngine
{
public:
	int operator()() const
	{
		return Value();
	}
	// Commented out because Ares reimplemented it already!
	// 
	// void operator()(char datum)
	// {
	// 	StagingBuffer.Buffer[Index++] = datum;
	// 
	// 	if (Index == sizeof(int))
	// 	{
	// 		CRC = Value();
	// 		StagingBuffer.Composite = 0;
	// 		Index = 0;
	// 	}
	// }
	// 
	// int operator()(void* buffer, int length)
	// {
	// 	return (*this)((const void*)buffer, length);
	// }
	// 
	// int operator()(const void* buffer, int length)
	// {
	// 	if (buffer != nullptr && length > 0)
	// 	{
	// 		const char* dataptr = (char const*)buffer;
	// 		int bytes_left = length;
	// 
	// 		while (bytes_left && Buffer_Needs_Data())
	// 		{
	// 			operator()(*dataptr);
	// 			++dataptr;
	// 			--bytes_left;
	// 		}
	// 
	// 		const int* intptr = (const int*)dataptr;
	// 		int intcount = bytes_left / sizeof(int);
	// 		while (intcount--)
	// 		{
	// 			CRC = Memory(intptr, 4, CRC);
	// 			++intptr;
	// 			bytes_left -= sizeof(int);
	// 		}
	// 
	// 		dataptr = (char const*)intptr;
	// 		while (bytes_left)
	// 		{
	// 			operator()(*dataptr);
	// 			++dataptr;
	// 			--bytes_left;
	// 		}
	// 	}
	// 
	// 	return Value();
	// } 
	template<typename T>
	int operator()(const T& data)
	{
		return (*this)((const void*)&data, static_cast<int>(sizeof(data)));
	}

	unsigned int Compute_uchar(unsigned char datum)
	{
		JMP_THIS(0x4A1C10);
	}

	unsigned int Compute_bool(bool datum)
	{
		JMP_THIS(0x4A1CA0);
	}

	unsigned int Compute_short(short datum)
	{
		JMP_THIS(0x4A1D30);
	}

	unsigned int Compute_int(int datum)
	{
		JMP_THIS(0x4A1D50);
	}

	unsigned int Compute_dword(DWORD datum)
	{
		JMP_THIS(0x4A1D50);
	}

	unsigned int Compute_float(float datum)
	{
		JMP_THIS(0x4A1D70);
	}

	unsigned int Compute_double(double datum)
	{
		JMP_THIS(0x4A1D90);
	}

	operator int() const
	{
		return Value();
	}

	unsigned int operator()(char datum)
	{
		JMP_THIS(0x4A1C10);
	}

	unsigned int operator()(bool datum)
	{
		JMP_THIS(0x4A1CA0);
	}

	unsigned int operator()(short datum)
	{
		JMP_THIS(0x4A1D30);
	}

	unsigned int operator()(int datum)
	{
		JMP_THIS(0x4A1D50);
	}

	unsigned int operator()(float datum)
	{
		JMP_THIS(0x4A1D70);
	}

	unsigned int operator()(double datum)
	{
		JMP_THIS(0x4A1D90);
	}

	unsigned int operator()(const void* buffer, int length)
	{
		JMP_THIS(0x4A1DE0);
	}

	unsigned int operator()(const char* buffer)
	{
		if (!buffer)
			return 0u;

		return this->operator()(buffer, strlen(buffer));
	}

	static COMPILETIMEEVAL reference<unsigned int, 0x81F7B4, 256> const Table {};

	static int Memory(const void* data, int bytes, int crc)
	{
		auto buffer = reinterpret_cast<const unsigned char*>(data);
		unsigned int ret = ~crc;

		for (int i = 0; i < bytes; ++i)
			ret = (ret >> 8) ^ Table[*buffer++ ^ (ret & 0xFF)];

		return ~ret;
	}

	static int String(const char* buffer, int crc)
	{
		unsigned int ret = ~crc;

		while (*buffer)
			ret = (ret >> 8) ^ Table[*buffer++ ^ (ret & 0xFF)];

		return ~ret;
	}

protected:
	bool Buffer_Needs_Data() const
	{
		return Index != 0;
	}
#pragma warning( push )
#pragma warning (disable : 4244)
	int Value() const
	{
		if (!Buffer_Needs_Data())
			return CRC;

		(char&)StagingBuffer.Buffer[Index] = Index;
		for (int i = Index + 1; i < 4; ++i)
			(char&)StagingBuffer.Buffer[i] = this->StagingBuffer.Buffer[0];
		return Memory(StagingBuffer.Buffer, 4, CRC);
	}
#pragma warning( pop )

public:
	int CRC;
	int Index;
	union
	{
		int Composite;
		char Buffer[sizeof(int)];
	} StagingBuffer;
};

// when using the original game implementation, use this to allocate space on
// the stack. this class has a byte of padding to prevent out of bounds writes.
class SafeChecksummer : public CRCEngine
{
public:
	SafeChecksummer() : CRCEngine() { }

protected:
	/*
	* this is not entirely correct
	* the original class doesn't have this member and as such its sizeof == 0xC,
	* but the code writes to the 0xC'th byte anyway... when the class is
	* allocated through the heap, it works because windows apparently aligns
	* memory blocks to 8 byte boundaries but when it's allocated on the stack,
	* all hell breaks loose
	*/
	BYTE  Padding;
};