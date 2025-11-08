#include "Body.h"

#include <AircraftClass.h>

#include <AircraftTrackerClass.h>
#include <Locomotor/FlyLocomotionClass.h>
#include <Misc/DamageArea.h>

FacingType NOINLINE GetPoseDir(AircraftClass* pAir , BuildingClass* pBld)
{
	FacingType ret = (FacingType)TechnoTypeExtContainer::Instance.Find(pAir->Type)->LandingDir.Get(RulesClass::Instance->PoseDir);

	if (pBld || pAir->HasAnyLink())
	{
		if (!pBld){

			for (auto i = 0; i < pAir->RadioLinks.Capacity; ++i) {
				if (auto possiblebld = cast_to<BuildingClass*>(pAir->RadioLinks[i])) {
					pBld = possiblebld;
				}
			}

			if(!pBld && pAir->RadioLinks[0] && ret < FacingType::Min) { //spawner
				return FacingType((((pAir->PrimaryFacing.Current().Raw >> 12) + 1) >> 1) & 7);
			}
		}

		if(pBld) {
			const auto pBldTypeExt = BuildingTypeExtContainer::Instance.Find(pBld->Type);
			const int nIdx = pBld->FindLinkIndex(pAir);
			const auto dir = &pBldTypeExt->DockPoseDir;

			if (nIdx <= -1 || (size_t)nIdx >= dir->size() || ret < FacingType::Min)
			{
				return pBldTypeExt->LandingDir.Get(FacingType((((pBld->PrimaryFacing.Current().Raw >> 12) + 1) >> 1) & 7));

			}
			else
				return (*dir)[nIdx];
		}
	}

	if (!pAir->Type->AirportBound && ret < FacingType::Min) {
		return FacingType((((pAir->PrimaryFacing.Current().Raw >> 12) + 1) >> 1) & 7);
	}

	return FacingType((((((int)ret) >> 4) + 1) >> 1) & 7);
}

// replace the entire function
ASMJIT_PATCH(0x41B760, IFlyControl_LandDirection, 0x6)
{
	GET_STACK(IFlyControl*, pThis, 0x4);

	const FacingType result = GetPoseDir(static_cast<AircraftClass*>(pThis), nullptr);
	R->EAX(result);
	return 0x41B7C1;
}

// request radio contact then get land dir
ASMJIT_PATCH(0x446FA2, BuildingClass_GrandOpening_PoseDir, 0x6)
{
	GET(BuildingClass*, pThis, EBP);
	GET(AircraftClass*, pAir, ESI);
	pThis->SendCommand(RadioCommand::RequestTether, pAir);
	const DirStruct dir { GetPoseDir(pAir, pThis) };

	if (RulesExtData::Instance()->ExpandAircraftMission)
		pAir->PrimaryFacing.Set_Current(dir);

	pAir->SecondaryFacing.Set_Current(dir);

//	if (pThis->GetHeight() > 0)
//		AircraftTrackerClass::Instance->Add(pThis);

	return 0x446FB0;
}

// request radio contact then get land dir
ASMJIT_PATCH(0x444014, BuildingClass_ExitObject_PoseDir_AirportBound, 0x5)
{
	GET(BuildingClass*, pThis, ESI);
	GET(AircraftClass*, pAir, ECX);

	pThis->SendCommand(RadioCommand::RequestLink, pAir);
	pThis->SendCommand(RadioCommand::RequestTether, pAir);
	pAir->SetLocation(pThis->GetDockCoords(pAir));
	pAir->DockedTo = pThis;
	FacingType result = GetPoseDir(pAir, pThis);
	const DirStruct dir { result };

	if (RulesExtData::Instance()->ExpandAircraftMission)
		pAir->PrimaryFacing.Set_Current(dir);

	pAir->SecondaryFacing.Set_Current(dir);

	//if (pAir->GetHeight() > 0)
	//	AircraftTrackerClass::Instance->Add(pAir);

	return 0x444053;
}

// there no radio contact happening here
// so the result mostlikely building facing
ASMJIT_PATCH(0x443FD8, BuildingClass_ExitObject_PoseDir_NotAirportBound, 0x8)
{
	enum { RetCreationFail = 0x444EDE, RetCreationSucceeded = 0x443FE0 };

	GET(BuildingClass*, pThis, ESI);
	GET(AircraftClass*, pAir, EBP);

	if (R->AL())
	{
		pAir->DockedTo = pThis;
		const DirStruct dir { ((int)GetPoseDir(pAir, pThis) << 13) };

		if (RulesExtData::Instance()->ExpandAircraftMission)
			pAir->PrimaryFacing.Set_Current(dir);

		pAir->SecondaryFacing.Set_Current(dir);

		//if (pAir->GetHeight() > 0)
		//	AircraftClass::AircraftTracker_4134A0(pAir);

		return RetCreationSucceeded;
	}

	return RetCreationFail;
}

ASMJIT_PATCH(0x687AF4, CCINIClass_InitializeStuffOnMap_AdjustAircrafts, 0x5)
{
	AircraftClass::Array->for_each([](AircraftClass* const pThis) {
		if (pThis && pThis->Type->AirportBound) {
			if (auto pCell = pThis->GetCell()) {
				if (auto pBuilding = pCell->GetBuilding()) {
					if (pBuilding->Type->Helipad && pThis->Type->Dock.contains(pBuilding->Type)) {
						pBuilding->SendCommand(RadioCommand::RequestLink, pThis);
						pBuilding->SendCommand(RadioCommand::RequestTether, pThis);
						pThis->SetLocation(pBuilding->GetDockCoords(pThis));
						pThis->DockedTo = pBuilding;
						const DirStruct dir { ((int)GetPoseDir(pThis, pBuilding) << 13) };
						pThis->SecondaryFacing.Set_Current(dir);

						if (pThis->GetHeight() > 0)
							AircraftTrackerClass::Instance->Add(pThis);
					}
				}
			}
		}
	});

	return 0x0;
}

ASMJIT_PATCH(0x4CF31C, FlyLocomotionClass_FlightUpdate_LandingDir, 0x9)
{
	enum { SkipGameCode = 0x4CF3D0, SetSecondaryFacing = 0x4CF351 };

	GET(FootClass** const, pFootPtr, ESI);
	GET_STACK(IFlyControl* const, iFly, STACK_OFFSET(0x48, -0x38));
	REF_STACK(unsigned int, dir, STACK_OFFSET(0x48, 0x8));

	const auto pFoot = *pFootPtr;
	dir = 0;

	if (iFly) {

		if (iFly->Is_Locked())
			return SkipGameCode;

		if (const auto pAircraft = cast_to<AircraftClass*, true>(pFoot))
			dir = DirStruct(GetPoseDir(pAircraft, nullptr)).Raw;
		else
			dir = (iFly->Landing_Direction() << 13);
	}

	return SetSecondaryFacing;
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

		auto horizontalDistance = [&destination](const CoordStruct& location) {
			const auto delta = Point2D { location.X, location.Y } - Point2D { destination.X, destination.Y };
			return static_cast<int>(delta.Length());
		};

		const auto pFoot = *pFootPtr;
		const auto pAircraft = cast_to<AircraftClass*, true>(pFoot);

		// Rewrite vanilla implement
		if (!RulesExtData::Instance()->ExpandAircraftMission || !pAircraft)
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

			const auto footCoords = pAircraft->GetCoords();
			const auto landingDir = DirStruct(GetPoseDir(pAircraft , nullptr));

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

	return SkipGameCode;
}


ASMJIT_PATCH(0x4CF3D0, FlyLocomotionClass_FlightUpdate_SetFlightLevel, 0x7) // Make aircraft not have to fly directly above the airport before starting to descend
{
	GET_STACK(FlyLocomotionClass* const, pThis, STACK_OFFSET(0x48, -0x28));

	auto const pFootPtr = pThis->LinkedTo;
	const auto pType = pFootPtr->GetTechnoType();
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

	if (const auto pAircraft = cast_to<AircraftClass*, false>(pFootPtr)) {

		if (RulesExtData::Instance()->ExpandAircraftMission)
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