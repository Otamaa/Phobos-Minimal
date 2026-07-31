#pragma once

#include "Blitters.h"

#include "AlphaCursor.h"

// ============================================================================
//  AlphaBlitContext.h
//
//  Replacement for the two TLS slots allocated in YR_GameInit_Pre:
//
//      dword_1028DB60  ("slot A") -> BufferedAlphaBlitter*  armed alpha blitter
//      dwTlsIndex      ("slot B") -> void*                  custom-blit CALLBACK
//
//  WHY TLS AND NOT REGISTERS*
//  Syringe's REGISTERS* is a snapshot of one instant at one patch site. Eight
//  hooks spread across DrawSHP / DoubleIntersectLock / Blit_With_RLE /
//  Blit_Plain needed shared per-draw state; they have no common `this`, no
//  common stack frame and no common call chain. TLS was the only thing that
//  survived *between* the patch sites. A plain global would have worked too -
//  TLS was chosen for thread safety.
//
//  DIFF: both slots become one `thread_local` object. TlsAlloc/TlsGetValue/
//  TlsSetValue disappear, and so does the "Out of index oc.\n" fatal path in
//  YR_GameInit_Pre.
//
//  WHO WRITES SLOT B: not present in any supplied dump. The DrawSHP handlers
//  only ever read it. Whatever does write it is installing a blit callback, so
//  it is almost certainly the same code that sets the 0x888/0x889 flag family
//  on the draw. Track down its producer before deleting the TLS index
//  from game init, or `Secondary` will silently stay null.
// ============================================================================
class AlphaBlitContext
{
public:
	// Holds the blitter pointer, exactly as the TLS slot did - it may be
	// either BufferedAlphaBlitter or RLEBufferedAlphaBlitter depending on
	// which DrawSHP handler armed it. The hooks only ever drive the cursor,
	// which sits at the same offset in both, so they go through
	// GetAlphaCursor() rather than committing to one type.
	void* Active { nullptr };                  // was dword_1028DB60
	// was dwTlsIndex. NOT a buffer - it carries a CustomPixelBlender or a
	// CustomSpanBlitter, and the blit-flag family decides which slot on
	// BlitterCustom it lands in. See BlitterCallbacks.h.
	void* Secondary { nullptr };

	// Several hooks test slot A for non-null and then operate on a *different*
	// object pulled from EBP or from the host stack frame. That is not a bug -
	// it is how the original arms and disarms the alpha pass.
	bool IsArmed() const { return this->Active != nullptr; }

	// FIX: this used to be a second copy of the free GetAlphaCursor() from
	// AlphaCursor.h - a non-static member that never touched `this`, and one
	// that hid the free function from unqualified lookup inside this class.
	// Replaced by the accessor the hooks actually want, which is the only
	// place `Active` needs converting.
	AlphaCursor* Cursor() const
	{
		return this->Active ? GetAlphaCursor(this->Active) : nullptr;
	}

	void Reset()
	{
		// handler 0x10052B10 clears BOTH slots
		this->Active = nullptr;
		this->Secondary = nullptr;
	}

	static AlphaBlitContext& Instance()
	{
		static thread_local AlphaBlitContext instance {};
		return instance;
	}
};
