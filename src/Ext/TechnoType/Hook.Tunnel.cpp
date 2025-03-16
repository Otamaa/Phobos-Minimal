#include "Body.h"
#include <Locomotor/Cast.h>

ASMJIT_PATCH(0x7294E0,TunnelLocomotionClass_7294E0_Handle, 0x6){
	GET(TunnelLocomotionClass* const, pLoco, ECX);

	const auto pLinkedTo = pLoco->LinkedTo;
	const auto pType = pLinkedTo->GetTechnoType();
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	CoordStruct nCoord = pLinkedTo->Location;
	const auto _height = pTypeExt->SubterraneanHeight.Get(RulesExtData::Instance()->SubterraneanHeight);

	if(nCoord.Z <= _height) {
		pLinkedTo->UpdatePlacement(PlacementType::Remove);
		pLoco->State = TunnelLocomotionClass::State::DIGGING;
	} else {
		const auto _addSpeed = TechnoTypeExtContainer::Instance.Find(pType)->Tunnel_Speed.Get(RulesClass::Instance->TunnelSpeed);
		auto curSpeed = pLinkedTo->GetCurrentSpeed();

		curSpeed = int(curSpeed * _addSpeed);

		if(curSpeed <= 5){
			curSpeed = 5;
		}

		nCoord.Z -= curSpeed;
		if(nCoord.Z < _height)
			nCoord.Z = _height;

		pLinkedTo->SetLocation(nCoord);
	}

	return 0x729564;
}

ASMJIT_PATCH(0x72994F, TunnelLocomotionClass_7298F0_Speed, 0x8)
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

ASMJIT_PATCH(0x728F89, TunnelLocomotionClass_Process_SubterraneanHeight1, 0x5)
{
	enum { Skip = 0x728FA6, Continue = 0x728F90 };

	GET(TechnoClass*, pLinkedTo, ECX);
	GET(int, height, EAX);

	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pLinkedTo->GetTechnoType());

	if (height == pTypeExt->SubterraneanHeight.Get(RulesExtData::Instance()->SubterraneanHeight))
		return Continue;

	return Skip;
}

ASMJIT_PATCH(0x728FC6, TunnelLocomotionClass_Process_SubterraneanHeight2, 0x5)
{
	enum { Skip = 0x728FCD, Continue = 0x729021 };

	GET(TechnoClass*, pLinkedTo, ECX);
	GET(int, height, EAX);

	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pLinkedTo->GetTechnoType());

	if (height <= pTypeExt->SubterraneanHeight.Get(RulesExtData::Instance()->SubterraneanHeight))
		return Continue;

	return Skip;
}

ASMJIT_PATCH(0x728FF2, TunnelLocomotionClass_Process_SubterraneanHeight3, 0x6)
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

ASMJIT_PATCH(0x7295E2, TunnelLocomotionClass_ProcessStateDigging_SubterraneanHeight, 0xC)
{
	enum { SkipGameCode = 0x7295EE };

	GET(TechnoClass*, pLinkedTo, EAX);
	REF_STACK(int, height, STACK_OFFSET(0x44, -0x8));

	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pLinkedTo->GetTechnoType());
	height = pTypeExt->SubterraneanHeight.Get(RulesExtData::Instance()->SubterraneanHeight);

	return SkipGameCode;
}

#pragma endregion