#pragma once

#include "../Blitters/Blitter.h"
#include "BlitterDetail.h"

template<typename T, int N>
class BlitTransLucentUniversalAlphaZRead final : public Blitter
{
public:
	inline explicit BlitTransLucentUniversalAlphaZRead(T* data, int shadecount) noexcept
	{
		PaletteData = data;
		AlphaRemapper = AlphaLightingRemapClass::Global->FindOrAllocate(shadecount);

		// NOTE: the DLL's allocator also stores a WORD 0 at +0x0C. That field
		// is the `Mask` every other blitter family carries, and this function
		// never reads it - confirmed by the disassembly, which only ever
		// touches this+0x04 and this+0x08. Not declared here.
	}

	virtual ~BlitTransLucentUniversalAlphaZRead() override final = default;

	virtual void Blit_Copy(void* dst, byte* src, int len, int zval,
		WORD* zbuf, WORD* abuf, int alvl, int warp) override final
	{
		Blit<false>(dst, src, len, zval, zbuf, abuf, alvl, warp, 0);
	}

	virtual void Blit_Copy_Tinted(void* dst, byte* src, int len, int zval,
		WORD* zbuf, WORD* abuf, int alvl, int warp, WORD tint) override final
	{
		Blit<true>(dst, src, len, zval, zbuf, abuf, alvl, warp, tint);
	}

	// VERIFY: in .rdata every plain blitter shares 0x1004B400 / 0x1004B430 for
	// these two slots, so they are one non-template body on the common base
	// that re-dispatches. Vanilla forwards with warp forced to 0; whether the
	// DLL's shared thunk does the same was not checked.
	virtual void Blit_Move(void* dst, byte* src, int len, int zval,
		WORD* zbuf, WORD* abuf, int alvl, int warp) override final
	{
		Blit_Copy(dst, src, len, zval, zbuf, abuf, alvl, 0);
	}

	virtual void Blit_Move_Tinted(void* dst, byte* src, int len, int zval,
		WORD* zbuf, WORD* abuf, int alvl, int warp, WORD tint) override final
	{
		Blit_Copy_Tinted(dst, src, len, zval, zbuf, abuf, alvl, 0, tint);
	}

private:
	using Weights = BlitterDetail::UniversalWeights<N>;

	// -----------------------------------------------------------------------
	//  Blend math, the alpha-level lookup and the level list all live in
	//  UniversalAlphaBlend.h - the RLE sibling's arithmetic is
	//  instruction-for-instruction identical, so it is shared rather than
	//  duplicated. Only the pixel walk below is specific to this family.
	// -----------------------------------------------------------------------
	template<bool Tinted>
	void Blit(void* dst, byte* src, int len, int zval,
		WORD* zbuf, WORD* abuf, int alvl, int warp, WORD tint)
	{
		// `warp` is never referenced by either function. It stays in the
		// signature for vtable compatibility only.
		(void)warp;

		// this+0x08 + (level << 9). A row is 256 WORDs = 512 bytes, so the
		// shift is just Table[level] and Table sits at offset 0 of the
		// remapper - consistent with Blitter::Lookup_Alpha_Remapper.
		const WORD* adata = AlphaRemapper->Table[BlitterDetail::Lookup_Level(alvl)];

		// BUG (preserved): the original tests `len != 0`, not `len > 0`. A
		// negative length runs the do/while ~4 billion times instead of
		// returning. Every vanilla blitter guards with `if (len < 0) return;`.
		if (len == 0)
			return;

		auto dest = reinterpret_cast<T*>(dst);

		do
		{
			const WORD zbufv = *zbuf++;

			// DIFF: the compare is an unsigned 16-bit one (`cmp cx, arg_C` /
			// `jbe`), so zval is effectively truncated to a WORD. Vanilla
			// compares `int zval < WORD zbufv`, which differs for negative zval.
			if (zbufv > static_cast<WORD>(zval))
			{
				// src advances only on a z-pass. Vanilla does the same.
				if (const byte srcv = *src++)
				{
					// BUG: abuf advances ONLY when a pixel is actually drawn,
					// while dest advances every iteration. Vanilla increments
					// abuf unconditionally alongside dest. As written the alpha
					// buffer desynchronises from the destination the moment any
					// pixel is skipped by the z-test or by transparency.
					// Preserved verbatim - fixing it changes visible output.
					const WORD abufv = *abuf++;

					// SUSPECT: abufv is a full WORD but a remap row is only 256
					// entries, so a large alpha value indexes into the next
					// row. Vanilla writes the identical expression.
					WORD colour = PaletteData[srcv | adata[abufv]];

					if constexpr (Tinted)
					{
						// The original ORs the full 32-bit stack slot; the
						// channel masks discard anything above bit 15, so a
						// WORD OR is equivalent.
						colour |= tint;
					}

					*dest = BlitterDetail::Pack<Weights::Shift>(
						  Weights::Dest * BlitterDetail::Spread(*dest)
						+ Weights::Src * BlitterDetail::Spread(colour));
				}
			}

			// Ring-buffer wrap, inlined in the original as
			//     if (Instance->End <= p) p -= Instance->SizeInBytes >> 1;
			// with End at +0x1C and SizeInBytes at +0x20.
			// Globals: ZBuffer::Instance = 0x887644, ABuffer::Instance = 0x87E8A4.
			ZBuffer::Instance->AdjustPointer(zbuf);
			ABuffer::Instance->AdjustPointer(abuf);

			++dest;
		}
		while (--len);

		// NOT PORTED: both originals leave the zbuf cursor in EAX and IDA types
		// them `signed int`. The vtable slot is void and no caller reads it -
		// on the len == 0 path EAX is the leftover of the alvl computation,
		// which is meaningless. Dropped.
	}

	T* PaletteData;
	AlphaLightingRemapClass* AlphaRemapper;
};
