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
