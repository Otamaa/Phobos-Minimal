#pragma once

#include <FileFormats/SHP.h>
#include <Utilities/VectorHelper.h>

// ---------------------------------------------------------------------------
// Raw MSVC std::vector header, 0x0C bytes.
//
// Reconstructed from the DTOR: for each of the two triples the code does
//   if (First != Last) Last = First;    // vector::clear(), trivial element type
//   ArrayClear(&triple);                // _Tidy / deallocate, __thiscall
// and from the CTOR, which zeroes all six dwords individually (an inlined
// default vector ctor).
//
// VERIFY: element type is unknown, but it must be trivially destructible --
// otherwise MSVC would emit a destroy loop instead of the bare `Last = First`.
// ---------------------------------------------------------------------------
struct RawVectorHeader
{
	void* First;   // +0x00
	void* Last;    // +0x04
	void* End;     // +0x08

	bool empty() const { return this->First == this->Last; }

	// Inlined std::vector<T>::clear() for trivially-destructible T.
	// The redundant compare is in the original (1006FCD1 / 1006FCDC); kept.
	void ResetSize()
	{
		if (this->First != this->Last)
			this->Last = this->First;
	}
};

static_assert(sizeof(RawVectorHeader) == 0x0C, "RawVectorHeader must be 0x0C bytes.");

class Blitter;
class RLEBlitter;
class SHPExtData
{
public:
	static HelperedVector <SHPExtData*> Array;

public:

	SHPCaches* AttachedToObject {};
	SHPCaches* AlphaSHP {};
	RawVectorHeader Cache10 {};       // 0x10 .. 0x18
	RawVectorHeader Cache1C {};       // 0x1C .. 0x24

public:

	~SHPExtData() = default;
	SHPExtData() = default;

	bool LoadAlphaImage();

	static void FinalizeAlpha(SHPExtData* pExt, SHPCaches* pAlpha, SHPCaches* pShape);
};