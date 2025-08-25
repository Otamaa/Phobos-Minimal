#include "AdvancedDriveLocomotionClass.h"

#include <InfantryClass.h>
#include <UnitClass.h>
#include <HouseClass.h>
#include <AnimClass.h>
#include <OverlayTypeClass.h>
#include <SpawnManagerClass.h>
#include <TubeClass.h>

#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>

#include <Locomotor/Cast.h>

bool  AdvancedDriveLocomotionClass::IsReversing(FootClass* pFoot)
{
	const auto pLoco = locomotion_cast<AdvancedDriveLocomotionClass*>(pFoot->Locomotor);

	return pLoco && !pLoco->IsForward;
}

bool AdvancedDriveLocomotionClass::Is_Moving()
{
	if (this->TargetCoord != CoordStruct::Empty)
		return true;

	return this->HeadToCoord != CoordStruct::Empty
		&& (this->HeadToCoord.X != this->LinkedTo->Location.X
			|| this->HeadToCoord.Y != this->LinkedTo->Location.Y);
}

Matrix3D* AdvancedDriveLocomotionClass::Draw_Matrix(Matrix3D*  buff, VoxelIndexKey* key)
{
	// Completely rewrite

	const auto pLinked = this->LinkedTo;
	const auto pType = pLinked->GetTechnoType();
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	const bool shouldTilt = !pTypeExt->AdvancedDrive_Hover || pTypeExt->AdvancedDrive_Hover_Tilt;
	const double rate = this->SlopeTimer.GetRatePassed();
	const float ars = Math::abs(pLinked->AngleRotatedSideways);
	const float arf = Math::abs(pLinked->AngleRotatedForwards);

	auto getLerpVoxelRampMatrix = [&rate](int previous, int current)
	{
		Matrix3D mtx;
		Matrix3D::used_Voxel_Draw_Matrix(&mtx , previous, current, rate);
		return mtx;
	};

	if (ars < 0.005 && arf < 0.005)
	{
		// Should set key first, then call base draw_matrix

		if (shouldTilt && rate < 1.0)
		{
			if (key)
				key->Invalidate();

			Matrix3D locoMtx;
			 LocomotionClass::Draw_Matrix(&locoMtx, key);
			const auto rampMtx = getLerpVoxelRampMatrix(this->PreviousRamp, this->CurrentRamp);
			*buff = rampMtx * locoMtx;
			return buff;
		}

		if (key && key->Is_Valid_Key())
			key->Value = (key->Value << 6) + this->CurrentRamp;

		Matrix3D locoMtx;
		LocomotionClass::Draw_Matrix(&locoMtx, key);

		if(shouldTilt){
			*buff = (Game::VoxelRampMatrix[this->CurrentRamp]) * locoMtx;
		} else {
			*buff = locoMtx;
		}

		return buff;
	}

	const auto scaleX = pType->VoxelScaleX;
	const auto scaleY = pType->VoxelScaleY;

	auto baseMtx = Matrix3D::GetIdentity();
	baseMtx.TranslateZ(static_cast<float>(Math::abs(Math::sin(ars)) * scaleX + Math::abs(Math::sin(arf)) * scaleY));

	auto extraMtx = Matrix3D::GetIdentity();
	extraMtx.TranslateX(static_cast<float>(Math::signum(arf) * ((1 - Math::cos(arf)) * scaleY)));
	extraMtx.TranslateY(static_cast<float>(Math::signum(-ars) * ((1 - Math::cos(ars)) * scaleX)));
	extraMtx.RotateX(ars);
	extraMtx.RotateY(arf);

	if (key)
		key->Invalidate();

	Matrix3D locoMtx;
	LocomotionClass::Draw_Matrix(&locoMtx , key);

	if(shouldTilt){
		const auto rampMtx = rate >= 1.0
		? Game::VoxelRampMatrix[this->CurrentRamp]
		: getLerpVoxelRampMatrix(this->PreviousRamp, this->CurrentRamp);

		*buff = baseMtx * rampMtx * locoMtx * extraMtx;
	}else {
		*buff = baseMtx * locoMtx * extraMtx;
	}

	return buff;
}

Matrix3D* AdvancedDriveLocomotionClass::Shadow_Matrix(Matrix3D* mtx , VoxelIndexKey* key)
{
	// Completely rewrite

	const auto pLinked = this->LinkedTo;
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pLinked->GetTechnoType());
	const bool shouldTilt = !pTypeExt->AdvancedDrive_Hover || pTypeExt->AdvancedDrive_Hover_Tilt;

	if ((shouldTilt && this->SlopeTimer.GetRatePassed() != 1.0)
		|| Math::abs(pLinked->AngleRotatedSideways) >= 0.005
		|| Math::abs(pLinked->AngleRotatedForwards) >= 0.005)
	{
		if (key)
			key->Invalidate();
	}

	LocomotionClass::Shadow_Matrix(mtx , key);
	return mtx;
}

bool AdvancedDriveLocomotionClass::Process()
{
	const auto pLinked = this->LinkedTo;
	const auto slopeIndex = pLinked->GetCell()->SlopeIndex;
	const auto pType = pLinked->GetTechnoType();
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (slopeIndex != this->CurrentRamp)
	{
		this->PreviousRamp = this->CurrentRamp;
		this->CurrentRamp = slopeIndex;
		// Dynamic slope change
		const auto speed = pType->Speed;
		this->SlopeTimer.Start((speed > 0) ? (90 / speed) : 0);
	}

	// Record target cell for reversing
	if (pTypeExt->AdvancedDrive_Reverse)
		this->UpdateSituation();

	const auto notInMotion = !this->InMotion();

	// Update hover state
	if (pTypeExt->AdvancedDrive_Hover)
		this->UpdateHoverState();

	if (notInMotion)
		return false;

	if (this->Is_Moving_Now() && !(Unsorted::CurrentFrame % 10))
	{
		if (!pLinked->OnBridge && pLinked->GetCell()->LandType == LandType::Water)
		{
			// Customized wake
			if (const auto pAnimType = pTypeExt->Wake.Get(RulesClass::Instance->Wake))
				GameCreate<AnimClass>(pAnimType, pLinked->Location);
		}
	}

	if (this->TargetCoord == CoordStruct::Empty && this->HeadToCoord == CoordStruct::Empty
		&& pLinked->PathDirections[0] == -1 && pLinked->SpeedPercentage > 0.0)
	{
		pLinked->SetSpeedPercentage(0.0);
	}

	return this->Is_Moving();
}

void AdvancedDriveLocomotionClass::Move_To(CoordStruct to)
{
	const auto pLinked = this->LinkedTo;

	if (!pLinked->IsUnderEMP() && !pLinked->IsParalyzed()
		&& !pLinked->IsBeingWarpedOut() && !pLinked->IsWarpingIn())
	{
		this->TargetCoord = to;

		if (to != CoordStruct::Empty && MapClass::Instance->GetCellAt(to)->ContainsBridge())
			this->TargetCoord.Z += CellClass::BridgeHeight;
	}
}

void AdvancedDriveLocomotionClass::Stop_Moving()
{
	const auto pLinked = this->LinkedTo;

	if (this->HeadToCoord != CoordStruct::Empty && pLinked->GetTechnoType()->IsTrain)
	{
		const auto pUnit = static_cast<UnitClass*>(pLinked);

		if (!pUnit->IsFollowerCar)
		{
			if (auto pFollowerCar = pUnit->FollowerCar)
			{
				do
				{
					pFollowerCar->Locomotor->Stop_Moving();
					pFollowerCar = pFollowerCar->FollowerCar;
				}
				while (pFollowerCar && pFollowerCar != pFollowerCar->FollowerCar);
			}
		}
	}

	// I think no body want to see slowly~ slowly~ moving, so I change this one
	if (pLinked->GetTechnoType()->Accelerates)
	{
		if (pLinked->Location.DistanceFromSquared(this->HeadToCoord) < 16384)
		{
			if (this->MovementSpeed >= 0.5)
				this->MovementSpeed = 0.5;

			// Slow down according to normal conditions
			this->TargetCoord = this->HeadToCoord;
			return;
		}
	}

	this->TargetCoord = CoordStruct::Empty;
}

bool AdvancedDriveLocomotionClass::Power_Off()
{
	const auto pLinked = this->LinkedTo;
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pLinked->GetTechnoType());

	if (pTypeExt->AdvancedDrive_Hover)
	{
		if (this->Is_Powered())
		{
			const auto mission = pLinked->CurrentMission;

			if (mission != Mission::Sleep && mission != Mission::Enter)
			{
				this->OutOfControl = true;
				const int spin = ScenarioClass::Instance->Random.RandomRanged(10, 15);
				this->TailSpin = ScenarioClass::Instance->Random.RandomRanged(0, 99) < 50 ? -spin : spin;
			}
		}
	}

	if (this->Is_Moving())
		this->Stop_Moving();

	return this->LocomotionClass::Power_Off();
}

bool AdvancedDriveLocomotionClass::Is_Powered()
{
	if (this->LocomotionClass::Is_Powered())
		return true;

	const auto pLinked = this->LinkedTo;
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pLinked->GetTechnoType());
	return pTypeExt->AdvancedDrive_Hover && pLinked->GetHeight() > 0;
}

void AdvancedDriveLocomotionClass::Force_Track(int track, CoordStruct coord)
{
	this->TrackNumber = track;
	this->TrackIndex = 0;

	if (coord != CoordStruct::Empty)
	{
		this->HeadToCoord = coord;
		this->IsDriving = true;

		const auto pLinked = this->LinkedTo;
		const auto pCell = MapClass::Instance->GetCellAt(coord);

		if (!pCell->CollectCrate(pLinked) || pLinked->InLimbo)
		{
			if (pLinked->IsAlive)
				this->StopDriving();
		}
		else
		{
			this->MarkOccupation(coord, MarkType::Down);
			this->TargetCoord = coord;
			this->MovementSpeed = 1.0;
		}
	}
}

void AdvancedDriveLocomotionClass::Force_New_Slope(int ramp)
{
	this->PreviousRamp = ramp;
	this->CurrentRamp = ramp;
	this->SlopeTimer.Start(0);
}

bool AdvancedDriveLocomotionClass::Is_Moving_Now()
{
	if (this->LinkedTo->PrimaryFacing.Is_Rotating())
		return true;

	return (this->TargetCoord != CoordStruct::Empty
			|| this->HeadToCoord.X != this->LinkedTo->Location.X
			|| this->HeadToCoord.Y != this->LinkedTo->Location.Y)
		&& this->HeadToCoord != CoordStruct::Empty
		&& this->LinkedTo->GetCurrentSpeed() > 0;
}

void AdvancedDriveLocomotionClass::Mark_All_Occupation_Bits(int mark)
{
	if (this->HeadToCoord != CoordStruct::Empty)
		this->MarkOccupation(this->HeadToCoord, (MarkType)mark);
}

bool AdvancedDriveLocomotionClass::Is_Moving_Here(CoordStruct to)
{
	const auto headToCoord = this->Head_To_Coord();

	if (headToCoord == CoordStruct::Empty)
		return false;

	if (!this->IsOnShortTrack)
	{
		const auto trackNum = this->TrackNumber;

		if (trackNum != -1)
		{
			if (const auto trackStructIndex = DriveLocomotionClass::TurnTrack[trackNum].NormalTrackStructIndex)
			{
				const auto trackIdx = DriveLocomotionClass::RawTrack[trackStructIndex].CellIndex;

				if (trackIdx > -1 && this->TrackIndex < trackIdx)
				{
					const auto trackPt = DriveLocomotionClass::RawTrack[trackStructIndex].TrackPoint;
					const auto& trackPtr = trackPt[trackIdx];
					auto face = trackPtr.Face; // copy
					const auto location = this->GetTrackOffset(trackPtr.Point, face, this->LinkedTo->Location.Z);

					if (CellClass::Coord2Cell(location) == CellClass::Coord2Cell(to)
						&& Math::abs(location.Z - to.Z) <= Unsorted::CellHeight)
					{
						return true;
					}
				}
			}
		}
	}

	return (CellClass::Coord2Cell(headToCoord) == CellClass::Coord2Cell(to)
		&& Math::abs(headToCoord.Z - to.Z) <= Unsorted::CellHeight);
}

bool AdvancedDriveLocomotionClass::Will_Jump_Tracks()
{
	const auto pathDir = this->LinkedTo->PathDirections[0];

	if (pathDir < 0 || pathDir >= 8)
		return false;

	const auto& data = DriveLocomotionClass::TurnTrack[this->TrackNumber];
	const auto dir = DirStruct(data.Face << 8).GetValue<3>();

	if (static_cast<int>(dir) == pathDir || !this->TrackIndex)
		return false;

	const auto trackStructIndex = this->IsOnShortTrack ? data.ShortTrackStructIndex : data.NormalTrackStructIndex;

	if (DriveLocomotionClass::RawTrack[trackStructIndex].JumpIndex != this->TrackIndex)
		return false;

	const auto dirIndex = DriveLocomotionClass::TurnTrack[8 * dir + pathDir].NormalTrackStructIndex;

	return dirIndex && DriveLocomotionClass::RawTrack[dirIndex].EntryIndex;
}

// Non-virtual

bool AdvancedDriveLocomotionClass::MovingProcess(bool fix)
{
	const auto pLinked = this->LinkedTo;
	const auto pType = pLinked->GetTechnoType();

	if ((!this->IsDriving || this->TrackNumber == -1) && pLinked->PathDirections[0] != 8
		|| this->IsRotating && !pType->Turret)
	{
		this->SpeedAccum = 0;
		return false;
	}

	if (!pType->Accelerates)
	{
		pLinked->SetSpeedPercentage(this->MovementSpeed);
	}
	else if (this->TrackNumber < 64
		&& (pLinked->WhatAmI() != AbstractType::Unit || !static_cast<UnitTypeClass*>(pType)->Passive))
	{
		do
		{
			auto coords = this->TargetCoord;
			coords.Z = MapClass::Instance->GetCellFloorHeight(coords);

			if (MapClass::Instance->GetCellAt(coords)->ContainsBridge())
				coords.Z += CellClass::BridgeHeight;

			const auto defaultSpeed = pLinked->GetDefaultSpeed();
			auto speed = pLinked->SpeedPercentage;

			if (int((pLinked->Location - coords).Length()) < pType->SlowdownDistance)
			{
				speed -= defaultSpeed * pType->DeaccelerationFactor;

				if (speed < 0.3)
					speed = 0.3;

				if (pLinked->IsCrushingSomething)
				{
					// Customized crush slow down speed
					const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

					if (this->MovementSpeed > pTypeExt->CrushSlowdownMultiplier)
						this->MovementSpeed = pTypeExt->CrushSlowdownMultiplier;

					speed = this->MovementSpeed;
				}
			}
			else if (pLinked->IsSinking)
			{
				speed -= defaultSpeed * 0.0015;

				if (speed < 0.1)
					speed = 0.1;
			}
			else if (pLinked->IsCrushingSomething)
			{
				// Customized crush slow down speed
				const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

				if (this->MovementSpeed > pTypeExt->CrushSlowdownMultiplier)
					this->MovementSpeed = pTypeExt->CrushSlowdownMultiplier;

				speed = this->MovementSpeed;
			}
			else if (pLinked->SpeedPercentage < this->MovementSpeed)
			{
				speed = pType->AccelerationFactor + pLinked->SpeedPercentage;

				if (this->MovementSpeed < speed)
					speed = this->MovementSpeed;
			}
			else if (pLinked->SpeedPercentage > this->MovementSpeed)
			{
				speed = pLinked->SpeedPercentage - defaultSpeed * pType->DeaccelerationFactor;

				if (this->MovementSpeed > speed)
					speed = this->MovementSpeed;
			}
			else
			{
				break;
			}

			pLinked->SetSpeedPercentage(speed);
		}
		while (false);

		if (pLinked->WhatAmI() == AbstractType::Unit)
		{
			if (auto pFollowerCar = static_cast<UnitClass*>(pLinked)->FollowerCar)
			{
				do
				{
					pFollowerCar->SetSpeedPercentage(pLinked->SpeedPercentage);
					pFollowerCar = pFollowerCar->FollowerCar;
				}
				while (pFollowerCar && pFollowerCar != pFollowerCar->FollowerCar);
			}
		}
	}

	if (pLinked->PathDirections[0] == 8 && this->TrackNumber == -1)
	{
		pLinked->Mark(MarkType::Up);
		this->StopDriving<true>();

		const int tubeIndex = pLinked->GetCell()->TubeIndex;

		if (tubeIndex >= 0 && tubeIndex < TubeClass::Array->Count)
		{
			const auto pTube = TubeClass::Array->Items[tubeIndex];
			this->HeadToCoord = CellClass::Cell2Coord(pTube->ExitCell);

			memmove(&pLinked->PathDirections[0], &pLinked->PathDirections[1], 0x5Cu);
			pLinked->PathDirections[23] = -1;
			pLinked->TubeIndex = static_cast<char>(tubeIndex);
			pLinked->TubeFaceIndex = 0;

			const auto nextCell = pTube->EnterCell + CellSpread::GetNeighbourOffset(pTube->Faces[0] & 7);
			const auto pNextCell = MapClass::Instance->GetCellAt(nextCell);
			pNextCell->GetCellCoords(&pLinked->CurrentTunnelCoords);

			const auto currentHeight = MapClass::Instance->GetCellFloorHeight(pLinked->Location);
			const auto exitHeight = MapClass::Instance->GetCellFloorHeight(this->HeadToCoord);
			pLinked->CurrentTunnelCoords.Z = currentHeight + (exitHeight - currentHeight) / pTube->FaceCount;

			this->IsDriving = true;
			this->TrackNumber = -1;
			return false;
		}

		pLinked->PathDirections[0] = -1;
		this->TrackNumber = -1;
		this->HeadToCoord = CoordStruct::Empty;
		return false;
	}

	auto speedAccum = this->SpeedAccum;

	if (!fix)
		speedAccum += pLinked->GetCurrentSpeed();

	if (const auto result = this->UpdateSpeedAccum(speedAccum))
		return static_cast<bool>(result - 1);

	this->SpeedAccum = speedAccum;

	if (this->SpeedAccum <= 0)
		return false;

	if (this->TrackNumber <= -1)
		return false;

	const auto pTrackData = &DriveLocomotionClass::TurnTrack[this->TrackNumber];
	const int trackStructIndex = this->IsOnShortTrack ? pTrackData->ShortTrackStructIndex : pTrackData->NormalTrackStructIndex;
	const auto pTrackPoint = &DriveLocomotionClass::RawTrack[trackStructIndex].TrackPoint[this->TrackIndex];

	if (pTrackPoint->Point == Point2D::Empty && this->TrackIndex)
		return false;

	int face = pTrackPoint->Face;
	auto location = this->GetTrackOffset(pTrackPoint->Point, face);
	const auto record = location;
	const auto pTrackCell = MapClass::Instance->GetCellAt(location);
	location -= pLinked->Location;

	// Fix UpdatePosition bug
	if (this->IsShifting)
	{
		const auto curDir = pLinked->PrimaryFacing.Current();
		const auto nextDir = DirStruct((this->TrackNumber & 7) << 13);

		if (Math::abs(static_cast<short>(static_cast<short>(curDir.Raw) - static_cast<short>(nextDir.Raw))) <= 4096)
		{
			this->IsShifting = false;
			const auto oldPlan = pLinked->PlanningPathIdx;
			pLinked->PlanningPathIdx = -1;
			pLinked->UpdatePosition(PCPType::Rotation);
			pLinked->PlanningPathIdx = oldPlan;
		}
	}

	const auto ratio = static_cast<float>(this->SpeedAccum * 0.1428571428571428);
	auto newPos = pLinked->Location + AdvancedDriveLocomotionClass::CoordLerp(CoordStruct::Empty, location, ratio);

	const auto pOldCell = MapClass::Instance->GetCellAt(pLinked->Location);
	auto pNewCell = MapClass::Instance->GetCellAt(newPos);

	if (pNewCell != pTrackCell && pNewCell != pOldCell && this->SpeedAccum > 3)
	{
		newPos.X = record.X;
		newPos.Y = record.Y;
		pNewCell = pTrackCell;
	}

	newPos.Z = pLinked->Location.Z;

	if (CellClass::Coord2Cell(newPos) == CellClass::Coord2Cell(pLinked->Location))
	{
		const bool wasOnMap = pLinked->IsOnMap;
		pLinked->IsOnMap = false;
		pLinked->SetLocation(newPos);
		pLinked->IsOnMap = wasOnMap;
	}
	else
	{
		pLinked->Mark(MarkType::Up);
		pLinked->SetLocation(newPos);
		this->UpdateOnBridge(pNewCell, pOldCell);
		pLinked->Mark(MarkType::Down);
	}

	return false;
}

bool AdvancedDriveLocomotionClass::PassableCheck(bool* pStop, bool force, bool check)
{
	const auto pLinked = this->LinkedTo;
	int pathDir = pLinked->PathDirections[0];

	if (!this->Is_Moving() && pathDir == -1)
	{
		this->IsTurretLockedDown = false;
		this->StopDriving<true>();

		if (pLinked->GetCurrentMission() == Mission::Move)
			*pStop = pLinked->EnterIdleMode(false, true);

		return false;
	}

	if (this->TargetCoord == CoordStruct::Empty || pLinked->IsBeingWarpedOut() || pLinked->IsWarpingIn())
		return false;

	if (const auto pSpawnManager = pLinked->SpawnManager)
	{
		if (pSpawnManager->CountLaunchingSpawns())
			return true;
	}

	if (pLinked->IsUnderEMP() || pLinked->IsParalyzed())
		return true;

	const auto pType = pLinked->GetTechnoType();

	do
	{
		if (pathDir != -1)
		{
			if (const auto pDest = pLinked->Destination)
			{
				const auto absType = pDest->WhatAmI();

				if (absType == AbstractType::Infantry || absType == AbstractType::Unit)
				{
					const int distance = int((pLinked->Location - this->TargetCoord).Length()) / 256;

					if (distance < 24)
					{
						pLinked->PathDirections[distance] = -1;
						pathDir = pLinked->PathDirections[0];
					}
				}
			}

			if (pathDir != -1)
				break;
		}

		auto& timer = pLinked->PathDelayTimer;

		if (timer.GetTimeLeft() > 0)
			return false;

		timer.Start(int(RulesClass::Instance->PathDelay * 900.0));

		if (!pLinked->UpdatePathfinding(CellClass::Coord2Cell(this->TargetCoord), false, 0))
		{
			if (!this->LinkedTo)
			{
				*pStop = true;
				return false;
			}

			if (!pLinked->IsInSameZone(&this->TargetCoord))
			{
				pLinked->SetDestination(nullptr, true);
				return false;
			}

			if (this->TargetCoord == CoordStruct::Empty)
				return false;

			const auto mission = pLinked->GetCurrentMission();

			if (mission != Mission::Enter
				&& (pLinked->Location - this->TargetCoord).Length() < RulesClass::Instance->CloseEnough
				&& (mission == Mission::Move || mission == Mission::Area_Guard))
			{
				this->StopDriving<true>();

				if (this->StopMotion())
					return true;

				if (!pLinked->IsAlive)
					return false;
			}
			else
			{
				auto primaryFace = pLinked->PrimaryFacing.Current();
				const auto primaryDir = (primaryFace.GetValue<3>() + (this->IsForward ? 0 : 4)) & 7;
				const auto faceCell = pLinked->GetMapCoords() + CellSpread::GetNeighbourOffset(primaryDir);

				if (MapClass::Instance->IsWithinUsableArea(faceCell, true))
				{
					const auto pCell = MapClass::Instance->GetCellAt(faceCell);
					const auto move = pLinked->IsCellOccupied(pCell, static_cast<FacingType>(primaryDir),
						pLinked->GetCellLevel(), nullptr, true);

					if (move == Move::ClosedGate)
					{
						MapClass::Instance->MakeTraversable(pLinked, faceCell);
					}
					else if (move == Move::Temp)
					{
						if (const auto pCellTechno = pCell->FindTechnoNearestTo(Point2D::Empty,
							(pLinked->Location.Z > (MapClass::Instance->GetCellFloorHeight(
								CellClass::Cell2Coord(faceCell)) + 2 * Unsorted::CellHeight)), nullptr))
						{
							if (pLinked->Owner->IsAlliedWith(pCellTechno) && !pType->IsTrain)
							{
								if ((pLinked->Location - this->TargetCoord).Length() < RulesClass::Instance->CloseEnough
									&& !pLinked->HasAnyLink()
									&& Math::abs(this->TargetCoord.Z - pLinked->Location.Z) < (2 * Unsorted::CellHeight)
									&& MapClass::Instance->GetCellAt(pLinked->Location)->LandType != LandType::Tunnel)
								{
									this->StopDriving<true>();
									return this->StopMotion();
								}

								const bool onBridge = pCell->ContainsBridge()
									&& (Math::abs(pLinked->Location.Z / Unsorted::CellHeight - pCell->Level) > 2);
								pCell->ScatterContent(CoordStruct::Empty, true, true, onBridge);
							}
						}
					}
				}

				const auto pathWaitTimes = pLinked->PathWaitTimes;

				if (pathWaitTimes <= 0)
				{
					this->StopDriving<true>();

					if (this->StopMotion())
						return true;

					if (!pLinked->IsAlive)
						return false;

					if (pLinked->ShouldScanForTarget)
						VocClass::PlayGlobal(RulesClass::Instance->ScoldSound, Panning::Center, 1.0);

					pLinked->ShouldScanForTarget = false;
				}
				else
				{
					pLinked->PathWaitTimes = pathWaitTimes - 1;
				}
			}

			if (!this->Is_Moving())
			{
				if (const auto pTarget = pLinked->Target)
				{
					if (!pLinked->IsCloseEnoughToAttack(pTarget))
					{
						pLinked->IsScanLimited = true;

						if (const auto pTeam = pLinked->Team)
							pTeam->ScanLimit();

						pLinked->SetTarget(nullptr);
					}
				}
			}

			this->StopDriving<true>();
			this->TrackNumber = -1;
			this->IsTurretLockedDown = false;
			return false;
		}

		const auto nowDir = pLinked->PathDirections[0];

		if (nowDir == 8)
			return false;

		const auto pathCell = pLinked->GetMapCoords() + CellSpread::GetNeighbourOffset(nowDir & 7);

		if (MapClass::Instance->IsWithinUsableArea(pathCell, true))
		{
			const auto pCell = MapClass::Instance->GetCellAt(pathCell);
			const auto move = pLinked->IsCellOccupied(pCell, static_cast<FacingType>(nowDir),
				pLinked->GetCellLevel(), nullptr, true);

			if (move == Move::ClosedGate)
			{
				MapClass::Instance->MakeTraversable(pLinked, pathCell);
			}
			else if (move == Move::Temp)
			{
				if (const auto pCellTechno = pCell->FindTechnoNearestTo(Point2D::Empty,
					(pLinked->Location.Z > (MapClass::Instance->GetCellFloorHeight(
						CellClass::Cell2Coord(pathCell)) + 2 * Unsorted::CellHeight)), nullptr))
				{
					if (pLinked->Owner->IsAlliedWith(pCellTechno) && !pType->IsTrain)
					{
						if ((pLinked->Location - this->TargetCoord).Length() < RulesClass::Instance->CloseEnough
							&& !pLinked->HasAnyLink()
							&& Math::abs(this->TargetCoord.Z - pLinked->Location.Z) < (2 * Unsorted::CellHeight)
							&& MapClass::Instance->GetCellAt(pLinked->Location)->LandType != LandType::Tunnel)
						{
							this->StopDriving<true>();
							return this->StopMotion();
						}

						const bool onBridge = pCell->ContainsBridge()
							&& (Math::abs(pLinked->Location.Z / Unsorted::CellHeight - pCell->Level) > 2);
						pCell->ScatterContent(CoordStruct::Empty, true, true, onBridge);
					}
				}
			}
		}

		pLinked->PathWaitTimes = 10;
		pathDir = pLinked->PathDirections[0];
	}
	while (false);

	if (pathDir == 8)
		return false;

	auto nextPos = pLinked->Location;
	AdvancedDriveLocomotionClass::SetAdjacentCoord(nextPos, (pathDir & 7));

	const int cellLevel = MapClass::Instance->GetCellAt(pLinked->Location)->Level + (pLinked->OnBridge ? 4 : 0);
	auto pNextCell = MapClass::Instance->GetCellAt(nextPos);

	if (pLinked->OnBridge != pNextCell->ContainsBridge())
		pLinked->IsPlanningToLook = true; // Seems like useless

	if (!pLinked->vt_entry_29C()) // Unknown unload state check
		return true;

	auto nextCell = CellClass::Coord2Cell(nextPos);

	if (!MapClass::Instance->MakeTraversable(pLinked, nextCell))
		return true;

	// Reverse movement
	const int desiredRaw = pathDir << 13;
	this->UpdateForwardState(desiredRaw);

	const auto desDir = DirStruct(this->IsForward ? desiredRaw : (desiredRaw + 32768));

	if (pLinked->PrimaryFacing.Current() != desDir)
	{
		this->Do_Turn(desDir);
		return true;
	}

	pLinked->Mark(MarkType::Up);
	auto moveResult = pLinked->IsCellOccupied(pNextCell, static_cast<FacingType>(pathDir), cellLevel, nullptr, true);
	pLinked->Mark(MarkType::Down);

	if (moveResult < Move::No && pType->IsTrain
		|| (moveResult == Move::Destroyable || moveResult == Move::FriendlyDestroyable)
			&& pType->Crusher && !pNextCell->OverlayTypeIndex)
	{
		moveResult = Move::OK;
	}

	bool crushableOverlay = false;

	do
	{
		if (pNextCell->OverlayTypeIndex != -1)
		{
			if (moveResult == Move::OK)
			{
				const auto pOverlay = OverlayTypeClass::Array->Items[pNextCell->OverlayTypeIndex];

				if (pOverlay->Crushable || pOverlay->Wall && pType->MovementZone == MovementZone::CrusherAll)
					crushableOverlay = true;

				break;
			}
		}
		else if (moveResult == Move::OK)
		{
			break;
		}

		if (moveResult == Move::ClosedGate)
		{
			MapClass::Instance->MakeTraversable(pLinked, nextCell);
		}
		else if (moveResult == Move::Temp)
		{
			if (!pType->IsTrain)
			{
				if (force)
				{
					pLinked->PathDirections[0] = -1;
					pLinked->PathDelayTimer.Start(0);
					return this->PassableCheck(pStop, false, false);
				}

				if ((pLinked->Location - this->TargetCoord).Length() < RulesClass::Instance->CloseEnough
					&& Math::abs(this->TargetCoord.Z - pLinked->Location.Z) < (2 * Unsorted::CellHeight)
					&& MapClass::Instance->GetCellAt(pLinked->Location)->LandType != LandType::Tunnel)
				{
					this->StopDriving<true>();

					if (this->StopMotion())
						return true;
				}
				else
				{
					const bool onBridge = pNextCell->ContainsBridge()
						&& (Math::abs(pLinked->Location.Z / Unsorted::CellHeight - pNextCell->Level) > 2);
					pNextCell->ScatterContent(CoordStruct::Empty, true, true, onBridge);
				}
			}
		}
		else if (moveResult == Move::Cloak)
		{
			pNextCell->RevealCellObjects();

			if (force)
			{
				pLinked->PathDirections[0] = -1;
				return this->PassableCheck(pStop, false, false);
			}

			this->StopDriving<true>();
			return this->StopMotion();
		}

		this->StopDriving<true>();

		do
		{
			if (moveResult == Move::MovingBlock)
			{
				if (!pLinked->IsWaitingBlockagePath)
				{
					pLinked->IsWaitingBlockagePath = true;
					pLinked->BlockagePathTimer.Start(RulesClass::Instance->BlockagePathDelay);
				}

				if (!pLinked->PathDelayTimer.GetTimeLeft())
				{
					const int findMode = static_cast<int>(pLinked->IsWaitingBlockagePath
						&& !pLinked->BlockagePathTimer.HasTimeLeft()) + 1;
					const bool pathFound = pLinked->UpdatePathfinding(CellClass::Coord2Cell(this->TargetCoord), false, findMode);

					if (!this->LinkedTo)
					{
						*pStop = true;
						return false;
					}

					if (pathFound || pLinked->IsInSameZone(&this->TargetCoord))
					{
						pLinked->PathDelayTimer.Start(int(RulesClass::Instance->PathDelay * 900.0));
						return true;
					}

					pLinked->SetDestination(nullptr, true);
					return false;
				}

				if (pLinked->ShouldScanForTarget)
					VocClass::PlayGlobal(RulesClass::Instance->ScoldSound, Panning::Center, 1.0);

				break;
			}

			if (moveResult != Move::Destroyable && moveResult != Move::FriendlyDestroyable)
			{
				if (pLinked->ShouldScanForTarget)
					VocClass::PlayGlobal(RulesClass::Instance->ScoldSound, Panning::Center, 1.0);

				break;
			}

			if (force)
			{
				pLinked->PathDirections[0] = -1;
				pLinked->PathDelayTimer.Start(0);
				return this->PassableCheck(pStop, false, false);
			}

			if (const auto pObject = pNextCell->GetSomeObject(Point2D::Empty, false))
			{
				if (!pLinked->Owner->IsAlliedWith(pObject))
					pLinked->Override_Mission(Mission::Attack, pObject, nullptr);
			}
			else if (pNextCell->OverlayTypeIndex != -1 && OverlayTypeClass::Array->Items[pNextCell->OverlayTypeIndex]->Wall)
			{
				pLinked->Override_Mission(Mission::Attack, pNextCell, nullptr);
			}
		}
		while (false);

		if (moveResult != Move::No)
		{
			pLinked->ShouldScanForTarget = false;
			this->TrackNumber = -1;
			return true;
		}

		if (force)
		{
			pLinked->PathDirections[0] = -1;
			pLinked->PathDelayTimer.Start(0);
			return this->PassableCheck(pStop, false, false);
		}

		this->StopDriving<true>();
		return this->StopMotion();
	}
	while (false);

	const bool different = Math::abs(cellLevel - pNextCell->Level) >= 2;
	const auto landType = different ? LandType::Road : pNextCell->LandType;
	const auto landLevel = different ? cellLevel : pNextCell->Level;

	double speedFactor = GroundType::Array[static_cast<int>(landType)].Cost[static_cast<int>(pType->SpeedType)];

	if (speedFactor > 1.0)
		speedFactor = 1.0;

	if (pLinked->WhatAmI() == AbstractType::Unit)
	{
		int currentHeight = MapClass::Instance->GetCellFloorHeight(pLinked->Location);
		CoordStruct cellCoords;
		pNextCell->GetCellCoords(&cellCoords);

		int nextHeight = MapClass::Instance->GetCellFloorHeight(cellCoords);

		if (nextHeight > currentHeight)
		{
			if (pType->SpeedType == SpeedType::Track)
				speedFactor *= RulesClass::Instance->TrackedUphill;
			else
				speedFactor *= RulesClass::Instance->WheeledUphill;
		}
		else if (nextHeight < currentHeight)
		{
			if (pType->SpeedType == SpeedType::Track)
				speedFactor *= RulesClass::Instance->TrackedDownhill;
			else
				speedFactor *= RulesClass::Instance->WheeledDownhill;
		}
	}

	if (speedFactor == 0.0)
		speedFactor = 0.5;

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	// Customized backward speed
	if (!this->IsForward)
		speedFactor *= pTypeExt->AdvancedDrive_Reverse_Speed;

	// Customized damaged speed
	if (pLinked->GetHealthPercentage() <= RulesClass::Instance->ConditionYellow)
		speedFactor *= pTypeExt->DamagedSpeed.Get(RulesExtData::Instance()->DamagedSpeed);

	if (this->TrackNumber >= 64)
		pLinked->SetSpeedPercentage(speedFactor);
	else
		this->MovementSpeed = speedFactor;

	pLinked->TryCrushCell(nextCell, true);
	auto nextDir = pLinked->PathDirections[1];

	do
	{
		if (nextDir == -1)
		{
			if (int((pLinked->Location - this->TargetCoord).Length()) <= 512)
			{
				nextDir = pathDir;
				break;
			}
			else
			{
				const bool pathFound = pLinked->UpdatePathfinding(CellClass::Coord2Cell(this->TargetCoord), pType->IsTrain, 0);

				if (!pathFound)
				{
					if (!this->LinkedTo)
					{
						*pStop = true;
						return false;
					}

					if (!pLinked->IsInSameZone(&this->TargetCoord))
						pLinked->SetDestination(nullptr, true);
				}

				nextDir = pLinked->PathDirections[1];
			}
		}

		if (nextDir == 8 || nextDir == -1 || check)
			nextDir = pathDir;
	}
	while (false);

	do
	{
		if (nextDir != -1)
		{
			const auto pAdjCell = pNextCell->GetNeighbourCell(static_cast<FacingType>(nextDir));

			if (pAdjCell->OverlayTypeIndex != -1)
			{
				const auto pOverlay = OverlayTypeClass::Array->Items[pAdjCell->OverlayTypeIndex];

				if (pOverlay->Crushable
					|| ((pOverlay->Wall || pAdjCell->GetUnit(false))
						&& pType->MovementZone == MovementZone::CrusherAll))
				{
					this->IsRocking = true;
					nextDir = pathDir;
					break;
				}
			}
		}

		if (crushableOverlay)
		{
			this->IsRocking = true;
			nextDir = pathDir;
			break;
		}

		this->IsRocking = false;

		// Reset is crushing flag
		pLinked->IsCrushingSomething = false;
	}
	while (false);

	this->IsOnShortTrack = false;
	this->TrackNumber = nextDir + 8 * pathDir;

	if (!DriveLocomotionClass::TurnTrack[this->TrackNumber].NormalTrackStructIndex)
		this->TrackNumber = 9 * pathDir;

	if (DriveLocomotionClass::TurnTrack[this->TrackNumber].Flag & 8)
	{
		this->IsShifting = true;
		auto nextMoveResult = Move::No;

		if (pNextCell->CollectCrate(pLinked) || pLinked->InLimbo)
		{
			if (!pLinked->IsAlive)
				return false;

			AdvancedDriveLocomotionClass::SetAdjacentCoord(nextPos, (nextDir & 7));
			nextCell = CellClass::Coord2Cell(nextPos);
			pNextCell = MapClass::Instance->GetCellAt(nextCell);
			nextMoveResult = pLinked->IsCellOccupied(pNextCell, static_cast<FacingType>(nextDir), landLevel, nullptr, true);

			if (nextMoveResult < Move::No && pType->IsTrain
				|| (nextMoveResult == Move::FriendlyDestroyable || nextMoveResult == Move::Destroyable)
					&& pType->Crusher && !pNextCell->OverlayTypeIndex)
			{
				nextMoveResult = Move::OK;
			}
		}
		else if (!pLinked->IsAlive)
		{
			return false;
		}

		if (nextMoveResult != Move::OK)
		{
			if (nextMoveResult == Move::ClosedGate)
			{
				MapClass::Instance->MakeTraversable(pLinked, nextCell);
			}
			else if (nextMoveResult == Move::MovingBlock)
			{
				return this->PassableCheck(pStop, force, true);
			}
			else if (nextMoveResult == Move::Temp)
			{
				if (!pType->IsTrain)
				{
					if (force)
					{
						pLinked->PathDirections[0] = -1;
						pLinked->PathDelayTimer.Start(0);
						return this->PassableCheck(pStop, false, false);
					}

					if ((pLinked->Location - this->TargetCoord).Length() < RulesClass::Instance->CloseEnough
						&& Math::abs(this->TargetCoord.Z - pLinked->Location.Z) < (2 * Unsorted::CellHeight)
						&& MapClass::Instance->GetCellAt(pLinked->Location)->LandType != LandType::Tunnel)
					{
						this->StopDriving<true>();

						if (this->StopMotion())
							return true;
					}
					else
					{
						const bool onBridge = pNextCell->ContainsBridge()
							&& (Math::abs(pLinked->Location.Z / Unsorted::CellHeight - pNextCell->Level) > 2);
						pNextCell->ScatterContent(CoordStruct::Empty, true, true, onBridge);
					}
				}
			}
			else if (nextMoveResult == Move::Cloak)
			{
				pNextCell->RevealCellObjects();

				if (force)
				{
					pLinked->PathDirections[0] = -1;
					return this->PassableCheck(pStop, false, false);
				}

				this->StopDriving<true>();
				return this->StopMotion();
			}
			else if (nextMoveResult == Move::No)
			{
				if (force)
				{
					pLinked->PathDirections[0] = -1;
					pLinked->PathDelayTimer.Start(0);
					return this->PassableCheck(pStop, false, false);
				}

				this->StopDriving<true>();
				return this->StopMotion();
			}

			pLinked->PathDirections[0] = -1;
			this->TrackNumber = -1;
			nextPos = CoordStruct::Empty;

			if (nextMoveResult == Move::Destroyable || nextMoveResult == Move::FriendlyDestroyable)
				return this->PassableCheck(pStop, force, true);
		}
		else
		{
			memmove(&pLinked->PathDirections[0], &pLinked->PathDirections[2], 0x58u);
			pLinked->PathDirections[22] = -1;
			pLinked->IsPlanningToLook = true; // Seems like useless
		}
	}
	else
	{
		memmove(&pLinked->PathDirections[0], &pLinked->PathDirections[1], 0x5Cu);
	}

	pLinked->PathDirections[23] = -1;
	pLinked->CurrentMapCoords = nextCell;
	pLinked->ShouldScanForTarget = false;
	this->TrackIndex = 0;
	this->StopDriving<true>();

	if (nextPos != CoordStruct::Empty)
	{
		this->IsDriving = true;
		this->HeadToCoord = nextPos;

		if (pNextCell->CollectCrate(pLinked) && !pLinked->InLimbo)
		{
			this->MarkOccupation(nextPos, MarkType::Down);
			return false;
		}
		else if (pLinked->IsAlive)
		{
			this->StopDriving();
		}
	}

	this->TrackNumber = -1;
	pLinked->PathDirections[0] = -1;
	pLinked->SetSpeedPercentage(0.0);
	return false;
}

void AdvancedDriveLocomotionClass::MarkOccupation(const CoordStruct& to, MarkType mark)
{
	if (to == CoordStruct::Empty)
		return;

	if (!this->IsOnShortTrack)
	{
		const auto trackNum = this->TrackNumber;

		if (trackNum != -1)
		{
			if (const auto trackStructIndex = DriveLocomotionClass::TurnTrack[trackNum].NormalTrackStructIndex)
			{
				const auto& track = DriveLocomotionClass::RawTrack[trackStructIndex];
				const auto trackIdx = track.CellIndex;

				if (trackIdx > -1 && this->TrackIndex < trackIdx)
				{
					if (mark == MarkType::Up)
					{
						const auto& trackPt = track.TrackPoint[trackIdx];
						auto face = trackPt.Face; // copy
						const auto pLinked = this->LinkedTo;
						pLinked->UnmarkAllOccupationBits(this->GetTrackOffset(trackPt.Point, face, pLinked->Location.Z));
					}
					else if (mark == MarkType::Down || mark == MarkType::ChangeRedraw)
					{
						const auto& trackPt = track.TrackPoint[trackIdx];
						auto face = trackPt.Face; // copy
						const auto pLinked = this->LinkedTo;
						pLinked->MarkAllOccupationBits(this->GetTrackOffset(trackPt.Point, face, pLinked->Location.Z));
					}
				}
			}
		}
	}

	if (mark == MarkType::Up)
		this->LinkedTo->UnmarkAllOccupationBits(to);
	else if (mark == MarkType::Down || mark == MarkType::ChangeRedraw)
		this->LinkedTo->MarkAllOccupationBits(to);
}

CoordStruct AdvancedDriveLocomotionClass::GetTrackOffset(const Point2D& base, int& face, int z)
{
	const auto dataFlag = DriveLocomotionClass::TurnTrack[this->TrackNumber].Flag;
	auto pt = base;

	if (dataFlag & 1)
	{
		pt.X = base.Y;
		pt.Y = base.X;
		face = static_cast<unsigned char>(0xC0 - face);
	}

	if (dataFlag & 2)
	{
		pt.X = -pt.X;
		face = static_cast<unsigned char>(-static_cast<char>(face));
	}

	if (dataFlag & 4)
	{
		pt.Y = -pt.Y;
		face = static_cast<unsigned char>(0x80 - face);
	}

	return CoordStruct { this->HeadToCoord.X + pt.X, this->HeadToCoord.Y + pt.Y, z };
}

void AdvancedDriveLocomotionClass::UpdateHoverState()
{
	const auto pLinked = this->LinkedTo;
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pLinked->GetTechnoType());
	const int hoverHeight = pTypeExt->AdvancedDrive_Hover_Height.Get(RulesClass::Instance->HoverHeight);
	const int oldHeight = pLinked->GetHeight();
	int adjustHeight = oldHeight;
	const int pathDir = pLinked->PathDirections[0];

	if (pathDir != -1)
	{
		auto coords = pLinked->Location;
		int floorHeight = MapClass::Instance->GetCellFloorHeight(coords);

		// Calculate bridge height
		if (pLinked->OnBridge)
			floorHeight += CellClass::BridgeHeight;

		AdvancedDriveLocomotionClass::SetAdjacentCoord(coords, (pathDir & 7));

		if (MapClass::Instance->GetCellFloorHeight(coords) > floorHeight)
			adjustHeight = oldHeight - hoverHeight;
	}

	const bool outOfBunker = !pLinked->BunkerLinkedItem;
	int newHeight = 0;

	// Unit in the bunker should not hover
	do
	{
		if (outOfBunker)
		{
			const int id = static_cast<int>(pLinked->UniqueID);
			const double hoverBob = pTypeExt->AdvancedDrive_Hover_Bob.Get(RulesClass::Instance->HoverBob);
			const double bobDelay = ((id & 1) ? 1.0 : 1.1) * hoverBob * 900.0;
			const double bobHeight = Math::sin(((Unsorted::CurrentFrame + 2 * id) % static_cast<int>(bobDelay)) / bobDelay * Math::TwoPi);
			newHeight = static_cast<int>(2 * bobHeight) + static_cast<int>(oldHeight + this->Wobbles);

			if (newHeight >= 0)
				break;

			newHeight = 0;
		}

		this->Wobbles = 0.0;
	}
	while (false);

	const bool wasOnMap = pLinked->IsOnMap;
	pLinked->IsOnMap = false;
	pLinked->SetHeight(newHeight);
	pLinked->IsOnMap = wasOnMap;

	if (outOfBunker)
	{
		if (adjustHeight < hoverHeight)
		{
			if (this->LocomotionClass::Is_Powered())
				this->Wobbles += static_cast<double>(2 * hoverHeight - adjustHeight) / hoverHeight * RulesClass::Instance->Gravity;

			if (adjustHeight < hoverHeight / 4)
				this->Wobbles += static_cast<double>(RulesClass::Instance->Gravity / 3);
		}

		const double hoverDampen = pTypeExt->AdvancedDrive_Hover_Dampen.Get(RulesClass::Instance->HoverDampen);
		this->Wobbles = (this->Wobbles - RulesClass::Instance->Gravity) * hoverDampen;
	}

	if (this->OutOfControl)
	{
		if (this->Is_Powered())
		{
			if (pTypeExt->AdvancedDrive_Hover_Spin && outOfBunker)
				pLinked->PrimaryFacing.Set_Current(DirStruct(pLinked->PrimaryFacing.Current().Raw + (this->TailSpin << 8)));

			if (this->TailSpin > 0)
				--this->TailSpin;
			else if (this->TailSpin < 0)
				++this->TailSpin;

			if (!this->TailSpin)
				this->OutOfControl = false;
		}
		else
		{
			this->OutOfControl = false;
		}

		if (!this->OutOfControl && outOfBunker)
		{
			const auto pCell = pLinked->GetCell();
			pCell->ActivateVeins();

			if (pTypeExt->AdvancedDrive_Hover_Sink
				&& pCell->LandType == LandType::Water
				&& pLinked->Location.Z < Unsorted::LevelHeight + MapClass::Instance->GetCellFloorHeight(pLinked->Location))
			{
				pLinked->DropAsBomb();
			}
		}
	}
}

CoordStruct AdvancedDriveLocomotionClass::CoordLerp(const CoordStruct& crd1, const CoordStruct& crd2, float alpha)
{
	const float i_alpha = 1.0f - alpha;
	return CoordStruct
		{
			int(crd2.X * alpha + crd1.X * i_alpha),
			int(crd2.Y * alpha + crd1.Y * i_alpha),
			int(crd2.Z * alpha + crd1.Z * i_alpha)
		};
}

// Auxiliary

inline void AdvancedDriveLocomotionClass::UpdateSituation()
{
	const auto pLinked = this->LinkedTo;
	auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pLinked->GetTechnoType());;

	if (const auto pTarget = pLinked->MegaMissionIsAttackMove() ? nullptr : pLinked->Target)
	{
		if (pLinked->DistanceFrom(pTarget) <= pTypeExt->AdvancedDrive_Reverse_FaceTargetRange.Get())
		{
			this->ForwardTo = pTarget->GetCoords();
			this->TargetFrame = Unsorted::CurrentFrame;
			this->TargetDistance = 0;
			return;
		}
	}

	if (this->ForwardTo != CoordStruct::Empty)
	{
		const auto currentDistance = static_cast<int>(pLinked->Location.DistanceFrom(this->ForwardTo));

		if (currentDistance > pTypeExt->AdvancedDrive_Reverse_FaceTargetRange.Get()
			|| (Unsorted::CurrentFrame - this->TargetFrame) > pTypeExt->AdvancedDrive_Reverse_RetreatDuration
			|| currentDistance < this->TargetDistance)
		{
			this->ForwardTo = CoordStruct::Empty;
			this->TargetFrame = 0;
		}

		this->TargetDistance = currentDistance;
	}
}

inline void AdvancedDriveLocomotionClass::UpdateForwardState(int desiredRaw)
{
	if (this->LinkedTo->WhatAmI() != AbstractType::Unit)
		return;

	const auto pLinked = static_cast<UnitClass*>(this->LinkedTo);
	const auto pType = pLinked->Type;
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (!pTypeExt->AdvancedDrive_Reverse)
		return;

	// Force forward movement for certain states
	if (pLinked->Team && pLinked->Team->NeedsReGrouping ||  // Always move forward when regrouping
		pLinked->CurrentMission == Mission::Move ||          // Always forward for basic movement
		pLinked->CurrentMission == Mission::Guard)           // Always forward when guarding
	{
		this->IsForward = true;
		return;
	}

	// Special cases that should always move forward
	const auto pLink = cast_to<BuildingClass*>(pLinked->GetNthLink());
	if (pLink && pLink->Type->Bunker)
	{
		this->IsForward = true;
		return;
	}

	// Harvester/Weeder special logic
	if (pType->Harvester || pType->Weeder)
	{
		if (pLinked->CurrentMission == Mission::Harvest)
		{
			this->IsForward = true;
			return;
		}

		// Only reverse when very close to refinery
		if (pLink && pLink->Type->Refinery &&
			pLinked->CurrentMission == Mission::Enter &&
			!pLinked->MissionStatus &&
			pLinked->DistanceFrom(pLinked->Destination) <= 363 &&
			!pLinked->GetCell()->GetBuilding())
		{
			this->IsForward = false;
			return;
		}
	}

	// Reverse movement decision logic
	bool shouldReverse = false;

	// Check if we have a target to face
	if (this->ForwardTo != CoordStruct::Empty) {
		// Calculate angle to target
		const DirStruct targetDir = DirStruct(Math::atan2(
			double(pLinked->Location.Y - this->ForwardTo.Y),
			double(this->ForwardTo.X - pLinked->Location.X)));

		// Get angle difference
		const short angleDiff = static_cast<short>(desiredRaw - targetDir.Raw);

		// Reverse if we're facing away from target and reverse is enabled
		shouldReverse = Math::abs(angleDiff) > 16384 && // More than 90 degrees
			pTypeExt->AdvancedDrive_Reverse_FaceTarget;
	}
	// Check if we should retreat
	else if (Unsorted::CurrentFrame - TechnoExtContainer::Instance.Find(pLinked)->LastHurtFrame
			 <= pTypeExt->AdvancedDrive_Reverse_RetreatDuration) {
		// When retreating, reverse if we're facing the threat
		const auto currentDir = pLinked->PrimaryFacing.Current();
		const short angleDiff = static_cast<short>(desiredRaw - currentDir.Raw);
		shouldReverse = Math::abs(angleDiff) < 8192; // Less than 45 degrees
	}
	// Check minimum distance for regular movement
	else if (pLinked->Destination &&
			 pLinked->DistanceFrom(pLinked->Destination) <= pTypeExt->AdvancedDrive_Reverse_MinimumDistance.Get()) {
		// Use reverse for fine positioning at close range
		shouldReverse = true;
	}

	this->IsForward = !shouldReverse;

	// Apply speed penalties for reverse movement
	if (!this->IsForward) {
		this->MovementSpeed *= pTypeExt->AdvancedDrive_Reverse_Speed;
	}
}

inline bool AdvancedDriveLocomotionClass::InMotion()
{
	const auto pLinked = this->LinkedTo;

	if (this->TrackNumber != -1 && this->IsDriving)
	{
		if (this->MovingProcess(false) || !pLinked->IsAlive)
			return false;

		if (this->TrackNumber != -1 || !this->Is_Moving() && pLinked->PathDirections[0] == -1)
			return true;

		if (pLinked->WhatAmI() == AbstractType::Unit)
		{
			if (static_cast<UnitClass*>(pLinked)->Unloading)
				return true;

			if (const auto pDestination = cast_to<InfantryClass*>(pLinked->Destination))
			{
				const auto coord = pDestination->GetDestination(pLinked);

				if (coord != this->TargetCoord)
					this->Move_To(coord);
			}
		}

		return this->TakeMovingAction(true);
	}

	const auto pDest = pLinked->Destination;

	if ((!pDest || pDest->WhatAmI() != AbstractType::Cell
			|| pLinked->GetMapCoords() != static_cast<CellClass*>(pDest)->MapCoords)
		&& (pLinked->CurrentMission != Mission::Guard || this->IsDriving
			|| this->TargetCoord == CoordStruct::Empty || this->TargetCoord != pLinked->Location))
	{
		if (pLinked->PrimaryFacing.Is_Rotating())
		{
			this->IsRotating = true;
			return true;
		}
		else if (this->IsRotating)
		{
			this->IsRotating = false;
			pLinked->UpdatePosition(PCPType::Rotation);

			if (this->LinkCannotMove())
				return false;
		}

		const auto mission = pLinked->GetCurrentMission();

		if (mission == Mission::Guard)
		{
			if (!this->Is_Moving())
				return true;
		}
		else if (mission == Mission::Unload)
		{
			// Unload stuck fix
			if (pLinked->GetTechnoType()->Passengers <= 0 || !pLinked->Passengers.GetFirstPassenger())
				return true;
		}

		if (!this->Is_Moving() && pLinked->PathDirections[0] == -1)
		{
			if (pLinked->IsSinking)
			{
				this->StopDriving<true>();
				this->MovementSpeed = 0.0;
			}
			else if (const auto pDestination = pLinked->Destination)
			{
				this->Move_To(pDestination->GetDestination(pLinked));
			}

			return true;
		}

		if (pLinked->IsInPlayfield && mission != Mission::Enter && this->Is_Moving()
			&& !pLinked->IsInSameZone(&this->TargetCoord))
		{
			this->StopDriving<true>();

			if (this->StopMotion())
				return false;
		}

		return this->TakeMovingAction(false);
	}

	this->StopMotion();
	return false;
}

inline int AdvancedDriveLocomotionClass::UpdateSpeedAccum(int& speedAccum)
{
	if (speedAccum <= 7)
		return 0;

	const auto pLinked = this->LinkedTo;
	auto pTrackData = &DriveLocomotionClass::TurnTrack[this->TrackNumber];
	int trackStructIndex = this->IsOnShortTrack ? pTrackData->ShortTrackStructIndex : pTrackData->NormalTrackStructIndex;
	auto pTrackPoints = DriveLocomotionClass::RawTrack[trackStructIndex].TrackPoint;
	const auto pathDir = pLinked->PathDirections[0];

	if (pathDir < -1 || pathDir > 8)
	{
		pLinked->PathDirections[0] = -1;
		return 1;
	}

	bool dirChanged = pathDir != 8 && pathDir != -1
		&& static_cast<int>(DirStruct(pTrackData->Face << 8).GetValue<3>()) != pathDir;

	const auto pType = pLinked->GetTechnoType();
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	while (true)
	{
		int trackIndex = this->TrackIndex;
		const auto& trackPoint = pTrackPoints[trackIndex];
		speedAccum -= 7;

		if (trackPoint.Point == Point2D::Empty && trackIndex)
			break;

		if (pLinked->IsStandingStill())
		{
			pLinked->UnmarkAllOccupationBits(pLinked->Location);
			pLinked->FrozenStill = false;
			pLinked->IsWaitingBlockagePath = false;
		}

		CellStruct previousCell;

		if (trackIndex)
		{
			const auto& prevTrackPoint = pTrackPoints[trackIndex - 1];
			auto prevFace = prevTrackPoint.Face;
			previousCell = CellClass::Coord2Cell(this->GetTrackOffset(prevTrackPoint.Point, prevFace));
		}
		else
		{
			previousCell = pLinked->GetMapCoords();
		}

		auto face = trackPoint.Face;
		const auto newPos = this->GetTrackOffset(trackPoint.Point, face, pLinked->Location.Z);

		if (CellClass::Coord2Cell(newPos) == CellClass::Coord2Cell(pLinked->Location))
		{
			const bool wasOnMap = pLinked->IsOnMap;
			pLinked->IsOnMap = false;
			pLinked->SetLocation(newPos);
			pLinked->IsOnMap = wasOnMap;
		}
		else
		{
			pLinked->Mark(MarkType::Up);
			pLinked->SetLocation(newPos);

			const auto pNewCell = MapClass::Instance->GetCellAt(newPos);
			this->UpdateOnBridge(pNewCell, MapClass::Instance->GetCellAt(previousCell));

			if (pLinked->GetTechnoType()->IsTrain && !static_cast<UnitClass*>(pLinked)->IsFollowerCar)
			{
				auto pObject = (pLinked->OnBridge || (pLinked->Location.Z >= (CellClass::BridgeHeight
						+ MapClass::Instance->GetCellFloorHeight(pLinked->Location))))
					? pNewCell->AltObject : pNewCell->FirstObject;

				while (pObject)
				{
					const auto pNext = pObject->NextObject;

					if (!pObject->IsCrushable(pLinked))
					{
						auto damage = 10000;
						pObject->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead, nullptr, true, true, nullptr);
						damage = 20;
						pLinked->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, nullptr);
					}

					pObject = pNext;
				}
			}

			if (!pLinked->IsAlive)
				return 1;

			pLinked->Mark(MarkType::Down);

			if (this->IsRocking)
			{
				if ((pType->MovementZone == MovementZone::CrusherAll && pNewCell->GetUnit(false))
					|| (pNewCell->OverlayTypeIndex != -1
						&& (pType->Crusher || pLinked->HasAbility(AbilityType::Crusher))
						&& OverlayTypeClass::Array->Items[pNewCell->OverlayTypeIndex]->Wall))
				{
					pLinked->IsCrushingSomething = true;

					if (pType->TiltsWhenCrushes)
					{
						// Customized crush tilt speed
						pLinked->RockingForwardsPerFrame = this->IsForward
							? static_cast<float>(pTypeExt->CrushForwardTiltPerFrame.Get(-0.05))
							: static_cast<float>(-pTypeExt->CrushForwardTiltPerFrame.Get(-0.05));
					}
				}
			}
		}

		if (!pLinked->IsAlive)
			return 1;

		// Maintain height
		if (!pTypeExt->AdvancedDrive_Hover)
		{
			const bool wasOnMap = pLinked->IsOnMap;
			pLinked->IsOnMap = false;
			pLinked->SetHeight(0);
			pLinked->IsOnMap = wasOnMap;
		}

		pLinked->PrimaryFacing.Set_Current(DirStruct((face << 8) + (this->IsForward ? 0 : 32768)));
		trackIndex = this->TrackIndex;

		if (trackIndex && DriveLocomotionClass::RawTrack[trackStructIndex].CellIndex == trackIndex)
			pLinked->UnmarkAllOccupationBits(pLinked->Location);

		if (pathDir != 8 && pathDir != -1 && dirChanged
			&& DriveLocomotionClass::RawTrack[trackStructIndex].JumpIndex == trackIndex
			&& trackIndex)
		{
			const int newTrack = pathDir + 8 * DirStruct(pTrackData->Face << 8).GetValue<3>();
			const auto pNewTrackData = &DriveLocomotionClass::TurnTrack[newTrack];
			const auto normalIndex = pNewTrackData->NormalTrackStructIndex;

			if (normalIndex && DriveLocomotionClass::RawTrack[normalIndex].EntryIndex)
			{
				auto coords = this->HeadToCoord;
				AdvancedDriveLocomotionClass::SetAdjacentCoord(coords, pathDir);
				const auto pCell = MapClass::Instance->GetCellAt(coords);

				switch (pLinked->IsCellOccupied(pCell, static_cast<FacingType>(pathDir),
					pLinked->GetCellLevel(), nullptr, true))
				{
					case Move::OK:
					case Move::MovingBlock:
					{
						if (pLinked->WhatAmI() == AbstractType::Unit && !static_cast<UnitClass*>(pLinked)->Type->Passive)
							break;

						const auto speedPercent = pLinked->SpeedPercentage;
						this->IsOnShortTrack = false;
						this->TrackNumber = newTrack;
						pTrackData = pNewTrackData;
						dirChanged = false;
						trackStructIndex = pNewTrackData->NormalTrackStructIndex;
						this->TrackIndex = DriveLocomotionClass::RawTrack[trackStructIndex].EntryIndex - 1;
						pTrackPoints = DriveLocomotionClass::RawTrack[trackStructIndex].TrackPoint;

						this->StopDriving<true>();
						this->IsDriving = true;
						pLinked->UpdatePosition(PCPType::End);
						this->IsDriving = false;

						if (this->LinkCannotMove())
							return 1;

						this->StopDriving<true>();

						if (coords != CoordStruct::Empty)
						{
							this->IsDriving = true;
							this->HeadToCoord = coords;

							if (!pCell->CollectCrate(pLinked) || pLinked->InLimbo)
							{
								if (pLinked->IsAlive)
									this->StopDriving();
							}
							else
							{
								this->MarkOccupation(coords, MarkType::Down);
								pLinked->SetSpeedPercentage(speedPercent);
								memmove(&pLinked->PathDirections[0], &pLinked->PathDirections[1], 0x5Cu);
								pLinked->PathDirections[23] = -1;
							}
						}

						break;
					}

					case Move::Cloak:
					{
						pCell->RevealCellObjects();
						break;
					}

					case Move::ClosedGate:
					{
						MapClass::Instance->MakeTraversable(pLinked, CellClass::Coord2Cell(coords));
						break;
					}

					case Move::Temp:
					{
						const bool onBridge = pCell->ContainsBridge()
							&& (Math::abs(pLinked->Location.Z / Unsorted::CellHeight - pCell->Level) > 2);
						MapClass::Instance->GetCellAt(this->HeadToCoord)->ScatterContent(CoordStruct::Empty,
							true, true, onBridge);

						break;
					}

					default:
					{
						break;
					}
				}
			}
		}

		++this->TrackIndex;

		if (speedAccum <= 7)
			return 0;
	}

	const auto delta = this->HeadToCoord - pLinked->Location;
	const auto distance = Math::abs(delta.X) + Math::abs(delta.Y);
	speedAccum += int((1.0 - distance / 11.0) * 7.0);

	pLinked->FrozenStill = true;
	pLinked->IsWaitingBlockagePath = false;

	if (CellClass::Coord2Cell(this->HeadToCoord) == CellClass::Coord2Cell(pLinked->Location))
	{
		const bool wasOnMap = pLinked->IsOnMap;
		pLinked->IsOnMap = false;

		// Maintain height
		if (pTypeExt->AdvancedDrive_Hover)
		{
			auto newPos = this->HeadToCoord;
			newPos.Z = pLinked->Location.Z;
			pLinked->SetLocation(newPos);
		}
		else
		{
			pLinked->SetLocation(this->HeadToCoord);
			pLinked->SetHeight(0);
		}

		pLinked->IsOnMap = wasOnMap;
	}
	else
	{
		pLinked->Mark(MarkType::Up);

		// Maintain height
		if (pTypeExt->AdvancedDrive_Hover)
		{
			auto newPos = this->HeadToCoord;
			newPos.Z = pLinked->Location.Z;
			pLinked->SetLocation(newPos);
		}
		else
		{
			pLinked->SetLocation(this->HeadToCoord);
			pLinked->SetHeight(0);
		}

		pLinked->Mark(MarkType::Down);
	}

	this->StopDriving<true>();
	this->TrackNumber = -1;
	this->TrackIndex = 0;
	bool reachedDestination = false;

	if (const auto pDestination = pLinked->Destination)
	{
		if (pLinked->GetMapCoords() == CellClass::Coord2Cell(pDestination->GetDestination(pLinked))
			&& Math::abs(pLinked->GetDestination(pLinked).Z - this->TargetCoord.Z) < 2 * Unsorted::CellHeight)
		{
			reachedDestination = true;
			this->TargetCoord = CoordStruct::Empty;
			this->StopDriving<true>();
			this->IsDriving = false;
		}
	}

	pLinked->UpdatePosition(PCPType::End);

	if (!this->LinkedTo || this->LinkCannotMove())
		return 2;

	if (reachedDestination)
	{
		pLinked->AbortMotion();
		pLinked->PathDirections[0] = -1;

		if (pLinked->GetCurrentMission() == Mission::Move && pLinked->EnterIdleMode(false, true))
			return 2;
	}

	if (pLinked->TryEnterIdle())
		return 2;

	return pLinked->IsAlive ? 0 : 1;
}

// Hooks

ASMJIT_PATCH(0x4DA9FB, FootClass_Update_WalkedFrames, 0x6)
{
	enum { SkipGameCode = 0x4DAA01 };

	GET(FootClass* const, pThis, ESI);

	if (AdvancedDriveLocomotionClass::IsReversing(pThis))
	{
		--pThis->WalkedFramesSoFar;
		return SkipGameCode;
	}

	return 0; // ++pThis->WalkedFramesSoFar;
}