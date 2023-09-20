#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <RectangleStruct.h>
#include <CellStruct.h>
#include <Unsorted.h>

#include "Header.h"

DEFINE_OVERRIDE_HOOK_AGAIN(0x6D5573 , TacticalClass_DrawPlacement_CustomFoundation, 0x6)
DEFINE_OVERRIDE_HOOK(0x6D50FB , TacticalClass_DrawPlacement_CustomFoundation, 0x5)
{
	RectangleStruct nDispRect {};
	const bool bOnFB = R->Origin() == 0x6D50FB;
	CustomFoundation::GetDisplayRect(nDispRect, (!bOnFB ?
		Unsorted::CursorSizeSecond() : Unsorted::CursorSize()));

	int16_t nX = 0;
	if (nDispRect.Width - nDispRect.X >= 0)
		nX = LOWORD(nDispRect.Width) - LOWORD(nDispRect.X);

	int16_t nY = 0;
	if (nDispRect.Height - nDispRect.Y >= 0)
		nY = LOWORD(nDispRect.Height) - LOWORD(nDispRect.Y);

	const CellStruct v9res { (nX + 1)  , (nY + 1) };
	const CellStruct v10res { (int16_t)nDispRect.X , (int16_t)nDispRect.Y };

	R->Stack(0x14, v10res.Pack());
	R->Stack(0x18, v9res.Pack());
	R->EAX(v9res.Pack());
	R->ESI(nDispRect.Y);

	return (!bOnFB) ? 0x6D558F : 0x6D5116 ;
}
