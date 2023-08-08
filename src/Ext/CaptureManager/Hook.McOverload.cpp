#include "Body.h"

#include <Ext/Techno/Body.h>

DEFINE_HOOK(0x6FA726, TechnoClass_AI_MCOverload, 0x6)
{
	enum {
		SelfHeal = 0x6FA743, //continue ares check here
		DoNotSelfHeal = 0x6FA941,
		ReturnFunc = 0x6FAFFD,
		Continue = 0x0
	};

	GET(TechnoClass*, pThis, ESI);

	TechnoExt::UpdateMCOverloadDamage(pThis);

	if(!pThis->IsAlive)
		return ReturnFunc;

	return SelfHeal;
}