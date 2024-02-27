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

DEFINE_HOOK(0x420A71, AlphaShapeClass_CTOR_Anims, 0x5)
{
	GET(AlphaShapeClass*, pThis, ESI);

	if (pThis->AttachedTo && pThis->AttachedTo->WhatAmI() == AnimClass::AbsID)
	{
		PointerExpiredNotification::NotifyInvalidAnim->Add(pThis);
	}

	return 0;
}

DEFINE_HOOK(0x421798, AlphaShapeClass_SDDTOR_Anims, 0x6)
{
	GET(AlphaShapeClass*, pThis, ESI);
	PointerExpiredNotification::NotifyInvalidAnim->Remove(pThis);
	return 0;
}

DEFINE_HOOK(0x420E70, AlphaLightClass_Detach_ClearPointer, 0x7)
{
	GET(AlphaShapeClass*, pThis, ECX);
	GET_STACK(AbstractClass*, pObject, 0x4);
	GET_STACK(bool, removed, 0x8);

	if(pThis->AttachedTo == pObject){
		pThis->IsObjectGone = true;
	}

	return 0x420E7F;
}

#ifndef ALPHA_DRAWING
// This part is inevitable
// the `AlphaShapeClass` dtor is executed first before objects
DEFINE_HOOK(0x421730, AlphaShapeClass_SDDTOR, 8)
{
	GET(AlphaShapeClass* const, pAlpha, ECX);

	if (auto pOldAlpha = StaticVars::ObjectLinkedAlphas.erase(pAlpha->AttachedTo)) {
		pAlpha->IsObjectGone = true;
		pAlpha->AttachedTo = nullptr;
	}

	return 0;
}

DEFINE_HOOK(0x420960, AlphaShapeClass_CTOR, 5)
{
	GET_STACK(ObjectClass* const, pSource, 0x4);
	GET(AlphaShapeClass*, pThis, ECX);

	const auto it = StaticVars::ObjectLinkedAlphas.get_key_iterator(pSource);

	if (it != StaticVars::ObjectLinkedAlphas.end()) {
		if (it->second) {
			//sddtor delete the pKey
			GameDelete<true, false>(std::exchange(it->second , nullptr));
		} else {
			it->second = pThis;
			return 0;
		}
	}


	//insert new key ,..
	StaticVars::ObjectLinkedAlphas.empalace_unchecked(pSource, pThis);
	return 0;
}

DEFINE_HOOK(0x5F3D65, ObjectClass_DTOR, 6)
{
	GET(ObjectClass*, pThis, ESI);

	if (auto pOldAlpha = StaticVars::ObjectLinkedAlphas.get_or_default(pThis)) {
		GameDelete<true, false>(std::exchange(pOldAlpha , nullptr));
	}

	return 0;
}

DEFINE_HOOK(0x5F3E70, ObjectClass_Update_AlphaLight, 5)
{
	GET(ObjectClass*, pThis, ECX);
	TechnoExt_ExtData::UpdateAlphaShape(pThis);
	return 0;
}

DEFINE_HOOK(0x423B0B, AnimClass_Update_AlphaLight, 6)
{
	GET(AnimClass*, pThis, ESI);
	// flaming guys do the update via base class
	if (!pThis->Type->IsFlamingGuy)
	{
		TechnoExt_ExtData::UpdateAlphaShape(pThis);
	}

	return 0;
}
#endif