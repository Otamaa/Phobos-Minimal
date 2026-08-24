#pragma once
#include <numeric>

struct BlitterDetail
{
	// Must live at namespace scope. A static data member's initializer runs in
	// the complete-class context, but a member function's BODY is only defined
	// after the class is complete - so calling a member constexpr function
	// from a sibling static data member initializer is ill-formed:
	//   MSVC  C2131 "expression did not evaluate to a constant"
	//   GCC   "called in a constant expression before its definition is complete"
	static constexpr int Log2(int v) { return v <= 1 ? 0 : 1 + Log2(v >> 1); }

	// The twelve levels this family provides, in selector order.
	static inline constexpr int UniversalAlphaLevels[] = { 2, 4, 6, 10, 12, 14, 18, 20, 22, 26, 28, 30 };

	// -----------------------------------------------------------------------
	//  N is the DESTINATION weight out of 32:
	//      result = (N*dst + (32-N)*src) / 32
	//
	//  For N = 2 the compiler reduced by gcd(2,32) = 2 and emitted
	//  (1*dst + 15*src) / 16, which is what both disassemblies do:
	//  `shl edi,4 / sub edi,esi` builds 15*src, adds dst, then the shift
	//  triple divides by 16.
	//
	//  VERIFY: only N = 2 was disassembled, for either family. The N/32
	//          weighting is inferred from the reduced (1,15)/16 form plus the
	//          missing-8/16/24 pattern. Check one more instantiation - say
	//          <WORD,30> at 0x10050280 - before trusting the generalisation.
	//
	//  Weights are DWORD, not int: they multiply Spread()'s DWORD result, and
	//  an int operand there is a signed/unsigned mismatch, which is a hard
	//  error in a build that promotes C4365 with /we.
	// -----------------------------------------------------------------------
	template<int N>
	struct UniversalWeights
	{
		static constexpr int Divisor = 32;
		static constexpr int Common = std::gcd(N, Divisor);

		static constexpr DWORD Dest = static_cast<DWORD>(N / Common);
		static constexpr DWORD Src = static_cast<DWORD>((Divisor - N) / Common);
		static constexpr DWORD Total = static_cast<DWORD>(Divisor / Common);
		static constexpr int Shift = Log2(static_cast<int>(Total));

		static_assert(N > 0 && N < 32, "N is the destination weight out of 32");
		static_assert(Dest + Src == Total, "weights must sum to the divisor");
		static_assert((1u << Shift) == Total, "Total must be a power of two");
	};

	// -----------------------------------------------------------------------
	//  A 565 pixel is fanned out across a dword so each channel gets five
	//  spare bits of headroom to accumulate in:
	//
	//      bits  0.. 4   R      bits 10..15   G      bits 21..25   B
	//
	//  Both disassemblies build this as
	//      (c & 0x1F) | 32 * ((c & 0x7E0) | 32 * (c & 0xF800))
	//  which is the same thing written as two nested shifts.
	// -----------------------------------------------------------------------
	static constexpr DWORD Spread(WORD colour)
	{
		return static_cast<DWORD>(colour & 0x001Fu)
			| (static_cast<DWORD>(colour & 0x07E0u) << 5)
			| (static_cast<DWORD>(colour & 0xF800u) << 10);
	}

	template<int Shift>
	static constexpr WORD Pack(DWORD sum)
	{
		return static_cast<WORD>(
			  ((sum >> Shift) & 0x001Fu)
			| ((sum >> (Shift + 5)) & 0x07E0u)
			| ((sum >> (Shift + 10)) & 0xF800u));
	}

	// The originals pre-multiply the sum by 4 and then shift by 6/11/16. With
	// Shift == 4 that is identical to the 4/9/14 above; the *4 exists only so
	// the three shift counts land on convenient encodings. Dropped.

	// -----------------------------------------------------------------------
	//  Same as Blitter::Lookup_Alpha_Remapper, with two faithfulness notes.
	// -----------------------------------------------------------------------
	static inline int Lookup_Level(int alvl)
	{
		// DIFF: both DLL functions read alvl as a SIGNED 16-BIT value
		// (`arg = word`, then `movsx`), while YRpp declares it int. Harmless
		// for the documented [0, 2000] range, not identical above it.
		const int level = static_cast<short>(alvl);

		// SUSPECT: the originals compute max(0, alvl) twice - once via cmovle
		// on the sign-extended value and again on the zero-extended one -
		// which is a compiler artefact, not two different clamps.
		const int scaled = 261 * (level > 0 ? level : 0);

		return scaled >= (254 << 11) ? 254 : (scaled >> 11);
	}

	// =======================================================================
	//  PACKED 888 - for the saturating (Add) family
	//
	//  A 565 pixel expanded to byte-aligned channels in a dword:
	//      bits 0..7  R      bits 8..15  G      bits 16..23  B
	//
	//  The original builds it as
	//      HIBYTE(c) & 0xF8 | ((((byte)(8*c) << 8) | (c >> 3) & 0xFC) << 8)
	//  which is the three masks below folded into one expression.
	// =======================================================================
	static constexpr DWORD Expand888(WORD colour)
	{
		return ((static_cast<DWORD>(colour) >> 8) & 0x00F8u)
			| (((static_cast<DWORD>(colour) >> 3) & 0x00FCu) << 8)
			| (((static_cast<DWORD>(colour) << 3) & 0x00F8u) << 16);
	}

	static constexpr WORD Pack565(DWORD rgb)
	{
		const DWORD r = rgb & 0xFFu;
		const DWORD g = (rgb >> 8) & 0xFFu;
		const DWORD b = (rgb >> 16) & 0xFFu;

		// Exactly the original's shift chain:
		//   ((R & 0xF8) << 5 | (G & 0xFC)) << 3 | (B >> 3)
		return static_cast<WORD>(((((r & 0xF8u) << 5) | (g & 0xFCu)) << 3) | (b >> 3));
	}

	static constexpr DWORD SaturateByte(DWORD v)
	{
		return v > 0xFFu ? 0xFFu : v;
	}

	// Per-channel saturating add. That is the entire Add blitter.
	static constexpr DWORD AddSaturate888(DWORD dest, DWORD source)
	{
		return SaturateByte((dest & 0xFFu) + (source & 0xFFu))
			| (SaturateByte(((dest >> 8) & 0xFFu) + ((source >> 8) & 0xFFu)) << 8)
			| (SaturateByte(((dest >> 16) & 0xFFu) + ((source >> 16) & 0xFFu)) << 16);
	}

	// -----------------------------------------------------------------------
	//  Per-channel modulate: (dest * source) / 255. The entire Multiply
	//  blitter.
	//
	//  The division is EXACT, not the usual `>> 8` approximation. The original
	//  emits `mov eax, 80808081h / mul / shr edx, 7` three times, which is
	//  MSVC's magic-number sequence for unsigned division by 255 (verified
	//  against every byte*byte product).
	//
	//  How much that matters: after requantising to 565 the two disagree for
	//  14 of 1024 five-bit channel pairs and 200 of 4096 six-bit pairs - so it
	//  is observable, but only just. Kept exact because the original is.
	//
	//  No clamping is needed: 255*255/255 == 255 is already the maximum.
	//
	//  Note also that Expand888 does NOT replicate bits, so full white expands
	//  to 248/252/248 rather than 255/255/255 and modulate is lossy even
	//  against white - 0xFFFF * 0xFFFF comes out 0xF7DE. That is the vanilla
	//  behaviour, not a rounding artefact of this backport.
	// -----------------------------------------------------------------------
	static constexpr DWORD ModulateByte(DWORD dest, DWORD source)
	{
		return (dest * source) / 255u;
	}

	static constexpr DWORD Modulate888(DWORD dest, DWORD source)
	{
		return ModulateByte(dest & 0xFFu, source & 0xFFu)
			| (ModulateByte((dest >> 8) & 0xFFu, (source >> 8) & 0xFFu) << 8)
			| (ModulateByte((dest >> 16) & 0xFFu, (source >> 16) & 0xFFu) << 16);
	}

	// -----------------------------------------------------------------------
	//  Per-channel Luna: src*src/255 + dest*(255-src)/255. The entire Luna
	//  blitter.
	//
	//  This is source-over compositing in which the source's own value acts as
	//  its alpha, with the colour already premultiplied by it:
	//
	//      out = s*s + d*(1 - s)          for s, d normalised to [0, 1]
	//
	//  So a bright source pixel is nearly opaque and a dark one nearly
	//  invisible - a glow/light blend rather than a uniform translucency. Over
	//  black it degenerates to s*s (a gamma-ish darkening); over white it never
	//  falls below 191.
	//
	//  Both divisions are the same exact magic-number /255 the Multiply family
	//  uses. Each term is truncated to a byte before the add, which is a no-op:
	//  neither can exceed 255.
	//
	//  The original clamps the sum to 255. That clamp is DEAD CODE - checked
	//  over all 65536 (dest, source) byte pairs, the maximum unclamped sum is
	//  exactly 255. Preserved anyway, since removing it would rely on the
	//  reader trusting that analysis.
	// -----------------------------------------------------------------------
	static constexpr DWORD LunaByte(DWORD dest, DWORD source)
	{
		return SaturateByte((source * source) / 255u
			+ (dest * (255u - source)) / 255u);
	}

	static constexpr DWORD Luna888(DWORD dest, DWORD source)
	{
		return LunaByte(dest & 0xFFu, source & 0xFFu)
			| (LunaByte((dest >> 8) & 0xFFu, (source >> 8) & 0xFFu) << 8)
			| (LunaByte((dest >> 16) & 0xFFu, (source >> 16) & 0xFFu) << 16);
	}

	// -----------------------------------------------------------------------
	//  Per-channel DoubleMultiply: min(255, 2 * (dest * source / 255)).
	//
	//  Literally Multiply with the result doubled - `2*d*s` is the same
	//  expression that forms the dark half of Overlay / Hard Light, but here
	//  it is applied unconditionally, with no branch on the destination.
	//  Mid-grey against mid-grey stays put; anything brighter blows out.
	//
	//  ORDER MATTERS. The original divides first and doubles afterwards:
	//      2 * ((d * s) / 255)      NOT      (2 * d * s) / 255
	//  Those disagree for 27156 of the 65536 byte pairs - the second form
	//  keeps a bit of precision the original throws away, so writing it the
	//  tidier way would visibly brighten dark pixels. Kept as emitted.
	//
	//  Unlike Luna's, this clamp is live: it fires for 10015 of the 65536
	//  pairs.
	// -----------------------------------------------------------------------
	static constexpr DWORD DoubleMultiplyByte(DWORD dest, DWORD source)
	{
		return SaturateByte(2u * ModulateByte(dest, source));
	}

	static constexpr DWORD DoubleMultiply888(DWORD dest, DWORD source)
	{
		return DoubleMultiplyByte(dest & 0xFFu, source & 0xFFu)
			| (DoubleMultiplyByte((dest >> 8) & 0xFFu, (source >> 8) & 0xFFu) << 8)
			| (DoubleMultiplyByte((dest >> 16) & 0xFFu, (source >> 16) & 0xFFu) << 16);
	}
};
