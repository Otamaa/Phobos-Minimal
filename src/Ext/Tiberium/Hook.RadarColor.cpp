#include "Body.h"

#include <CellClass.h>
#include <OverlayClass.h>

#include <Utilities/GeneralUtils.h>

DEFINE_HOOK(0x47C20B, CellClass_CellColor_TiberiumRadarColor, 5)
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

		arg0->R = color.R;
		arg0->G = color.G;
		arg0->B = color.B;

		arg4->R = color.R;
		arg4->G = color.G;
		arg4->B = color.B;

		R->ECX(arg4);
		R->AL(color.B);

		return 0x47C23F;
	}

	return 0x47C210;
}

//
//DEFINE_HOOK(0x47C210, CellClass_CellColor_TiberiumRadarColor, 0x6)
//{
//	enum { ReturnFromFunction = 0x47C23F };
//
//	GET(CellClass*, pThis, ESI);
//	GET_STACK(ColorStruct*, arg0, STACK_OFFS(0x14, -0x4));
//	GET_STACK(ColorStruct*, arg4, STACK_OFFS(0x14, -0x8));
//
//	int tiberiumType = OverlayClass::GetTiberiumType(pThis->OverlayTypeIndex);
//
//	if (tiberiumType < 0)
//		return 0;
//
//	const auto pTiberiumExt = TiberiumExtContainer::Instance.Find(TiberiumClass::Array->Items[tiberiumType));
//
//	{
//		if (pTiberiumExt->MinimapColor.isset())
//		{
//			auto& color = pTiberiumExt->MinimapColor.Get();
//
//			arg0->R = color.R;
//			arg0->G = color.G;
//			arg0->B = color.B;
//
//			arg4->R = color.R;
//			arg4->G = color.G;
//			arg4->B = color.B;
//
//			R->ECX(arg4);
//			R->AL(color.B);
//
//			return ReturnFromFunction;
//		}
//	}
//
//	return 0;
//}