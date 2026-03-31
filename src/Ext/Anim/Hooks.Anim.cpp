#include "Body.h"

#include <Ext/AnimType/Body.h>

ASMJIT_PATCH(0x4232CE, AnimClass_Draw_SetPalette, 6)
{
	GET(AnimClass*, pThis, ESI);
	//GET(AnimTypeClass*, AnimType, EAX);

	const auto pData = AnimTypeExtContainer::Instance.TryFind(pThis->Type);

	if (pData ) {
		if(const auto pConvert = pData->Palette.GetConvert()) {
			R->ECX<ConvertClass*>(pConvert);
			return 0x4232D4;
		}
	}

	return 0;
}