#include "LevitateLocomotionClass.h"

#include <CellSpread.h>
#include <ScenarioClass.h>

#include <ParticleSystemClass.h>
#include <ParticleSystemTypeClass.h>

#include <AnimClass.h>
#include <AircraftClass.h>
#include <InfantryClass.h>
#include <UnitClass.h>

#include <Ext/Techno/Body.h>

WORD GetFacingVal(CoordStruct a2, CoordStruct a3)
{
	if (a2 == a3)
		return 0;

	return (WORD)(DWORD)(
		(__int64)((Math::atan2(double(a2.Y - a3.Y), double(a2.X - a3.X))
			- Math::DEG90_AS_RAD) * Math::BINARY_ANGLE_MAGIC_VALUE));
}

void JumpTo4(LevitateLocomotionClass* pThis, float a2)
{

	const auto v6 = (float)((__int16)(a2 - 0x3FFF) * -0.00009587672516830327);
	const auto v5 = std::sinf(v6);
	const auto v3 = std::cosf(v6);
	pThis->CurrentSpeed = 0.0;
	pThis->AccelerationDurationNegSinus = 0.0;
	pThis->AccelerationDurationCosinus = 0.0;
	pThis->AccelerationDuration = 0;
	pThis->Delta.X = pThis->Characteristic.Intentional_DriftVelocity * v3;
	pThis->Delta.Y = -(pThis->Characteristic.Intentional_DriftVelocity * v5);
	pThis->CurrentVelocity = pThis->Characteristic.Intentional_DriftVelocity;
	pThis->State = 4;
}

void LevitateLocomotionClass::ProcessHovering()
{
	//GameDebugLog::Log(__FUNCTION__" Called !  ");
	const auto nInitialHeight = LinkedTo->GetHeight();
	auto nInitialHeight_c1 = nInitialHeight;

	if (LinkedTo->PathDirections[0] != 0)
	{
		const auto nCoord = LinkedTo->GetCenterCoords();
		const auto nZ = MapClass::Instance->GetCellFloorHeight(nCoord);
		const CellStruct nAdjentCoord = CellSpread::AdjacentCell[(((unsigned char)LinkedTo->PathDirections[0]) & 7)];
		const auto nDestCoord = nCoord + CoordStruct { (int)nAdjentCoord.X, (int)nAdjentCoord.Y,0 };

		if (!(MapClass::Instance->GetCellFloorHeight(nDestCoord) <= nZ))
		{
			nInitialHeight_c1 -= RulesClass::Instance->HoverHeight;
		}
	}

	const auto nDampen = nInitialHeight + Dampen;
	const auto pTechWhat = (int)LinkedTo->WhatAmI();
	const double dMult = (pTechWhat & 1) ? 1.1 : 1.0;
	const auto nFrame = Unsorted::CurrentFrame + 2 * pTechWhat;
	const auto nHoverBob = dMult * RulesClass::Instance->HoverBob * 900.0;
	const auto nVal2 = (nFrame % int(nHoverBob)) * Math::TwoPi / nHoverBob;
	const auto nVal3 = Math::sin((float)nVal2);
	int nDampenResult = (int)(nVal3 + nVal3 + nDampen);

	if (nDampenResult < 0)
	{
		Dampen = 0.0;
		nDampenResult = 0;
	}

	const auto bIsOnMap = LinkedTo->IsOnMap;
	LinkedTo->IsOnMap = false;
	LinkedTo->SetHeight(nDampenResult);
	LinkedTo->IsOnMap = bIsOnMap;

	if (nInitialHeight_c1 < RulesClass::Instance->HoverHeight)
	{
		if (this->Is_Powered())
		{
			const auto nDoubleHeight = RulesClass::Instance->HoverHeight * 2.0;
			const auto nHeight1 = (nDoubleHeight - (double)nInitialHeight_c1);
			Dampen = nHeight1 / (double)RulesClass::Instance->HoverHeight * (double)RulesClass::Instance->Gravity + Dampen;
		}

		if (nInitialHeight_c1 < (RulesClass::Instance->HoverHeight / 4))
		{
			Dampen = double(RulesClass::Instance->Gravity / 3) + Dampen;
		}
	}

	Dampen -= (double)RulesClass::Instance->Gravity;
	Dampen *= RulesClass::Instance->HoverDampen;
}

// Done
void LevitateLocomotionClass::DoPhase1()
{
	if (const auto pTargetT = flag_cast_to<TechnoClass*, false>(LinkedTo->Target))
	{
		if (pTargetT->IsAlive && pTargetT->IsOnMap)
		{
			const auto nTargetCoord = pTargetT->GetCoords();;
			if ((int)LinkedTo->GetCoords().DistanceFromXY(nTargetCoord) < 128)
			{
				this->AccelerationDurationNegSinus = 0.0;
				this->AccelerationDurationCosinus = 0.0;
				this->CurrentVelocity = 0.0;
				this->Delta.Y = 0;
				this->Delta.X = 0;
				this->State = 5;
			} else{

				if (LevitateLocomotionClass::IsCloseEnough(nTargetCoord))
					this->CalculateDir_Close(nTargetCoord);
				else
					this->CalculateDir_Far(nTargetCoord);
			}

			if (this->State) {

				if (LinkedTo->IsPathBlocked)
					LinkedTo->IsPathBlocked = false;

				LinkedTo->UnmarkAllOccupationBits(LinkedTo->GetCoords());
			}

			return;
		}

		LinkedTo->SetTarget(nullptr);
	}

	if(LinkedTo->Destination) {
		const auto pTargetNav = flag_cast_to<TechnoClass*>(LinkedTo->Destination);

		if (!pTargetNav || !pTargetNav->IsAlive || !pTargetNav->IsOnMap)
		{
			LinkedTo->Destination = nullptr;
			auto Mission = LinkedTo->GetCurrentMission();
			if (Mission != Mission::Sticky && Mission != Mission::Sleep)
			{
				if (ScenarioClass::Instance->Random.RandomDouble() < 0.01) {
					DirtoSomething(ScenarioClass::Instance->Random.RandomDouble() * Math::TwoPi);
				}
			}

			if (this->State) {
				if (LinkedTo->IsPathBlocked)
					LinkedTo->IsPathBlocked = false;

				LinkedTo->UnmarkAllOccupationBits(LinkedTo->GetCoords());
			}
		}

		const auto nTargetCoord = pTargetNav->GetCoords();
		if ((int)LinkedTo->GetCoords().DistanceFromXY(nTargetCoord) < 128)
		{
			this->AccelerationDurationNegSinus = 0.0;
			this->AccelerationDurationCosinus = 0.0;
			this->CurrentVelocity = 0.0;
			this->Delta.Y = 0;
			this->Delta.X = 0;
			this->State = 5;
		}
		else
		{

			if (LevitateLocomotionClass::IsCloseEnough(nTargetCoord))
				this->CalculateDir_Close(nTargetCoord);
			else
				this->CalculateDir_Far(nTargetCoord);
		}

		if (this->State)
		{

			if (LinkedTo->IsPathBlocked)
				LinkedTo->IsPathBlocked = false;

			LinkedTo->UnmarkAllOccupationBits(LinkedTo->GetCoords());
		}

		return;
	}

	auto Mission = LinkedTo->GetCurrentMission();
	if (Mission != Mission::Sticky && Mission != Mission::Sleep) {
		if (ScenarioClass::Instance->Random.RandomDouble() < 0.01) {
			DirtoSomething(ScenarioClass::Instance->Random.RandomDouble() * Math::TwoPi);
		}
	}

	if (this->State)
	{
		if (LinkedTo->IsPathBlocked)
			LinkedTo->IsPathBlocked = false;

		LinkedTo->UnmarkAllOccupationBits(LinkedTo->GetCoords());
	}
}

void LevitateLocomotionClass::DoPhase2()
{
	if (LevitateLocomotionClass::IsTargetValid()
		&& LevitateLocomotionClass::IsCloseEnough(LinkedTo->Target->GetCoords()))
	{
		this->AccelerationDuration = 0;
		this->State = 3;
		this->CurrentSpeed = Characteristic.Intentional_Deacceleration;
	}
	else
		if (LevitateLocomotionClass::IsDestValid()
			&& LevitateLocomotionClass::IsCloseEnough(LinkedTo->Destination->GetCoords()))
		{
			this->AccelerationDuration = 0;
			this->State = 3;
			this->CurrentSpeed = Characteristic.Intentional_Deacceleration;
		}
		else
			if (!this->AccelerationDuration)
			{
				this->CurrentSpeed = Characteristic.Drag;
				this->State = 2;
			}
}

bool LevitateLocomotionClass::IsCloseEnough(CoordStruct nCoord)
{
	return Characteristic.ProximityDistance * 256.0 > nCoord.DistanceFromXY(LinkedTo->GetCoords());
}

bool LevitateLocomotionClass::IsLessSameThanProximityDistance(CoordStruct nCoord)
{
	return Characteristic.ProximityDistance * 256.0 <= nCoord.DistanceFromXY(LinkedTo->GetCoords());
}

// Done
void LevitateLocomotionClass::CalculateDir_Close(CoordStruct nTarget)
{
	//GameDebugLog::Log(__FUNCTION__" Called !  ");
	const auto Coord = LinkedTo->GetCoords();
	const auto TCoord = nTarget;
	CoordStruct _remaining = Coord - TCoord;
	DirStruct _dirCoord { (double)_remaining.Y , (double)_remaining.X };
	const auto nMath_2 = _dirCoord.GetRadian<65536>();
	const auto nMath_3 = Math::sin((float)nMath_2);
	const auto nMath_4 = Math::cos((float)nMath_2);

	this->CurrentSpeed = 0.0;
	this->AccelerationDurationNegSinus = 0.0;
	this->AccelerationDurationCosinus = 0.0;
	this->AccelerationDuration = 0;
	this->Delta.Y = Characteristic.Intentional_DriftVelocity * nMath_4;
	this->Delta.X = -(Characteristic.Intentional_DriftVelocity * nMath_3);
	this->CurrentVelocity = Characteristic.Intentional_DriftVelocity;
	this->State = 4;

	if ((int)Math::abs(this->Delta.X) > Math::abs(Coord.X - TCoord.X))
		this->Delta.X = (double)(Coord.X - TCoord.X);
	if ((int)Math::abs(this->Delta.Y) > Math::abs(Coord.Y - TCoord.Y))
		this->Delta.Y = (double)(Coord.Y - TCoord.Y);
}

// Done
void LevitateLocomotionClass::DirtoSomething(double dValue)
{
	if (!Characteristic.Propulsion_Sounds.empty())
	{
		const auto nRand = Random2Class::NonCriticalRandomNumber->RandomFromMax(Characteristic.Propulsion_Sounds.size() - 1);
		VocClass::PlayGlobal(nRand, Panning::Center, 1.0f);
	}

	AccelerationDuration = Characteristic.Accel_Dur;
	const auto nAccel = Characteristic.Accel;
	const auto nInitboost = Characteristic.Initial_Boost;
	const auto nSin = Math::sin((float)dValue);
	const auto nCos = Math::cos((float)dValue);
	AccelerationDurationCosinus = nAccel * nCos;
	AccelerationDurationNegSinus = -(nAccel * nCos);
	Delta.X = nInitboost * nCos + Delta.X;
	Delta.Y = Delta.X - nInitboost * nSin;;
	CurrentVelocity = Delta.Y * Delta.Y + Delta.X * Delta.X;
	auto nCenter = LinkedTo->GetCoords();
	const auto pSys = GameCreate<ParticleSystemClass>(RulesExtData::Instance()->DefaultGlobalParticleInstance, nCenter);
	auto nEmpty = CoordStruct { 0,0,0 };
	const auto pParticle = pSys->SpawnHeldParticle(&nCenter, &nEmpty);
	pParticle->GasVelocity.X = (int)(nCos * -16.0);
	pParticle->GasVelocity.Y = (int)(nSin * 16.0);
	pParticle->GasVelocity.Z = -12;
	CurrentSpeed = Characteristic.Drag;
	State = 1;
}

void LevitateLocomotionClass::CalculateDir_Far(CoordStruct nTarget)
{
	//GameDebugLog::Log(__FUNCTION__" Called !  ");
	const auto nCoord = LinkedTo->GetCoords();
	CoordStruct _remaining = nCoord - nTarget;
	DirStruct _dirCoord { (double)_remaining.Y , (double)_remaining.X };
	DirtoSomething(_dirCoord.GetRadian<65536>());
}

void LevitateLocomotionClass::DoPhase3()
{

	if(LinkedTo->Target) {
		const auto pTargetT = flag_cast_to<TechnoClass*>(LinkedTo->Target);

		if (pTargetT && pTargetT->IsAlive && pTargetT->IsOnMap)
		{
			if (CurrentVelocity >= Characteristic.Vel_Max_WhenPissedOff) {
				if (this->IsLessSameThanProximityDistance(pTargetT->GetCoords())) {
					return;
				}
			}

			AccelerationDuration = 0;
			State = 3;
			CurrentSpeed = Characteristic.Intentional_Deacceleration;
			return;
		}

		LinkedTo->SetTarget(nullptr);
	}

	if(LinkedTo->Destination) {
		const auto pNavT = flag_cast_to<TechnoClass*>(LinkedTo->Destination);

		if (pNavT && pNavT->IsAlive && pNavT->IsOnMap)
		{
			if (CurrentVelocity >= Characteristic.Vel_Max_WhenFollow)
			{
				if (this->IsCloseEnough(pNavT->GetCoords()))
				{
					return;
				}
			}

			AccelerationDuration = 0;
			State = 3;
			CurrentSpeed = Characteristic.Intentional_Deacceleration;
			return;
		}

		LinkedTo->Destination = nullptr;
	}

	if (CurrentVelocity >= 0.01)
	{
		if (!AccelerationDuration && CurrentVelocity < Characteristic.Vel_Max_Happy &&
			ScenarioClass::Instance->Random.RandomDouble() < Characteristic.Accel_Prob)
		{
			this->DirtoSomething(ScenarioClass::Instance->Random.RandomDouble()
				* Math::TwoPi);
		}

	}
	else
	{
		CurrentSpeed = Characteristic.Drag;
		Delta.X = 0.0;
		Delta.Y = 0.0;
		State = 0;

		if (!LinkedTo->IsPathBlocked){
			LinkedTo->IsPathBlocked = true;

			LinkedTo->UnmarkAllOccupationBits(LinkedTo->GetCoords());
		}
	}
}

void LevitateLocomotionClass::DoPhase4()
{
	//GameDebugLog::Log(__FUNCTION__" Called !  ");
	if (CurrentVelocity < 0.01)
	{
		if (auto pTargetT = flag_cast_to<TechnoClass*>(LinkedTo->Target))
		{
			if (pTargetT->IsAlive && pTargetT->IsOnMap)
			{
				if ((int)LinkedTo->GetCoords().DistanceFromXY(pTargetT->GetCoords()) < 128)
				{
					this->AccelerationDurationNegSinus = 0.0;
					this->AccelerationDurationCosinus = 0.0;
					this->CurrentVelocity = 0.0;
					this->Delta.Y = 0.0;
					this->Delta.X = 0.0;
					this->State = 5;
					this->AccelerationDuration = 0;
					return;
				}
				else
				{

					auto nTargetCoord = pTargetT->GetCoords();

					if (LevitateLocomotionClass::IsCloseEnough(nTargetCoord))
					{
						LevitateLocomotionClass::CalculateDir_Far(nTargetCoord);
					}

					LevitateLocomotionClass::CalculateDir_Close(nTargetCoord);
				}
			}

			LinkedTo->SetTarget(nullptr);
		}

		const auto pNavT = flag_cast_to<TechnoClass*>(LinkedTo->Destination);

		if (!pNavT || !pNavT->IsAlive || !pNavT->IsOnMap)
		{
			LinkedTo->Destination = nullptr;
			CurrentSpeed = Characteristic.Drag;
			Delta.X = 0.0;
			Delta.Y = 0.0;
			State = 0;
			LinkedTo->SetSpeedPercentage(0.0);
			LinkedTo->UnmarkAllOccupationBits(LinkedTo->GetCoords());
			return;
		}

		const auto nDist_Loc = LinkedTo->Destination->GetCoords();
		if ((int)LinkedTo->GetCoords().DistanceFromXY(nDist_Loc) < 128)
		{
			this->AccelerationDurationNegSinus = 0.0;
			this->AccelerationDurationCosinus = 0.0;
			this->CurrentVelocity = 0.0;
			this->Delta.Y = 0.0;
			this->Delta.X = 0.0;
			this->State = 5;
			this->AccelerationDuration = 0;
			return;
		}

		if (this->IsLessSameThanProximityDistance(nDist_Loc))
		{
			const auto nDouble = (double)((__int16)GetFacingVal(LinkedTo->GetCoords(), nDist_Loc) - 0x3FFF) * -0.00009587672516830327;;
			this->DirtoSomething(nDouble);
			return;
		}

		LevitateLocomotionClass::CalculateDir_Close(nDist_Loc);
	}
}

void LevitateLocomotionClass::DoPhase5(CoordStruct coord)
{
	//GameDebugLog::Log(__FUNCTION__" Called !  ");
	if ((int)coord.DistanceFromXY(LinkedTo->GetCoords()) >= 128)
	{
		if (this->IsLessSameThanProximityDistance(coord))
		{
			const auto nCoord = LinkedTo->GetCenterCoords();
			const auto atan = Math::atan2(double(nCoord.Y - coord.Y), double(nCoord.X - coord.X));
			const auto nDir = (double)((__int16)(__int64)((atan - Math::DEG90_AS_RAD) * Math::BINARY_ANGLE_MAGIC_VALUE) - 0x3FFF) *
				-0.00009587672516830327;
			this->DirtoSomething(nDir);
		}
		else
		{
			const auto Coord = LinkedTo->GetCenterCoords();
			const auto atan2 = Math::atan2((double)(Coord.Y - coord.Y), (double)(Coord.X - coord.X));
			const auto nMath_2 = (double)((__int16)(__int64)((atan2 - Math::DEG90_AS_RAD) * Math::BINARY_ANGLE_MAGIC_VALUE) - 0x3FFF) * -0.00009587672516830327;;
			const auto nMath_3 = Math::sin((float)nMath_2);
			const auto nMath_4 = Math::cos((float)nMath_2);

			this->CurrentSpeed = 0.0;
			this->AccelerationDurationNegSinus = 0.0;
			this->AccelerationDurationCosinus = 0.0;
			this->AccelerationDuration = 0;
			this->Delta.Y = Characteristic.Intentional_DriftVelocity * nMath_4;
			this->Delta.X = -(Characteristic.Intentional_DriftVelocity * nMath_3);
			this->CurrentVelocity = Characteristic.Intentional_DriftVelocity;
			this->State = 4;

			if ((int)Math::abs(this->Delta.X) > Math::abs(Coord.X - coord.X))
				this->Delta.X = (double)(Coord.X - coord.X);
			if ((int)Math::abs(this->Delta.Y) > Math::abs(Coord.Y - coord.Y))
				this->Delta.Y = (double)(Coord.Y - coord.Y);
		}
	}
	else
	{
		this->State = 5;
		this->AccelerationDurationNegSinus = 0.0;
		this->AccelerationDurationCosinus = 0.0;
		this->CurrentVelocity = 0.0;
		this->Delta.Y = 0.0;
		this->Delta.X = 0.0;
		this->AccelerationDuration = 0;
	}
}

void LevitateLocomotionClass::DoPhase6()
{
	//GameDebugLog::Log(__FUNCTION__" Called !  ");
	if (auto pTargetT = flag_cast_to<TechnoClass*>(LinkedTo->Target))
	{
		if (pTargetT->IsAlive && pTargetT->IsOnMap)
		{
			if ((int)LinkedTo->GetCoords().DistanceFromXY(pTargetT->GetCoords()) < 128)
			{
				this->AccelerationDurationNegSinus = 0.0;
				this->AccelerationDurationCosinus = 0.0;
				this->CurrentVelocity = 0.0;
				this->Delta.Y = 0.0;
				this->Delta.X = 0.0;
				this->State = 5;
				this->AccelerationDuration = 0;
				return;
			}
			else
			{

				auto nTargetCoord = pTargetT->GetCoords();

				if (LevitateLocomotionClass::IsCloseEnough(nTargetCoord))
				{
					LevitateLocomotionClass::CalculateDir_Far(nTargetCoord);
					return;
				}

				LevitateLocomotionClass::CalculateDir_Close(nTargetCoord);
				return;
			}
		}

		LinkedTo->SetTarget(nullptr);
	}

	if (auto pDestT = flag_cast_to<TechnoClass*>(LinkedTo->Destination))
	{
		if (pDestT->IsAlive && pDestT->IsOnMap)
		{
			if (!((int)LinkedTo->GetCoords().DistanceFromXY(pDestT->GetCoords()) < 128))
			{
				auto nTargetCoord = pDestT->GetCoords();
				if (LevitateLocomotionClass::IsCloseEnough(nTargetCoord))
				{
					LevitateLocomotionClass::CalculateDir_Far(nTargetCoord);
					return;
				}

				LevitateLocomotionClass::CalculateDir_Close(nTargetCoord);
				return;
			}
		}
	}

	LinkedTo->Destination = nullptr;
	this->CurrentSpeed = Characteristic.Drag;
	this->CurrentVelocity = 0.0;
	this->Delta.Y = 0.0;
	this->Delta.X = 0.0;
	this->State = 0;
	return;
}

void LevitateLocomotionClass::DoPhase7()
{
	//GameDebugLog::Log(__FUNCTION__" Called !  ");
	const auto nCoord = LinkedTo->GetCenterCoords();
	const auto nCoordCell = CoordStruct{ (nCoord.X * 256) + 128  , (nCoord.Y * 256) + 128 ,nCoord.Z };
	const auto nDistance = (nCoordCell  - LinkedTo->GetCoords()).LengthXY();

	if (nDistance < 5)
	{
		if (const auto pTargetT = flag_cast_to<TechnoClass*>(LinkedTo->Target))
		{
			if (pTargetT->IsAlive && pTargetT->IsOnMap)
			{
				const auto nTargetCoord = pTargetT->GetCoords();
				const auto nTargetCoordCell = CellClass::Coord2Cell(nTargetCoord);

				LinkedTo->UpdatePathfinding(nTargetCoordCell, CellStruct::Empty, 0);
				const auto nSelected = LinkedTo->IsSelected;
				LinkedTo->IsSelected = false;
				LinkedTo->SetLocation(nCoordCell);
				LinkedTo->IsSelected = nSelected;
				const auto nCoord_diff = nTargetCoord - LinkedTo->GetCenterCoords();
				const auto nFaceRaw = LinkedTo->PathDirections[0] != 0 ?
					(int)LinkedTo->PathDirections[0] << 13 :
					(unsigned short)(((Math::atan2(double(nCoord_diff.Y), double(nCoord_diff.X))) - Math::DEG90_AS_RAD) * Math::BINARY_ANGLE_MAGIC_VALUE);

				const auto nFace_Value = float((double)(nFaceRaw - 0x3FFF) * -0.00009587672516830327);
				const auto nSin = std::sinf(nFace_Value);
				const auto nCos = std::cosf(nFace_Value);
				this->CurrentSpeed = 0.0;
				this->AccelerationDurationNegSinus = 0.0;
				this->AccelerationDurationCosinus = 0.0;
				this->AccelerationDuration = 0;
				this->Delta.X = nSin * Characteristic.Intentional_DriftVelocity;
				this->Delta.Y = -(nCos * Characteristic.Intentional_DriftVelocity);
				this->CurrentVelocity = Characteristic.Intentional_DriftVelocity;
				this->State = 7;
				return;
			}

			LinkedTo->SetTarget(nullptr);
		}

		if (!LinkedTo->Destination)
		{
			this->CurrentSpeed = Characteristic.Drag;
			this->State = 2;
			return;
		}

		const auto pDestT = flag_cast_to<TechnoClass*>(LinkedTo->Destination);

		if (pDestT && pDestT->IsAlive && pDestT->IsOnMap)
		{
			const auto nTargetCoord = pDestT->GetCenterCoords();
			const auto nTargetCoordCell = CellClass::Coord2Cell(nTargetCoord);

			LinkedTo->UpdatePathfinding(nTargetCoordCell, CellStruct::Empty, 0);
			const auto nSelected = LinkedTo->IsSelected;
			LinkedTo->IsSelected = false;
			LinkedTo->SetLocation(nCoordCell);
			LinkedTo->IsSelected = nSelected;
			const auto nCoord_diff = nTargetCoord - LinkedTo->GetRenderCoords();
			const auto nFaceRaw = LinkedTo->PathDirections[0] != 0 ?
				(int)LinkedTo->PathDirections[0] << 13 :
				(unsigned short)(((Math::atan2(double(nCoord_diff.Y), double(nCoord_diff.X))) - Math::DEG90_AS_RAD) * Math::BINARY_ANGLE_MAGIC_VALUE);

			const auto nFace_Value = (double)(nFaceRaw - 0x3FFF) * -0.00009587672516830327;
			const auto nSin = std::sinf(float(nFace_Value));
			const auto nCos = std::cosf(float(nFace_Value));
			this->CurrentSpeed = 0.0;
			this->AccelerationDurationNegSinus = 0.0;
			this->AccelerationDurationCosinus = 0.0;
			this->AccelerationDuration = 0;
			this->Delta.X = nSin * Characteristic.Intentional_DriftVelocity;
			this->Delta.Y = -(nCos * Characteristic.Intentional_DriftVelocity);
			this->CurrentVelocity = Characteristic.Intentional_DriftVelocity;
			this->State = 7;
			return;
		}
		else
		{
			LinkedTo->Destination = nullptr;
			this->CurrentSpeed = Characteristic.Drag;
			this->State = 2;
			return;
		}
	}

	const auto nCoordHere = LinkedTo->GetRenderCoords();
	JumpTo4(this, (float)GetFacingVal(nCoordHere, nCoordCell));

	if ((int)Math::abs(this->Delta.Y) > Math::abs(nCoordCell.X - nCoordHere.X))
		this->Delta.X = (double)(nCoordCell.X - nCoordHere.X);
	if ((int)Math::abs(this->Delta.Y) > Math::abs(nCoordCell.Y - nCoordHere.Y))
		this->Delta.Y = (double)(nCoordCell.Y - nCoordHere.Y);

	this->State = 6;
}

bool LevitateLocomotionClass::IsDestValid()
{
	//GameDebugLog::Log(__FUNCTION__" Called !  ");
	if (const auto pNavT = flag_cast_to<TechnoClass*>(LinkedTo->Destination))
	{
		if (pNavT->IsAlive && pNavT->IsOnMap)
		{ return true; }
		LinkedTo->Destination = nullptr;
	}

	return false;
}

bool LevitateLocomotionClass::IsTargetValid()
{
	//GameDebugLog::Log(__FUNCTION__" Called !  ");
	if (const auto pNavT = flag_cast_to<TechnoClass*>(LinkedTo->Target))
	{
		if (pNavT->IsAlive && pNavT->IsOnMap)
		{ return true; }
		LinkedTo->SetTarget(nullptr);
	}
	return false;
}

void LevitateLocomotionClass::ProcessSomething()
{
	//GameDebugLog::Log(__FUNCTION__" Called !  ");
	const auto v39 = this->CurrentVelocity - this->CurrentSpeed;
	if (v39 > 0.0)
	{
		if (this->CurrentVelocity > 0.0)
		{
			const auto v2 = v39 / this->CurrentVelocity;
			this->Delta.X = v2 * this->Delta.X;
			this->Delta.Y = v2 * this->Delta.Y;
		}
	}
	else
	{
		this->Delta.Y = 0.0;
		this->Delta.X = 0.0;
	}

	if (this->AccelerationDuration > 0)
	{
		--this->AccelerationDuration;
		this->Delta.X = this->AccelerationDurationCosinus + this->Delta.X;
		this->Delta.Y = this->AccelerationDurationNegSinus + this->Delta.Y;
	}

	this->CurrentVelocity = Math::sqrt(this->Delta.Y * this->Delta.Y + this->Delta.X * this->Delta.X);

	const CoordStruct nDelta = { (int)this->Delta.X , (int)this->Delta.Y , 0 };
	const CoordStruct nRender = LinkedTo->GetCenterCoords();
	const CoordStruct nDeltaCoord = nRender + nDelta;
	const CellStruct nDeltaCell = CellClass::Coord2Cell(nDelta);
	const CellStruct nRenderCell = CellClass::Coord2Cell(nDeltaCoord);

	if (nDeltaCell == nRenderCell)
	{
		const auto v34 = LinkedTo->IsSelected;
		LinkedTo->IsSelected = false;
		LinkedTo->SetLocation(nDeltaCoord);
		LinkedTo->IsSelected = v34;
		return;
	}

	CoordStruct nDeltaMod {};
	if (!this->IsAdjentCellEligible(nDeltaCoord))
	{
		if (nDeltaCoord.X <= 0)
		{
			if (nDeltaCoord.X < 0)
				nDeltaMod.X = nDeltaCoord.X - 1;
		}
		else
		{
			nDeltaMod.X = nDeltaCoord.X + 1;
		}

		if (nDeltaCoord.Y <= 0)
		{
			nDeltaMod.Y = nDeltaCoord.Y - 1;
		}
		else
		{
			nDeltaMod.Y = nDeltaCoord.Y + 1;
		}
	}
	else
	{
		nDeltaMod = nDeltaCoord;
	}

	if (this->IsAdjentCellEligible(nDeltaMod))
	{
		const bool isSelected = LinkedTo->IsSelected;
		if (isSelected)
			LinkedTo->UpdatePlacement(PlacementType::Remove);

		LinkedTo->IsSelected = false;
		LinkedTo->SetLocation(nDeltaMod);
		LinkedTo->IsSelected = isSelected;
		this->BlocksCounter = Characteristic.BlockCount_Max;
		const auto pCell = MapClass::Instance->GetCellAt(nDeltaMod);

		if (!LinkedTo->OnBridge
			&& pCell->ContainsBridge()
			&& LinkedTo->GetHeight() >= Unsorted::LevelHeight)
		{
			LinkedTo->OnBridge = true;
		}

		if (LinkedTo->OnBridge
			&& !pCell->ContainsBridge())
		{
			LinkedTo->OnBridge = false;
		}

		if (isSelected)
			LinkedTo->UpdatePlacement(PlacementType::Put);

		if (this->State == 7)
		{
			this->CurrentSpeed = Characteristic.Drag;
			this->State = 2;
		}

		return;
	}

	if (--this->BlocksCounter <= 0)
	{
		if (!this->IsDestValid())
			LinkedTo->Destination = nullptr;

		if (!this->IsTargetValid())
			LinkedTo->SetTarget(nullptr);
	}

	auto const nRender_Coord = LinkedTo->GetCenterCoords();
	auto const nRender_coord_trans =
		CoordStruct { ((nRender_Coord.X / 256) << 8) + 128 , ((nRender_Coord.Y / 256) << 8) + 128 , nRender_Coord.Z };

	JumpTo4(this, (float)GetFacingVal(nRender_Coord, nRender_coord_trans));

	if ((int)Math::abs(this->Delta.X) > Math::abs(nRender_coord_trans.X - nRender_Coord.X))
		this->Delta.X = (double)(nRender_coord_trans.X - nRender_Coord.X);
	if ((int)Math::abs(this->Delta.Y) > Math::abs(nRender_coord_trans.Y - nRender_Coord.Y))
		this->Delta.Y = (double)(nRender_coord_trans.Y - nRender_Coord.Y);

	this->State = 6;
}

bool LevitateLocomotionClass::IsAdjentCellEligible(CoordStruct nArgsCoord)
{
	//GameDebugLog::Log(__FUNCTION__" Called !  ");
	const auto nCoord = LinkedTo->GetCenterCoords();
	const auto nCoordCell = MapClass::Instance->GetCellAt(nCoord);
	const auto pCellInput = MapClass::Instance->GetCellAt(nArgsCoord);

	int i = 8;
	for (; i > 0 && nCoordCell->GetAdjacentCell((FacingType)i) != pCellInput; --i);

	const auto CanEnter = LinkedTo->IsCellOccupied(pCellInput, (FacingType)i, LinkedTo->GetCellLevel(), nullptr, true);

	if (CanEnter == Move::OK)
		return true;

	const auto nRes_ = (int)CanEnter - 2;

	if (!nRes_)
		return true;

	if (nRes_ != 3)
		return false;

	bool bObjectIsFoot = true;
	const auto nZDis = nCoord.Z - nArgsCoord.Z;
	const auto nCoord_plus = (nZDis * nZDis) - nZDis;

	ObjectClass* pObje = nullptr;
	if (nCoord_plus >= 512 && ((pCellInput->Flags & CellFlags::BridgeDir) != CellFlags::Empty))
	{
		pObje = pCellInput->AltObject;
	}
	else
	{
		pObje = pCellInput->FirstObject;
	}

	auto pObj = pObje;

	do
	{
		if (!pObj)
			break;

		if (pObj->IsAlive && pObj->IsOnMap)
			bObjectIsFoot = (pObj->AbstractFlags & AbstractFlags::Foot) != AbstractFlags::None;

		pObj = pObj->NextObject;
	}
	while (bObjectIsFoot);

	return bObjectIsFoot;
}

bool __stdcall LevitateLocomotionClass::Process()
{
	//GameDebugLog::Log(__FUNCTION__" Called !  ");
	switch (this->State)
	{
	case 0u:
		this->DoPhase1();
		break;
	case 1u:
		this->DoPhase2(); //done
		break;
	case 2u:
		this->DoPhase3(); //done
		break;
	case 3u:
		this->DoPhase4(); //done
		break;
	case 4u: //done
	{
		if (this->IsTargetValid())
			this->DoPhase5(LinkedTo->Target->GetCoords());
		else if (this->IsDestValid())
			this->DoPhase5(LinkedTo->Destination->GetCoords());
		else
		{
			CurrentSpeed = Characteristic.Drag;
			State = 2;
		}
	}
	break;
	case 5u:
		this->DoPhase6(); //done
		break;
	case 6u:
		this->DoPhase7(); //done
		break;
	default:
		break;
	}

	//Done
	this->ProcessSomething();

	//Done
	if (this->Is_Moving_Now())
	{
		if (!(Unsorted::CurrentFrame % 10))
		{
			if (!LinkedTo->IsOnBridge() && LinkedTo->GetCell()->LandType == LandType::Water)
			{
				if (auto pAnimType = RulesClass::Instance->Wake)
				{
					auto nCoord = LinkedTo->GetCenterCoords();
					GameCreate<AnimClass>(pAnimType, nCoord);
				}
			}
		}
	}

	this->ProcessHovering(); //Done

	return this->Is_Moving(); //?
}