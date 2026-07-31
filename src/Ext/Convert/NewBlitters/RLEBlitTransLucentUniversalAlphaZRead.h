#pragma once

#include "../Blitters/Blitter.h"
#include "BlitterDetail.h"

template<typename T, int N>
class RLEBlitTransLucentUniversalAlphaZRead final : public RLEBlitter
{
public:
	inline explicit RLEBlitTransLucentUniversalAlphaZRead(T* data, int shadecount) noexcept
	{
		PaletteData = data;
		AlphaRemapper = AlphaLightingRemapClass::Global->FindOrAllocate(shadecount);
	}

	virtual ~RLEBlitTransLucentUniversalAlphaZRead() override final = default;

	virtual void Blit_Copy(void* dst, byte* src, int len, int line, int zbase,
		WORD* zbuf, WORD* abuf, int alvl, int warp, byte* zadjust) override final
	{
		// `warp` is never referenced. It stays in the signature for vtable
		// compatibility only.
		(void)warp;

		// this+0x08 + (level << 9). A row is 256 WORDs = 512 bytes, so the
		// shift is just Table[level] and Table sits at offset 0 of the remapper.
		const WORD* adata = AlphaRemapper->Table[BlitterDetail::Lookup_Level(alvl)];

		auto dest = reinterpret_cast<T*>(dst);

		Skip_Leading_Lines(dest, src, len, line, zbuf, abuf);
		Blit_Spans(dest, src, len, zbase, zbuf, abuf, zadjust, adata);

		// NOT PORTED: the original leaves the advanced src cursor in EAX and
		// IDA types the function `unsigned __int8*`. The vtable slot is void
		// and no caller reads it. Dropped.
	}

	// The original's slot 2 is 0x1004C4F0 for EVERY RLE blitter - one shared
	// body, unlike the plain family where slot 2 is a distinct per-level
	// address. Vanilla's RLE headers forward and discard the tint, which is
	// exactly what a single shared body would look like.
	// VERIFY: 0x1004C4F0 was not disassembled; this is inference from the
	//         sharing pattern plus the vanilla source.
	virtual void Blit_Copy_Tinted(void* dst, byte* src, int len, int line, int zbase,
		WORD* zbuf, WORD* abuf, int alvl, int warp, byte* zadjust, WORD tint) override final
	{
		(void)tint;
		Blit_Copy(dst, src, len, line, zbase, zbuf, abuf, alvl, warp, zadjust);
	}

private:
	using Weights = BlitterDetail::UniversalWeights<N>;

	// -----------------------------------------------------------------------
	//  Vanilla's Process_Pre_Lines, inlined by the compiler.
	//
	//  Walks the RLE stream forward until it has consumed `line` pixels, then
	//  advances every cursor by the overshoot. Note that a run can straddle
	//  the boundary, so the overshoot is generally positive, not zero.
	//
	//  BUG in the YRpp reference implementation: Process_Pre_Lines takes zbuf
	//  and abuf BY VALUE while dest/src/len are by reference, so its `zbuf +=
	//  off` and `abuf += off` are discarded and the caller's main loop starts
	//  from unadvanced buffers. The DLL keeps them live, as here.
	// -----------------------------------------------------------------------
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

	// -----------------------------------------------------------------------
	//  Vanilla's Process_Pixel_Datas, inlined.
	//
	//  A non-zero source byte is one opaque pixel; a zero byte introduces a
	//  transparent run whose length is the next byte. Either way every cursor
	//  advances by the same pixel count, so the two branches only differ in
	//  how that count is obtained and whether anything is drawn.
	// -----------------------------------------------------------------------
	void Blit_Spans(T* dest, byte* src, int len, int zbase,
		WORD* zbuf, WORD* abuf, byte* zadjust, const WORD* adata) const
	{
		while (len > 0)
		{
			int step;

			if (const byte srcv = *src++)
			{
				// Signed 32-bit compare; *zadjust and *zbuf are both
				// zero-extended before the subtraction.
				if (zbase - *zadjust < *zbuf)
				{
					// SUSPECT: *abuf is a full WORD but a remap row is only
					// 256 entries, so a large alpha value indexes into the
					// next row. Vanilla writes the identical expression.
					const WORD colour = PaletteData[srcv | adata[*abuf]];

					*dest = BlitterDetail::Pack<Weights::Shift>(
						  Weights::Dest * BlitterDetail::Spread(*dest)
						+ Weights::Src * BlitterDetail::Spread(colour));
				}

				step = 1;
			}
			else
			{
				step = *src++;
			}

			// SUSPECT: the original derives one BYTE stride for dest, zbuf and
			// abuf alike (`v27 = 2 * step`), so this instantiation is 16bpp-
			// only. A T of any other width would desynchronise dest from the
			// two buffers. Expressed here in elements, which is equivalent for
			// T = WORD and clearer about the assumption.
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
