#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/House/Body.h>

DEFINE_HOOK(0x703A09, TechnoClass_VisualCharacter_CloakVisibility_A, 0x7)
{
	enum { UseShadowyVisual = 0x703A5A, CheckMutualAlliance = 0x703A16 };

	// Allow observers to always see cloaked objects.
	// Skip IsCampaign check (confirmed being useless from Mental Omega mappers)
	if (HouseExt::IsObserverPlayer())
		return UseShadowyVisual;

	return CheckMutualAlliance;
}

DEFINE_HOOK(0x4544D1, BuildingClass_VisualCharacter_CloakVisibility_B, 0x6)
{
	enum { UseShadowyVisual = 0x45452D, Continue = 0x0};

	// Allow observers to always see cloaked objects.
	return HouseExt::IsObserverPlayer() ? 
		UseShadowyVisual : Continue;
}

DEFINE_HOOK_AGAIN(0x703998, TechnoClass_VisualCharacter_CloakVisibility_B, 0x6)
DEFINE_HOOK(0x7038FE, TechnoClass_VisualCharacter_CloakVisibility_B, 0x6)
{
	enum { UseShadowyVisual = 0x70395E, Continue = 0x0 };

	// Allow observers to always see cloaked objects.
	return HouseExt::IsObserverPlayer() ? UseShadowyVisual : Continue
		;
}
DEFINE_HOOK(0x45455B, BuildingClass_VisualCharacter_CloakVisibility_A, 0x5)
{
	enum { UseShadowyVisual = 0x45452D, CheckMutualAlliance = 0x454564 };

	if (HouseExt::IsObserverPlayer())
		return UseShadowyVisual;

	return CheckMutualAlliance;
}
DEFINE_HOOK(0x702819, TechnoClass_ReceiveDamage_Decloak, 0xA)
{
	GET(TechnoClass* const, pThis, ESI);
	GET_STACK(WarheadTypeClass*, pWarhead, STACK_OFFS(0xC4, -0xC));

	if (WarheadTypeExt::ExtMap.Find(pWarhead)->DecloakDamagedTargets.Get())
		pThis->Uncloak(false);

	return 0x702823;
}