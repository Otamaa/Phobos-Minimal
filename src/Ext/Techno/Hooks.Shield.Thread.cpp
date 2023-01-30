#include "Body.h"
#include <Ext/TechnoType/Body.h>

static inline int ReplaceThreadPosed(TechnoClass* pThis, TechnoTypeClass* pType)
{
	if (const auto pShieldData = TechnoExt::ExtMap.Find(pThis)->GetShield())
	{
		if (pShieldData->IsAvailable())
		{
			auto const pShiedType = pShieldData->GetType();
			if (pShiedType->ThreadPosed.isset())
				return pType->ThreatPosed + pShiedType->ThreadPosed.Get();
		}
	}

	return pType->ThreatPosed;
}

DEFINE_HOOK(0x708BA3, TechnoClass_ThreatPosed_Shield, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(TechnoTypeClass*, pType, EDI);
	R->EAX(ReplaceThreadPosed(pThis, pType));
	return 0x708BA9;
}

DEFINE_HOOK(0x708B9A, TechnoClass_ThreadPosed_BunkerLinked_Shield, 0x6)
{
	GET(TechnoClass*, pThis, ECX);
	GET(TechnoTypeClass*, pType, EAX);
	R->EAX(ReplaceThreadPosed(pThis, pType));
	return 0x708BA0;
}

DEFINE_HOOK(0x4F6B97, HouseClass_Apparent_Category_Power_Shield, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(TechnoTypeClass*, pType, EAX);
	R->ECX(ReplaceThreadPosed(pThis, pType));
	return 0x4F6B9D;
}

DEFINE_HOOK(0x4F6B27, HouseClass_Category_Power_Shield, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(TechnoTypeClass*, pType, EAX);
	R->ECX(ReplaceThreadPosed(pThis, pType));
	return 0x4F6B2D;
}
