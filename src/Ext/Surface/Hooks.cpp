#include "Body.h"

#include <Syringe.h>
#include <Utilities/Patch.h>
#include <Utilities/Macro.h>

ASMJIT_PATCH(0x437C29, Buffer_To_RLE_Surface_With_Z_Shape_Lock_Bound_Fix, 7)
{
	GET_STACK(int const, nX_comp, 0x30);
	GET_STACK(int const, nY_comp, 0x58);
	GET(Surface*, pSurface, ECX);
	GET(int, nX, EAX);
	GET(int, nY, EDX);

	if (nX >= nX_comp || nX < 0)
		nX = 0;
	if (nY >= nY_comp || nY < 0)
		nY = 0;

	R->EAX(pSurface->Lock(nX, nY));
	return 0x437C30;
}


int __fastcall XSurface_Get_Pixel_Checked(XSurface* pSurface, discard_t , const Point2D& xy)
{
	// Out of range reads as 0. Note that 0 is also a legitimate pixel value -
	// black, or palette index 0 - so callers cannot tell the two apart. That
	// matches what vanilla returns when Lock fails, so nothing regresses.
	if (xy.X < 0 || xy.X >= pSurface->Width)
		return 0;

	if (xy.Y < 0 || xy.Y >= pSurface->Height)
		return 0;

	auto pPixel = pSurface->Lock(xy.X, xy.Y);

	if (!pPixel)
		return 0;

	// DIFF, harmless: vanilla asks `BPP == 2 ? word : byte`; this asks
	// `BPP == 1 ? byte : word`. Identical for the only two values that occur.
	// They would disagree at BPP 4 - vanilla reads a byte, this reads a word -
	// but that is a phrasing artefact of the rewrite, not an intent.
	const int colour = (pSurface->Get_Bytes_Per_Pixel() == 1)
		? *static_cast<byte*>(pPixel)
		: *static_cast<WORD*>(pPixel);

	pSurface->Unlock();

	return colour;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x7BAE60, XSurface_Get_Pixel_Checked)