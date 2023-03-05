#ifndef ENABLE_NEWHOOKS
//TODO : desync test
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

static const HoverTypeClass* GetHover(TechnoClass* pThis)
{
	auto const pDefault = HoverTypeClass::FindOrAllocate(DEFAULT_STR2);

	if (!pThis)
		return pDefault;

	auto const pTechnoType = pThis->GetTechnoType();
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoType);
	return pTypeExt->HoverType.Get(pDefault);
}

static TechnoClass* GetOwner(HoverLocomotionClass* pThis)
{
	return pThis->Owner ? pThis->Owner : pThis->LinkedTo;
}

DEFINE_HOOK(0x513DD6, HoverLocomotionClass_513D20_HoverHeight1, 0x6)
{
	GET(HoverLocomotionClass* const, pLoco, ESI);
	GET_STACK(int, heightGet, STACK_OFFS(0x2C, 0x18));
	const auto height = GetHover(GetOwner(pLoco))->GetHeight();
	R->EAX(heightGet - height);
	return 0x513DDC;
}

DEFINE_HOOK(0x513E8F, HoverLocomotionClass_513D20_HoverHeight2, 0x6)
{
	enum { ContinueCheck = 0x513E9D, SetHoverDampen = 0x513F0E };

	GET(HoverLocomotionClass* const, pLoco, ESI);
	GET_STACK(int, comparator, STACK_OFFS(0x2C, 0x14));
	const auto height = GetHover(GetOwner(pLoco))->GetHeight();
	return comparator < height ? ContinueCheck : SetHoverDampen;
}

DEFINE_HOOK(0x513EAA, HoverLocomotionClass_513D20_HoverHeight3, 0x5)
{
	GET(HoverLocomotionClass* const, pLoco, ESI);
	const auto height = GetHover(GetOwner(pLoco))->GetHeight();
	_asm fild dword ptr[height];
	R->EAX(RulesClass::Instance());
	return 0x513EB5;
}

DEFINE_HOOK(0x513ECD, HoverLocomotionClass_513D20_HoverHeight4, 0x6)
{
	GET(HoverLocomotionClass* const, pLoco, ESI);
	const auto height = GetHover(GetOwner(pLoco))->GetHeight();
	R->ECX(RulesClass::Instance());
	R->EAX(height);
	return 0x513ED9;
}

DEFINE_HOOK(0x513F1B, HoverLocomotionClass_513D20_HoverDampen, 0x6)
{
	GET(HoverLocomotionClass* const, pLoco, ESI);
	const auto dampen = GetHover(GetOwner(pLoco))->GetDampen();
	_asm fmul qword ptr[dampen];
	return 0x513F27;
}

DEFINE_HOOK(0x513E14, HoverLocomotionClass_513D20_HoverBob, 0x6)
{
	R->ECX(Unsorted::CurrentFrame()); //Uhh ...
	GET(HoverLocomotionClass* const, pLoco, ESI);
	const auto bob = GetHover(GetOwner(pLoco))->GetBob();
	_asm fmul qword ptr[bob];
	return 0x513E20;
}

DEFINE_HOOK(0x514A32, HoverLocomotionClass_513D20_Anim, 0x5) //B
{
	GET(HoverLocomotionClass* const, pLoco, ESI);

	if (!(Unsorted::CurrentFrame % 10))
	{
		const auto Linked = GetOwner(pLoco);

		if (!Linked->IsOnBridge() && Linked->GetCell()->LandType == LandType::Water)
		{
			if (const auto pAnimType = GetHover(Linked)->GetAboveWaterAnim())
			{
				const auto nCoord = Linked->GetCoords();
				if (auto pAnim = GameCreate<AnimClass>(pAnimType, nCoord)){
					AnimExt::SetAnimOwnerHouseKind(pAnim, Linked->Owner, nullptr, Linked, false);
				}
			}
		}
	}

	return 0x514AC8;
}

DEFINE_HOOK(0x516179, HoverLocomotionClass_515ED0_HoverAccel, 0x6)
{
	GET(HoverLocomotionClass* const, pLoco, ESI);
	const auto accel = GetHover(GetOwner(pLoco))->GetAccel();
	_asm fld qword ptr[accel];
	return 0x516185;
}

DEFINE_HOOK(0x5161B1, HoverLocomotionClass_515ED0_HoverBrake, 0x6)
{
	GET(HoverLocomotionClass* const, pLoco, ESI);
	const auto brake = GetHover(GetOwner(pLoco))->GetBrake();
	_asm fld qword ptr[brake];
	return 0x5161BD;
}

DEFINE_HOOK(0x5167FC, HoverLocomotionClass_515ED0_ScoldSound, 0x5)
{
	GET(HoverLocomotionClass* const, pLoco, ESI);

	if (const auto sound = GetHover(GetOwner(pLoco))->GetScoldSound())
		VocClass::PlayGlobal(sound, Panning::Center, 1.0);

	return 0x516818;
}

DEFINE_HOOK(0x51613B, HoverLocomotionClass_515ED0_HoverBoost, 0x6) // C
{
	GET(HoverLocomotionClass* const, pLoco, ESI);
	const auto nBoostExt = GetHover(GetOwner(pLoco))->GetBoost();
	pLoco->__Boost += nBoostExt;
	return 0x516152;
}
#endif