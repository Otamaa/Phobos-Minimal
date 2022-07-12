#include "Body.h"
#include <TunnelLocomotionClass.h>

static double GetTunnelSpeed(TechnoTypeClass* pThis , RulesClass* pRules)
{
	if (const auto pExt = TechnoTypeExt::ExtMap.Find(pThis))
		return pExt->Tunnel_Speed.Get(pRules->TunnelSpeed);

	return pRules->TunnelSpeed;
}

DEFINE_HOOK(0x72929D, TunnelLocomotionClass_7291F0_Speed, 0x6)
{
	GET(TechnoClass*, pLinkedTo, ECX);

	auto nRot = pLinkedTo->GetTechnoType()->ROT;
	R->EAX(Game::F2I(64.0 / nRot / GetTunnelSpeed(pLinkedTo->GetTechnoType() , RulesGlobal)));

	return 0x7292BF;
}

DEFINE_HOOK(0x72951C, TunnelLocomotionClass_7294E0_Speed, 0x8)
{
	GET(ILocomotion* const, pThis, ESI);
	auto const pLoco = static_cast<TunnelLocomotionClass*>(pThis);
	GET(RulesClass*, pRules, ECX);
	GET(int, nCurrentMovementSpeed, EAX);

	R->EAX(Game::F2I((nCurrentMovementSpeed) * GetTunnelSpeed(pLoco->LinkedTo->GetTechnoType() , pRules)));

	return 0x72952C;
}

DEFINE_HOOK(0x72994F, TunnelLocomotionClass_7298F0_Speed, 0x8)
{
	GET(ILocomotion* const, pThis, ESI);
	auto const pLoco = static_cast<TunnelLocomotionClass*>(pThis);
	GET(RulesClass*, pRules, ECX);
	GET(int, nCurrentMovementSpeed, EAX);

	R->EAX(Game::F2I((nCurrentMovementSpeed) * GetTunnelSpeed(pLoco->LinkedTo->GetTechnoType(), pRules)));

	return 0x72995F;
}