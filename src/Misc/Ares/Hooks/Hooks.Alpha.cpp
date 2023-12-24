#include <AlphaShapeClass.h>
#include <TacticalClass.h>

#include <ObjectClass.h>
#include <CellClass.h>
#include <BuildingClass.h>

#include <Ext/Anim/Body.h>
#include <Ext/Techno/Body.h>

#include <FootClass.h>

#include <Misc/PhobosGlobal.h>

#include <Notifications.h>

#include "Header.h"

DEFINE_OVERRIDE_HOOK(0x420A71, AlphaShapeClass_CTOR_Anims, 0x5)
{
	GET(AlphaShapeClass*, pThis, ESI);

	if (pThis->AttachedTo && pThis->AttachedTo->WhatAmI() == AnimClass::AbsID)
	{
		PointerExpiredNotification::NotifyInvalidAnim->Add(pThis);
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x421798, AlphaShapeClass_SDDTOR_Anims, 0x6)
{
	GET(AlphaShapeClass*, pThis, ESI);
	PointerExpiredNotification::NotifyInvalidAnim->Remove(pThis);
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x421730, AlphaShapeClass_SDDTOR, 8)
{
	GET(AlphaShapeClass*, pAlpha, ECX);
	StaticVars::ObjectLinkedAlphas.erase(pAlpha->AttachedTo);
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x420960, AlphaShapeClass_CTOR, 5)
{
	GET_STACK(ObjectClass*, pSource, 0x4);
	GET(AlphaShapeClass*, pThis, ECX);

	auto iter = StaticVars::ObjectLinkedAlphas.get_key_iterator(pSource);

	if(iter != StaticVars::ObjectLinkedAlphas.end()) {
		if (auto pOldAlpha = std::exchange(iter->second , pThis)) {
			GameDelete<true, false>(pOldAlpha);
		}
	} else {
		StaticVars::ObjectLinkedAlphas.empalace_unchecked(pSource, pThis);
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x5F3D65, ObjectClass_DTOR, 6)
{
	GET(ObjectClass*, pThis, ESI);

	if (auto pAlpha = StaticVars::ObjectLinkedAlphas.get_or_default(pThis)) {
		GameDelete<true, false>(pAlpha);
		// pThis is erased from map
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x5F3E70, ObjectClass_Update_AlphaLight, 5)
{
	GET(ObjectClass*, pThis, ECX);
	TechnoExt_ExtData::UpdateAlphaShape(pThis);
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x423B0B, AnimClass_Update_AlphaLight, 6)
{
	GET(AnimClass*, pThis, ESI);
	// flaming guys do the update via base class
	if (!pThis->Type->IsFlamingGuy)
	{
		TechnoExt_ExtData::UpdateAlphaShape(pThis);
	}

	return 0;
}