#pragma once

// ============================================================================
//  RLEBlitTransLucentBufferedAlphaZRead<T>
//
//  Backported from 0x1004D090. Slot 1 of a 4-entry vtable; slot 2 is the
//  shared RLE Blit_Copy_Tinted forwarder at 0x1004C4F0 and slot 3 is
//  Set_Alpha_Cursor at 0x1004D080, the same body the plain buffered blitter
//  uses.
//
//  THE CURSOR IS IN SOURCE SPACE, NOT DESTINATION SPACE
//  ----------------------------------------------------
//  This function is what proves it. In the leading-line skip, the cursor and
//  the destination advance by DIFFERENT amounts:
//
//      dest, zbuf, abuf  +=  off               (destination pixels)
//      Cursor            +=  line + off        (source pixels consumed)
//
//  and `line + off` is exactly how many source pixels the RLE decoder walked.
//  In the main loop both advance by the same step, so the two only diverge
//  across the clip. The alpha image therefore indexes by position within the
//  SHAPE, which is why SHPReference_CTOR rejects an .APH whose Width, Height
//  and FrameCount do not match the shape it is attached to.
//
//  The plain buffered blitter agrees, less obviously: there `*Cursor++` and
//  `*src++` sit in the same branch and always move together.
// ============================================================================

#include "../Blitters/Blitter.h"
// FIX: this file calls BlitterDetail::Lookup_Level / Spread / Pack but only
// included AlphaCursor.h - it compiled solely on include order.
#include "BlitterDetail.h"
#include "AlphaCursor.h"

template<typename T>
class RLEBlitTransLucentBufferedAlphaZRead final : public RLEBlitter
{
public:
	inline explicit RLEBlitTransLucentBufferedAlphaZRead(T* data, int shadecount) noexcept
	{
		PaletteData = data;
		AlphaRemapper = AlphaLightingRemapClass::Global->FindOrAllocate(shadecount);
		Mask = 0;

		Alpha = AlphaCursor { nullptr, nullptr, 0 };
	}

	virtual ~RLEBlitTransLucentBufferedAlphaZRead() override final = default;

	virtual void Blit_Copy(void* dst, byte* src, int len, int line, int zbase,
		WORD* zbuf, WORD* abuf, int alvl, int warp, byte* zadjust) override final
	{
		(void)warp;

		// The whole body is inside `if (this[4])`. An unarmed blitter draws
		// nothing rather than falling back to opaque.
		if (!Alpha.Cursor)
			return;

		const WORD* adata = AlphaRemapper->Table[BlitterDetail::Lookup_Level(alvl)];

		auto dest = reinterpret_cast<T*>(dst);
		byte* cursor = Alpha.Cursor;

		Skip_Leading_Lines(dest, src, len, line, zbuf, abuf, cursor);
		Blit_Spans(dest, src, len, zbase, zbuf, abuf, zadjust, cursor, adata);

		// The original walks this[4] through a stack copy and never writes the
		// advanced value back, so the member still holds whatever the hooks
		// last set when the call returns. Mirrored by keeping the walk local.
		// The plain buffered blitter does the same - see AlphaCursor.h.
	}

	// Slot 2 is 0x1004C4F0 for every RLE blitter - one shared forwarder that
	// discards the tint, matching vanilla's RLE headers.
	virtual void Blit_Copy_Tinted(void* dst, byte* src, int len, int line, int zbase,
		WORD* zbuf, WORD* abuf, int alvl, int warp, byte* zadjust, WORD tint) override final
	{
		(void)tint;
		Blit_Copy(dst, src, len, line, zbase, zbuf, abuf, alvl, warp, zadjust);
	}

	// NOT `override` - RLEBlitter declares no such method. A new virtual in
	// slot 3, sharing its body (0x1004D080) with the plain buffered blitter
	// because Cursor is at +0x10 in both.
	virtual void Set_Alpha_Cursor(byte* cursor)
	{
		Alpha.Cursor = cursor;
	}

private:
	static constexpr DWORD AlphaSteps = 32;
	static constexpr int AlphaShift = 5;

	static void Skip_Leading_Lines(T*& dest, byte*& src, int& len, int line,
		WORD*& zbuf, WORD*& abuf, byte*& cursor)
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

		// The decoder consumed `line + off` source pixels to reach this point;
		// only `off` of them land on the destination. See the header comment.
		cursor += line + off;

		ZBuffer::Instance->AdjustPointer(zbuf);
		ABuffer::Instance->AdjustPointer(abuf);
	}

	void Blit_Spans(T* dest, byte* src, int len, int zbase, WORD* zbuf, WORD* abuf,
		byte* zadjust, byte* cursor, const WORD* adata) const
	{
		while (len > 0)
		{
			int step;

			if (const byte srcv = *src++)
			{
				// Signed 32-bit compare, as in the RLE Universal blitter.
				if (zbase - *zadjust < *zbuf)
				{
					const WORD destv = static_cast<WORD>(*dest);
					const WORD colour = PaletteData[srcv | adata[*abuf]];

					// DIFF: no thresholds here. The plain buffered blitter
					// short-circuits on `alpha < 2` (skip) and `alpha > 30`
					// (store unblended); this one always runs the blend. With
					// alpha 0 or 31 the result is near enough either way, so
					// the visible difference is cost, not output.
					const DWORD alpha = static_cast<DWORD>(*cursor) >> 3;

					*dest = BlitterDetail::Pack<AlphaShift>(
						  alpha * BlitterDetail::Spread(colour)
						+ (AlphaSteps - alpha) * BlitterDetail::Spread(destv));
				}

				step = 1;
			}
			else
			{
				step = *src++;
			}

			// Everything advances by the same pixel count, cursor included -
			// no desynchronisation, unlike the plain buffered blitter where
			// abuf moves only on drawn pixels.
			len -= step;
			zadjust += step;
			cursor += step;
			dest += step;
			zbuf += step;
			abuf += step;

			ZBuffer::Instance->AdjustPointer(zbuf);
			ABuffer::Instance->AdjustPointer(abuf);
		}
	}

	// Layout mirrors the DLL object: PaletteData +0x04, AlphaRemapper +0x08,
	// Mask +0x0C (never read), cursor triple at +0x10.
	T* PaletteData;
	AlphaLightingRemapClass* AlphaRemapper;
	WORD Mask;
	WORD Padding;

public:
	AlphaCursor Alpha;
};
