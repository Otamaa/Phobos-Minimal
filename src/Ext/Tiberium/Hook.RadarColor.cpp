#include "Body.h"

#include <CellClass.h>
#include <OverlayClass.h>

#include <Utilities/GeneralUtils.h>

ASMJIT_PATCH(0x47C20B, CellClass_CellColor_TiberiumRadarColor, 5)
{
	GET(int, nTibIDx, EAX);
	GET_STACK(ColorStruct*, arg0, STACK_OFFS(0x14, -0x4));
	GET_STACK(ColorStruct*, arg4, STACK_OFFS(0x14, -0x8));

	if (nTibIDx == -1)
		return 0x47C24A;

	const auto pTib = TiberiumClass::Array->Items[nTibIDx];
	const auto pTiberiumExt = TiberiumExtContainer::Instance.Find(pTib);

	if (pTiberiumExt->MinimapColor.isset()) {

		auto& color = pTiberiumExt->MinimapColor.Get();

		arg0->operator=(color);
		arg4->operator=(color);

		R->ECX(arg4);
		R->AL(color.B);

		return 0x47C23F;
	}

	return 0x47C210;
}
