#include "Body.h"

AnimClassExt::ExtContainer AnimClassExt::ExtMap;
AnimClassExt::ExtContainer::ExtContainer() : TExtensionContainer("SecondAnimClass") { }
AnimClassExt::ExtContainer::~ExtContainer() = default;

//Only Extend Anim that Has "Type" Pointer
DEFINE_HOOK(0x422131, AnimClass_CTOR_AbsExt, 0x6)
{
	GET(AnimClass*, pItem, ESI);
	AnimClassExt::ExtMap.JustAllocate(pItem, pItem->Fetch_ID() != -2, "Creating an animation with null Type !");
	return 0;
}

DEFINE_HOOK(0x422A59, AnimClass_DTOR_AbsExt, 0x6)
{
	GET(AnimClass* const, pItem, ESI);
	AnimClassExt::ExtMap.Remove(pItem);
	return 0;
}