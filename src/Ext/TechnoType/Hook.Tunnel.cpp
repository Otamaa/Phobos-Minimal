#include "Body.h"
#include <Locomotor/Cast.h>

DEFINE_HOOK(0x72929D, TunnelLocomotionClass_7291F0_Speed, 0x8) //6
{
	GET(TechnoClass*, pLinkedTo, ECX);
	auto const pType = pLinkedTo->GetTechnoType();
	R->EAX(int(64.0 / pType->ROT /
		TechnoTypeExtContainer::Instance.Find(pLinkedTo->GetTechnoType())->Tunnel_Speed.Get(RulesClass::Instance->TunnelSpeed)
	));
	return 0x7292BF;
}

DEFINE_HOOK(0x72951C, TunnelLocomotionClass_7294E0_Speed, 0x8)
{
	GET(TunnelLocomotionClass* const, pLoco, ESI);
	GET(RulesClass*, pRules, ECX);
	GET(int, nCurrentMovementSpeed, EAX);
	R->EAX(int((nCurrentMovementSpeed) *
	TechnoTypeExtContainer::Instance.Find(pLoco->LinkedTo->GetTechnoType())->Tunnel_Speed.Get(pRules->TunnelSpeed)
	));
	return 0x72952C;
}

DEFINE_HOOK(0x72994F, TunnelLocomotionClass_7298F0_Speed, 0x8)
{
	GET(ILocomotion* const, pThis, ESI);
	auto const pLoco = static_cast<TunnelLocomotionClass*>(pThis);
	GET(RulesClass*, pRules, ECX);
	GET(int, nCurrentMovementSpeed, EAX);
	R->EAX(int((nCurrentMovementSpeed) *
	TechnoTypeExtContainer::Instance.Find(pLoco->LinkedTo->GetTechnoType())->Tunnel_Speed.Get(pRules->TunnelSpeed)
	));
	return 0x72995F;
}

#pragma region SubterraneanHeight

DEFINE_HOOK(0x728F89, TunnelLocomotionClass_Process_SubterraneanHeight1, 0x5)
{
	enum { Skip = 0x728FA6, Continue = 0x728F90 };

	GET(TechnoClass*, pLinkedTo, ECX);
	GET(int, height, EAX);

	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pLinkedTo->GetTechnoType());

	if (height == pTypeExt->SubterraneanHeight.Get(RulesExtData::Instance()->SubterraneanHeight))
		return Continue;

	return Skip;
}

DEFINE_HOOK(0x728FC6, TunnelLocomotionClass_Process_SubterraneanHeight2, 0x5)
{
	enum { Skip = 0x728FCD, Continue = 0x729021 };

	GET(TechnoClass*, pLinkedTo, ECX);
	GET(int, height, EAX);

	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pLinkedTo->GetTechnoType());

	if (height <= pTypeExt->SubterraneanHeight.Get(RulesExtData::Instance()->SubterraneanHeight))
		return Continue;

	return Skip;
}

DEFINE_HOOK(0x728FF2, TunnelLocomotionClass_Process_SubterraneanHeight3, 0x6)
{
	enum { SkipGameCode = 0x72900C };

	GET(TechnoClass*, pLinkedTo, ECX);
	GET(int, heightOffset, EAX);
	REF_STACK(int, height, 0x14);

	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pLinkedTo->GetTechnoType());
	int subtHeight = pTypeExt->SubterraneanHeight.Get(RulesExtData::Instance()->SubterraneanHeight);
	height -= heightOffset;

	if (height < subtHeight)
		height = subtHeight;

	return SkipGameCode;
}

DEFINE_HOOK(0x7295E2, TunnelLocomotionClass_ProcessStateDigging_SubterraneanHeight, 0xC)
{
	enum { SkipGameCode = 0x7295EE };

	GET(TechnoClass*, pLinkedTo, EAX);
	REF_STACK(int, height, STACK_OFFSET(0x44, -0x8));

	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pLinkedTo->GetTechnoType());
	height = pTypeExt->SubterraneanHeight.Get(RulesExtData::Instance()->SubterraneanHeight);

	return SkipGameCode;
}

#pragma endregion