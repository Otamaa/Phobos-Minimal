#pragma once

#include <ConvertClass.h>

#pragma once

// ---------------------------------------------------------------------------
//  BlitFlags.h -- blitter selector decoding, recovered from sub_1004C270.
//
//  Annotation tags follow the rest of the tree:
//    // VERIFY:   not recoverable from the dump
//    // NOTE:     invariant worth knowing before editing
// ---------------------------------------------------------------------------

#include <YRPP.h>

// ---------------------------------------------------------------------------
//  Position within a family. Both families use the same ordering, which is
//  what lets one resolver serve both selectors.
//
//  IDA cross-reference, for reading the pseudocode:
//      this[N]  ==  Blitters[N - 3]      for N in 3..20
//      this[N]  ==  RLEBlitters[N - 21]  for N in 21..38
// ---------------------------------------------------------------------------
struct BlitterIndex
{
	enum : int
	{
		// Twelve translucency levels. N is the DESTINATION weight out of 32:
		//     result = (N*dst + (32-N)*src) / 32
		// 8, 16 and 24 are absent because those are 25% / 50% / 75%, which
		// vanilla already ships as BlitTransLucent25 / 50 / 75.
		Alpha2 = 0, Alpha4, Alpha6, Alpha10, Alpha12, Alpha14,
		Alpha18, Alpha20, Alpha22, Alpha26, Alpha28, Alpha30,

		BufferedAlpha = 12,

		// Family-selected entries. Contiguous, and in the same order as the
		// family ids below -- see BlitFlags::FamilyTable.
		Add = 13,
		Multiply = 14,
		DoubleMultiply = 15,
		Luna = 16,
		Custom = 17,

		Count = 18,

		// Sentinel for "the flags selected nothing".
		None = -1,
	};
};

// ---------------------------------------------------------------------------
//  Flag decoding.
//
//  Two encodings share one dword:
//
//   (a) flags <= 0x80000000 : single-bit selector, LOWEST set bit wins.
//       Bits 18..30 pick the twelve translucency levels and the buffered-alpha
//       blitter. Nothing below bit 18 selects anything.
//
//   (b) flags >  0x80000000 : (flags & FamilyMask) is a family id.
//
//  NOTE: the two encodings are NOT separated by their bit patterns -- they
//  overlap heavily. FamilyMask covers bits 20..31 and the single-bit selectors
//  occupy bits 18..30, so every family id also contains several "single bit"
//  selectors. FamilyAdd (0x88400000) holds bits 22, 23, 27 and 31; fed to path
//  (a) it would resolve to Alpha12. The ONLY thing keeping the two apart is the
//  magnitude test. Since every family id has bit 31 set, that test is
//  equivalent to `(flags & 0x80000000) != 0`, but it is kept in its original
//  form here to stay faithful to the if-chain.
//
//  Consequence worth keeping in mind: the BufferedAlpha path and the
//  0x888/0x889 custom paths can NEVER both be taken for one draw. That
//  disjointness is the only thing keeping GetAlphaCursor() away from a
//  BlitterCustom, whose callback slots sit at the same +0x10 the alpha cursor
//  uses.
//
//  Dead zone: flags == 0x80000000 exactly takes path (a) (the test is `<=`),
//  where bit 31 matches no selector, so it resolves to None. It is not a valid
//  family id either. Preserved -- the original has the same hole.
// ---------------------------------------------------------------------------
struct BlitFlags
{
	// -----------------------------------------------------------------------
	//  Path (a): single-bit selectors.
	// -----------------------------------------------------------------------

	// Threshold, not a selector value. `flags <= SingleBitPathMax` picks path (a).
	static constexpr DWORD SingleBitPathMax = 0x80000000u;

	// The twelve alpha levels plus BufferedAlpha occupy 13 CONTIGUOUS bits
	// starting at bit 18, so SingleBits[i] == FirstSingleBit << i. The array is
	// still spelled out because the if-chain in sub_1004C270 tests the bits in
	// this order, and the explicit list documents the priority rather than
	// leaving it implied by a bit-scan.
	static constexpr DWORD FirstSingleBit = 0x00040000u;   // bit 18

	// The extent is derived from BufferedAlpha, not from Count, so that adding
	// a family entry to BlitterIndex cannot silently resize this array.
	static constexpr DWORD SingleBits[BlitterIndex::BufferedAlpha + 1] =
	{
		0x00040000u, // bit 18  Alpha2         N=2, dst weight 2/32
		0x00080000u, // bit 19  Alpha4
		0x00100000u, // bit 20  Alpha6
		0x00200000u, // bit 21  Alpha10
		0x00400000u, // bit 22  Alpha12
		0x00800000u, // bit 23  Alpha14
		0x01000000u, // bit 24  Alpha18
		0x02000000u, // bit 25  Alpha20
		0x04000000u, // bit 26  Alpha22
		0x08000000u, // bit 27  Alpha26
		0x10000000u, // bit 28  Alpha28
		0x20000000u, // bit 29  Alpha30
		0x40000000u, // bit 30  BufferedAlpha
	};

	static constexpr int SingleBitCount =
		static_cast<int>(sizeof(SingleBits) / sizeof(SingleBits[0]));

	static constexpr DWORD SingleBitMin = SingleBits[0];
	static constexpr DWORD SingleBitMax = SingleBits[SingleBitCount - 1];

	// Kept for callers that only need to know whether the alpha cursor applies.
	static constexpr DWORD BufferedAlpha = 0x40000000u;

	// -----------------------------------------------------------------------
	//  Path (b): family ids.
	// -----------------------------------------------------------------------

	static constexpr DWORD FamilyMask = 0xFFF00000u;

	static constexpr DWORD FamilyAdd = 0x88400000u;
	static constexpr DWORD FamilyMultiply = 0x88500000u;
	static constexpr DWORD FamilyDoubleMultiply = 0x88600000u;
	static constexpr DWORD FamilyLuna = 0x88700000u;

	// UPDATED: these were FamilyCustomA / FamilyCustomB back when the two slots
	// on BlitterCustom were mistaken for source buffers. They select which
	// CALLBACK the caller is installing out of TLS slot B. Both resolve to the
	// same BlitterIndex::Custom -- the distinction lives in the callback slot,
	// not the blitter.
	static constexpr DWORD FamilyCustomPixel = 0x88800000u; // -> PixelBlender
	static constexpr DWORD FamilyCustomSpan = 0x88900000u; // -> SpanBlitter

	struct FamilyEntry
	{
		DWORD Id;
		int   Index;
	};

	// NOTE: the six ids are contiguous in their top 12 bits (0x884 .. 0x889),
	// so a resolver could compute the index arithmetically. The table is
	// explicit because the last two ids collapse onto one blitter, which
	// arithmetic would have to special-case anyway.
	static constexpr FamilyEntry FamilyTable[] =
	{
		{ FamilyAdd,            BlitterIndex::Add            },
		{ FamilyMultiply,       BlitterIndex::Multiply       },
		{ FamilyDoubleMultiply, BlitterIndex::DoubleMultiply },
		{ FamilyLuna,           BlitterIndex::Luna           },
		{ FamilyCustomPixel,    BlitterIndex::Custom         },
		{ FamilyCustomSpan,     BlitterIndex::Custom         },
	};

	static constexpr int FamilyCount =
		static_cast<int>(sizeof(FamilyTable) / sizeof(FamilyTable[0]));
};

// ---------------------------------------------------------------------------
//  The OTHER half of the same dword: vanilla's low draw flags.
//
//  BlitFlags above decodes bits 18..31. Everything below bit 18 is vanilla
//  BlitterFlags and is invisible to the selector, so one dword carries both a
//  vanilla draw mode and an extension blitter choice without interference.
//
//  Recovered from AnimClass::Draw_It (.text:00422CA0), where the composite
//  0x2601 is spelled BF_2000|BF_400|BF_CENTER|BF_DARKEN, and from the three
//  `or ebx, 2 / 4 / 6` sites that set the translucency field.
//
//  NOTE: bits 1..2 are a two-bit FIELD, not independent flags -- 0x6 is 75%,
//  not "25% and 50% together". This is the field BlitterIndex's twelve alpha
//  levels supplement: vanilla can only express 25/50/75, which is why 8, 16
//  and 24 are missing from the Alpha* list.
//
//  Names below are this tree's, not Westwood's. VERIFY: BF_400 and BF_2000
//  have no recovered meaning; they are only ever seen as literals.
// ---------------------------------------------------------------------------
struct DrawFlags
{
	static constexpr DWORD None = 0x0u;

	static constexpr DWORD Darken = 0x1u;             // BF_DARKEN

	static constexpr DWORD TransLucent25 = 0x2u;
	static constexpr DWORD TransLucent50 = 0x4u;
	static constexpr DWORD TransLucent75 = 0x6u;
	static constexpr DWORD TranslucencyMask = 0x6u;

	static constexpr DWORD Center = 0x200u;           // BF_CENTER
	static constexpr DWORD Unknown400 = 0x400u;       // BF_400   -- VERIFY
	static constexpr DWORD Unknown2000 = 0x2000u;     // BF_2000  -- VERIFY

	// 0x601. The shadow pass strips the translucency field and forces this in.
	// Used by AnimClass_Draw_SetMaskBuffer_2 / _3 and by Draw_It's own shadow
	// call, all of which spell it as `and ~6 / or 0x601`.
	static constexpr DWORD ShadowOverride = Darken | Center | Unknown400;

	// 0x2601. Draw_It's __HasExtras pass passes this as a literal.
	static constexpr DWORD Extras = Unknown2000 | Unknown400 | Center | Darken;

	// Every bit this tree has a name for. Used by the disjointness assert.
	static constexpr DWORD AllKnown =
		Darken | TranslucencyMask | Center | Unknown400 | Unknown2000;

	// `and edx, 0FFFFFFF9h / or edx, 601h`. IDA renders the mask as 0xF9F8,
	// which is a pseudocode artifact -- the instruction clears bits 1 and 2.
	static constexpr DWORD ToShadow(DWORD flags)
	{
		return (flags & ~TranslucencyMask) | ShadowOverride;
	}

	static constexpr bool IsCentered(DWORD flags)
	{
		return (flags & Center) != 0u;
	}
};


// ---------------------------------------------------------------------------
//  The ENCODER side, recovered from AnimClass_Draw_SetFlags (.text:100527D0).
//
//  AnimTypeClass +0x2EC is `Translucency` -- the same field Draw_It compares
//  against 25 / 50 / 75 at 0x00423183. This table is the switch that maps it
//  onto the selector bits BlitFlags decodes.
//
//  WHERE IT LIVES IN THE BINARY: nowhere, as a function. The original switch is
//  inlined into the tail of AnimClass_Draw_SetFlags, spanning
//  .text:10052836 (mov eax, [ecx+2ECh] -- load Translucency) through
//  .text:100529AE (the default case, which writes the flags back unchanged).
//  MSVC emitted it as a comparison tree, with a four-entry jump table
//  (jpt_10052954) covering only keys 8849..8852. Each table row below cites the
//  `or esi, <bits>` site it came from, so it can be checked against the listing.
//
//  KEY FORMAT: the percentage with its decimal point removed.
//      "18.75" -> 1875      "12.5" -> 125       "6.25" -> 625
//  Eleven of the twelve keys fit this exactly, which is what confirms that
//  BlitterIndex's N is the destination weight out of 32: N/32 as a percentage
//  reproduces every key. It also explains the three gaps -- 8/32, 16/32 and
//  24/32 are 25%, 50% and 75%, which vanilla already handles.
//
//  BUG: the format is ambiguous, because the scale depends on how many decimal
//  places the number happens to have. "6.25" (N=2) and "62.5" (N=20) both strip
//  to 625. The original resolves the clash by keying Alpha2 off 10625 instead,
//  which as a percentage is 106.25 -- not a value any modder would write. Net
//  effect: `Translucency=6.25` silently selects Alpha20 (62.5%), and Alpha2 is
//  unreachable from a sane INI value. Preserved verbatim; the assert below pins
//  the workaround so it cannot be "tidied" back into a collision.
//
//  NOTE: BlitterIndex::Custom has no key here. The two custom families are
//  selected by the presence of a callback on the anim type's extension, not by
//  Translucency, and they short-circuit before this switch is reached.
// ---------------------------------------------------------------------------
struct TranslucencyKeys
{
	struct Entry
	{
		int   Key;      // Translucency value, decimal point stripped
		int   Index;    // BlitterIndex
		int   Weight;   // N, destination weight out of 32
		DWORD Flags;    // what the original ORs into the flags dword
	};

	// Ordered by BlitterIndex, not by the order the compiled binary search
	// tests them -- the switch is unordered, so any order is faithful.
	static constexpr Entry Table[] =
	{
		{ 10625, BlitterIndex::Alpha2,   2, BlitFlags::SingleBits[BlitterIndex::Alpha2]  }, //  6.25%  see BUG above   [100529A8]
		{   125, BlitterIndex::Alpha4,   4, BlitFlags::SingleBits[BlitterIndex::Alpha4]  }, // 12.5%   [1005287D]
		{  1875, BlitterIndex::Alpha6,   6, BlitFlags::SingleBits[BlitterIndex::Alpha6]  }, // 18.75%   [100528B2]
		{  3125, BlitterIndex::Alpha10, 10, BlitFlags::SingleBits[BlitterIndex::Alpha10] }, // 31.25%   [100528F5]
		{   375, BlitterIndex::Alpha12, 12, BlitFlags::SingleBits[BlitterIndex::Alpha12] }, // 37.5%   [1005286F]
		{  4375, BlitterIndex::Alpha14, 14, BlitFlags::SingleBits[BlitterIndex::Alpha14] }, // 43.75%   [100528E7]
		{  5625, BlitterIndex::Alpha18, 18, BlitFlags::SingleBits[BlitterIndex::Alpha18] }, // 56.25%   [100528D9]
		{   625, BlitterIndex::Alpha20, 20, BlitFlags::SingleBits[BlitterIndex::Alpha20] }, // 62.5%   [1005288B]
		{  6875, BlitterIndex::Alpha22, 22, BlitFlags::SingleBits[BlitterIndex::Alpha22] }, // 68.75%   [10052903]
		{  8125, BlitterIndex::Alpha26, 26, BlitFlags::SingleBits[BlitterIndex::Alpha26] }, // 81.25%   [10052925]
		{   875, BlitterIndex::Alpha28, 28, BlitFlags::SingleBits[BlitterIndex::Alpha28] }, // 87.5%   [100528A4]
		{  9375, BlitterIndex::Alpha30, 30, BlitFlags::SingleBits[BlitterIndex::Alpha30] }, // 93.75%   [10052993]

		// Not part of the N/32 ladder -- four consecutive magic values plus the
		// buffered-alpha one just below them. VERIFY: whether 8848 was chosen
		// to sit adjacent to the family block or is coincidental.
		{  8848, BlitterIndex::BufferedAlpha,  0, BlitFlags::BufferedAlpha       }, // [10052933]
		{  8849, BlitterIndex::Add,            0, BlitFlags::FamilyAdd            }, // [1005295B] jpt case 0
		{  8850, BlitterIndex::Multiply,       0, BlitFlags::FamilyMultiply       }, // [10052969] jpt case 1
		{  8851, BlitterIndex::DoubleMultiply, 0, BlitFlags::FamilyDoubleMultiply }, // [10052977] jpt case 2
		{  8852, BlitterIndex::Luna,           0, BlitFlags::FamilyLuna           }, // [10052985] jpt case 3
	};

	static constexpr int Count =
		static_cast<int>(sizeof(Table) / sizeof(Table[0]));

	// The key that would have been correct for Alpha2 if 62.5% had not already
	// claimed it. Kept so the collision is documented in code, not just prose.
	static constexpr int Alpha2CollidingKey = 625;

	// Returns the bits to OR into the flags dword, or 0 for an unrecognised
	// value. The original's default case leaves the flags untouched, which is
	// the same thing as OR-ing zero.
	//
	// DIFF: the original is a compiler-generated binary search plus a four-case
	// jump table. A linear scan over 17 entries is behaviourally identical and
	// keeps the mapping in one readable place.
	static constexpr DWORD Resolve(int translucency)
	{
		for (int i = 0; i < Count; ++i)
		{
			if (Table[i].Key == translucency)
				return Table[i].Flags;
		}

		return 0u;
	}
};

// ---------------------------------------------------------------------------
//  Invariants. These are the assumptions a resolver relies on; if any of them
//  stops holding, the resolver is silently wrong rather than broken.
// ---------------------------------------------------------------------------
struct BlitFlagsChecks
{
	// Union of every bit the selector half can look at.
	static constexpr DWORD SelectorBits()
	{
		DWORD bits = BlitFlags::FamilyMask;

		for (int i = 0; i < BlitFlags::SingleBitCount; ++i)
			bits |= BlitFlags::SingleBits[i];

		return bits;
	}

	// The array index IS the BlitterIndex value, so a resolver can return the
	// loop counter directly.
	static constexpr bool SingleBitsAreContiguous()
	{
		for (int i = 0; i < BlitFlags::SingleBitCount; ++i)
		{
			if (BlitFlags::SingleBits[i] != (BlitFlags::FirstSingleBit << i))
				return false;
		}

		return true;
	}

	// Every family id must survive its own mask, or the `flags & FamilyMask`
	// lookup misses.
	static constexpr bool FamilyIdsAreMaskStable()
	{
		for (int i = 0; i < BlitFlags::FamilyCount; ++i)
		{
			const DWORD id = BlitFlags::FamilyTable[i].Id;

			if ((id & BlitFlags::FamilyMask) != id)
				return false;
		}

		return true;
	}

	// Path (b) is only reachable above the threshold, so any family id at or
	// below it would be permanently dead.
	static constexpr bool FamilyIdsAreAboveThreshold()
	{
		for (int i = 0; i < BlitFlags::FamilyCount; ++i)
		{
			if (BlitFlags::FamilyTable[i].Id <= BlitFlags::SingleBitPathMax)
				return false;
		}

		return true;
	}


	// Every key must be distinct, or the switch would be unreachable in part.
	static constexpr bool TranslucencyKeysAreUnique()
	{
		for (int i = 0; i < TranslucencyKeys::Count; ++i)
		{
			for (int j = i + 1; j < TranslucencyKeys::Count; ++j)
			{
				if (TranslucencyKeys::Table[i].Key == TranslucencyKeys::Table[j].Key)
					return false;
			}
		}

		return true;
	}

	// Every BlitterIndex except Custom must be reachable from a Translucency
	// value; Custom is reachable only through the callback path.
	static constexpr bool TranslucencyCoversEveryIndex()
	{
		int seen[BlitterIndex::Count] = {};

		for (int i = 0; i < TranslucencyKeys::Count; ++i)
			++seen[TranslucencyKeys::Table[i].Index];

		for (int i = 0; i < BlitterIndex::Count; ++i)
		{
			const int expected = (i == BlitterIndex::Custom) ? 0 : 1;

			if (seen[i] != expected)
				return false;
		}

		return true;
	}

	// The twelve alpha entries must agree with the N/32 ladder. Alpha2 is the
	// one documented exception -- see the BUG note on TranslucencyKeys.
	static constexpr bool TranslucencyWeightsMatchKeys()
	{
		for (int i = 0; i < TranslucencyKeys::Count; ++i)
		{
			const auto& entry = TranslucencyKeys::Table[i];

			if (entry.Weight == 0)
				continue;   // the five magic values are not on the ladder

			// key == N/32 as a percentage with the decimal point removed, i.e.
			// N * 10000 / 32 with trailing zeros stripped.
			int expected = entry.Weight * 10000 / 32;

			while (expected % 10 == 0)
				expected /= 10;

			const bool isAlpha2Workaround =
				entry.Index == BlitterIndex::Alpha2
				&& expected == TranslucencyKeys::Alpha2CollidingKey;

			if (entry.Key != expected && !isAlpha2Workaround)
				return false;
		}

		return true;
	}

	// Together the two tables must cover every blitter slot exactly once,
	// except Custom, which two family ids share.
	static constexpr bool TablesCoverEveryIndex()
	{
		int seen[BlitterIndex::Count] = {};

		for (int i = 0; i < BlitFlags::SingleBitCount; ++i)
			++seen[i];

		for (int i = 0; i < BlitFlags::FamilyCount; ++i)
			++seen[BlitFlags::FamilyTable[i].Index];

		for (int i = 0; i < BlitterIndex::Count; ++i)
		{
			const int expected = (i == BlitterIndex::Custom) ? 2 : 1;

			if (seen[i] != expected)
				return false;
		}

		return true;
	}
};

static_assert(BlitFlags::SingleBitCount == 13,
	"SingleBits must cover Alpha2..BufferedAlpha.");

static_assert(BlitFlags::SingleBits[BlitterIndex::BufferedAlpha] == BlitFlags::BufferedAlpha,
	"BufferedAlpha's standalone constant disagrees with its table entry.");

static_assert(BlitFlagsChecks::SingleBitsAreContiguous(),
	"SingleBits must be 13 contiguous bits from bit 18, index == BlitterIndex.");

static_assert(BlitFlagsChecks::FamilyIdsAreMaskStable(),
	"A family id has bits below FamilyMask and will never match.");

static_assert(BlitFlagsChecks::FamilyIdsAreAboveThreshold(),
	"A family id is unreachable: path (b) only runs above SingleBitPathMax.");

static_assert(BlitFlagsChecks::TablesCoverEveryIndex(),
	"Every BlitterIndex must be selectable exactly once, except Custom (twice).");

static_assert((DrawFlags::AllKnown& BlitFlagsChecks::SelectorBits()) == 0u,
	"Low draw flags must not overlap the blitter selector bits.");

static_assert((DrawFlags::ShadowOverride& DrawFlags::TranslucencyMask) == 0u,
	"ShadowOverride must not re-set the translucency field ToShadow just cleared.");

static_assert(DrawFlags::ToShadow(DrawFlags::TransLucent75) == DrawFlags::ShadowOverride,
	"ToShadow must clear an existing translucency level outright.");

static_assert(DrawFlags::IsCentered(DrawFlags::ShadowOverride),
	"The shadow pass forces centering; AlphaMask::Blit's Center branch depends on it.");

static_assert(BlitFlagsChecks::TranslucencyKeysAreUnique(),
	"Translucency keys must be distinct.");

static_assert(BlitFlagsChecks::TranslucencyCoversEveryIndex(),
	"Every BlitterIndex except Custom must have exactly one Translucency key.");

static_assert(BlitFlagsChecks::TranslucencyWeightsMatchKeys(),
	"An alpha key stopped matching N/32-as-a-percentage with the point stripped.");

static_assert(TranslucencyKeys::Resolve(TranslucencyKeys::Alpha2CollidingKey)
	== BlitFlags::SingleBits[BlitterIndex::Alpha20],
	"BUG pin: 6.25%% and 62.5%% both strip to 625, and 62.5%% wins. "
	"Alpha2 is only reachable via its 10625 workaround key.");

static_assert(TranslucencyKeys::Resolve(0) == 0u,
	"An unrecognised Translucency must leave the flags untouched.");
class ConvertExtData
{
public:
	static constexpr int FamilySize = BlitterIndex::Count;

public:

	bool IsPlainReady() const { return this->Blitters[0] != nullptr; }
	bool IsRLEReady() const { return this->RLEBlitters[0] != nullptr; }
	bool IsReady(bool rle) const { return rle ? this->IsRLEReady() : this->IsPlainReady(); }

	void Alloc();
	void Dealloc();
	
	Blitter* Select(DWORD flags) const {
		const int index = ResolveIndex(flags);
		return static_cast<unsigned>(index) < BlitterIndex::Count ? this->Blitters[index] : nullptr;;
	}

	RLEBlitter* SelectRLE(DWORD flags) const {
		const int index = ResolveIndex(flags);
		return  static_cast<unsigned>(index) < BlitterIndex::Count ? this->RLEBlitters[index] : nullptr;;
	}

	static void AllocTLS();
private:

	static int ResolveIndex(DWORD flags);

public:

	ConvertClass* AttachedToObject {};
	Blitter* Blitters[FamilySize] {};
	RLEBlitter* RLEBlitters[FamilySize] {};
};

class NOVTABLE ConvertClassExt : ConvertClass
{
public:
	void DeallocBlitters();
	void AllocBlitters();

private:
	void AllocBlitters8();
	void DeallocBlitters8();
	void AllocBlitters16();
	void DeallocBlitters16();
};