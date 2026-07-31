#pragma once

// FIX: this header had no includes at all but uses size_t and unsigned char*.
// Compiling it first in a TU failed with "'size_t' does not name a type".
#include <cstddef>

class AlphaCursor
{
public:
	/* +0x00 */ unsigned char* Cursor;  // start of the current row, set by the hooks
	/* +0x04 */ unsigned char* Base;    // start of the frame's bounding box
	/* +0x08 */ int            Stride;  // = alpha SHP header Width

	// BUG (preserved): a null Cursor/Base silently halts the walk instead of
	// faulting, so the alpha overlay just disappears for that draw. The
	// consuming blitter agrees - BlitTransLucentBufferedAlphaZRead::Blit_Copy
	// opens with `if (!this[4]) return;`, so an unarmed blitter draws nothing
	// rather than falling back to opaque.
	//
	// OWNERSHIP: the hooks own the position; the blitters are read-only with
	// respect to it. Both Blit_Copy implementations load Cursor into a local
	// and walk THAT - there is no write-back to +0x10 anywhere in
	// 0x1004FF60..0x100500F9 or 0x1004D090..0x1004D2F8. So a blit leaves the
	// cursor exactly where the hooks put it and the relative row hooks
	// (`Cursor += Stride`) are correct rather than compounding.
	//
	// An earlier revision of this file carried a SUSPECT saying the blit
	// "CONSUMES Cursor" and that the relative hooks drift. That was wrong, and
	// it contradicted the note in BlitTransLucentBufferedAlphaZRead.h. Any
	// reimplementation must keep walking a local, never the member.

	void SeedFrom(int x, int y)   // handler 0x10052A20
	{
		if (this->Stride && this->Base)
			this->Cursor = this->Base + this->Stride * y + x;
	}

	void SeedRows(int rows)       // handler 0x100529D0 (cmovg: negatives clamp)
	{
		if (this->Base && this->Stride)
			this->Cursor = this->Base + this->Stride * (rows > 0 ? rows : 0);
	}

	void SkipRows(int rows)       // handler 0x10052A60
	{
		if (this->Stride && this->Cursor)
			this->Cursor += this->Stride * rows;
	}

	void NextRow()                // handlers 0x10052AE0 / 0x10052AA0
	{
		if (this->Stride && this->Cursor)
			this->Cursor += this->Stride;
	}
};

static_assert(sizeof(AlphaCursor) == 0x0C, "three dwords at +0x10");

// The triple sits at the same offset in both buffered blitters, in two
// unrelated hierarchies, which is also why one shared Set_Alpha_Cursor body
// (0x1004D080) serves both. The hooks receive an untyped pointer out of the
// TLS slot and drive the cursor through this, without caring which family
// armed the pass.
constexpr size_t AlphaCursorOffset = 0x10;

inline AlphaCursor* GetAlphaCursor(void* pBlitter)
{
	return reinterpret_cast<AlphaCursor*>(static_cast<char*>(pBlitter) + AlphaCursorOffset);
}
