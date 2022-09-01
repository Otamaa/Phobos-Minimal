#pragma once
#ifdef COMPILE_PORTED_DP_FEATURES
#include <TechnoClass.h>
#include <BulletClass.h>
#include <Utilities/Constructs.h>
#include "RadialFire.h"
#include "EffectHelpers.h"
//#include <Misc/Otamaa/Delegates.h>

#include <unordered_set>
//typedef Delegate<BulletClass*> FoundBullet;
//typedef Delegate<TechnoClass*> FoundTechno;
//typedef Delegate<AircraftClass*> FoundAircraft;

typedef bool(__stdcall* FireBulletToTarget)(int index, int burst, BulletClass* pBullet, AbstractClass* pTarget);

struct Helpers_DP
{
private:
	NO_CONSTRUCT_CLASS(Helpers_DP)
public:


	static bool DamageMe(TechnoClass* pThis, int damage, int distanceFromEpicenter, WarheadTypeClass* warheadType, int& realDamage, bool effectsRequireDamage = false)
	{
		// 计算实际伤害
		if (damage > 0)
		{
			realDamage = MapClass::GetTotalDamage(damage, warheadType, pThis->GetType()->Armor, distanceFromEpicenter);
		}
		else
		{
			realDamage = -MapClass::GetTotalDamage(-damage, warheadType, pThis->GetType()->Armor, distanceFromEpicenter);
		}
		/*
		if (true)
		{
			if (damage == 0)
			{
				return warheadTypeExt.Ares.AllowZeroDamage;
			}
			else
			{
				if (warheadTypeExt.Ares.EffectsRequireVerses)
				{
					// 必须要可以造成伤害
					if (MapClass.GetTotalDamage(RulesClass.Global().MaxDamage, warheadTypeExt.OwnerObject, OwnerObject.Ref.Base.Type.Ref.Armor, 0) == 0)
					{
						// 弹头无法对该类型护甲造成伤害
						return false;
					}
					// 伤害非零，当EffectsRequireDamage=yes时，必须至少造成1点实际伤害
					if (effectsRequireDamage || warheadTypeExt.Ares.EffectsRequireDamage)
					{
						// Logger.Log("{0} 收到伤害 {1}, 弹头 {2}, 爆心距离{3}, 实际伤害{4}", OwnerObject.Ref.Type.Ref.Base.Base.ID, damage, warheadTypeExt.OwnerObject.Ref.Base.ID, distanceFromEpicenter, realDamage);
						return realDamage != 0;
					}
				}
			}
		}*/
		return true;
	}

	static CoordStruct OneCellOffsetToTarget(CoordStruct& sourcePos, CoordStruct& targetPos)
	{
		const double angle = Math::atan2(static_cast<double>(targetPos.Y - sourcePos.Y), static_cast<double>(targetPos.X - sourcePos.X));
		int y = static_cast<int>(256.0 * Math::tan(angle));
		int x = static_cast<int>(256.0 / Math::tan(angle));
		CoordStruct offset = CoordStruct::Empty;
		if (y == 0)
		{
			offset.Y = 0;
			if (angle < Math::Pi)
			{
				offset.X = 256;
			}
			else
			{
				offset.X = -256;
			}
		}
		else if (x == 0)
		{
			offset.X = 0;
			if (angle < 0)
			{
				offset.Y = -256;
			}
			else
			{
				offset.Y = 256;
			}
		}
		else
		{
			if (abs(x) <= 256)
			{
				offset.X = x;
				if (angle > 0)
				{
					offset.Y = 256;
				}
				else
				{
					offset.X = -offset.X;
					offset.Y = -256;
				}
			}
			else
			{
				offset.Y = y;
				if (abs(angle) < 0.5 * Math::Pi)
				{
					offset.X = 256;
				}
				else
				{
					offset.X = -256;
					offset.Y = -offset.Y;
				}
			}
		}
		return offset;
	}

	static int ColorAdd2RGB565(ColorStruct colorAdd)
	{
		return ((((colorAdd.R + 4)) / 255) << 5) +
			((((colorAdd.G + 2)) / 255) << 6) +
			((((colorAdd.B + 4)) / 255) << 5);
	}

	static int Dir2FacingIndex(DirStruct& dir, int facing)
	{
		size_t bits = static_cast<size_t>(std::round(std::sqrt(facing)));
		double face = static_cast<double>(dir.GetValue(bits));
		auto nDivider = static_cast<int>(bits);
		auto nDivider_shrOne = (1 << nDivider);
		double x = (face / nDivider_shrOne) * facing;
		return static_cast<int>(std::round(x));
	}

	static double GetROFMult(TechnoClass const * pTech)
	{
		bool rofAbility = false;
		if (pTech->Veterancy.IsElite())
			rofAbility = pTech->GetTechnoType()->VeteranAbilities.ROF || pTech->GetTechnoType()->EliteAbilities.ROF;
		else if (pTech->Veterancy.IsVeteran())
			rofAbility = pTech->GetTechnoType()->VeteranAbilities.ROF;

		return !rofAbility ? 1.0 :
			RulesClass::Instance->VeteranROF * ((!pTech->Owner || !pTech->Owner->Type) ?
				1.0 : pTech->Owner->Type->ROFMult);
	}

	static DirStruct DirNormalized(int index, int facing)
	{
		double radians = Math::deg2rad_Alternate((-360 / facing * index));
		return DirStruct(static_cast<short>(radians / Math::BINARY_ANGLE_MAGIC_ALTERNATE));
	}

	static DirStruct Radians2Dir(double radians)
	{
		return DirStruct(radians);
	}

	static bool IsDead(TechnoClass* pTechno)
	{
		return !pTechno || IsDead((ObjectClass*)pTechno) || pTechno->IsCrashing || pTechno->IsSinking;
	}

	static bool IsDead(ObjectClass* pObject)
	{
		return !pObject || pObject->Health <= 0 || !pObject->IsAlive;
	}

	static bool IsCloaked(TechnoClass* pTechno, bool includeCloaking = true)
	{
		return !pTechno ||
			pTechno->CloakState == CloakState::Cloaked ||
			!includeCloaking ||
			pTechno->CloakState == CloakState::Cloaking;
	}

	static bool IsInvisible(TechnoClass* pTechno)
	{
		return !pTechno || IsInvisible((ObjectClass*)pTechno);
	}

	static bool IsInvisible(ObjectClass* pObject)
	{
		return !pObject || pObject->InLimbo;
	}

	static bool IsDeadOrInvisible(TechnoClass* pTarget)
	{
		return IsDead(pTarget) || IsInvisible(pTarget);
	}

	static bool IsDeadOrInvisible(BulletClass* pBullet)
	{
		ObjectClass* pObject = (ObjectClass*)pBullet;
		return IsDead(pObject) || IsInvisible(pObject);
	}

	static bool IsDeadOrInvisibleOrCloaked(TechnoClass* pTarget)
	{
		return IsDeadOrInvisible(pTarget) || IsCloaked(pTarget);
	}

	static bool IsDeadOrStand(TechnoClass* pTarget)
	{
		// 检查死亡和发射者
		if (!pTarget || IsDeadOrInvisible(pTarget) || pTarget->IsImmobilized)
			return true;

		// 过滤掉替身 , ignore stand
		//TechnoExt targetExt = TechnoExt.ExtMap.Find(pTarget);
		//if (null == targetExt || !targetExt.MyMaster.IsNull)
		//	return true;

		return false;
	}

	static CoordStruct GetFLHAbsoluteCoords(TechnoClass* pTechno, CoordStruct& flh, bool isOnTurret = true, int flipY = 1, bool nextFrame = true);

	static CoordStruct GetFLH(CoordStruct& source, CoordStruct& flh, DirStruct& dir, bool flip = false)
	{
		if (flh)
		{
			double radians = dir.radians();

			double rF = flh.X;
			double xF = rF * Math::cos(-radians);
			double yF = rF * Math::sin(-radians);
			CoordStruct offsetF = { static_cast<int>(xF),static_cast<int>(yF), 0 };

			double rL = flip ? flh.Y : -flh.Y;
			double xL = rL * Math::sin(radians);
			double yL = rL * Math::cos(radians);
			CoordStruct offsetL = { static_cast<int>(xL), static_cast<int>(yL), 0 };

			CoordStruct nZFLHBuff { 0, 0, flh.Z };
			return source + offsetF + offsetL + nZFLHBuff;
		}

		return source;
	}

	static Vector3D<float> GetForwardVector(TechnoClass* pTechno, bool getTurret = false)
	{
		FacingStruct facing = getTurret ? pTechno->SecondaryFacing : pTechno->PrimaryFacing;
		auto nDir = facing.current();
		return ToVector3D(nDir);
	}

	static Vector3D<float> ToVector3D(DirStruct& dir)
	{
		double rad = -dir.radians();
		return { static_cast<float>(Math::cos(rad)), static_cast<float>(Math::sin(rad)), 0.0f };
	}

	static Matrix3D GetMatrix3D(TechnoClass* pTechno)
	{
		// Step 1: get body transform matrix

		if (auto const pFoot = abstract_cast<FootClass*>(pTechno)) {
			if (auto const pLoco = pFoot->Locomotor.get()) {
				return pLoco->Draw_Matrix(nullptr);
			}
		}

		Matrix3D matrix3D { };
		matrix3D.MakeIdentity();
		return matrix3D;
	}

	static const Matrix3D& RotateMatrix3D(const Matrix3D& matrix3D, TechnoClass* pTechno, bool isOnTurret, bool nextFrame)
	{
		if (isOnTurret)
		{
			if (pTechno->HasTurret())
			{
				DirStruct turretDir = nextFrame ? pTechno->SecondaryFacing.next() : pTechno->SecondaryFacing.current();

				if (pTechno->WhatAmI() == AbstractType::Building)
				{
					double turretRad = turretDir.radians();
					matrix3D.RotateZ(static_cast<float>(turretRad));
				}
				else
				{
					matrix3D.RotateZ(-matrix3D.GetZRotation());
					matrix3D.RotateZ(static_cast<float>(turretDir.radians()));
				}
			}
		}
		else if (nextFrame)
		{
			matrix3D.RotateZ(-matrix3D.GetZRotation());
			matrix3D.RotateZ(static_cast<float>(pTechno->PrimaryFacing.next().radians()));
		}
		return matrix3D;
	}

	static Vector3D<float> GetFLHOffset(const Matrix3D& matrix3D, CoordStruct& flh)
	{
		matrix3D.Translate(static_cast<float>(flh.X), static_cast<float>(flh.Y), static_cast<float>(flh.Z));
		Vector3D<float> result = Matrix3D::MatrixMultiply(matrix3D, Vector3D<float>{0.0f, 0.0f, 0.0f });
		result.Y *= -1;
		return result;
	}

	static CoordStruct GetFLHAbsoluteCoords(TechnoClass* pTechno, const CoordStruct& flh, bool isOnTurret, int flipY, CoordStruct& turretOffset, bool nextFrame)
	{
		if (!pTechno)
			return CoordStruct::Empty;

		auto const nCoord = pTechno->GetCoords();
		Vector3D<float> res = { static_cast<float>(nCoord.X), static_cast<float>(nCoord.Y), static_cast<float>(nCoord.Z) };

		CoordStruct sourceOffset = turretOffset;
		CoordStruct tempFLH = flh;

		if (nextFrame && pTechno->WhatAmI() != AbstractType::Building)
		{
			if (FootClass* pFoot = (FootClass*)pTechno)
			{
				CoordStruct nBuffer { 0,0,0 };
				int speed = 0;
				if (pFoot->Locomotor->Is_Moving() && (speed = pFoot->GetCurrentSpeed()) > 0)
				{
					nBuffer.X = speed;
					sourceOffset += nBuffer;
				}
			}
		}
		else
		{
			if (pTechno->WhatAmI() == AbstractType::Building)
			{
				tempFLH.Z += Unsorted::LevelHeight;
			}
		}

		if (flh)
		{
			const Matrix3D& matrix3D = GetMatrix3D(pTechno);
			matrix3D.Translate(static_cast<float>(turretOffset.X), static_cast<float>(turretOffset.Y), static_cast<float>(turretOffset.Z));
			RotateMatrix3D(matrix3D, pTechno, isOnTurret, nextFrame);
			tempFLH.Y *= flipY;
			Vector3D<float> offset = GetFLHOffset(matrix3D, tempFLH);
			// Step 5: offset techno location
			res += offset;
		}

		return { static_cast<int>(res.X), static_cast<int>(res.Y), static_cast<int>(res.Z) };
	}

	static DirStruct Point2Dir(CoordStruct& sourcePos, CoordStruct& targetPos)
	{
		// get angle
		double radians = Math::atan2(static_cast<double>(sourcePos.Y - targetPos.Y), static_cast<double>(targetPos.X - sourcePos.X));
		// Magic form tomsons26
		radians -= Math::deg2rad_Alternate(90);
		return DirStruct(static_cast<short>(radians / Math::BINARY_ANGLE_MAGIC_ALTERNATE));
	}

	static Vector3D<float> GetFLHAbsoluteOffset(CoordStruct& flh, DirStruct& dir, const CoordStruct& turretOffset)
	{
		if (flh)
		{
			Matrix3D matrix3D {  };
			matrix3D.MakeIdentity();
			matrix3D.Translate(static_cast<float>(turretOffset.X), static_cast<float>(turretOffset.Y), static_cast<float>(turretOffset.Z));
			matrix3D.RotateZ(static_cast<float>(dir.radians()));
			return GetFLHOffset(matrix3D, flh);
		}
		return Vector3D<float>::Empty;
	}

	static VelocityClass GetBulletVelocity(CoordStruct sourcePos, CoordStruct targetPos)
	{
		CoordStruct bulletFLH { 1, 0, 0 };
		DirStruct bulletDir = Point2Dir(sourcePos, targetPos);
		Vector3D<float> bulletV = GetFLHAbsoluteOffset(bulletFLH, bulletDir, CoordStruct::Empty);
		return { static_cast<double>(bulletV.X) , static_cast<double>(bulletV.Y) , static_cast<double>(bulletV.Z) };
	}

	static CoordStruct GetFLHAbsoluteCoords(CoordStruct source, CoordStruct& flh, DirStruct& dir, const CoordStruct& turretOffset = CoordStruct::Empty)
	{
		if (flh)
		{
			Vector3D<float> offset = GetFLHAbsoluteOffset(flh, dir, turretOffset);
			source.X += static_cast<int>(offset.X);
			source.Y += static_cast<int>(offset.Y);
			source.Z += static_cast<int>(offset.Z);
		}

		return source;
	}

	/*
	static void FindBulletTargetHouse(TechnoClass* pTechno, FoundBullet& func, bool allied = true)
	{
		func.BindLambda([pTechno, &allied]()
		{
			auto bullets = *BulletClass::Array();
			for (int i = bullets.Count - 1; i >= 0; i--)
			{
				auto const pBullet = bullets.GetItem(i);
				if (IsDeadOrInvisible(pBullet)
					|| pBullet->Type->Inviso
					|| !pBullet->Owner || pBullet->Owner->Owner == pTechno->Owner
					|| (allied && pBullet->Owner->Owner->IsAlliedWith(pTechno->Owner)))
				{
					continue;
				}
				else
					return pBullet;
			}

			return (BulletClass*)nullptr;
		});
	}

	static void FindBulletTargetMe(TechnoClass* pTechno, FoundBullet& func, bool allied = true)
	{
		FindBulletTargetHouse(pTechno, func, allied);
		if (auto const pBullet = func.Execute())
		{
			func.BindLambda([pBullet, pTechno, &allied]()
			{
				if (pBullet->Target == pTechno)
					return pBullet;
				else
					return (BulletClass*)nullptr;
			});
		}
	}

	static void FindTechno(HouseClass* pHouse, FoundTechno& func, bool owner = true, bool allied = false, bool enemies = false, bool civilian = false)
	{
		func.BindLambda([pHouse, &owner, &allied, &enemies, &civilian]()
		{
			auto technos = *TechnoClass::Array();
			for (int i = technos.Count - 1; i >= 0; i--)
			{
				auto pTechno = technos.GetItem(i);
				if (IsDeadOrInvisible(pTechno)
					|| !pTechno->Owner
					|| (pTechno->Owner == pHouse ? !owner : (pTechno->Owner->IsAlliedWith(pHouse) ? !allied : !enemies)))
				{
					continue;
				}
				else
					return pTechno;
			}

			return (TechnoClass*) nullptr;
		});
	}

	static void FindOwnerTechno(HouseClass* pHouse, FoundTechno& func, bool allied = false, bool enemies = false)
	{
		FindTechno(pHouse, func, true, allied, enemies);
		func.Execute();
	}

	static void FindAircraft(HouseClass* pHouse, FoundAircraft& func, bool owner = true, bool allied = false, bool enemies = false, bool civilian = false)
	{
		func.BindLambda([pHouse, &owner, &allied, &enemies, &civilian]()
		{
			auto const aircrafts = *AircraftClass::Array();
			for (int i = aircrafts.Count - 1; i >= 0; i--)
			{
				auto pAircraft = aircrafts.GetItem(i);
				if (IsDeadOrInvisible(pAircraft)
					|| !pAircraft->Owner
					|| (pAircraft->Owner == pHouse ? !owner : (pAircraft->Owner->IsAlliedWith(pHouse) ? !allied : !enemies)))
				{
					continue;
				}
				else
					return pAircraft;
			}

			return (AircraftClass*) nullptr;
		});
	}*/

	static void FireWeaponTo(TechnoClass* pShooter,
		TechnoClass* pAttacker,
		AbstractClass* pTarget,
		WeaponTypeClass* pWeapon,
		const CoordStruct& flh,
		FireBulletToTarget callback = nullptr,
		const CoordStruct& bulletSourcePos = CoordStruct::Empty,
		bool radialFire = false, int splitAngle = 180);

	static double GetDamageMult(TechnoClass* pTechno)
	{
		if (!pTechno || !pTechno->IsAlive)
			return 1.0;

		bool firepower = false;
		if (pTechno->Veterancy.IsElite())
		{
			firepower = pTechno->GetTechnoType()->VeteranAbilities.FIREPOWER || pTechno->GetTechnoType()->EliteAbilities.FIREPOWER;
		}
		else if (pTechno->Veterancy.IsVeteran())
		{
			firepower = pTechno->GetTechnoType()->VeteranAbilities.FIREPOWER;
		}
		return (!firepower ? 1.0 : RulesClass::Instance->VeteranCombat) * pTechno->FirepowerMultiplier * ((!pTechno->Owner || !pTechno->Owner->Type) ? 1.0 : pTechno->Owner->Type->FirepowerMult);
	}

	static void DrawBulletEffect(WeaponTypeClass* pWeapon, CoordStruct& sourcePos, CoordStruct& targetPos, TechnoClass* pAttacker, AbstractClass* pTarget)
	{
		// IsLaser
		if (pWeapon->IsLaser)
		{
			LaserType laserType = LaserType(false);
			ColorStruct houseColor = ColorStruct::Empty;
			if (pWeapon->IsHouseColor && pAttacker)
				houseColor = pAttacker->Owner->LaserColor;

			laserType.InnerColor = pWeapon->LaserInnerColor;
			laserType.OuterColor = pWeapon->LaserOuterColor;
			laserType.OuterSpread = pWeapon->LaserOuterSpread;
			laserType.IsHouseColor = pWeapon->IsHouseColor;
			laserType.Duration = pWeapon->LaserDuration;
			/*
			WeaponTypeExt ext = WeaponTypeExt.ExtMap.Find(pWeapon);
			if (null != ext)
			{
				if (ext.LaserThickness > 0)
				{
					laserType.Thickness = ext.LaserThickness;
				}
				laserType.Fade = ext.LaserFade;
				laserType.IsSupported = ext.IsSupported;
			}*/

			EffectHelpers::DrawLine(sourcePos, targetPos, laserType, houseColor);
		}

		// IsRadBeam
		if (pWeapon->IsRadBeam)
		{
			RadBeamType radBeamType = RadBeamType::RadBeam;
			if (pWeapon->Warhead && pWeapon->Warhead->Temporal)
				radBeamType = RadBeamType::Temporal;

			BeamType beamType = BeamType(radBeamType);
			EffectHelpers::DrawBeam(sourcePos, targetPos, beamType , ColorStruct::Empty);
		}

		//IsElectricBolt
		if (pWeapon->IsElectricBolt)
		{
			if (pAttacker && pTarget)
			{
				EffectHelpers::DrawBolt(pAttacker, pTarget, pWeapon, sourcePos);
			}
			else
			{
				EffectHelpers::DrawBolt(sourcePos, targetPos, pWeapon->IsAlternateColor);
			}
		}
	}

	static void AttachedParticleSystem(WeaponTypeClass* pWeapon, CoordStruct& sourcePos, AbstractClass* pTarget, TechnoClass* pAttacker, CoordStruct& targetPos)
	{
		//ParticleSystem
		if (auto const psType = pWeapon->AttachedParticleSystem)
			EffectHelpers::DrawParticle(psType, sourcePos, pTarget, pAttacker, targetPos, nullptr);
	}

	static void PlayReportSound(WeaponTypeClass* pWeapon, CoordStruct& sourcePos)
	{
		if (pWeapon->Report.Count > 0) {
			const int soundIndex = pWeapon->Report.GetItem(ScenarioGlobal->Random.RandomFromMax(pWeapon->Report.Count - 1));
			if (soundIndex != -1) {
				VocClass::PlayAt(soundIndex, sourcePos, nullptr);
			}
		}
	}

	static void DrawWeaponAnim(WeaponTypeClass* pWeapon, CoordStruct& sourcePos, CoordStruct& targetPos ,TechnoClass* pOwner , AbstractClass* pTarget);
	static BulletClass* FireBulletTo(TechnoClass* pAttacker, AbstractClass* pTarget, WeaponTypeClass* pWeapon, CoordStruct& sourcePos, CoordStruct& targetPos, VelocityClass& bulletVelocity);
	static BulletClass* FireBullet(TechnoClass* pAttacker, AbstractClass* pTarget, WeaponTypeClass* pWeapon, CoordStruct& sourcePos, CoordStruct& targetPos, VelocityClass& bulletVelocity);

	static TechnoClass* CreateAndPutTechno(TechnoTypeClass* pType, HouseClass* pHouse, CoordStruct& location, CellClass* pCell = nullptr);

	static ValueableVector<BulletClass*> GetCellSpreadBullets(CoordStruct& location, double spread)
	{
		ValueableVector<BulletClass*> pBulletSet { };

		double dist = (spread <= 0 ? 1 : std::ceil(spread)) * 256;

		auto const bullets = BulletClass::Array();

		for (int i = bullets->Count - 1; i >= 0; i--)
		{
			if (auto const pBullet = bullets->GetItem(i))
			{
				CoordStruct targetLocation = pBullet->GetCoords();

				if (targetLocation.DistanceFrom(location) <= dist)
					if (!pBulletSet.Contains(pBullet))
						pBulletSet.push_back(pBullet);
			}
		}

		return pBulletSet;
	}

	static int CountAircraft(HouseClass* pHouse, Iterator<AircraftTypeClass*> padList)
	{
		//bool owner = true;
		//bool allied = false;
		//bool enemies = false;

		int count = std::count_if(AircraftClass::Array->begin(), AircraftClass::Array->end() , [pHouse,&padList](AircraftClass* const pAircraft){

			if (!IsDeadOrInvisible(pAircraft) && pAircraft->Owner && pAircraft->Owner == pHouse && pAircraft->Type->AirportBound)
				return padList.contains(pAircraft->Type);

			return false;
		});

		/*
		for (auto const& pAircraft : *AircraftClass::Array())
		{
			if (IsDeadOrInvisible(pAircraft)
				|| !pAircraft->Owner
				|| (pAircraft->Owner == pHouse ? !owner : (pAircraft->Owner->IsAlliedWith(pHouse) ? !allied : !enemies)))
			{ continue; }

			if (padList.contains(pAircraft->Type) && pAircraft->Type->AirportBound)
				count++;
		}*/

		return count;
	}

	static bool Bingo(ValueableVector<double>& chances, int index)
	{
		if (chances.empty() || chances.size() < (size_t)(index + 1)) {
			return true;
		}

		return Bingo(chances[index]);
	}

	static bool Bingo(double chance)
	{
		if (chance <= 0)
		{
			return false;
		}
		return chance >= 1 || chance >= ScenarioGlobal->Random.RandomDouble();
	}
};
#endif