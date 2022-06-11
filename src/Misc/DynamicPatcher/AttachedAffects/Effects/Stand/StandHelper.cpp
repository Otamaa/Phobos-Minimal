#include "StandHelpers.h"
#include "Stand.h"

#include "../../AttachEffectManager.h"

#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <ObjectClass.h>

/*
void FindAndDamageStand(CoordStruct location, int damage, ObjectClass* pAttacker,
		   WarheadTypeClass* pWH, bool affectsTiberium, HouseClass* pAttackingHouse)
{

	double spread = pWH->CellSpread * 256;

	DynamicVectorClass<DamageGroup> stands {};
	ExHelper.FindTechno(IntPtr.Zero, (pTechno) = >
	{
		if (!pTechno.Ref.Base.IsOnMap && !(pTechno.Ref.Base.IsIronCurtained() || pTechno.Ref.IsForceShilded)) // Stand always not on map.
		{
			TechnoExt ext = TechnoExt.ExtMap.Find(pTechno);
			if (null != ext && !ext.MyMaster.IsNull && null != ext.StandType)
			{
				// 检查距离
				CoordStruct targetPos = pTechno.Ref.Base.Base.GetCoords();
				double dist = targetPos.DistanceFrom(location);
				if (pTechno.Ref.Base.Base.WhatAmI() == AbstractType.Aircraft && pTechno.InAir(true))
				{
					dist *= 0.5;
				}
				if (dist <= spread)
				{
					// 找到一个最近的替身，检查替身是否可以受伤，以及弹头是否可以影响该替身
					if (!ext.StandType.Immune
						&& ext.AffectMe(pAttacker, pWH, pAttackingHouse, out WarheadTypeExt whExt) // 检查所属权限
						&& ext.DamageMe(damage, (int)dist, whExt, out int realDamage) // 检查护甲
					)
					{
						DamageGroup damageGroup;
						damageGroup.Target = pTechno;
						damageGroup.Distance = dist;
						stands.Add(damageGroup);
					}
				}
			}
		}
		return false;
	}, true, true, true, true);

	for(DamageGroup damageGroup : stands)
	{
		damageGroup.Target->ReceiveDamage(damage, (int)damageGroup.Distance, pWH, pAttacker, false, false, pAttackingHouse);
	}
}*/

void StandHelper::UpdateStandLocation(AttachEffectManager* manager, ObjectClass* pObject, Stand* stand, int& markIndex)
{
	if (!stand || !manager)
		return;

	if (!stand->TypeData)
		return;

	if (stand->TypeData->IsTrain)
	{
		double length = 0;
		LocationMark preMark;

		for (size_t j = markIndex; j < manager->LocationMarks.size(); j++)
		{
			markIndex = (int)j;
			LocationMark mark = manager->LocationMarks[j];

			if (!preMark)
			{
				preMark = mark;
				continue;
			}

			length += mark.Location.DistanceFrom(preMark.Location);
			preMark = mark;

			if ((int)length >= manager->LocationSpace)
			{
				break;
			}
		}

		if (preMark)
		{
			stand->UpdateLocation(preMark);
			return;
		}
	}

	LocationMark locationMark = GetLocation(pObject, stand->TypeData);
	stand->UpdateLocation(locationMark);
}

LocationMark StandHelper::GetLocation(ObjectClass* pObject,StandType* standType, bool createdInthespot)
{
	if (!standType)
		return LocationMark();

	CoordStruct sourcePos = pObject->Location;
	CoordStruct targetPos = sourcePos;
	DirStruct targetDir;

	if (standType->IsOnWorld.Get())
	{
		targetDir = DirStruct();
		targetPos = Helpers_DP::GetFLHAbsoluteCoords(sourcePos, standType->Offset, targetDir);
	}
	else
	{
		switch (pObject->WhatAmI())
		{
		case AbstractType::Unit:
		case AbstractType::Aircraft:
		case AbstractType::Building:
		case AbstractType::Infantry:
		{
			auto pTechno = (TechnoClass*)pObject;
			targetDir = GetDirection(pTechno, standType->Direction, standType->IsOnTurret);
			//toDo:
			targetPos = Helpers_DP::GetFLHAbsoluteCoords(pTechno, standType->Offset, standType->IsOnTurret);
		}
		break;
		case AbstractType::Bullet:
		{
			auto pBullet = (BulletClass*)pObject;
			CoordStruct vel { (int)pBullet->Velocity.X,(int)pBullet->Velocity.Y,(int)pBullet->Velocity.Z };
			sourcePos += vel;
			//toDo:
			targetDir = Helpers_DP::Point2Dir(sourcePos, pBullet->TargetCoords);
			targetPos = Helpers_DP::GetFLHAbsoluteCoords(sourcePos, standType->Offset, targetDir);
		}
		break;
		}
	}

	if (createdInthespot)
		GameDelete(standType);

	return LocationMark(targetPos, targetDir);
}

DirStruct StandHelper::GetDirection(TechnoClass* pMaster, int dir, bool isOnTurret)
{
	// turn offset
	//toDo:
	DirStruct targetDir = Helpers_DP::DirNormalized(dir, 16);

	if (pMaster->WhatAmI() != AbstractType::Building)
	{
		double targetRad = targetDir.radians();

		DirStruct sourceDir = pMaster->PrimaryFacing.current();
		if (!pMaster->IsSinking)
		{
			if (isOnTurret)
			{
				sourceDir = (pMaster->GetRealFacing()).current();
			}

			double sourceRad = sourceDir.radians();

			float angle = (float)(sourceRad - targetRad);
			targetDir = Helpers_DP::Radians2Dir(angle);
		}
	}

	return targetDir;
}
