#pragma once

#include "../Blitters/Blitter.h"

// ============================================================================
//  PCXBlitter<T>
//
//  Not a backport - new code. Four issues were fixed relative to the first
//  draft; see the FIX notes inline.
//
//  QUESTION worth settling before this is used in anger: every other blitter
//  in this tree treats `src` as 8-bit PALETTE INDICES and resolves them
//  through PaletteData[...]. This one reads `src` as raw T pixels, so it only
//  makes sense if the caller genuinely hands over a 16bpp buffer. If it is fed
//  the usual 8-bit SHP data it will read two bytes per pixel and run off the
//  end of the row.
//
//  Also note `zval` is being repurposed as the destination Y coordinate, and
//  zbuf / abuf / alvl / warp are ignored entirely.
// ============================================================================

template <typename T>
class PCXBlitter final : public Blitter
{
public:
	OPTIONALINLINE explicit PCXBlitter(T mask, int imageWidth, int imageHeight, int cornerSize = 2) noexcept
		: Mask(mask), Width(imageWidth), Height(imageHeight), CornerSize(cornerSize) {}

	virtual ~PCXBlitter() override final = default;

	// FIX 1: zbuf / abuf were declared T* and tint T. The base declares them
	// WORD* and WORD. That happens to match when T == WORD and silently does
	// NOT match for any other T, at which point these stop being overrides.
	//
	// FIX 2: `override` was missing on Blit_Copy_Tinted, Blit_Move and
	// Blit_Move_Tinted. With the mismatched signatures above that combination
	// is the worst case - no diagnostic at the declaration, and instead
	//     error: invalid new-expression of abstract class type PCXBlitter<byte>
	// at the construction site, because the base's pure virtuals were never
	// implemented. Verified: instantiating PCXBlitter<byte> reproduced exactly
	// that. All four are marked now.
	virtual void Blit_Copy(void* dst, byte* src, int len, int zval,
		WORD* zbuf, WORD* abuf, int alvl, int warp) override final
	{
		auto pDst = static_cast<T*>(dst);

		// FIX 3: was static_cast<T*>(src). That is ill-formed between
		// unrelated object pointer types and only survived because nothing
		// instantiated the template:
		//     error: invalid 'static_cast' from 'byte*' to 'short unsigned int*'
		auto pSrc = reinterpret_cast<T*>(src);

		const int y = zval; // see the note in the header comment

		for (int i = 0; i < len; ++i)
		{
			const int x = i;
			const T pixel = *pSrc++;

			const bool leftEdge = x < CornerSize;
			const bool rightEdge = x >= Width - CornerSize;
			const bool topEdge = y < CornerSize;
			const bool bottomEdge = y >= Height - CornerSize;

			const bool isCorner = (leftEdge || rightEdge) && (topEdge || bottomEdge);

			// Mask colour is dropped only in the corners; everywhere else it
			// is written opaque.
			if (pixel != Mask || !isCorner)
				*pDst = pixel;

			++pDst;
		}
	}

	virtual void Blit_Copy_Tinted(void* dst, byte* src, int len, int zval,
		WORD* zbuf, WORD* abuf, int alvl, int warp, WORD tint) override final
	{
		// Tint deliberately discarded, as in the vanilla RLE tinted forwarders.
		Blit_Copy(dst, src, len, zval, zbuf, abuf, alvl, warp);
	}

	virtual void Blit_Move(void* dst, byte* src, int len, int zval,
		WORD* zbuf, WORD* abuf, int alvl, int warp) override final
	{
		Blit_Copy(dst, src, len, zval, zbuf, abuf, alvl, warp);
	}

	virtual void Blit_Move_Tinted(void* dst, byte* src, int len, int zval,
		WORD* zbuf, WORD* abuf, int alvl, int warp, WORD tint) override final
	{
		Blit_Copy(dst, src, len, zval, zbuf, abuf, alvl, warp);
	}

private:
	T Mask;
	int Width;
	int Height;
	int CornerSize;
};
