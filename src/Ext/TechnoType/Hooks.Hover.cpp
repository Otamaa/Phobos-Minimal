#ifdef ENABLE_NEWHOOKS
#include <New/Type/HoverTypeClass.h>

#include <Ext/TechnoType/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <HoverLocomotionClass.h>
#include <DriveLocomotionClass.h>
#include <DropPodLocomotionClass.h>
/*; hover vehicle characteristics

1484    HoverHeight=120         ; height of hovering vehicles
1520    HoverDampen=40%         ; dampening effect on hover vehicle bounciness
1488    HoverBob=.04            ; time between hover 'bobs'
1496    HoverBoost=150%         ; hover speed when traveling on straight away
1504    HoverAcceleration=.02   ; time to accelerate to full speed
1512    HoverBrake=.03          ; time to decelerate to full stop
*/

static const HoverTypeClass* GetHover(TechnoTypeClass* pThis)
{
	auto const pDefault = HoverTypeClass::FindOrAllocate(NONE_STR);
	if (auto pExt = TechnoTypeExt::ExtMap.Find(pThis))
			return pExt->HoverType.Get(pDefault);

	return pDefault;
}

DEFINE_HOOK(0x513DD6, HoverLocomotionClass_513D20_HoverHeight1, 0x6)
{
	GET(HoverLocomotionClass* const, pLoco, ESI);
	GET_STACK(int, heightGet, STACK_OFFS(0x2C, 0x18));

	auto const Linked = pLoco->Owner;
	auto const pType = Linked->GetTechnoType();
	auto const pHover = GetHover(pType);
	auto height = pHover->GetHeight();

	R->EAX(heightGet - height);

	return 0x513DDC;
}

DEFINE_HOOK(0x513E8F, HoverLocomotionClass_513D20_HoverHeight2, 0x6)
{
	enum { ContinueCheck = 0x513E9D, SetHoverDampen = 0x513F0E };

	GET(HoverLocomotionClass* const, pLoco, ESI);
	GET_STACK(int, comparator, STACK_OFFS(0x2C, 0x14));

	auto const Linked = pLoco->Owner;
	auto const pType = Linked->GetTechnoType();
	auto const pHover = GetHover(pType);
	auto height = pHover->GetHeight();

	return comparator < height ? ContinueCheck : SetHoverDampen;
}

//513EAA
DEFINE_HOOK(0x513EAA, HoverLocomotionClass_513D20_HoverHeight3, 0x5)
{
	GET(HoverLocomotionClass* const, pLoco, ESI);

	auto const Linked = pLoco->Owner;
	auto const pType = Linked->GetTechnoType();
	auto const pHover = GetHover(pType);
	auto height = pHover->GetHeight();

	_asm fild dword ptr[height];

	R->EAX(RulesClass::Instance());
	return 0x513EB5;
}

DEFINE_HOOK(0x513ECD, HoverLocomotionClass_513D20_HoverHeight4, 0x6)
{
	GET(HoverLocomotionClass* const, pLoco, ESI);

	auto const Linked = pLoco->Owner;
	auto const pType = Linked->GetTechnoType();
	auto const pHover = GetHover(pType);
	auto height = pHover->GetHeight();

	R->ECX(RulesClass::Instance());
	R->EAX(height);

	return 0x513ED9;
}

DEFINE_HOOK(0x513F1B, HoverLocomotionClass_513D20_HoverDampen, 0x6)
{
	GET(HoverLocomotionClass* const, pLoco, ESI);

	auto const Linked = pLoco->Owner;
	auto const pType = Linked->GetTechnoType();
	auto const pHover = GetHover(pType);
	auto dampen = pHover->GetDampen();

	_asm fmul qword ptr[dampen];

	return 0x513F27;
}

DEFINE_HOOK(0x513E14, HoverLocomotionClass_513D20_HoverBob, 0x6)
{
	R->ECX(Unsorted::CurrentFrame); //Uhh ...
	GET(HoverLocomotionClass* const, pLoco, ESI);

	auto const Linked = pLoco->Owner;
	auto const pType = Linked->GetTechnoType();
	auto const pHover = GetHover(pType);
	auto bob = pHover->GetBob();

	_asm fmul qword ptr[bob];

	return 0x513E20;
}

/*
DEFINE_HOOK(0x514A65, HoverLocomotionClass_513D20_AnimUnderWater, 0xB)
{
	GET(HoverLocomotionClass* const, pLoco, ESI);

	auto const Linked = pLoco->Owner;
	auto const pType = Linked->GetTechnoType();
	auto const pExt = TechnoTypeExt::ExtMap.Find(pType);
	auto pAnimType = pExt->HoverType->GetAboveWaterAnim();

	if (pAnimType && (Linked->CloakState != CloakState::Cloaked || Linked->CloakState != CloakState::Cloaking))
	{
		auto nCoord = Linked->GetCenterCoords();
		if (auto pAnim = GameCreate<AnimClass>(pAnimType, nCoord))
			AnimExt::SetAnimOwnerHouseKind(pAnim, Linked->Owner, nullptr, AnimTypeExt::ExtMap.Find(pAnim->Type)->Anim_Owner.Get(), false);
	}

	return 0x514AC8;
}*/

DEFINE_HOOK(0x514A32, HoverLocomotionClass_513D20_Anim, 0xB)
{
	GET(HoverLocomotionClass* const, pLoco, ESI);

	if (!(Unsorted::CurrentFrame % 10))
	{
		auto const Linked = pLoco->Owner;
		auto const pType = Linked->GetTechnoType();
		auto const pHover = GetHover(pType);
		auto pAnimType = pHover->GetAboveWaterAnim();
		bool bIsCloked = (Linked->CloakState == CloakState::Cloaked || Linked->CloakState == CloakState::Cloaking);

		if (!Linked->IsOnBridge() && Linked->GetCell()->LandType == LandType::Water)
		{
			if (pAnimType && !bIsCloked)
			{
				auto nCoord = Linked->GetCoords();
				if (auto pAnim = GameCreate<AnimClass>(pAnimType, nCoord)){
					AnimExt::SetAnimOwnerHouseKind(pAnim, Linked->Owner, nullptr, false);
					if (auto pExt = AnimExt::GetExtData(pAnim))
						pExt->Invoker = Linked;
				}
			}
		}
	}

	return 0x514AC8;
}

//
DEFINE_HOOK(0x516179, HoverLocomotionClass_515ED0_HoverAccel, 0x6)
{
	GET(HoverLocomotionClass* const, pLoco, ESI);

	auto const Linked = pLoco->Owner;
	auto const pType = Linked->GetTechnoType();
	auto const pHover = GetHover(pType);
	auto accel = pHover->GetAccel();

	_asm fld qword ptr[accel];

	return 0x516185;
}

DEFINE_HOOK(0x5161B1, HoverLocomotionClass_515ED0_HoverBrake, 0x6)
{
	GET(HoverLocomotionClass* const, pLoco, ESI);

	auto const Linked = pLoco->Owner;
	auto const pType = Linked->GetTechnoType();
	auto const pHover = GetHover(pType);
	auto brake = pHover->GetBrake();

	_asm fld qword ptr[brake];

	return 0x5161BD;
}

DEFINE_HOOK(0x5167FC, HoverLocomotionClass_515ED0_ScoldSound, 0x5)
{
	GET(HoverLocomotionClass* const, pLoco, ESI);

	auto const Linked = pLoco->Owner;
	auto const pType = Linked->GetTechnoType();
	auto const pHover = GetHover(pType);

	if (auto sound = pHover->GetScoldSound())
		VocClass::PlayGlobal(sound, Panning::Center, 1.0);

	return 0x516818;
}

//#define LODWORD(x)  (*((unsigned int*)&(x)))  // low dword
//#define HIDWORD(x)  (*((unsigned int*)&(x)+1))

DEFINE_HOOK(0x51613B, HoverLocomotionClass_515ED0_HoverBoost, 0xC)
{
	GET(HoverLocomotionClass* const, pLoco, ESI);

	auto const Linked = pLoco->Owner;
	auto const pType = Linked->GetTechnoType();
	auto const pHover = GetHover(pType);
	auto nBoostExt = pHover->GetBoost();

	//auto nVal_Lo = (*((unsigned int*)&(nBoostExt)));
	//auto nVal_HI = (*((unsigned int*)&(nBoostExt)+1));
	pLoco->__Boost += nBoostExt;

	return 0x516152;
}
#endif