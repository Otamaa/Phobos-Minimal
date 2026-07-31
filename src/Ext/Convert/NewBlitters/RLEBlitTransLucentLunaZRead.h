#pragma once

// ============================================================================
//  RLEBlitTransLucentLunaZRead<T>
//
//  Backported from 0x1004C520. Same combine as the plain sibling - see
//  BlitTransLucentLunaZRead.h and BlitterMath.h.
//
//  As in the RLE Multiply, the pseudocode appears to fetch the palette entry
//  twice; the asm reads it once. IDA failed to common the subexpression.
//
//  The walk behaves like every other RLE blitter here: `len > 0` guards, src
//  always advanced, every cursor moving by the same pixel step.
// ============================================================================

#include "../Blitters/Blitter.h"
#include "BlitterDetail.h"

template<typename T>
class RLEBlitTransLucentLunaZRead final : public RLEBlitter
{
public:
	inline explicit RLEBlitTransLucentLunaZRead(T* data, int shadecount) noexcept
	{
		PaletteData = data;
		AlphaRemapper = AlphaLightingRemapClass::Global->FindOrAllocate(shadecount);
	}

	virtual ~RLEBlitTransLucentLunaZRead() override final = default;

	virtual void Blit_Copy(void* dst, byte* src, int len, int line, int zbase,
		WORD* zbuf, WORD* abuf, int alvl, int warp, byte* zadjust) override final
	{
		(void)warp;

		const WORD* adata = AlphaRemapper->Table[BlitterDetail::Lookup_Level(alvl)];

		auto dest = reinterpret_cast<T*>(dst);

		Skip_Leading_Lines(dest, src, len, line, zbuf, abuf);
		Blit_Spans(dest, src, len, zbase, zbuf, abuf, zadjust, adata);

		// NOT PORTED: the original leaves a stale ABuffer value in EAX.
	}

	// Slot 2 is the shared 0x1004C4F0 forwarder for every RLE blitter.
	virtual void Blit_Copy_Tinted(void* dst, byte* src, int len, int line, int zbase,
		WORD* zbuf, WORD* abuf, int alvl, int warp, byte* zadjust, WORD tint) override final
	{
		(void)tint;
		Blit_Copy(dst, src, len, line, zbase, zbuf, abuf, alvl, warp, zadjust);
	}

private:
	// See RLEBlitTransLucentUniversalAlphaZRead.h for the by-value bug this
	// avoids in the YRpp reference Process_Pre_Lines.
	static void Skip_Leading_Lines(T*& dest, byte*& src, int& len, int line,
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

	void Blit_Spans(T* dest, byte* src, int len, int zbase,
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

					*dest = BlitterDetail::Pack565(
						BlitterDetail::Luna888(
							BlitterDetail::Expand888(static_cast<WORD>(*dest)),
							BlitterDetail::Expand888(colour)));
				}

				step = 1;
			}
			else
			{
				step = *src++;
			}

			// SUSPECT: the original derives one BYTE stride for dest, zbuf and
			// abuf alike (`2 * step`), so this instantiation is 16bpp-only.
			len -= step;
			zadjust += step;
			dest += step;
			zbuf += step;
			abuf += step;

			ZBuffer::Instance->AdjustPointer(zbuf);
			ABuffer::Instance->AdjustPointer(abuf);
		}
	}

	T* PaletteData;
	AlphaLightingRemapClass* AlphaRemapper;
};