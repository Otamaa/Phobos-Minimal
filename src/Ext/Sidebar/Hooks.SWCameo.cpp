#include <Ext/SWType/Body.h>
#include <Utilities/Macro.h>

#pragma region SW TabIndex
DEFINE_HOOK(0x6A5F6E, SidebarClass_6A5F20_TabIndex, 0x8)
{
	enum { ApplyTabIndex = 0x6A5FD3 };

	GET(AbstractType const, absType, ESI);
	GET(int const, typeIdx, EAX);

	R->EAX(SidebarClass::GetObjectTabIdx(absType, typeIdx, 0));
	return ApplyTabIndex;
}

DEFINE_HOOK(0x6ABC9D, SidebarClass_GetObjectTabIndex_Super, 0x5)
{
	enum { ApplyTabIndex = 0x6ABCA2 };

	GET(int const, typeIdx, EDX);

	if (typeIdx < 0 || typeIdx >= SuperWeaponTypeClass::Array->Count)
		return 0;

	const auto pSWType = SuperWeaponTypeClass::Array->Items[typeIdx];
	const auto pSWTypExt = SWTypeExtContainer::Instance.Find(pSWType);

	R->EAX(pSWTypExt->TabIndex);
	return ApplyTabIndex;
}

DEFINE_HOOK(0x6CE1A0, SuperClass_AI_FlashingBar, 0x5) {
	GET(SuperClass*, pThis, ECX);

	enum { Continue = 0x0 , RetFalse = 0x6CE1E7 };

	return SWTypeExtContainer::Instance.Find(pThis->Type)->ExclusiveSidebar_Allow ?
		RetFalse : Continue;
}

 // Skip tabIndex check
#pragma endregion