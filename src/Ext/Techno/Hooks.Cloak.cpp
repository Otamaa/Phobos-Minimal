#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>

DEFINE_HOOK(0x703A09, TechnoClass_VisualCharacter_CloakVisibility, 0x7)
{
	enum { UseShadowyVisual = 0x703A5A, CheckMutualAlliance = 0x703A16 };

	// Allow observers to always see cloaked objects.
	// Skip IsCampaign check (confirmed being useless from Mental Omega mappers)
	if (HouseExt::IsObserverPlayer())
		return UseShadowyVisual;

	return CheckMutualAlliance;
}

DEFINE_HOOK(0x45455B, BuildingClass_VisualCharacter_CloakVisibility, 0x5)
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