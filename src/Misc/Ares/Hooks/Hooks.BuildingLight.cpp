#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>

#include <BuildingLightClass.h>
#include <SpotlightClass.h>

ASMJIT_PATCH(0x436459, BuildingLightClass_Update, 6)
{
	GET(BuildingLightClass*, pThis, EDI);

	TechnoClass* Owner = pThis->OwnerObject;

	if (Owner && (Owner->AbstractFlags & AbstractFlags::Foot) != AbstractFlags::None)
	{
		TechnoTypeExtData* pTypeData = TechnoTypeExtContainer::Instance.Find(Owner->GetTechnoType());
		CoordStruct Loc = Owner->Location;
		const FacingClass* Facing = pTypeData->Spot_AttachedTo == SpotlightAttachment::Turret ?
			&Owner->SecondaryFacing : &Owner->PrimaryFacing;

		const double angle = double((int16_t)Facing->Current().Raw - Math::BINARY_ANGLE_MASK) * Math::DIRECTION_FIXED_MAGIC;
		const double distance = pTypeData->Spot_Distance;
		Loc.Y = Loc.Y - (static_cast<int>(Math::sin(angle) * distance));
		Loc.X = static_cast<int>(Math::cos(angle) * distance) + Loc.X;

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

ASMJIT_PATCH(0x4370c0, BuildingLightClass_SDDTOR, 0xA)
{
	GET(BuildingLightClass*, pThis, ECX);

	if (auto pTech = pThis->OwnerObject)
		TechnoExtContainer::Instance.Find(pTech)->BuildingLight = nullptr;

	return 0;
}

ASMJIT_PATCH(0x435820, BuildingLightClass_CTOR, 6)
{
	GET_STACK(TechnoClass*, pTech , 0x4);
	GET(BuildingLightClass*, pThis, ECX);

	if (pTech)
		TechnoExtContainer::Instance.Find(pTech)->BuildingLight = pThis;

	return 0;
}


static int Height;

#ifdef FINISH_THIS

#include <TacticalClass.h>

class NOVTABLE FakeBuildingLightClass : public BuildingLightClass
{
public:
	void _Draw(Point2D* pLocation, RectangleStruct* pBounds) {
		if (this->BehaviourMode != SpotlightBehaviour::None) {
			if (auto pBld = cast_to<BuildingClass*>(this->OwnerObject)) {
				if (pBld->IsAlive && pBld->IsPowerOnline() && !pBld->IsFogged &&
					((ScenarioClass::Instance->SpecialFlags.RawFlags & 0x1000) == 0 || !MapClass::Instance->IsLocationFogged(pBld->Location))) {

					auto pSpot = GameCreate<SpotlightClass>(pBld->Location, 16);
					auto caster_center = pBld->GetCoords();
					auto this_center = this->GetCoords();
					CoordStruct difference = this_center - caster_center;
					auto difference_sqrt = (int)difference.Length();
					int radius = RulesClass::Instance->SpotlightLocationRadius;
					int LocRad = 0;
					if (difference_sqrt >= radius) {
						LocRad = (difference_sqrt - radius) / ((RulesClass::Instance->SpotlightMovementRadius - radius) / 10);
					}

					int spot_movement_rad = 89;
					if (difference_sqrt > radius && this->BehaviourMode == SpotlightBehaviour::Follow) {
						spot_movement_rad = LocRad + 80 <= 0 ? 0 : LocRad + 80;
						if (spot_movement_rad > 89) {
							spot_movement_rad = 89;
						}
					}

					pSpot->MovementRadius = spot_movement_rad;
					pSpot->Draw();
					GameDelete<true, false>(pSpot);
					pSpot = nullptr;
					caster_center = pBld->GetCoords();
					this_center = this->Location;
					CoordStruct difference = this_center - caster_center;
					difference_sqrt = (int)difference.Length();
					int radius_2 = RulesClass::Instance->SpotlightRadius + (int)(this->GetMovementRadius() * Math::BUILDINGLIGHT_SCALLING_FACTOR);
					if (difference_sqrt >= radius_2)
					{
						auto asin = Math::asin((double)radius_2 / (double)difference_sqrt);
						Matrix3D mtx {};
						mtx.MakeIdentity();
						mtx.RotateZ(asin);
						const auto difference_here = this_center - caster_center;
						Vector3D<float> mult {};
						Vector3D<float> vec { (float)difference_here.X , (float)difference_here.Y , (float)difference_here.Z };
						Matrix3D::MatrixMultiply(&mult , &mtx, &vec);
						auto a_coord = difference + CoordStruct{ (int)mult.X, (int)mult.Y, (int)mult.Z };
						mtx.MakeIdentity();
						mtx.RotateZ(asin);
						auto a = TacticalClass::Instance->CoordsToClient(a_coord);
						auto b = TacticalClass::Instance->CoordsToClient(caster_center);
						auto caster_center_c = caster_center;
						caster_center_c.Z += 430;
						auto c = TacticalClass::Instance->CoordsToClient(caster_center_c);
						Point2D XY { Drawing::SurfaceDimensions_Hidden->X , Drawing::SurfaceDimensions_Hidden->Y };
						a += XY;
						b += XY;
						c += XY;

						auto height = Game::AdjustHeight(caster_center.Z + 400);
						
					}
				}
			}
		}
	}
};
#endif

ASMJIT_PATCH(0x436072, BuildingLightClass_Draw_430, 6)
{
	int lightamount = 0;
	if (Height >= 0)
		lightamount = Height;

	R->EAX(R->EBX<int>() + lightamount);
	return 0x436078;
}

ASMJIT_PATCH(0x4360D8, BuildingLightClass_Draw_400, 6)
{
	int lightamount = Height - 30;
	if (lightamount < 400)
		lightamount = 400;

	R->ECX(lightamount + R->EBX<int>());
	return 0x4360DE;
}

ASMJIT_PATCH(0x4360FF, BuildingLightClass_Draw_250, 6)
{
	int lightamount = Height - 180;
	if (lightamount < 250)
		lightamount = 250;

	R->ECX(lightamount + R->EDI<int>());
	return 0x436105;
}

ASMJIT_PATCH(0x435bfa, BuildingLightClass_Draw_Start, 6)
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

	Height = TechnoTypeExtContainer::Instance.Find(pOwner->GetTechnoType())->Spot_Height;
	return 0x435C52;
}

ASMJIT_PATCH(0x435cd3, BuildingLightClass_Draw_Spotlight, 6)
{
	GET_STACK(SpotlightClass*, Spot, 0x14);
	GET(BuildingLightClass*, BL, ESI);

	TechnoClass* Owner = BL->OwnerObject;
	TechnoTypeExtData* pTypeData = TechnoTypeExtContainer::Instance.Find(Owner->GetTechnoType());

	SpotlightFlags Flags = SpotlightFlags::None;
	if (pTypeData->Spot_DisableColor) {
		Flags |= SpotlightFlags::NoColor;

	} else {
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
	}

	Spot->DisableFlags = Flags;

	return 0;
}

ASMJIT_PATCH(0x436A2D, BuildingLightClass_PointerGotInvalid_OwnerCloak, 6)
{
	GET_STACK(bool, bRemoved, 0x10);
	return bRemoved ? 0x0 : 0x436A33;
}

ASMJIT_PATCH(0x4368C9, BuildingLightClass_Update_Trigger, 0x5)
{
	GET(TechnoClass*, pTechno, EAX);

	if (pTechno->AttachedTag) {
		pTechno->AttachedTag->RaiseEvent(TriggerEvent::EnemyInSpotlight, pTechno, CellStruct::Empty, 0, 0);
	}

	if (pTechno->IsAlive && pTechno->AttachedTag) {
		//66
		pTechno->AttachedTag->RaiseEvent((TriggerEvent)AresTriggerEvents::EnemyInSpotlightNow, pTechno, CellStruct::Empty, 0, 0);
	}

	return 0x4368D9;
}