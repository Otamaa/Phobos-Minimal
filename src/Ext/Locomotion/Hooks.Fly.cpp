#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>

#include <AircraftTrackerClass.h>

#include <InfantryClass.h>

#include <Locomotor/FlyLocomotionClass.h>
#include <Locomotor/Cast.h>

#include <Ext/Anim/Body.h>
#include <Ext/AircraftType/Body.h>
#include <Ext/AircraftType/Body.h>
#include <Ext/Aircraft/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>

#include <Misc/DamageArea.h>

ASMJIT_PATCH(0x4CDE64, FlyLocomotionClass_sub_4CD600_HunterSeeker_Ascent, 6)
{
	GET(FlyLocomotionClass* const, pThis, ESI);
	GET(int const, unk, EDI);
	auto const pObject = pThis->LinkedTo;
	auto const pType = GET_TECHNOTYPE(pObject);
	auto const pExt = TechnoTypeExtContainer::Instance.Find(pType);

	auto ret = pThis->FlightLevel - unk;
	auto max = 16;

	if (!pType->IsDropship)
	{
		if (!pType->HunterSeeker)
		{
			// ordinary aircraft
			max = (R->BL() != 0) ? 10 : 20;

		}
		else
		{
			// is hunter seeker
			if (pThis->IsTakingOff)
			{
				max = pExt->HunterSeekerEmergeSpeed.Get(RulesExtData::Instance()->HunterSeekerEmergeSpeed);
			}
			else
			{
				max = pExt->HunterSeekerAscentSpeed.Get(RulesExtData::Instance()->HunterSeekerAscentSpeed);
			}
		}
	}

	if (ret > max)
	{
		ret = max;
	}

	R->EAX(ret);
	return 0x4CDE8F;
}

ASMJIT_PATCH(0x4CDF54, FlyLocomotionClass_sub_4CD600_HunterSeeker_Descent, 5)
{
	GET(FlyLocomotionClass* const, pThis, ESI);
	GET(int const, max, EDI);
	auto const pObject = pThis->LinkedTo;
	auto const pType = GET_TECHNOTYPE(pObject);
	auto const pExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pType->HunterSeeker)
	{
		auto ret = pExt->HunterSeekerDescentSpeed.Get(RulesExtData::Instance()->HunterSeekerDescentSpeed);
		if (max < ret)
		{
			ret = max;
		}

		R->ECX(ret);
		return 0x4CDF81;
	}

	return 0;
}

ASMJIT_PATCH(0x4CDCFD, FlyLocomotionClass_MovingUpdate_HoverAttack, 0x7)
{
	GET(FlyLocomotionClass*, pFly, ESI);

	AircraftClass* pAir = cast_to<AircraftClass*, false>(pFly->LinkedTo);

	if (pAir && !pAir->Type->MissileSpawn && !pAir->Type->Fighter && !pAir->Is_Strafe() && pAir->CurrentMission == Mission::Attack)
	{
		if (AbstractClass* pDest = pAir->Destination)
		{
			CoordStruct sourcePos = pAir->GetCoords();
			int dist = pAir->DistanceFrom(pDest);

			if (dist < 64 && dist >= 16)
			{
				CoordStruct targetPos = pDest->GetCoords();
				sourcePos.X = targetPos.X;
				sourcePos.Y = targetPos.Y;
				dist = 0;
			}

			if (dist < 16)
			{
				R->Stack(0x50, sourcePos);
			}
		}
	}
	return 0;
}


ASMJIT_PATCH(0x4CD9C8, FlyLocomotionClass_sub_4CD600_HunterSeeker_UpdateTarget, 0x6)
{
	GET(FlyLocomotionClass*, pThis, ESI);
	const auto pObject = pThis->LinkedTo;
	const auto pType = GET_TECHNOTYPE(pObject);

	if (pType->HunterSeeker)
	{

		if (const auto pTarget = pObject->Target)
		{

			// update the target's position, considering units in tunnels
			auto crd = pTarget->GetCoords();

			const auto abs = pTarget->WhatAmI();
			if (abs == UnitClass::AbsID || abs == InfantryClass::AbsID)
			{
				const auto pFoot = static_cast<FootClass* const>(pObject);
				if (pFoot->TubeIndex >= 0)
				{
					crd = pFoot->CurrentTunnelCoords;
				}
			}

			const auto  height = MapClass::Instance->GetCellFloorHeight(crd);

			if (crd.Z < height)
			{
				crd.Z = height;
			}

			pThis->MovingDestination = crd;

			// update the facing
			const auto crdSource = pObject->GetCoords();

			DirStruct const tmp(double(crdSource.Y - crd.Y), double(crd.X - crdSource.X));
			pObject->PrimaryFacing.Set_Current(tmp);
			pObject->SecondaryFacing.Set_Current(tmp);
		}
		else
		{
			pThis->Acquire_Hunter_Seeker_Target();
		}
	}

	return 0;
}

ASMJIT_PATCH(0x4CE85A, FlyLocomotionClass_UpdateLanding, 0x8)
{
	GET(FlyLocomotionClass*, pThis, ESI);
	const auto pObject = pThis->LinkedTo;
	const auto pType = GET_TECHNOTYPE(pObject);

	if (pType->HunterSeeker)
	{
		if (!pObject->Target)
		{

			pThis->Acquire_Hunter_Seeker_Target();

			if (pObject->Target)
			{
				pThis->IsLanding = false;
				pThis->FlightLevel = pType->GetFlightLevel();

				pObject->SendToFirstLink(RadioCommand::NotifyUnlink);
				pObject->QueueMission(Mission::Attack, false);
				pObject->NextMission();
			}
		}

		// return 0
		R->EAX(0);
		return 0x4CE852;
	}

	return 0;
}

ASMJIT_PATCH(0x4CCB84, FlyLocomotionClass_ILocomotion_Process_HunterSeeker, 0x6)
{
	GET(ILocomotion* const, pThis, ESI);
	auto const pLoco = static_cast<FlyLocomotionClass*>(pThis);
	const auto pObject = pLoco->Owner ? pLoco->Owner : pLoco->LinkedTo;
	const auto pType = GET_TECHNOTYPE(pObject);

	if (pType->HunterSeeker)
	{
		if (!pObject->Target)
		{

			pLoco->Acquire_Hunter_Seeker_Target();

			if (pObject->Target)
			{

				pLoco->IsLanding = false;
				pLoco->FlightLevel = pType->GetFlightLevel();

				pObject->SendToFirstLink(RadioCommand::NotifyUnlink);
				pObject->QueueMission(Mission::Attack, false);
				pObject->NextMission();
			}
		}
	}

	return 0;
}

ASMJIT_PATCH(0x4CFE80, FlyLocomotionClass_ILocomotion_AcquireHunterSeekerTarget, 5)
{
	GET_STACK(ILocomotion* const, pThis, 0x4);

	const auto pFly = static_cast<FlyLocomotionClass*>(pThis);

	// replace the entire function
	TechnoExtData::AcquireHunterSeekerTarget(pFly->Owner ? pFly->Owner : pFly->LinkedTo);

	return 0x4D016F;
}

ASMJIT_PATCH(0x4CDA6F, FlyLocomotionClass_MovementAI_SpeedModifiers, 0x9)
{
	GET(FlyLocomotionClass* const, pThis, ESI);

	if (const auto pLinked = pThis->LinkedTo)
	{
		const double currentSpeed = GET_TECHNOTYPE(pLinked)->Speed
			* pThis->CurrentSpeed *
			TechnoExtData::GetCurrentSpeedMultiplier(pLinked);

		R->EAX(int(currentSpeed));
		return 0x4CDA78;
	}

	return 0;
}

ASMJIT_PATCH(0x4CE4B3, FlyLocomotionClass_4CE4B0_SpeedModifiers, 0x6)
{
	GET(FlyLocomotionClass* const, pThis, ECX);

	if (const auto pLinked = pThis->LinkedTo)
	{
		const double currentSpeed = GET_TECHNOTYPE(pLinked)->Speed
			* pThis->CurrentSpeed *
			TechnoExtData::GetCurrentSpeedMultiplier(pLinked);

		R->EAX(int(currentSpeed));
		return 0x4CE4BF;
	}

	return 0;
}

ASMJIT_PATCH(0x4CE42A, FlyLocomotionClass_StateUpdate_NoLanding, 0x6) // Prevent aircraft from hovering due to cyclic enter Guard and AreaGuard missions when above buildings
{
	enum { SkipGameCode = 0x4CE441 };

	GET(FootClass* const, pLinkTo, EAX);

	const auto pAircraft = cast_to<AircraftClass*, true>(pLinkTo);

	if (!pAircraft || !AircraftTypeExtData::ExtendedAircraftMissionsEnabled(pAircraft) || pAircraft->Airstrike || pAircraft->Spawned || pAircraft->GetCurrentMission() == Mission::Enter)
		return 0;

	return SkipGameCode;
}

ASMJIT_PATCH(0x4CF68D, FlyLocomotionClass_DrawMatrix_OnAirport, 0x5)
{
	GET(ILocomotion*, iloco, ESI);
	auto loco = static_cast<FlyLocomotionClass*>(iloco);
	auto pAir = cast_to<AircraftClass*, false>(loco->LinkedTo);

	if (pAir && pAir->GetHeight() <= 0)
	{
		float ars = pAir->AngleRotatedSideways;
		float arf = pAir->AngleRotatedForwards;
		REF_STACK(Matrix3D, mat, STACK_OFFSET(0x38, -0x30));
		auto slope_idx = MapClass::Instance->GetCellAt(pAir->Location)->SlopeIndex;
		mat = Game::VoxelRampMatrix[slope_idx] * mat;

		if (Math::abs(ars) > 0.005 || Math::abs(arf) > 0.005)
		{
			mat.TranslateZ(float(Math::abs(Math::sin(ars))
				* pAir->Type->VoxelScaleX
				+ Math::abs(Math::sin(arf)) * pAir->Type->VoxelScaleY));

			R->ECX(pAir);
			return 0x4CF6AD;
		}

		return 0x4CF6A0;
	}

	return 0;
}

ASMJIT_PATCH(0x4CD64E, FlyLocomotionClass_MovementAI_UpdateSensors, 0xA)
{
	GET(FlyLocomotionClass* const, pThis, ESI);
	GET(CellStruct, currentCell, EDI);

	const auto pLinkedTo = pThis->LinkedTo;
	const auto pType = GET_TECHNOTYPE(pLinkedTo);

	if (pType->Sensors && pType->SensorsSight > 0)
	{
		pLinkedTo->RemoveSensorsAt(pLinkedTo->LastFlightMapCoords);

		if (pLinkedTo->IsAlive)
			pLinkedTo->AddSensorsAt(currentCell);
	}

	AircraftTrackerClass::Instance->Update(pLinkedTo, pLinkedTo->LastFlightMapCoords, currentCell);

	return 0x4CD664;
}


ASMJIT_PATCH(0x4CF190, FlyLocomotionClass_FlightUpdate_SetPrimaryFacing, 0x6) // Make aircraft not to fly directly to the airport before starting to land
{
	enum { SkipGameCode = 0x4CF29A };

	GET(IFlyControl* const, iFly, EAX);

	if (!iFly || !iFly->Is_Locked())
	{
		GET(FootClass** const, pFootPtr, ESI);
		//GET(const int, distance, EBX);
		// No const because it also need to be used by SecondaryFacing
		REF_STACK(CoordStruct, destination, STACK_OFFSET(0x48, 0x8));

		auto horizontalDistance = [&destination](const CoordStruct& location)
			{
				const auto delta = Point2D { location.X, location.Y } - Point2D { destination.X, destination.Y };
				return static_cast<int>(delta.Length());
			};

		const auto pFoot = *pFootPtr;
		const auto pAircraft = cast_to<AircraftClass*, true>(pFoot);

		// Rewrite vanilla implement
		if (!pAircraft || !AircraftTypeExtContainer::Instance.Find(pAircraft->Type)->ExtendedAircraftMissions_RearApproach
			.Get(AircraftTypeExtData::ExtendedAircraftMissionsEnabled(pAircraft)))
		{
			const auto footCoords = pFoot->GetCoords();
			const auto desired = DirStruct(Math::atan2((double)(footCoords.Y - destination.Y), (double)(destination.X - footCoords.X)));

			if (!iFly || !iFly->Is_Strafe() || horizontalDistance(footCoords) > 768 // I don't know why it's 3 cells' length, but its vanilla, keep it
				|| Math::abs(static_cast<short>(static_cast<short>(desired.Raw) - static_cast<short>(pFoot->PrimaryFacing.Current().Raw))) >= 8192)
			{
				pFoot->PrimaryFacing.Set_Desired(desired);
			}
		}
		else
		{
			if (pAircraft)
			{

				const auto footCoords = pAircraft->GetCoords();
				const auto landingDir = DirStruct(BuildingExtData::GetPoseDir(pAircraft, nullptr));

				// Try to land from the rear
				if (pAircraft->Destination && (pAircraft->DockedTo == pAircraft->Destination || pAircraft->SpawnOwner == pAircraft->Destination))
				{
					const auto pType = pAircraft->Type;

					// Like smooth moving
					const auto turningRadius = MaxImpl((pType->SlowdownDistance / 512), (8 / pType->ROT));

					// The direction of the airport
					const auto currentDir = DirStruct(Math::atan2((double)footCoords.Y - destination.Y, double(destination.X - footCoords.X)));

					// Included angle's raw
					const auto difference = static_cast<short>(static_cast<short>(currentDir.Raw) - static_cast<short>(landingDir.Raw));

					// Land from this direction of the airport
					const auto landingFace = landingDir.GetFacing<8>(4);
					auto cellOffset = CellSpread::AdjacentPoint[landingFace];

					// When the direction is opposite, moving to the side first, then automatically shorten based on the current distance
					if (Math::abs(difference) >= 12288) // 12288 -> 3/16 * 65536 (1/8 < 3/16 < 1/4, so the landing can begin at the appropriate location)
						cellOffset = (cellOffset + CellSpread::AdjacentPoint[((difference > 0) ? (landingFace + 2) : (landingFace - 2)) & 7]) * turningRadius;
					else // The purpose of doubling is like using two offsets above, to keep the destination point on the range circle (diameter = 2 * radius)
						cellOffset *= MinImpl((turningRadius * 2), ((landingFace & 1) ? (horizontalDistance(footCoords) / 724) : (horizontalDistance(footCoords) / 512))); // 724 -> 512√2

					// On the way back, increase the offset value of the destination so that it looks like a real airplane
					destination.X += cellOffset.X;
					destination.Y += cellOffset.Y;
				}

				if (footCoords.Y != destination.Y || footCoords.X != destination.X)
					pAircraft->PrimaryFacing.Set_Desired(DirStruct(Math::atan2(double(footCoords.Y - destination.Y), double(destination.X - footCoords.X))));
				else
					pAircraft->PrimaryFacing.Set_Desired(landingDir);
			}
		}
	}

	return SkipGameCode;
}

ASMJIT_PATCH(0x4CF3D0, FlyLocomotionClass_FlightUpdate_SetFlightLevel, 0x7) // Make aircraft not have to fly directly above the airport before starting to descend
{
	GET_STACK(FlyLocomotionClass* const, pThis, STACK_OFFSET(0x48, -0x28));

	auto const pFootPtr = pThis->LinkedTo;
	const auto pType = GET_TECHNOTYPE(pFootPtr);
	auto const pExt = TechnoTypeExtContainer::Instance.Find(pType);

	// Ares hook
	if (pType->HunterSeeker)
	{
		//target was invalidated , use the moving dest
		if (const auto pTarget =
			pFootPtr->Target ? pFootPtr->Target : MapClass::Instance->TryGetCellAt(pThis->MovingDestination)
			)
		{
			auto const DetonateProximity = pExt->HunterSeekerDetonateProximity.Get(RulesExtData::Instance()->HunterSeekerDetonateProximity);
			auto const DescendProximity = pExt->HunterSeekerDescendProximity.Get(RulesExtData::Instance()->HunterSeekerDescendProximity);

			// get th difference of our position to the target,
			// disregarding the Z component.
			auto crd = pFootPtr->GetCoords();
			crd -= pThis->MovingDestination;
			crd.Z = 0;

			auto const dist = int(crd.Length());

			if (dist >= DetonateProximity)
			{
				R->EBP(pThis);

				// not close enough to detonate, but we might start the decent
				if (dist < DescendProximity)
				{
					// the target's current height
					auto const z = pTarget->GetCoords().Z;

					// the hunter seeker's default flight level
					crd = pFootPtr->GetCoords();
					auto floor = MapClass::Instance->GetCellFloorHeight(crd);
					auto const height = floor + pType->GetFlightLevel();

					// linear interpolation between target's Z and normal flight level
					auto const ratio = dist / static_cast<double>(DescendProximity);
					auto const lerp = z * (1.0 - ratio) + height * ratio;

					// set the descending flight level
					auto level = int(lerp) - floor;
					if (level < 10)
					{
						level = 10;
					}

					pThis->FlightLevel = level;

				}
				else
				{

					// project the next steps using the current speed
					// and facing. if there's a height difference, use
					// the highest value as the new flight level.
					auto const speed = pThis->Apparent_Speed();
					if (speed > 0)
					{
						double const value = pFootPtr->PrimaryFacing.Current().GetRadian();
						double const cos = Math::cos(value);
						double const sin = Math::sin(value);

						int maxHeight = 0;
						int currentHeight = 0;
						auto crd2 = pFootPtr->GetCoords();
						for (int i = 0; i < 11; ++i)
						{
							auto const pCell = MapClass::Instance->GetCellAt(crd2);
							auto const z = pCell->GetCoordsWithBridge().Z;

							if (z > maxHeight)
							{
								maxHeight = z;
							}

							if (!i)
							{
								currentHeight = z;
							}

							// advance one step
							crd2.X += int(cos * speed);
							crd2.Y -= int(sin * speed);

							// result is never used in TS, but a break sounds
							// like a good idea.
							auto const cell = CellClass::Coord2Cell(crd2);
							if (!MapClass::Instance->CoordinatesLegal(cell))
							{
								break;
							}
						}

						// pull the old lady up
						if (maxHeight > currentHeight)
						{
							R->EBP(pThis);
							pThis->FlightLevel = pType->GetFlightLevel();

						}
					}
				}

				return 0x4CF4D2;
			}
			else
			{
				auto const pWeapon = pFootPtr->GetPrimaryWeapon()->WeaponType;

				// damage
				auto damage = pWeapon->Damage;

				//if the target exist , damage the target
				if (auto const pTechno = flag_cast_to<TechnoClass*>(pTarget))
				{
					pTechno->ReceiveDamage(&damage, 0, pWeapon->Warhead, pFootPtr, true, true, pFootPtr->Owner);
				}

				// damage the hunter seeker regardless the target state
				damage = pWeapon->Damage;
				pFootPtr->ReceiveDamage(&damage, 0, pWeapon->Warhead, nullptr, true, true, nullptr);

				// damage the map
				auto crd2 = pFootPtr->GetCoords();
				//WeaponTypeExtData::DetonateAt(pWeapon, crd2, pObject, true, pObject->Owner);
				MapClass::FlashbangWarheadAt(pWeapon->Damage, RulesClass::Instance->C4Warhead, crd2);
				DamageArea::Apply(&crd2, pWeapon->Damage, pFootPtr, pWeapon->Warhead, true, pFootPtr->Owner);

				R->EBX(0);
				return 0x4CF5F2;
			}
		}
	}

	if (const auto pAircraft = cast_to<AircraftClass*, false>(pFootPtr))
	{

		if (AircraftTypeExtData::ExtendedAircraftMissionsEnabled(pAircraft))
		{
			GET(const int, distance, EBX);

			// Restore skipped code
			R->EBP(pThis);

			// Same as vanilla
			if (pThis->IsElevating && distance < 768)
			{
				// Fast descent
				const auto floorHeight = MapClass::Instance->GetCellFloorHeight(pThis->MovingDestination);
				pThis->FlightLevel = pThis->MovingDestination.Z - floorHeight;
			}
			else
			{

				const auto flightLevel = pType->GetFlightLevel();

				// Check returning actions
				if (distance < pType->SlowdownDistance && pAircraft->Destination
					&& (pAircraft->DockedTo == pAircraft->Destination || pAircraft->SpawnOwner == pAircraft->Destination))
				{
					// Slow descent
					const auto floorHeight = MapClass::Instance->GetCellFloorHeight(pThis->MovingDestination);
					const auto destinationHeight = pThis->MovingDestination.Z - floorHeight + 1;
					pThis->FlightLevel = static_cast<int>((flightLevel - destinationHeight) * (static_cast<double>(distance) / pType->SlowdownDistance)) + destinationHeight;
				}
				else
				{
					// Horizontal flight
					pThis->FlightLevel = flightLevel;
				}
			}

			return 0x4CF4D2;
		}
	}

	return 0;
}


ASMJIT_PATCH(0x4CE689, FlyLocomotionClass_TakeOffAnim, 0x5)
{
	GET(FlyLocomotionClass*, pThis, ECX);

	if (const auto pAir = cast_to<AircraftClass*, false>(pThis->LinkedTo))
	{
		if (pAir->IsInAir())
			return 0x0;

		auto const pCell = pAir->GetCell();
		if (!pCell || pAir->GetHeight() > pCell->GetFloorHeight({ 1,1 }))
			return 0x0;

		if (auto pDecidedAnim = TechnoTypeExtContainer::Instance.Find(pAir->Type)->TakeOff_Anim.Get(RulesExtData::Instance()->Aircraft_TakeOffAnim.Get()))
		{
			auto const nCoord = pAir->GetCoords();
			AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pDecidedAnim, nCoord, 0, 1, AnimFlag::AnimFlag_600, 0, 0),
				pAir->GetOwningHouse(),
				nullptr,
				pAir,
				false, false
			);
		}
	}

	return 0x0;
}

ASMJIT_PATCH(0x4CEB51, FlyLocomotionClass_LandingAnim, 0x8)
{
	GET(AircraftClass*, pLinked, ECX);
	GET_STACK(CoordStruct, nCoord, STACK_OFFS(0x48, 0x18));

	const auto pType = pLinked->Type;
	auto const pExt = TechnoTypeExtContainer::Instance.Find(pType);

	{
		auto GetDefaultType = [pType]()
			{
				if (pType->IsDropship)
					return RulesExtData::Instance()->DropShip_LandAnim.Get();
				else if (pType->Carryall)
					return RulesExtData::Instance()->CarryAll_LandAnim.Get();

				return (AnimTypeClass*)nullptr;
			};

		const auto pCell = pLinked->GetCell();
		const auto pFirst = pCell->LandType == LandType::Water && !pCell->ContainsBridge() && pExt->Landing_AnimOnWater.Get()
			? pExt->Landing_AnimOnWater.Get() : pExt->Landing_Anim.Get(RulesExtData::Instance()->Aircraft_LandAnim.Get());

		if (AnimTypeClass* pDecidedType = pFirst ? pFirst : GetDefaultType())
		{
			AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pDecidedType, nCoord, 0, 1, AnimFlag::AnimFlag_600, 0, 0),
				pLinked->GetOwningHouse(),
				nullptr,
				pLinked,
				false, false
			);
		}

		return 0x4CEC5D;
	}

	//return 0x0;
}
