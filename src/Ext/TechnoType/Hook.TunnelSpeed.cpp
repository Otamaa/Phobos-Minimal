#include "Body.h"
#include <TunnelLocomotionClass.h>

/*
DEFINE_HOOK(0x7292B1, TunnelLocomotionClass_7291F0_Speed, 0x6)
{
	GET_LOCO(ESI);
	auto speed = pExt->TunnelSpeed.Get(RulesClass::Instance->TunnelSpeed);
	_asm fdiv qword ptr[speed];
	return 0x7292B7;
}

DEFINE_HOOK(0x729520, TunnelLocomotionClass_7294E0_Speed, 0x7)
{
	GET_LOCO(ESI);
	auto speed = pExt->TunnelSpeed.Get(RulesClass::Instance->TunnelSpeed);
	_asm fmul qword ptr[speed];
	return 0x729527;
}

DEFINE_HOOK(0x729953, TunnelLocomotionClass_7298F0_Speed, 0x7)
{
	GET_LOCO(ESI);
	auto speed = pExt->TunnelSpeed.Get(RulesClass::Instance->TunnelSpeed);
	_asm fmul qword ptr[speed];
	return 0x72995A;
}*/

// Tunnel Locomotor Speed multiplier
// Author : Otamaa
/*
DEFINE_HOOK(0x72929A, TunnelLocomotionClass_7291F0_Speed, 0x5)
{
	GET_LOCO(ESI);

	auto speed = pExt->TunnelSpeed.Get(RulesClass::Instance->TunnelSpeed);
	R->EAX(Game::F2I(64.0 / static_cast<double>(pType->ROT) / speed));

	return 0x7292BF;
}*/

//crash a lot
/*
DEFINE_HOOK(0x7294E6, TunnelLocomotionClass_7294E0_Speed, 0x9)
{
	enum { MarkForRedraw = 0x729565, ReturnAfterSetLocation = 0x729560 };
	GET(TunnelLocomotionClass*, pThis, ESI);

	auto pLinked = pThis->LinkedTo;
	auto& nLoc = pLinked->Location;

	auto nLinked = pThis->LinkedTo ? pThis->LinkedTo->get_ID() : "None";
	Debug::Log(__FUNCTION__" [%x] Owner [%s] \n", pThis, nLinked);

	if (nLoc.Z <= -256)
	{
		R->ECX(pLinked);
		return MarkForRedraw;
	}
	else
	{
		auto nSpeed = pLinked->GetCurrentSpeed();
		auto nTunnelSpeedMult = RulesGlobal->TunnelSpeed;

		auto nSpeedTotal = Game::F2I(nSpeed * nTunnelSpeedMult);
		if (nSpeedTotal <= 5)
			nSpeedTotal = 5;

		nLoc.Z -= nSpeedTotal;

		if (nLoc.Z < -256)
			nLoc.Z = -256;

		pLinked->SetLocation(nLoc);
	}

	return ReturnAfterSetLocation;
}
*/
static double GetTunnelSpeed(TechnoTypeClass* pThis , RulesClass* pRules)
{
	if (auto pExt = TechnoTypeExt::ExtMap.Find(pThis))
		return pExt->Tunnel_Speed.Get(pRules->TunnelSpeed);

	return pRules->TunnelSpeed;
}

DEFINE_HOOK(0x72929D, TunnelLocomotionClass_7291F0_Speed, 0x6)
{
	//GET(ILocomotion* const, pThis, ESI);
	//auto const pLoco = static_cast<TunnelLocomotionClass*>(pThis);
	GET(TechnoClass*, pLinkedTo, ECX);

	//auto nLinked = pLinkedTo ? pLinkedTo->get_ID() : "None";
	//Debug::Log(__FUNCTION__" [%x] Owner [%s] \n", pLoco, nLinked);

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

	//auto nLinked = pLoco->LinkedTo ? pLoco->LinkedTo->get_ID() : "None";
	//Debug::Log(__FUNCTION__" [%x] Owner [%s] \n", pThis, nLinked);

	R->EAX(Game::F2I((nCurrentMovementSpeed) * GetTunnelSpeed(pLoco->LinkedTo->GetTechnoType() , pRules)));

	return 0x72952C;
}

DEFINE_HOOK(0x72994F, TunnelLocomotionClass_7298F0_Speed, 0x8)
{
	GET(ILocomotion* const, pThis, ESI);
	auto const pLoco = static_cast<TunnelLocomotionClass*>(pThis);
	GET(RulesClass*, pRules, ECX);
	GET(int, nCurrentMovementSpeed, EAX);

	//auto nLinked = pLoco->LinkedTo ? pLoco->LinkedTo->get_ID() : "None";
	//Debug::Log(__FUNCTION__" [%x] Owner [%s] \n", pThis, nLinked);

	R->EAX(Game::F2I((nCurrentMovementSpeed) * GetTunnelSpeed(pLoco->LinkedTo->GetTechnoType(), pRules)));

	return 0x72995F;
}