#pragma once

#include <TechnoClass.h>
#include <BulletClass.h>
#include <AircraftClass.h>

#include <Utilities/Constructs.h>
#include <Utilities/LocationMark.h>
#include "RadialFire.h"
#include "EffectHelpers.h"
//#include <Misc/Otamaa/Delegates.h>

#include <unordered_set>

class VoxelAnimClass;

//typedef bool(__stdcall* FireBulletToTarget)(int index, int burst, BulletClass* pBullet, AbstractClass* pTarget);
struct ArcingVelocityData
{
	double m_StraightDistance;
	double m_RealSpeed;
	CellClass* m_TargetCell;
};

struct ILocomotion;
class FootClass;
struct Helpers_DP
{
private:
	NO_CONSTRUCT_CLASS(Helpers_DP)
public:

	static void ForceStopMoving(ILocomotion* loco);
	static void ForceStopMoving(FootClass* pFoot);
	static bool CanDamageMe(TechnoClass* pTechno, int damage, int distanceFromEpicenter, WarheadTypeClass* pWH, int& realDamage, bool effectsRequireDamage = false);
	static CoordStruct RandomOffset(double maxSpread, double minSpread = 0);
	static CoordStruct RandomOffset(int min, int max);
	static CoordStruct GetInaccurateOffset(float scatterMin, float scatterMax);
	static VelocityClass GetBulletArcingVelocity(const CoordStruct& sourcePos, CoordStruct& targetPos,
			double speed, int gravity, bool lobber, bool inaccurate, float scatterMin, float scatterMax,
			int zOffset , ArcingVelocityData& outData);

	static CoordStruct OneCellOffsetToTarget(CoordStruct& sourcePos, CoordStruct& targetPos);
	static int ColorAdd2RGB565(ColorStruct colorAdd);
	static int Dir2FacingIndex(DirStruct& dir, int facing);
	static int Dir2FrameIndex(DirStruct& dir, int facing);

	static double GetROFMult(TechnoClass const* pTech);
	static double GetDamageMult(TechnoClass* pTechno);

	static DirStruct DirNormalized(int index, int facing);

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

	static DirStruct Facing(BulletClass* pBullet, CoordStruct& location);
	static DirStruct Facing(VoxelAnimClass* pVoxelAnim, CoordStruct& location);
	static DirStruct Facing(AnimClass* pAnim, CoordStruct& location);
	static DirStruct Facing(BulletClass* pBullet);
	static DirStruct Facing(VoxelAnimClass* pVoxelAnim);
	static DirStruct Facing(AnimClass* pAnim);

	static CoordStruct GetFLHAbsoluteCoords(BulletClass* pBullet, CoordStruct& flh, int flipY = 1);
	static CoordStruct GetFLHAbsoluteCoords(AnimClass* pAnim, CoordStruct& flh, int flipY = 1);
	static CoordStruct GetFLHAbsoluteCoords(VoxelAnimClass* pVoxelAnim, CoordStruct& flh, int flipY = 1);
	static CoordStruct GetFLHAbsoluteCoords(TechnoClass* pTechno, CoordStruct& flh, bool isOnTurret = true, int flipY = 1, bool nextFrame = true);
	static CoordStruct GetFLHAbsoluteCoords(ObjectClass* pObject, CoordStruct& flh, bool isOnTurret = true, int flipY = 1);
	static CoordStruct GetFLH(CoordStruct& source, CoordStruct& flh, DirStruct& dir, bool flip = false);
	static Vector3D<float> GetForwardVector(TechnoClass* pTechno, bool getTurret = false);
	static Vector3D<float> ToVector3D(DirStruct& dir);
	static Matrix3D GetMatrix3D(TechnoClass* pTechno);
	static VelocityClass GetBulletVelocity(CoordStruct sourcePos, CoordStruct targetPos);
	static void RotateMatrix3D(Matrix3D& matrix3D, TechnoClass* pTechno, bool isOnTurret, bool nextFrame);
	static Vector3D<float> GetFLHOffset(Matrix3D& matrix3D, CoordStruct& flh);
	static CoordStruct GetFLHAbsoluteCoords(TechnoClass* pTechno, const CoordStruct& flh, bool isOnTurret, int flipY, CoordStruct& turretOffset, bool nextFrame);
	static DirStruct Point2Dir(CoordStruct& sourcePos, CoordStruct& targetPos);
	static Vector3D<float> GetFLHAbsoluteOffset(CoordStruct& flh, DirStruct& dir, const CoordStruct& turretOffset);
	static VelocityClass GetVelocityClass(CoordStruct sourcePos, CoordStruct targetPos);
	static CoordStruct GetFLHAbsoluteCoords(CoordStruct source, CoordStruct& flh, DirStruct& dir, const CoordStruct& turretOffset = CoordStruct::Empty);

	static void FireWeaponTo(TechnoClass* pShooter,
		TechnoClass* pAttacker,
		AbstractClass* pTarget,
		WeaponTypeClass* pWeapon,
		const CoordStruct& flh,
		const CoordStruct& bulletSourcePos = CoordStruct::Empty,
		bool radialFire = false, int splitAngle = 180);

	static void DrawBulletEffect(WeaponTypeClass* pWeapon, CoordStruct& sourcePos, CoordStruct& targetPos, TechnoClass* pAttacker, AbstractClass* pTarget);

	static void AttachedParticleSystem(WeaponTypeClass* pWeapon, CoordStruct& sourcePos, AbstractClass* pTarget, TechnoClass* pAttacker, CoordStruct& targetPos)
	{
		//ParticleSystem
		if (auto const psType = pWeapon->AttachedParticleSystem)
			EffectHelpers::DrawParticle(psType, sourcePos, pTarget, pAttacker, targetPos, pAttacker ? pAttacker->Owner : nullptr);
	}

	static void PlayReportSound(WeaponTypeClass* pWeapon, CoordStruct& sourcePos, TechnoClass* pTechno = nullptr);

	static void DrawWeaponAnim(WeaponTypeClass* pWeapon, CoordStruct& sourcePos, CoordStruct& targetPos ,TechnoClass* pOwner , AbstractClass* pTarget);
	static BulletClass* FireBulletTo(TechnoClass* pAttacker, AbstractClass* pTarget, WeaponTypeClass* pWeapon, CoordStruct& sourcePos, CoordStruct& targetPos, VelocityClass& VelocityClass);
	static BulletClass* FireBullet(TechnoClass* pAttacker, AbstractClass* pTarget, WeaponTypeClass* pWeapon, CoordStruct& sourcePos, CoordStruct& targetPos, VelocityClass& VelocityClass);

	static TechnoClass* CreateAndPutTechno(TechnoTypeClass* pType, HouseClass* pHouse, CoordStruct& location, CellClass* pCell = nullptr , bool bPathfinding = false);

	static std::vector<BulletClass*> GetCellSpreadBullets(CoordStruct& location, double spread)
	{
		std::vector<BulletClass*> pBulletSet { };

		double dist = (spread <= 0 ? 1 : std::ceil(spread)) * 256;

		auto const bullets = BulletClass::Array();

		for (int i = bullets->Count - 1; i >= 0; i--)
		{
			auto const pBullet = bullets->Items[i];
			if (pBullet->GetCoords().DistanceFrom(location) <= dist) {
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

	static std::map<Point2D, int> MakeTargetPad(const std::vector<int>& weights, int count, int& maxValue)
	{
		int weightCount = weights.empty() ? (int)weights.size() : 0;
		std::map<Point2D, int> targetPad {};
		maxValue = 0;
		// 将所有的概率加起来，获得上游指标
		for (int index = 0; index < count; index++)
		{
			Point2D target = Point2D::Empty;
			target.X = maxValue;
			int weight = 1;
			if (weightCount > 0 && index < weightCount)
			{
				int w = weights[index];
				if (w > 0)
				{
					weight = w;
				}
			}
			maxValue += weight;
			target.Y = maxValue;
			targetPad.emplace(target, index);
		}
		return targetPad;
	}

	static int Hit(const std::map<Point2D, int>& targetPad, int maxValue)
	{
		int index = 0;
		int p = ScenarioClass::Instance->Random.RandomFromMax(maxValue);

		for(const auto& [tKey, idx] : targetPad) {
			if (p >= tKey.X && p < tKey.Y)
			{
				index = idx;
				break;
			}
		}

		return index;
	}

	static bool Bingo(const ValueableVector<double>& chances, int index)
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
		return chance >= 1 || chance >= ScenarioClass::Instance->Random.RandomDouble();
	}

	static DirStruct GetDirectionRelative(TechnoClass* pMaster, int dir, bool isOnTurret);
	static std::optional<DirStruct> GetRelativeDir(ObjectClass* pOwner, int dir = 0, bool isOnTurret = false, bool isOnWorld = false);
	static LocationMark GetRelativeLocation(ObjectClass* pOwner, OffsetData data, CoordStruct offset = CoordStruct::Empty);
	static CoordStruct GetForwardCoords(Vector3D<int> const& sourceV, Vector3D<int> const& targetV, double speed, double dist = 0.0);
	static CoordStruct GetForwardCoords(Vector3D<float> const& sourceV, Vector3D<float> const& targetV, double speed, double dist = 0.0);
	static CoordStruct GetForwardCoords(CoordStruct sourcePos, CoordStruct targetPos, double speed, double dist = 0.0);
	static VelocityClass GetVelocity(BulletClass* pBullet);
	static VelocityClass GetVelocity(CoordStruct const& sourcePos, CoordStruct const& targetPos, int speed);
	static VelocityClass RecalculateVelocityClass(BulletClass* pBullet);
	static VelocityClass RecalculateVelocityClass(BulletClass* pBullet, CoordStruct const& targetPos);
	static VelocityClass RecalculateVelocityClass(BulletClass* pBullet, CoordStruct const& sourcePos, CoordStruct const& targetPos);
};
