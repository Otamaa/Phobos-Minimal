#include <Locomotor/Cast.h>

#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Scenario/Body.h>

#include <Utilities/Patch.h>
#include <Utilities/Macro.h>

ASMJIT_PATCH(0x7294E0,TunnelLocomotionClass_7294E0_Handle, 0x6){
	GET(TunnelLocomotionClass* const, pLoco, ECX);

	const auto pLinkedTo = pLoco->LinkedTo;
	const auto pType = GET_TECHNOTYPE(pLinkedTo);
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	CoordStruct nCoord = pLinkedTo->Location;
	const auto _height = pTypeExt->SubterraneanHeight.Get(RulesExtData::Instance()->SubterraneanHeight);

	if(nCoord.Z <= _height) {
		pLinkedTo->Mark(MarkType::Remove);
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
		GET_TECHNOTYPEEXT(pLoco->LinkedTo)->Tunnel_Speed.Get(pRules->TunnelSpeed)
	));
	return 0x72995F;
}

#pragma region SubterraneanHeight

ASMJIT_PATCH(0x728F89, TunnelLocomotionClass_Process_SubterraneanHeight1, 0x5)
{
	enum { Skip = 0x728FA6, Continue = 0x728F90 };

	GET(TechnoClass*, pLinkedTo, ECX);
	GET(int, height, EAX);

	auto const pTypeExt = GET_TECHNOTYPEEXT(pLinkedTo);

	if (height == pTypeExt->SubterraneanHeight.Get(RulesExtData::Instance()->SubterraneanHeight))
		return Continue;

	return Skip;
}

ASMJIT_PATCH(0x728FC6, TunnelLocomotionClass_Process_SubterraneanHeight2, 0x5)
{
	enum { Skip = 0x728FCD, Continue = 0x729021 };

	GET(TechnoClass*, pLinkedTo, ECX);
	GET(int, height, EAX);

	auto const pTypeExt = GET_TECHNOTYPEEXT(pLinkedTo);

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

	auto const pTypeExt = GET_TECHNOTYPEEXT(pLinkedTo);
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

	auto const pTypeExt = GET_TECHNOTYPEEXT(pLinkedTo);
	height = pTypeExt->SubterraneanHeight.Get(RulesExtData::Instance()->SubterraneanHeight);

	return SkipGameCode;
}

#pragma endregion

Matrix3D* __stdcall TunnelLocomotionClass_ShadowMatrix(ILocomotion* iloco, Matrix3D* ret, VoxelIndexKey* key)
{
	auto tLoco = static_cast<TunnelLocomotionClass*>(iloco);
	tLoco->LocomotionClass::Shadow_Matrix(ret, key);

	if (tLoco->State != TunnelLocomotionClass::State::IDLE)
	{
		double theta = 0.;
		switch (tLoco->State)
		{
		case TunnelLocomotionClass::State::DIGGING:
			if (key)key->Invalidate();
			theta = Math::PI_BY_TWO_ACCURATE;
			if (auto total = tLoco->Timer.Rate)
				theta *= 1.0 - double(tLoco->Timer.GetTimeLeft()) / double(total);
			break;
		case TunnelLocomotionClass::State::DUG_IN:
			theta = Math::PI_BY_TWO_ACCURATE;
			break;
		case TunnelLocomotionClass::State::PRE_DIG_OUT:
			theta = -Math::PI_BY_TWO_ACCURATE;
			break;
		case TunnelLocomotionClass::State::DIGGING_OUT:
			if (key)key->Invalidate();
			theta = -Math::PI_BY_TWO_ACCURATE;
			if (auto total = tLoco->Timer.Rate)
				theta *= double(tLoco->Timer.GetTimeLeft()) / double(total);
			break;
		case TunnelLocomotionClass::State::DUG_OUT:
			if (key)key->Invalidate();
			theta = Math::PI_BY_TWO_ACCURATE;
			if (auto total = tLoco->Timer.Rate)
				theta *= double(tLoco->Timer.GetTimeLeft()) / double(total);
			break;
		default:break;
		}
		ret->ScaleX((float)Math::cos(theta));// I know it's ugly
	}
	return ret;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5A4C, TunnelLocomotionClass_ShadowMatrix);

ASMJIT_PATCH(0x728F9A, TunnelLocomotionClass_Process_Track, 0x7)
{
	// GET(FootClass*, pTechno, ECX);
	GET(ILocomotion*, pThis, ESI);

	const auto pLoco = static_cast<TunnelLocomotionClass*>(pThis);
	auto pTechno = pLoco->LinkedTo;
	ScenarioExtData::Instance()->UndergroundTracker.emplace(pTechno);
	TechnoExtContainer::Instance.Find(pTechno)->UndergroundTracked = true;

	return 0;
}ASMJIT_PATCH_AGAIN(0x729029, TunnelLocomotionClass_Process_Track, 0x7);

ASMJIT_PATCH(0x7297F6, TunnelLocomotionClass_ProcessDigging_Track, 0x7)
{
	GET(FootClass*, pTechno, ECX);

	ScenarioExtData::Instance()->UndergroundTracker.erase(pTechno);
	TechnoExtContainer::Instance.Find(pTechno)->UndergroundTracked = false;

	return 0;
}

// In the following three places the distance check was hardcoded to compare with 20, 17 and 16 respectively,
// which means it didn't consider the actual speed of the unit. Now we check it and the units won't get stuck
// even at high speeds - NetsuNegi
ASMJIT_PATCH(0x72958E, TunnelLocomotionClass_ProcessDigging_SlowdownDistance, 0x8)
{
	enum { KeepMoving = 0x72980F, CloseEnough = 0x7295CE };

	//this fix reqire change of `pType->Speed`
	//which is ridicculus really - Otamaa
	GET(TunnelLocomotionClass* const, pLoco, ESI);

	auto& currLoc = pLoco->LinkedTo->Location;
	int distance = (int)CoordStruct { currLoc.X - pLoco->_CoordsNow.X, currLoc.Y - pLoco->_CoordsNow.Y,0 }.Length();

	auto const pTypeExt = GET_TECHNOTYPEEXT(pLoco->LinkedTo);
	int currentSpeed = pTypeExt->SubterraneanSpeed >= 0 ?
		pTypeExt->SubterraneanSpeed : RulesExtData::Instance()->SubterraneanSpeed;

	// Calculate speed multipliers.
	pLoco->LinkedTo->SpeedPercentage = 1.0; // Subterranean locomotor doesn't normally use this so it would be 0.0 here and cause issues.		int maxSpeed = pTypeExt->This()->Speed;
	int maxSpeed = pTypeExt->This()->Speed;
	pTypeExt->This()->Speed = currentSpeed;
	currentSpeed = pLoco->LinkedTo->GetCurrentSpeed();
	pTypeExt->This()->Speed = maxSpeed;

	if (distance > currentSpeed)
	{
		REF_STACK(CoordStruct, newLoc, STACK_OFFSET(0x40, -0xC));
		double angle = -Math::atan2((float)(currLoc.Y - pLoco->_CoordsNow.Y), (float)(pLoco->_CoordsNow.X - currLoc.X));
		newLoc = currLoc + CoordStruct { int((double)currentSpeed * Math::cos(angle)), int((double)currentSpeed * Math::sin(angle)), 0 };
		return 0x7298D3;
	}
	return 0x7295CE;
}

ASMJIT_PATCH(0x728F74, TunnelLocomotionClass_Process_KillAnims, 0x5)
{
	GET(ILocomotion*, pThis, ESI);

	const auto pLoco = static_cast<TunnelLocomotionClass*>(pThis);
	const auto pExt = TechnoExtContainer::Instance.Find(pLoco->LinkedTo);

	pExt->IsBurrowed = true;

	if (const auto pShieldData = TechnoExtContainer::Instance.Find(pLoco->LinkedTo)->GetShield())
	{
		pShieldData->SetAnimationVisibility(false);
	}

	for (auto& attachEffect : pExt->PhobosAE)
	{
		if (attachEffect)
			attachEffect->SetAnimationTunnelState(false);
	}

	return 0;
}

ASMJIT_PATCH(0x728E5F, TunnelLocomotionClass_Process_RestoreAnims, 0x7)
{
	GET(ILocomotion*, pThis, ESI);

	const auto pLoco = static_cast<TunnelLocomotionClass*>(pThis);

	if (pLoco->State == TunnelLocomotionClass::State::PRE_DIG_OUT)
	{
		const auto pExt = TechnoExtContainer::Instance.Find(pLoco->LinkedTo);
		pExt->IsBurrowed = false;

		if (const auto pShieldData = TechnoExtContainer::Instance.Find(pLoco->LinkedTo)->GetShield())
			pShieldData->SetAnimationVisibility(true);

		for (auto& attachEffect : pExt->PhobosAE)
		{
			if (attachEffect)
				attachEffect->SetAnimationTunnelState(true);
		}
	}

	return 0;
}

ASMJIT_PATCH(0x7290AD, TunnelLocomotionClass_Process_Stop, 0x5)
{
	GET(TunnelLocomotionClass* const, pLoco, ESI);

	if (const auto pLinked = pLoco->Owner ? pLoco->Owner : pLoco->LinkedTo)
		if (auto const pCell = pLinked->GetCell())
			pCell->CollectCrate(pLinked);

	return 0;
}


ASMJIT_PATCH(0x728EF0, TunnelLocomotionClass_ILocomotion_Process_Dig, 5)
{
	GET(FootClass*, pFoot, EAX);

	TechnoExtData::HandleTunnelLocoStuffs(pFoot, true, true);
	return 0x728F74;
}

ASMJIT_PATCH(0x72929A, TunnelLocomotionClass_sub_7291F0_Dig, 6)
{
	GET(TunnelLocomotionClass*, pThis, ESI);

	auto const pType = GET_TECHNOTYPE(pThis->LinkedTo);
	int time_left = (int(64.0 / pType->ROT /
		TechnoTypeExtContainer::Instance.Find(pType)->Tunnel_Speed.Get(RulesClass::Instance->TunnelSpeed)
		));

	pThis->State = TunnelLocomotionClass::State::DIGGING_IN;
	pThis->Timer.Start(time_left);
	TechnoExtData::HandleTunnelLocoStuffs(pThis->LinkedTo, true, true);
	return 0x729365;
}

ASMJIT_PATCH(0x7293DA, TunnelLocomotionClass_sub_729370_Dig, 6)
{
	GET(FootClass*, pFoot, ECX);

	TechnoExtData::HandleTunnelLocoStuffs(pFoot, true, true);
	return 0x72945E;
}

ASMJIT_PATCH(0x7297C4, TunnelLocomotionClass_sub_729580_Dig, 6)
{
	GET(FootClass*, pFoot, EAX);

	TechnoExtData::HandleTunnelLocoStuffs(pFoot, false, false);
	return 0x7297F3;
}

ASMJIT_PATCH(0x7299A9, TunnelLocomotionClass_sub_7298F0_Dig, 5)
{
	GET(TunnelLocomotionClass*, pThis, ESI);

	TechnoExtData::HandleTunnelLocoStuffs(pThis->LinkedTo, false, true);
	return 0x729A34;
}

ASMJIT_PATCH(0x72920C, TunnelLocomotionClass_Turning, 9)
{
	GET(TunnelLocomotionClass*, pThis, ESI);

	if (pThis->_CoordsNow.IsValid())
		return 0;

	pThis->State = TunnelLocomotionClass::State::DUG_OUT;
	return 0x729369;
}