#include "FPStateGuard.h"

#include <intrin.h>
#include <Utilities/Debug.h>


// ============================================================
// SSE FP-state (MXCSR) desync fix - renderer/GPU driver
// ============================================================
// On some machines the GPU driver corrupts the game thread's SSE control
// register (MXCSR). Confirmed with Intel D3D9-on-D3D12 UMD (igd9trinity32.dll):
// cnc-ddraw creates its D3D9 device synchronously on the game thread inside
// IDirectDraw::SetDisplayMode, and the driver's CreateDevice returns with
// MXCSR rounding control flipped from round-to-nearest (0x1F80) to
// round-toward-zero (0x7FA0).
//
// The vanilla game is pure x87 and immune, but everything compiled with SSE
// (this spawner, Phobos, Ares) then computes differently from every other
// player: '100%' parses as 0.99999994f instead of 1.0f, that lands in
// Ground.Cost -> movement speed -> desync as soon as a unit moves.
//
// Fixed:
//  1. right after SetDisplayMode returns - kills the observed corruption at
//     the source, before any INI parsing
//  2. at Spawner::StartGame
//  3. at the start of every logic frame - covers mid-game corruption

int FPStateGuard::LogBudget = 12;

bool FPStateGuard::IsClean()
{
	return IsSseClean(_mm_getcsr()) && IsX87Clean(GetControlWord());
}

void FPStateGuard::Apply()
{
	const unsigned short cw = GetControlWord();
	SetControlWord(static_cast<unsigned short>((cw & ~CwCompareMask) | CwExpected));

	const unsigned int mxcsr = _mm_getcsr();
	_mm_setcsr(MxcsrExpected | (mxcsr & MxcsrStatusMask));
}

bool FPStateGuard::Repair(const char* pSite)
{
	const unsigned int mxcsr = _mm_getcsr();
	const unsigned short cw = GetControlWord();

	const bool sseBad = !IsSseClean(mxcsr);
	const bool x87Bad = !IsX87Clean(cw);

	if (!sseBad && !x87Bad)
		return false;

	if (sseBad)
		_mm_setcsr(MxcsrExpected | (mxcsr & MxcsrStatusMask));

	if (x87Bad)
		SetControlWord(static_cast<unsigned short>((cw & ~CwCompareMask) | CwExpected));

	if (LogBudget > 0)
	{
		--LogBudget;

		if (sseBad) {
			Debug::Log("[FPStateGuard] %s: MXCSR %04X -> %04X (RC/FTZ/DAZ delta %04X)\n",
				pSite,
				mxcsr & 0xFFFFu,
				MxcsrExpected,
				(mxcsr ^ MxcsrExpected) & MxcsrResultMask);
		}

		if (x87Bad) {
			Debug::Log("[FPStateGuard] %s: x87 CW %04X -> %04X (PC/RC delta %04X, missing D3DCREATE_FPU_PRESERVE?)\n",
					pSite,
					static_cast<unsigned int>(cw),
					static_cast<unsigned int>((cw & ~CwCompareMask) | CwExpected),
					static_cast<unsigned int>((cw ^ CwExpected) & 0x0F00u));
		}

		if (LogBudget == 0)
			Debug::Log("[FPStateGuard] further repairs will not be logged\n");
	}

	return true;
}