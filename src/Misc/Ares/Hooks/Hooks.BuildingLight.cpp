#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>

#include <Ares_TechnoExt.h>
#include <BuildingLightClass.h>
#include <SpotlightClass.h>

static int Height;

DEFINE_OVERRIDE_HOOK(0x436459, BuildingLightClass_Update, 6)
{
	GET(BuildingLightClass*, pThis, EDI);

	TechnoClass* Owner = pThis->OwnerObject;

	if (Owner && Owner->WhatAmI() != AbstractType::Building)
	{
		TechnoTypeExt::ExtData* pTypeData = TechnoTypeExt::ExtMap.Find(Owner->GetTechnoType());
		CoordStruct Loc = Owner->Location;
		DirStruct Facing;
		switch (pTypeData->Spot_AttachedTo)
		{
		case SpotlightAttachment::Barrel:
			Facing = Owner->BarrelFacing.Current();
			break;
		case SpotlightAttachment::Turret:
			Facing = Owner->SecondaryFacing.Current();
			break;
		case SpotlightAttachment::Body:
		default:
			Facing = Owner->PrimaryFacing.Current();
		}

		static const double Facing2Rad = (2 * 3.14) / 0xFFFF;
		double Angle = Facing2Rad * static_cast<unsigned short>(Facing.GetValue());
		Loc.Y -= static_cast<int>(pTypeData->Spot_Distance * Math::cos(Angle));
		Loc.X += static_cast<int>(pTypeData->Spot_Distance * Math::sin(Angle));

		pThis->field_B8 = Loc;
		pThis->field_C4 = Loc;
		//		double zer0 = 0.0;
		__asm { fldz }
	}
	else
	{
		double Angle = RulesClass::Instance->SpotlightAngle;
		__asm { fld Angle }
	}

	return R->AL() ? 0x436461u : 0x4364C8u;
}

DEFINE_OVERRIDE_HOOK(0x4370c0, BuildingLightClass_SDDTOR, 0xA)
{
	GET(BuildingLightClass*, pThis, ECX);

	if (auto pTech = pThis->OwnerObject)
		TechnoExt::ExtMap.Find(pTech)->BuildingLight = nullptr;

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x435820, BuildingLightClass_CTOR, 6)
{
	GET_STACK(TechnoClass*, pTech , 0x4);
	GET(BuildingLightClass*, pThis, ECX);

	if (pTech)
		TechnoExt::ExtMap.Find(pTech)->BuildingLight = pThis;

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x436072, BuildingLightClass_Draw_430, 6)
{
	int lightamount = 0;
	if (Height > 0)
		lightamount = Height;

	R->EAX(R->EBX<int>() + lightamount);
	return 0x436078;
}

DEFINE_OVERRIDE_HOOK(0x4360D8, BuildingLightClass_Draw_400, 6)
{
	int lightamount = Height - 30;
	if (lightamount < 400)
		lightamount = 400;

	R->ECX(lightamount + R->EBX<int>());
	return 0x4360DE;
}

DEFINE_OVERRIDE_HOOK(0x4360FF, BuildingLightClass_Draw_250, 6)
{
	int lightamount = Height - 180;
	if (lightamount < 250)
		lightamount = 250;

	R->ECX(lightamount + R->EDI<int>());
	return 0x436105;
}

DEFINE_OVERRIDE_HOOK(0x436A2D, BuildingLightClass_PointerGotInvalid_OwnerCloak, 6)
{
	GET_STACK(bool, bRemoved, 0x10);
	return bRemoved ? 0x0 : 0x436A33;
}

DEFINE_OVERRIDE_HOOK(0x435bfa, BuildingLightClass_Draw_Start, 6)
{
	GET(BuildingLightClass*, pThis, ESI);

	const auto pOwner = pThis->OwnerObject;

	if (!pOwner
	  || pOwner->CloakState >= CloakState::Cloaked
	  || pOwner->Deactivated
	  || pOwner->IsBeingWarpedOut()
	  || pOwner->GetHeight() < -10
	  || pOwner->WhatAmI() == BuildingClass::AbsID &&
		(!pOwner->IsPowerOnline() || ((BuildingClass*)pOwner)->IsFogged))
	{
		return 0x4361BC;
	}

	Height = TechnoTypeExt::ExtMap.Find(pOwner->GetTechnoType())->Spot_Height;
	return 0x435C52;
}

DEFINE_OVERRIDE_HOOK(0x435cd3, BuildingLightClass_Draw_Spotlight, 6)
{
	GET_STACK(SpotlightClass*, Spot, 0x14);
	GET(BuildingLightClass*, BL, ESI);

	TechnoClass* Owner = BL->OwnerObject;
	TechnoTypeExt::ExtData* pTypeData = TechnoTypeExt::ExtMap.Find(Owner->GetTechnoType());

	SpotlightFlags Flags = SpotlightFlags::None;
	if (pTypeData->Spot_DisableColor)
	{
		Flags |= SpotlightFlags::NoColor;
	}
	if (pTypeData->Spot_DisableR)
	{
		Flags |= SpotlightFlags::NoRed;
	}
	if (pTypeData->Spot_DisableG)
	{
		Flags |= SpotlightFlags::NoGreen;
	}
	if (pTypeData->Spot_DisableB)
	{
		Flags |= SpotlightFlags::NoBlue;
	}

	Spot->DisableFlags = Flags;

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x4368C9, BuildingLightClass_Update_Trigger, 0x5)
{
	GET(TechnoClass*, pTechno, EAX);

	if (pTechno->AttachedTag)
	{
		pTechno->AttachedTag->RaiseEvent(TriggerEvent::EnemyInSpotlight, pTechno, CellStruct::Empty, 0, 0);
	}

	if (pTechno->IsAlive)
	{
		if (pTechno->AttachedTag)
		{
			//66
			pTechno->AttachedTag->RaiseEvent((TriggerEvent)AresTriggerEvents::EnemyInSpotlightNow, pTechno, CellStruct::Empty, 0, 0);
		}
	}

	return 0x4368D9;
}