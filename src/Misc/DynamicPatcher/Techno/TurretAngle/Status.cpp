#include "Status.h"
#include "Data.h"

#include <TechnoClass.h>
#include <Misc/DynamicPatcher/Helpers/Helpers.h>

void TurretAngle::OnPut(TechnoClass* pTechno, CoordStruct* pCoord, DirType dirType)
{

}

void TurretAngle::OnPut(CoordStruct* pCoord, DirType dirType)
{
	const auto what = OwnerObject->WhatAmI();

	if ((what == UnitClass::AbsID || what == BuildingClass::AbsID) && Data->Enable && hasTurret)
	{
		DirStruct bodyDir = OwnerObject->PrimaryFacing.Current();

		LockTurretDir = bodyDir;

		if (DefaultAngleIsChange(bodyDir)) {
			ChangeDefaultDir = true;
			OwnerObject->SecondaryFacing.Set_Current(LockTurretDir);
		}
	}
	else
	{
		ChangeDefaultDir = false;
	}
}

void TurretAngle::OnUpdate()
{
	const auto what = OwnerObject->WhatAmI();

	if ((what == UnitClass::AbsID || what == BuildingClass::AbsID) && Data->Enable && hasTurret)
	{
		CoordStruct sourcePos = OwnerObject->GetCoords();
		DirStruct bodyDir = OwnerObject->PrimaryFacing.Current();
		LockTurretDir = bodyDir;
		int bodyDirIndex = Helpers_DP::Dir2FacingIndex(bodyDir, 180) * 2;

		auto const& [ChangeDefaultDir, newDefaultDir] = TryGetDefaultAngle(bodyDirIndex);

		if (ChangeDefaultDir)
		{
			LockTurretDir = newDefaultDir;
		}

		bodyDir = LockTurretDir;

		auto pTarget = OwnerObject->Target;
		bool hasTarget = pTarget;

		if (!hasTarget)
		{
			pTarget = static_cast<FootClass*>(OwnerObject)->Destination;
		}

		if (pTarget)
		{

			DirStruct targetDir = OwnerObject->GetDirectionOverObject(pTarget);

			int targetDirIndex = Helpers_DP::Dir2FacingIndex(targetDir, 180) * 2;

			int bodyTargetDelta = IncludedAngle360(bodyDirIndex, targetDirIndex);

			bool isCloseEnough = OwnerObject->IsCloseEnoughToAttack(pTarget);

			if (hasTarget && isCloseEnough && Data->AutoTurn)
			{
				TryTurnBodyToAngle(targetDir, bodyDirIndex, bodyTargetDelta);
			}

			if (Data->AngleLimit)
			{
				int min = Data->Angle.X;
				int max = Data->Angle.Y;

				if (InDeadZone(bodyTargetDelta, min, max))
				{
					DeathZoneAction action = Data->Action;

					if ((!hasTarget || !isCloseEnough) && ChangeDefaultDir)
					{
						action = DeathZoneAction::BLOCK;
					}

					switch (action)
					{
					case DeathZoneAction::BLOCK:
						BlockTurretFacing(bodyDir, bodyDirIndex, min, max, bodyTargetDelta);
						break;
					case DeathZoneAction::TURN:
						BlockTurretFacing(bodyDir, bodyDirIndex, min, max, bodyTargetDelta);
						if (isCloseEnough)
						{
							// 转动车身朝向目标
							TryTurnBodyToAngle(targetDir, bodyDirIndex, bodyTargetDelta);
						}
						break;
					default:
						OwnerObject->ClearAllTarget();
						break;
					}
				}
				else
				{
					int range = max - min;
					if (range > 0 && range <= 180)
					{
						LockTurret = ForceTurretToForward(bodyDir, bodyDirIndex, min, max, bodyTargetDelta);
					}
					else
					{
						LockTurret = false;
					}
				}
			}
		}
	}
	else
	{
		LockTurret = false;
	}
}

bool TurretAngle::DefaultAngleIsChange(DirStruct bodyDir)
{
	int bodyDirIndex = Helpers_DP::Dir2FacingIndex(bodyDir ,180) * 2;
	auto const [changeDefaultDir, newDefaultDir] = TryGetDefaultAngle(bodyDirIndex);

	if (changeDefaultDir) {
		LockTurretDir = newDefaultDir;
	}

	return changeDefaultDir;
}

std::pair<bool, DirStruct> TurretAngle::TryGetDefaultAngle(int& bodyDirIndex)
{
	if (Data->DefaultAngle > 0)
	{
		bodyDirIndex += Data->DefaultAngle;
		if (bodyDirIndex > 360) {
			bodyDirIndex -= 360;
		}

		return { true , Helpers_DP::DirNormalized(bodyDirIndex, 360) };
	}

	return { false , DirStruct() };
}

bool TurretAngle::InDeadZone(int bodyTargetDelta, int min, int max)
{
	return bodyTargetDelta > min && bodyTargetDelta < max;
}

void TurretAngle::BlockTurretFacing(const DirStruct& bodyDir, int bodyDirIndex, int min, int max, int bodyTargetDelta)
{
	int targetAngle = TurretAngleData::GetTurnAngle(bodyTargetDelta, min, max) + bodyDirIndex;

	if (targetAngle > 360) {
		targetAngle -= 360;
	}

	LockTurretDir = Helpers_DP::DirNormalized(targetAngle, 360);
	LockTurret = true;
	int angle = IncludedAngle360(bodyDirIndex, targetAngle);

	if (max - min <= 180) {
		ForceTurretToForward(bodyDir, bodyDirIndex, min, max, angle);
	}
}

bool TurretAngle::ForceTurretToForward(const DirStruct& bodyDir, int bodyDirIndex, int min, int max, int bodyTargetDelta)
{
	DirStruct turretDir = OwnerObject->SecondaryFacing.Current();
	int turretDirIndex = Helpers_DP::Dir2FacingIndex(turretDir, 180) * 2;
	int turretAngle = IncludedAngle360(bodyDirIndex, turretDirIndex);

	if (turretAngle > 180)
	{
		if (InDeadZone(turretAngle, min, max))
		{
			int turnAngle = TurretAngleData::GetTurnAngle(turretAngle, min, max) + bodyDirIndex;
			if (turnAngle > 360) {
				turnAngle -= 360;
			}

			OwnerObject->SecondaryFacing.Set_Current(turretDir);
			LockTurretDir = Helpers_DP::DirNormalized(turnAngle, 360);
			return true;
		}
		else if (bodyTargetDelta < 180)
		{
			TurnToRight(turretAngle, bodyDirIndex, bodyDir);
			return true;
		}
	}
	else if (turretAngle > 0)
	{
		if (InDeadZone(turretAngle, min, max))
		{
			int turnAngle = TurretAngleData::GetTurnAngle(turretAngle, min, max) + bodyDirIndex;
			if (turnAngle > 360)
			{
				turnAngle -= 360;
			}
			OwnerObject->SecondaryFacing.Set_Current(turretDir);
			LockTurretDir = Helpers_DP::DirNormalized(turnAngle, 360);
			return true;
		}
		else if (bodyTargetDelta > 180)
		{
			TurnToLeft(turretAngle, bodyDirIndex, bodyDir);
			return true;
		}
	}

	return false;
}

void TurretAngle::TurnToLeft(int turretAngle, int bodyDirIndex, const DirStruct& bodyDir)
{
	int turnAngle = 0;

	if (turretAngle > 90)
	{
		turnAngle = turretAngle - 90 + bodyDirIndex;

		if (turnAngle > 360) {
			turnAngle -= 360;
		}

		LockTurretDir = Helpers_DP::DirNormalized(turnAngle, 360);
	}
	else
	{
		LockTurretDir = bodyDir;
	}
}

void TurretAngle::TurnToRight(int turretAngle, int bodyDirIndex, const DirStruct& bodyDir)
{
	int turnAngle = 0;

	if (360 - turretAngle > 90)
	{
		turnAngle = turretAngle + 90 + bodyDirIndex;

		if (turnAngle > 360) {
			turnAngle -= 360;
		}

		LockTurretDir = Helpers_DP::DirNormalized(turnAngle, 360);
	}
	else
	{
		LockTurretDir = bodyDir;
	}
}

bool TurretAngle::TryTurnBodyToAngle(const DirStruct& targetDir, int bodyDirIndex, int bodyTargetDelta)
{
	const auto pFoot = static_cast<FootClass*>(OwnerObject);
	const auto what = OwnerObject->WhatAmI();
	const bool isMoving = what == BuildingClass::AbsID ? false : pFoot->Locomotor->Is_Moving() && pFoot->GetCurrentSpeed() > 0;

	if (!isMoving && !OwnerObject->PrimaryFacing.Is_Rotating())
	{
		DirStruct turnDir = targetDir;

		if (Data->AutoTurn)
		{
			const Point2D& angleZone = bodyTargetDelta >= 180 ? Data->SideboardAngleL : Data->SideboardAngleR;

			if (bodyTargetDelta < angleZone.X || bodyTargetDelta > angleZone.Y)
			{
				int turnAngle = TurretAngleData::GetTurnAngle(bodyTargetDelta, angleZone.X , angleZone.Y);
				int turnDelta = 0;
				if (turnAngle < bodyTargetDelta)
				{
					turnDelta = bodyTargetDelta - turnAngle;
				}
				else if (turnAngle > bodyTargetDelta)
				{
					turnDelta = 360 - (turnAngle - bodyTargetDelta);
				}

				turnDelta += bodyDirIndex;

				if (turnDelta > 360)
				{
					turnDelta -= 360;
				}

				if (turnDelta > 0)
				{
					turnDir = Helpers_DP::DirNormalized(turnDelta, 360);
				}

				OwnerObject->PrimaryFacing.Set_Desired(turnDir);
			}
		}
		else
		{
			OwnerObject->PrimaryFacing.Set_Desired(turnDir);
		}
		return true;
	}
	return false;
}

int TurretAngle::IncludedAngle360(int bodyDirIndex, int targetDirIndex)
{
	int delta = 0;
	if (bodyDirIndex > 180 && targetDirIndex < bodyDirIndex)
	{
		delta = 360 - bodyDirIndex + targetDirIndex;
	}
	else
	{
		delta = targetDirIndex - bodyDirIndex;
	}
	if (delta < 0)
	{
		delta = 360 + delta;
	}
	return delta;
}