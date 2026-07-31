#pragma once

#include "BlitterCustom.h"

// ============================================================================
//  BlitterCustomRLE
//
//  Backported from 0x1004B440 (Blit_Copy) and 0x1004B630 (Blit_Copy_Tinted).
//  Same two escape hatches as BlitterCustom - see that header.
//
//  Its Blit_Copy_Tinted is NOT the shared 0x1004C4F0 forwarder every other RLE
//  blitter uses; it has its own body at 0x1004B630, which checks SpanBlitter
//  first and only then falls back to `(*vtable[1])(...)` - a virtual re-entry
//  into its own Blit_Copy, dropping the tint. Same end result, different
//  mechanism, and worth knowing because the tint IS forwarded to a span
//  callback while it is discarded on the fallback path.
//
//  Unlike the plain sibling, both entry points compute the remap level
//  correctly.
// ============================================================================

class BlitterCustomRLE final : public RLEBlitter
{
public:
	inline explicit BlitterCustomRLE(WORD* data, int shadecount) noexcept
	{
		PaletteData = data;
		AlphaRemapper = AlphaLightingRemapClass::Global->FindOrAllocate(shadecount);
		Mask = 0;

		PixelBlender = nullptr;
		SpanBlitter = nullptr;
	}

	virtual ~BlitterCustomRLE() override final = default;

	// -----------------------------------------------------------------------
	virtual void Blit_Copy(void* dst, byte* src, int len, int line, int zbase,
		WORD* zbuf, WORD* abuf, int alvl, int warp, byte* zadjust) override final
	{
		if (SpanBlitter)
		{
			SpanBlitter(dst, src, len, line, zbase,
				zbuf, abuf, alvl, zadjust, 0, AlphaRemapper, PaletteData);
			return;
		}

		// DIFF: clamps the full 32-bit alvl at both ends rather than
		// truncating to 16 bits first, so this is not quite
		// BlitterDetail::Lookup_Level either. Equivalent over [0, 2000].
		int level = 261 * (alvl < 0 ? 0 : alvl) >> 11;

		if (level > 254)
			level = 254;

		const WORD* adata = AlphaRemapper->Table[level];

		auto dest = static_cast<WORD*>(dst);

		Skip_Leading_Lines(dest, src, len, line, zbuf, abuf);
		Blit_Spans(dest, src, len, zbase, zbuf, abuf, zadjust, adata);
	}

	// -----------------------------------------------------------------------
	virtual void Blit_Copy_Tinted(void* dst, byte* src, int len, int line, int zbase,
		WORD* zbuf, WORD* abuf, int alvl, int warp, byte* zadjust, WORD tint) override final
	{
		if (SpanBlitter)
		{
			// The tint DOES reach a span callback.
			SpanBlitter(dst, src, len, line, zbase,
				zbuf, abuf, alvl, zadjust, tint, AlphaRemapper, PaletteData);
			return;
		}

		// The original re-enters through the vtable rather than calling
		// Blit_Copy directly - `call dword ptr [eax+4]`. Behaviourally the
		// same for a final class; written as a direct call.
		// The tint is DISCARDED on this path.
		Blit_Copy(dst, src, len, line, zbase, zbuf, abuf, alvl, warp, zadjust);
	}

private:
	// See RLEBlitTransLucentUniversalAlphaZRead.h for the by-value bug this
	// avoids in the YRpp reference Process_Pre_Lines.
	static void Skip_Leading_Lines(WORD*& dest, byte*& src, int& len, int line,
		WORD*& zbuf, WORD*& abuf)
	{
		if (line <= 0)
			return;

		int off = -line;

		do
		{
			if (*src++)
				++off;
			else
				off += *src++;
		}
		while (off < 0);

		dest += off;
		len -= off;
		zbuf += off;
		abuf += off;

		ZBuffer::Instance->AdjustPointer(zbuf);
		ABuffer::Instance->AdjustPointer(abuf);
	}

	void Blit_Spans(WORD* dest, byte* src, int len, int zbase,
		WORD* zbuf, WORD* abuf, byte* zadjust, const WORD* adata) const
	{
		while (len > 0)
		{
			int step;

			if (const byte srcv = *src++)
			{
				// Signed 32-bit compare, as in every RLE blitter here.
				if (zbase - *zadjust < *zbuf)
				{
					const WORD colour = PaletteData[srcv | adata[*abuf]];

					if (PixelBlender)
						*dest = PixelBlender(colour, *dest);
				}

				step = 1;
			}
			else
			{
				step = *src++;
			}

			len -= step;
			zadjust += step;
			dest += step;
			zbuf += step;
			abuf += step;

			ZBuffer::Instance->AdjustPointer(zbuf);
			ABuffer::Instance->AdjustPointer(abuf);
		}
	}

	WORD* PaletteData;                      // +0x04
	AlphaLightingRemapClass* AlphaRemapper; // +0x08
	WORD Mask;                              // +0x0C, never read
	WORD Padding;

public:
	CustomPixelBlender PixelBlender;        // +0x10, family 0x88800000
	CustomSpanBlitter SpanBlitter;          // +0x14, family 0x88900000
};

static_assert(sizeof(BlitterCustomRLE) == 0x18, "must match operator new(0x18)");