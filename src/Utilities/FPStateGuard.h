#pragma once

struct FPStateGuard
{
	// --- MXCSR -------------------------------------------------------------
		// Layout: [0..5] sticky exception STATUS flags, [6] DAZ, [7..12] exception
		// masks, [13..14] rounding control, [15] FTZ.
		//
		// Bits 0..5 are set by ordinary arithmetic and MUST be excluded from any
		// comparison - a full-word compare fires on essentially every call.
	static constexpr unsigned int MxcsrStatusMask = 0x003Fu;

	static constexpr unsigned int MxcsrMaskAllExceptions = 0x1F80u; // bits 7..12
	static constexpr unsigned int MxcsrRcNearest = 0x0000u;
	static constexpr unsigned int MxcsrRcChop = 0x6000u;			// bits 13..14 = 11

#if FPSTATEGUARD_MXCSR_CHOP
	static constexpr unsigned int MxcsrExpected = MxcsrMaskAllExceptions | MxcsrRcChop; // 0x7F80
#else
	static constexpr unsigned int MxcsrExpected = MxcsrMaskAllExceptions | MxcsrRcNearest; // 0x1F80
#endif

	// Result-changing subset (FTZ | RC | DAZ), diagnostics only. The actual
	// check covers the whole control half, because unmasked exceptions do not
	// desync but do produce spurious FP traps, and repairing them costs
	// nothing.
	static constexpr unsigned int MxcsrResultMask = 0xE040u;

	// --- x87 control word --------------------------------------------------
	// Layout: [0..5] exception masks, [6..7] reserved, [8..9] precision
	// control, [10..11] rounding control, [12] infinity control (legacy).
	static constexpr unsigned short CwMaskAllExceptions = 0x003Fu;
	static constexpr unsigned short CwPc53 = 0x0200u;	// bits 8..9 = 10
	static constexpr unsigned short CwRcChop = 0x0C00u;	// bits 10..11 = 11

	// Only the bits the game actually configures are compared and written.
	// Reserved bits are preserved on repair rather than forced, since the CRT
	// leaves bit 6 set and there is no reason to disturb it.
	static constexpr unsigned short CwCompareMask =
		static_cast<unsigned short>(CwMaskAllExceptions | 0x0300u | 0x0C00u); // 0x0F3F

	static constexpr unsigned short CwExpected =
		static_cast<unsigned short>(CwPc53 | CwRcChop | CwMaskAllExceptions);  // 0x0E3F

	static_assert(CwExpected == 0x0E3Fu, "x87 target drifted from PC=53 | RC=chop | all masked");
	static_assert((CwExpected & ~CwCompareMask) == 0, "x87 target sets bits outside the compare mask");
	static int LogBudget;

	// Establishes the expected state. Call from CRTHooks::_set_fp_mode() so
	// the initializer and the guard cannot drift apart, and from any point
	// that needs a known-good baseline.
	//
	// Reserved x87 bits are preserved; everything the game configures is
	// forced. MXCSR sticky status flags are preserved.
	static void Apply();

	static inline unsigned short GetControlWord()
	{
		unsigned short cw = 0;
		__asm { fnstcw cw }
		return cw;
	}

	static inline void SetControlWord(unsigned short cw)
	{
		__asm { fldcw cw }
	}

	static inline bool IsSseClean(unsigned int mxcsr)
	{
		return (mxcsr & ~MxcsrStatusMask) == MxcsrExpected;
	}

	static inline bool IsX87Clean(unsigned short cw)
	{
		return (cw & CwCompareMask) == CwExpected;
	}

	static bool IsClean();

	static bool Repair(const char* pSite);

	// RAII wrapper for "repair on the way out of a call that is known to
	// clobber FP state". Wrap the game's blit/flip/present call, or any
	// synchronous call into the wrapper DLL.
	//
	//     {
	//         FPStateGuard::ScopedRepair guard("IDirectDrawSurface::Flip");
	//         pSurface->Flip(nullptr, DDFLIP_WAIT);
	//     }
	//
	// Copy/move deleted deliberately: a copied guard would repair twice and
	// obscure which site actually caused the corruption.
	struct ScopedRepair
	{
		explicit ScopedRepair(const char* pSite) noexcept : Site(pSite) {}
		~ScopedRepair() { Repair(this->Site); }

		ScopedRepair(const ScopedRepair&) = delete;
		ScopedRepair(ScopedRepair&&) = delete;
		ScopedRepair& operator=(const ScopedRepair&) = delete;
		ScopedRepair& operator=(ScopedRepair&&) = delete;

	private:
		const char* Site;
	};
};