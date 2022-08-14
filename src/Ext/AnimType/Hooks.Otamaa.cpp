#include "Body.h"

#ifdef ENABLE_NEWHOOKS_
DEFINE_HOOK(0x422CC6, AnimClass_DrawIT_SpecialDraw , 0xA)
{
	GET(AnimClass* const, pThis, ESI);

	if (pThis && pThis->Type)
	{
		if (auto const pTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type)){
			R->AL(pTypeExt->SpecialDraw.Get());
			return 0x422CD0;
		}
	}

	return 0x0;
}
#endif