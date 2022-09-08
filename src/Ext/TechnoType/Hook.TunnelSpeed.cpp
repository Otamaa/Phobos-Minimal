#include "Body.h"
#include <TunnelLocomotionClass.h>

#ifdef COMPILE_PORTED_DP_FEATURES
DEFINE_HOOK(0x72929D, TunnelLocomotionClass_7291F0_Speed, 0x8) //6
{
	GET(TechnoClass*, pLinkedTo, ECX);
	auto const pType = pLinkedTo->GetTechnoType();
	R->EAX(Game::F2I(64.0 / pType->ROT / TechnoTypeExt::GetTunnelSpeed(pType, RulesGlobal)));
	return 0x7292BF;
}

DEFINE_HOOK(0x72951C, TunnelLocomotionClass_7294E0_Speed, 0x8)
{
	GET(ILocomotion* const, pThis, ESI);
	auto const pLoco = static_cast<TunnelLocomotionClass*>(pThis);
	GET(RulesClass*, pRules, ECX);
	GET(int, nCurrentMovementSpeed, EAX);
	R->EAX(Game::F2I((nCurrentMovementSpeed) * TechnoTypeExt::GetTunnelSpeed(pLoco->LinkedTo->GetTechnoType() , pRules)));
	return 0x72952C;
}

DEFINE_HOOK(0x72994F, TunnelLocomotionClass_7298F0_Speed, 0x8)
{
	GET(ILocomotion* const, pThis, ESI);
	auto const pLoco = static_cast<TunnelLocomotionClass*>(pThis);
	GET(RulesClass*, pRules, ECX);
	GET(int, nCurrentMovementSpeed, EAX);
	R->EAX(Game::F2I((nCurrentMovementSpeed) * TechnoTypeExt::GetTunnelSpeed(pLoco->LinkedTo->GetTechnoType(), pRules)));
	return 0x72995F;
}
#endif