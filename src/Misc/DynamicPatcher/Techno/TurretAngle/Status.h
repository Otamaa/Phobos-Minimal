#pragma once

#include <DirStruct.h>
#include <CoordStruct.h>

class TechnoClass;
struct TurretAngleData;
struct TurretAngle
{
	TechnoClass* OwnerObject;
	TurretAngleData* Data; //this were are dummy atm , better direcly access it from TechnoTypeExt
	DirStruct LockTurretDir;
	bool ChangeDefaultDir;
	bool LockTurret;
	bool hasTurret;

	static void OnPut(TechnoClass* pTechno, CoordStruct* pCoord, DirType dirType);

	void OnPut(CoordStruct* pCoord, DirType dirType);
	void OnUpdate();
	bool DefaultAngleIsChange(DirStruct bodyDir);
	std::pair<bool, DirStruct> TryGetDefaultAngle(int& bodyDirIndex);

private:
	bool InDeadZone(int bodyTargetDelta, int min, int max);
	void BlockTurretFacing(const DirStruct& bodyDir, int bodyDirIndex, int min, int max, int bodyTargetDelta);
	bool ForceTurretToForward(const DirStruct& bodyDir, int bodyDirIndex, int min, int max, int bodyTargetDelta);
	void TurnToLeft(int turretAngle, int bodyDirIndex, const DirStruct& bodyDir);
	void TurnToRight(int turretAngle, int bodyDirIndex, const DirStruct& bodyDir);
	bool TryTurnBodyToAngle(const DirStruct& targetDir, int bodyDirIndex, int bodyTargetDelta);
	int IncludedAngle360(int bodyDirIndex, int targetDirIndex);
};
