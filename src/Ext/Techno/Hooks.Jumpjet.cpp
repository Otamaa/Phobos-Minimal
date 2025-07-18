#include "Body.h"

#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>

#include <Utilities/Macro.h>

#include <Locomotor/Cast.h>

// Fix [JumpjetControls] obsolete in RA2/YR
// Author: Uranusian
ASMJIT_PATCH(0x7115AE, TechnoTypeClass_CTOR_JumpjetControls, 0xA)
{
	GET(TechnoTypeClass*, pThis, ESI);
	const auto pRulesExt = RulesExtData::Instance();

	pThis->JumpJetData.TurnRate = pRulesExt->AttachedToObject->TurnRate;
	pThis->JumpJetData.Speed = pRulesExt->AttachedToObject->Speed;
	pThis->JumpJetData.Climb = static_cast<float>(pRulesExt->AttachedToObject->Climb);
	pThis->JumpJetData.Crash = static_cast<float>(pRulesExt->JumpjetCrash.Get());
	pThis->JumpJetData.Height = pRulesExt->AttachedToObject->CruiseHeight;
	pThis->JumpJetData.Accel = static_cast<float>(pRulesExt->AttachedToObject->Acceleration);
	pThis->JumpJetData.Wobbles = static_cast<float>(pRulesExt->AttachedToObject->WobblesPerSecond);
	pThis->JumpJetData.NoWobbles = pRulesExt->JumpjetNoWobbles.Get();
	pThis->JumpJetData.Deviation = pRulesExt->AttachedToObject->WobbleDeviation;

	return 0x711601;
}

// skip vanilla JumpjetControls and make it earlier load
//DEFINE_SKIP_HOOK(0x668EB5 , RulesClass_Process_SkipJumpjetControls ,0x8 , 668EBD);
//DEFINE_JUMP(LJMP, 0x668EB5, 0x668EBD); // RulesClass_Process_SkipJumpjetControls

// ASMJIT_PATCH(0x52D0F9, InitRules_EarlyLoadJumpjetControls, 0x6)
// {
// 	GET(RulesClass*, pThis, ECX);
// 	GET(CCINIClass*, pINI, EAX);
//
// 	RulesExtData::LoadEarlyBeforeColor(pThis, pINI);
// 	pThis->Read_JumpjetControls(pINI);
//
// 	return 0;
// }

//ASMJIT_PATCH(0x6744E4, RulesClass_ReadJumpjetControls_Extra, 0x7)
//{
//	if (const auto pRulesExt = RulesExtData::Instance())
//	{
//		GET(CCINIClass*, pINI, EDI);
//
//		INI_EX exINI(pINI);
//
//		pRulesExt->JumpjetCrash.Read(exINI, GameStrings::JumpjetControls(), "Crash");
//		pRulesExt->JumpjetNoWobbles.Read(exINI, GameStrings::JumpjetControls(), "NoWobbles");
//	}
//
//	return 0;
//}

ASMJIT_PATCH(0x54C036, JumpjetLocomotionClass_State3_UpdateSensors, 0x7)
{
	GET(FootClass* const, pLinkedTo, ECX);
	GET(CellStruct const, currentCell, EAX);

	const auto pType = pLinkedTo->GetTechnoType();

	if (pType->Sensors && pType->SensorsSight > 0
		&& pLinkedTo->LastFlightMapCoords != currentCell) {
		pLinkedTo->RemoveSensorsAt(pLinkedTo->LastFlightMapCoords);

		if(pLinkedTo->IsAlive)
			pLinkedTo->AddSensorsAt(currentCell);
	}

	return 0;
}

#include <AircraftTrackerClass.h>

ASMJIT_PATCH(0x4CD64E , FlyLocomotionClass_MovementAI_UpdateSensors, 0xA)
{
	GET(FlyLocomotionClass* const, pThis, ESI);
	GET(CellStruct, currentCell, EDI);

	const auto pLinkedTo = pThis->LinkedTo;
	const auto pType = pLinkedTo->GetTechnoType();

	if (pType->Sensors && pType->SensorsSight > 0) {
		pLinkedTo->RemoveSensorsAt(pLinkedTo->LastFlightMapCoords);

		if(pLinkedTo->IsAlive)
			pLinkedTo->AddSensorsAt(currentCell);
	}

	AircraftTrackerClass::Instance->Update(pLinkedTo, pLinkedTo->LastFlightMapCoords, currentCell);

	return 0x4CD664;
}

//ASMJIT_PATCH(0x54B8E9, JumpjetLocomotionClass_In_Which_Layer_Deviation, 0x6)
//{
//	GET(TechnoClass*, pThis, EAX);
//
//	if (pThis->IsInAir())
//	{
//		if (!TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType())->JumpjetAllowLayerDeviation
//			.Get(RulesExtData::Instance()->JumpjetAllowLayerDeviation.Get()))
//		{
//			R->EDX(INT32_MAX); // Override JumpjetHeight / CruiseHeight check so it always results in 3 / Layer::Air.
//			return 0x54B96B;
//		}
//	}
//
//	return 0;
//}

ASMJIT_PATCH(0x54D138, JumpjetLocomotionClass_Movement_AI_SpeedModifiers, 0x6)
{
	GET(JumpjetLocomotionClass*, pThis, ESI);

	if (auto const pLinked = pThis->LinkedTo ? pThis->LinkedTo : pThis->Owner) {
		if (TechnoExtData::IsReallyTechno(pLinked) && pLinked->IsAlive) {
			const double multiplier = TechnoExtData::GetCurrentSpeedMultiplier(pLinked);
			pThis->Speed = int(pLinked->GetTechnoType()->JumpJetData.Speed * multiplier);
		}
	}

	return 0;
}

ASMJIT_PATCH(0x54CB0E, JumpjetLocomotionClass_State5_CrashRotation, 0x7)
{
	GET(JumpjetLocomotionClass*, pLoco, EDI);

	bool bRotate = RulesExtData::Instance()->JumpjetCrash_Rotate;

	if (const auto pOwner = pLoco->LinkedTo ? pLoco->LinkedTo : pLoco->Owner) {
		bRotate = TechnoTypeExtContainer::Instance.Find(pOwner->GetTechnoType())->JumpjetCrash_Rotate.Get(bRotate);
	}

	return bRotate ? 0 : 0x54CB3E;

}

//DEFINE_JUMP(LJMP, 0x54DCCF, 0x54DCE8);//JumpjetLocomotionClass_DrawMatrix_NoTiltCrashJumpjetHereBlyat

// We no longer explicitly check TiltCrashJumpjet when drawing, do it when crashing
ASMJIT_PATCH(0x70B649, TechnoClass_RigidBodyDynamics_NoTiltCrashBlyat, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	if (flag_cast_to<FootClass*, false>(pThis) && locomotion_cast<JumpjetLocomotionClass*>(((FootClass*)pThis)->Locomotor) && !pThis->GetTechnoType()->TiltCrashJumpjet)
		return 0x70BCA4;

	return 0;
}

// ASMJIT_PATCH(0x54DD3D, JumpjetLocomotionClass_DrawMatrix_AxisCenterInAir, 0x5)
// {
// 	GET(ILocomotion*, iloco, ESI);
// 	if (static_cast<JumpjetLocomotionClass*>(iloco)->NextState == JumpjetLocomotionClass::State::Grounded)
// 		return 0;
//
// 	return 0x54DE88;
// }

namespace JumpjetTiltReference
{
	constexpr auto BaseSpeed = 32;
	constexpr auto BaseTilt = Math::HalfPi / 4;
	constexpr auto BaseTurnRaw = 32768;
	constexpr auto MaxTilt = static_cast<float>(Math::HalfPi);
	constexpr auto ForwardBaseTilt = BaseTilt / BaseSpeed;
	constexpr auto SidewaysBaseTilt = BaseTilt / (BaseTurnRaw * BaseSpeed);
}

// Just rewrite this completely to avoid headache
Matrix3D* __stdcall JumpjetLocomotionClass_Draw_Matrix(ILocomotion* iloco, Matrix3D* ret, PhobosVoxelIndexKey* pIndex)
{
	auto const pThis = static_cast<JumpjetLocomotionClass*>(iloco);
	auto linked = pThis->LinkedTo;
	// no more TiltCrashJumpjet, do that above svp
	bool const onGround = pThis->NextState == JumpjetLocomotionClass::State::Grounded;
	// Man, what can I say, you don't want to stick your rotor into the ground
	auto slope_idx = MapClass::Instance->GetCellAt(linked->Location)->SlopeIndex;
	// Only use LocomotionFacing for general Jumpjet to avoid the problem that ground units being lifted will turn to attacker weirdly.
	auto curf = linked->IsAttackedByLocomotor ? &linked->PrimaryFacing : &pThis->Facing;
	auto pTypeExt = TechnoTypeExtContainer::Instance.Find(linked->GetTechnoType());

	*ret = Game::VoxelRampMatrix[onGround ? slope_idx : 0];
	ret->RotateZ((float)curf->Current().GetRadian<32>());

	float arf = linked->AngleRotatedForwards;
	float ars = linked->AngleRotatedSideways;
	size_t arfFace = 0;
	size_t arsFace = 0;

	if (Math::abs(ars) >= 0.005 || Math::abs(arf) >= 0.005)
	{
		if (pIndex) pIndex->Invalidate();

		if (onGround)
		{
			double scalex = linked->GetTechnoType()->VoxelScaleX;
			double scaley = linked->GetTechnoType()->VoxelScaleY;
			Matrix3D pre = Matrix3D::GetIdentity();
			pre.TranslateZ(float(Math::abs(Math::sin(ars)) * scalex + Math::abs(Math::sin(arf)) * scaley));
			ret->TranslateX(float(Math::signum(arf) * (scaley * (1 - Math::cos(arf)))));
			ret->TranslateY(float(Math::signum(-ars) * (scalex * (1 - Math::cos(ars)))));
			ret->RotateX(ars);
			ret->RotateY(arf);
			*ret = pre * *ret;
		}
		else
		{
			// No more translation because I don't like it
			ret->RotateX(ars);
			ret->RotateY(arf);
		}
	}

	else if (pTypeExt->JumpjetTilt.Get(RulesExtData::Instance()->JumpjetTilt)
		&& !onGround && linked->IsAlive && linked->Health > 0 && !linked->IsAttackedByLocomotor)
	{
		constexpr auto maxTilt = static_cast<float>(Math::HalfPi);
		constexpr auto baseSpeed = 32;
		constexpr auto baseTilt = Math::HalfPi / 4;

		if (pThis->__currentSpeed > 0.0)
		{

			constexpr auto forwardBaseTilt = baseTilt / baseSpeed;
			const auto forwardSpeedFactor = pThis->Speed * pTypeExt->JumpjetTilt_ForwardSpeedFactor;
			const auto forwardAccelFactor = pThis->Acceleration * pTypeExt->JumpjetTilt_ForwardAccelFactor;
			arf = std::clamp(static_cast<float>((forwardAccelFactor + forwardSpeedFactor)
			* forwardBaseTilt), -maxTilt, maxTilt);


			const auto& locoFace = pThis->Facing;

			if (locoFace.Is_Rotating()) {
				const float sidewaysSpeedFactor = static_cast<float>(pThis->Speed * pTypeExt->JumpjetTilt_SidewaysSpeedFactor);
				const float sidewaysRotationFactor = static_cast<float>(static_cast<short>(locoFace.Difference().Raw)
					* pTypeExt->JumpjetTilt_SidewaysRotationFactor);

				ars = std::clamp(static_cast<float>(sidewaysSpeedFactor * sidewaysRotationFactor
					* JumpjetTiltReference::SidewaysBaseTilt), -JumpjetTiltReference::MaxTilt, JumpjetTiltReference::MaxTilt);

				const auto arsDir = DirStruct(ars);
				arsFace = (arsDir.GetFacing<128>() + 96u) & 0x7Fu;

				if (arsFace)
					ret->RotateX(static_cast<float>(arsDir.GetRadian<128>()));
				}
			}
		}

	if (pIndex && pIndex->IsValidKey())
	{
		// It is currently unclear whether the passed key only has two situations:
		// all 0s and all 1s, so I use the safest approach for now
		if (pIndex->IsCleanKey() && (arfFace || arsFace))
		{
			pIndex->CustomIndexKey.JumpjetTiltVoxel.forwards = arfFace;
			pIndex->CustomIndexKey.JumpjetTiltVoxel.sideways = arsFace;

			if (onGround)
				pIndex->CustomIndexKey.JumpjetTiltVoxel.slopeIndex = slope_idx;

			pIndex->CustomIndexKey.JumpjetTiltVoxel.bodyFace = curf->Current().GetFacing<32>();

			// Outside the function, there is another step to add a frame number to the key for drawing
			pIndex->Value >>= 5;
		}
		else // Keep the original code
		{
			if (onGround)
				pIndex->Value = slope_idx + (pIndex->Value << 6);

			pIndex->Value <<= 5;
			pIndex->Value |= curf->Current().GetFacing<32>();;
		}
	}

	return ret;
}

ASMJIT_PATCH(0x73B748, UnitClass_DrawVXL_ResetKeyForTurretUse, 0x7)
{
	REF_STACK(PhobosVoxelIndexKey, key, STACK_OFFSET(0x1C4, -0x1B0));

	// Main body drawing completed, then enable accurate drawing of turrets and barrels
	if (key.IsValidKey() && key.IsExtraBodyKey()) // Flags used by JumpjetTilt units
		key.Invalidate();

	return 0;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7ECD8C, JumpjetLocomotionClass_Draw_Matrix);
//TODO : Issue #690 #655

// Otamaa
//ASMJIT_PATCH(0x54DCE8, JumpetLocomotionClass_DrawMatrix, 0x9)
//{
//	GET(ILocomotion*, pILoco, ESI);
//	auto pLoco = static_cast<JumpjetLocomotionClass*>(pILoco);
//
//	if (ILocomotionPtr pPiggy = pLoco->Owner->Locomotor)
//	{
//
//	}
//
//	return LocomotionClass::End_Piggyback(pLoco->Owner->Locomotor) ? 0x0 : 0x54DF13;
//}

ASMJIT_PATCH(0x54D208, JumpjetLocomotionClass_MovementAI_Wobbles, 0x5)
{
	enum
	{
		SetWobble = 0x54D20F,
		NoWobble = 0x54D22C
	};

	GET(JumpjetLocomotionClass* const, pThis, ESI);

	//prevent float zero division error
	if (pThis->LinkedTo->Deactivated || pThis->LinkedTo->IsUnderEMP() || Math::abs(pThis->Wobbles) < 0.001f || isnan(pThis->Wobbles)) {
		return NoWobble;
	}

	if (pThis->NoWobbles)
		return NoWobble;

	if (const auto pUnit = cast_to<UnitClass*, false>(pThis->LinkedTo ? pThis->LinkedTo : pThis->Owner)){
		if(TechnoExtData::IsReallyTechno(pUnit) && pUnit->IsAlive) {
			return pUnit->IsDeactivated() ? NoWobble : SetWobble;
		}
	}

	return SetWobble;
}

ASMJIT_PATCH(0x54D326, JumpjetLocomotionClass_MovementAI_CrashSpeedFix, 0x6)
{
	GET(JumpjetLocomotionClass*, pThis, ESI);
	return pThis->LinkedTo->IsCrashing ? 0x54D350 : 0;
}

ASMJIT_PATCH(0x54B6E0, JumpjetLocomotionClass_DoTurn, 0x8)
{
	GET_STACK(ILocomotion*, iloco, 0x4);
	GET_STACK(DirStruct, dir, 0x8);
	// This seems to be used only when unloading shit on the ground
	// Rewrite just in case
	auto pThis = static_cast<JumpjetLocomotionClass*>(iloco);
	pThis->Facing.Set_Desired(dir);
	pThis->LinkedTo->PrimaryFacing.Set_Desired(dir);
	return 0x54B6FF;
}

// Bugfix: Jumpjet turn to target when attacking
// Even though it's still not the best place to do this, given that 0x54BF5B has done the similar action, I'll do it here too
//ASMJIT_PATCH(0x54BD93, JumpjetLocomotionClass_State2_54BD30_TurnToTarget, 0x6)
//{
//	enum { ContinueNoTarget = 0x54BDA1, EndFunction = 0x54BFDE, ContinueFunc = 0x54BDA1 };
//
//	GET(JumpjetLocomotionClass* const, pLoco, ESI);
//	GET(FootClass* const, pLinkedTo, EDI);
//
//	const auto pTarget = pLinkedTo->Target;
//	if (!pTarget)
//		return ContinueNoTarget;
//
//	if (const auto pThis = abstract_cast<UnitClass*>(pLinkedTo))
//	{
//		if (TechnoTypeExtContainer::Instance.Find(pThis->Type)->JumpjetTurnToTarget.Get(RulesExtData::Instance()->JumpjetTurnToTarget))
//		{
//			CoordStruct& source = pThis->Location;
//			CoordStruct target = pTarget->GetCoords();
//			DirStruct tgtDir = DirStruct(Math::atan2(source.Y - target.Y, target.X - source.X));
//
//			if (pThis->GetRealFacing().Current().GetFacing<32>() != tgtDir.GetFacing<32>())
//				pLoco->Facing.Set_Desired(tgtDir);
//
//			R->EAX(pTarget);
//			return EndFunction;
//		}
//	}
//
//	return ContinueFunc;
//}

//ASMJIT_PATCH(0x54AEC0, JumpjetLocomotionClass_Process_TurnToTarget, 0x8)
//{
	//GET_STACK(ILocomotion*, iLoco, 0x4);
//	const auto pLoco = static_cast<JumpjetLocomotionClass*>(iLoco);
//const auto pThis = pLoco->Owner;
//	const auto pType = pThis->GetTechnoType();
//	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find<false>(pType);
//
//	if (pTypeExt && pTypeExt->JumpjetTurnToTarget.Get(RulesExtData::Instance()->JumpjetTurnToTarget) &&
	//	pThis->WhatAmI() == AbstractType::Unit && pThis->IsInAir() && !pType->TurretSpins && pLoco)
	//{
	//	if (const auto pTarget = pThis->Target)
	//	{
//			const CoordStruct source = pThis->Location;
	//		const CoordStruct target = pTarget->GetCoords();
//			const DirStruct tgtDir = DirStruct(static_cast<double>(source.Y - target.Y), static_cast<double>(target.X - source.X));
//			if (pThis->GetRealFacing().current().value32() != tgtDir.value32())
	//			pLoco->Facing.turn(tgtDir);
//		}
//	}
//	return 0;
//}

// Let the jumpjet increase their height earlier or simply skip the stop check
namespace JumpjetRushHelpers
{
	bool Skip = false;
	int GetJumpjetHeightWithOccupyTechno(Point2D location); // Replace sub_485080
	int JumpjetLocomotionPredictHeight(JumpjetLocomotionClass* pThis); // Replace sub_54D820
}

int JumpjetRushHelpers::GetJumpjetHeightWithOccupyTechno(Point2D location)
{
	CellClass* const pCell = MapClass::Instance->TryGetCellAt(CellStruct { short(location.X >> 8) , short(location.Y >> 8) });
	if(!pCell)
		return -1;

	int height = pCell->GetFloorHeight({ location.X & 0xFF, location.Y & 0xFF });

	for(auto pObject = pCell->FirstObject; pObject; pObject = pObject->NextObject) {
      if(auto pBld = cast_to<BuildingClass*, false>(pObject))  {
        CoordStruct dim2 = CoordStruct::Empty;
		pBld->Type->Dimension2(&dim2);
	    return dim2.Z + height;
	  }
   }

	if (pCell->FindTechnoNearestTo(Point2D::Empty, false))
		height += 85;

	if (pCell->Flags & CellFlags::BridgeHead)
		height += Unsorted::BridgeHeight;

	return height;
}

int JumpjetRushHelpers::JumpjetLocomotionPredictHeight(JumpjetLocomotionClass* pThis)
{
	FootClass* const pFoot = pThis->LinkedTo;
	const CoordStruct location = pFoot->Location;
	const int curHeight = location.Z - pFoot->GetTechnoType()->JumpJetData.Height;
	Point2D curCoord = { location.X, location.Y };
	int maxHeight = JumpjetRushHelpers::GetJumpjetHeightWithOccupyTechno(curCoord);

	if (pThis->NextState == JumpjetLocomotionClass::State::Cruising)
	{
		const double checkLength = Unsorted::BridgeHeight / pThis->Climb * pThis->__currentSpeed;
		const double angle = -pThis->Facing.Current().GetRadian<32>();
		Point2D stepCoord { static_cast<int>(checkLength * Math::cos(angle)), static_cast<int>(checkLength * Math::sin(angle)) };
		const int largeStep = std::max(Math::abs(stepCoord.X), Math::abs(stepCoord.Y));

		if (largeStep)
		{
			const double stepMult = static_cast<double>(Unsorted::LeptonsPerCell / 2) / largeStep;
			stepCoord = { static_cast<int>(stepCoord.X * stepMult), static_cast<int>(stepCoord.Y * stepMult) };

			curCoord += stepCoord;
			int height = JumpjetRushHelpers::GetJumpjetHeightWithOccupyTechno(curCoord);

			if (height > maxHeight)
				maxHeight = height;
			else
				JumpjetRushHelpers::Skip = true;

			if (maxHeight <= curHeight)
				JumpjetRushHelpers::Skip = true;

			const int checkStep = (largeStep >> 7) + ((largeStep & 0x7F) ? 2 : 1);

			for (int i = 0; i < checkStep && height >= 0; ++i)
			{
				curCoord += stepCoord;
				height = JumpjetRushHelpers::GetJumpjetHeightWithOccupyTechno(curCoord);

				if (height > maxHeight)
					maxHeight = height;
			}
		}
	}

	return maxHeight >= 0 ? maxHeight : curHeight;
}

ASMJIT_PATCH(0x54D827, JumpjetLocomotionClass_sub_54D820_PredictHeight, 0x8)
{
	GET(JumpjetLocomotionClass*, pThis, ESI);

	if (!RulesExtData::Instance()->JumpjetClimbPredictHeight)
		return 0;

	R->EAX(JumpjetRushHelpers::JumpjetLocomotionPredictHeight(pThis));

	return 0x54D928; // Completely skip the original calculate
}

ASMJIT_PATCH(0x54D4C0, JumpjetLocomotionClass_sub_54D0F0_NoStuck, 0x6)
{
	if (RulesExtData::Instance()->JumpjetClimbWithoutCutOut || JumpjetRushHelpers::Skip)
	{
		JumpjetRushHelpers::Skip = false;
		return 0x54D52F; // Skip the original check
	}

	return 0;
}

ASMJIT_PATCH(0x54DAC4, JumpjetLocomotionClass_EndPiggyback_Blyat, 0x6)
{
	GET(FootClass*, pLinked, EAX);
	auto const* pType = pLinked->GetTechnoType();

	pLinked->PrimaryFacing.Set_ROT(pType->ROT);

	if (pType->Sensors && pType->SensorsSight > 0)
	{
		pLinked->RemoveSensorsAt(pLinked->LastFlightMapCoords);
		pLinked->RemoveSensorsAt(pLinked->GetMapCoords());
		pLinked->AddSensorsAt(pLinked->GetMapCoords());
	}

	return 0;
}