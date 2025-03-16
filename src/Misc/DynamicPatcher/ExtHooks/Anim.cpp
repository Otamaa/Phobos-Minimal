#include <Misc/Otamaa/Hooks.Otamaa.h>

ASMJIT_PATCH(0x423136, AnimClass_Draw_Remap2, 0x6)
{
	GET(AnimClass*, pThis, ESI);

	if (pThis->Type && pThis->Type->AltPalette)
	{
		if (auto pOwner = pThis->Owner)
			R->ECX(pOwner);
		else if (auto pObject = pThis->OwnerObject)
			if (pObject->GetOwningHouse())
				R->ECX(pObject->GetOwningHouse());
	}

	return 0;
}

ASMJIT_PATCH(0x42312A, AnimClass_Draw_Remap, 0x6)
{
	HouseClass* pOwner = nullptr;
	GET(AnimClass*, pThis, ESI);

	if (auto pAOwner = pThis->Owner)
		pOwner = (pAOwner);
	else if (auto pObject = pThis->OwnerObject)
		if (pObject->GetOwningHouse())
			pOwner = (pObject->GetOwningHouse());

	return pThis->Type->AltPalette && pOwner ? 0x423130 : 0x4231F3;
}