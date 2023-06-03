#include "Helpers.h"

#include <Ext/Anim/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/WarheadType/Body.h>

#include <Locomotor/Cast.h>

#include <FootClass.h>

void EffectHelpers::DrawBolt(CoordStruct sourcePos, CoordStruct targetPos, WeaponTypeClass* pWeapon)
{
	const auto pTypeExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	if (pTypeExt->WeaponBolt_Data.isset())
		ElectricBoltClass::Create(sourcePos, targetPos,
		pTypeExt->WeaponBolt_Data.Get(), 0,
		pTypeExt->Bolt_ParticleSys.Get(RulesClass::Instance->DefaultSparkSystem), false);
	else
	{
		BoltType type {};
		type.IsAlternateColor = pWeapon->IsAlternateColor;

		if (pTypeExt->Bolt_Color1.isset())
			type.Color1 = pTypeExt->Bolt_Color1.Get();

		if (pTypeExt->Bolt_Color2.isset())
			type.Color2 = pTypeExt->Bolt_Color2.Get();

		if (pTypeExt->Bolt_Color2.isset())
			type.Color2 = pTypeExt->Bolt_Color2.Get();

		if (pTypeExt->Bolt_ParticleSys.isset())
			type.ParticleSystem = pTypeExt->Bolt_ParticleSys.Get(RulesClass::Instance->DefaultSparkSystem);

		type.Color1_disable = pTypeExt->Bolt_Disable1;
		type.Color2_disable = pTypeExt->Bolt_Disable2;
		type.Color3_disable = pTypeExt->Bolt_Disable3;

		EffectHelpers::DrawBolt(sourcePos, targetPos, type);
	}
}

void Helpers_DP::DrawBulletEffect(WeaponTypeClass* pWeapon, CoordStruct& sourcePos, CoordStruct& targetPos, TechnoClass* pAttacker, AbstractClass* pTarget)
{
	// IsLaser
	if (pWeapon->IsLaser)
	{
		LaserType laserType = LaserType(false);
		ColorStruct houseColor = ColorStruct::Empty;

		if (pWeapon->IsHouseColor && pAttacker && pAttacker->Owner)
			houseColor = pAttacker->Owner->LaserColor;

		laserType.InnerColor = pWeapon->LaserInnerColor;
		laserType.OuterColor = pWeapon->LaserOuterColor;
		laserType.OuterSpread = pWeapon->LaserOuterSpread;
		laserType.IsHouseColor = pWeapon->IsHouseColor; // house color will be 
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
		EffectHelpers::DrawBeam(sourcePos, targetPos, beamType, ColorStruct::Empty);
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
			EffectHelpers::DrawBolt(sourcePos, targetPos, pWeapon);
		}
	}
}

VelocityClass Helpers_DP::GetBulletVelocity(CoordStruct sourcePos, CoordStruct targetPos)
{
	CoordStruct bulletFLH { 1, 0, 0 };
	DirStruct bulletDir = Helpers_DP::Point2Dir(sourcePos, targetPos);
	const Vector3D<float> bulletV = Helpers_DP::GetFLHAbsoluteOffset(bulletFLH, bulletDir, CoordStruct::Empty);
	return { static_cast<double>(bulletV.X) , static_cast<double>(bulletV.Y) , static_cast<double>(bulletV.Z) };
}

CoordStruct Helpers_DP::GetForwardCoords(Vector3D<int> const& sourceV, Vector3D<int> const& targetV, double speed, double dist)
{
	if (dist <= 0)
	{
		dist = targetV.DistanceFrom(sourceV);
	}

	// 计算下一个坐标
	double d = speed / dist;
	double absX = std::abs(sourceV.X - targetV.X) * d;
	double x = sourceV.X;

	if (sourceV.X < targetV.X)
	{
		// Xa < Xb => Xa < Xc
		// Xc - Xa = absX
		x = absX + sourceV.X;
	}
	else if (sourceV.X > targetV.X)
	{
		// Xa > Xb => Xa > Xc
		// Xa - Xc = absX
		x = sourceV.X - absX;
	}

	double absY = std::abs(sourceV.Y - targetV.Y) * d;
	double y = sourceV.Y;
	if (sourceV.Y < targetV.Y)
	{
		y = absY + sourceV.Y;
	}
	else if (sourceV.Y > targetV.Y)
	{
		y = sourceV.Y - absY;
	}
	double absZ = std::abs(sourceV.Z - targetV.Z) * d;
	double z = sourceV.Z;
	if (sourceV.Z < targetV.Z)
	{
		z = absZ + sourceV.Z;
	}
	else if (sourceV.Z > targetV.Z)
	{
		z = sourceV.Z - absZ;
	}

	return { (int)x, (int)y, (int)z };
}

CoordStruct Helpers_DP::GetForwardCoords(CoordStruct sourcePos, CoordStruct targetPos, double speed, double dist)
{
	Vector3D<int> source { sourcePos.X  ,sourcePos.Y , sourcePos.Z };
	Vector3D<int> target { targetPos.X  ,targetPos.Y , targetPos.Z };
	return Helpers_DP::GetForwardCoords(source, target, speed, dist);
}

VelocityClass Helpers_DP::GetVelocity(BulletClass* pBullet)
{
	return Helpers_DP::GetVelocity(pBullet->SourceCoords, pBullet->TargetCoords, pBullet->Speed);
}

VelocityClass Helpers_DP::GetVelocity(CoordStruct const& sourcePos, CoordStruct const& targetPos, int speed)
{
	VelocityClass velocity { (double)(targetPos.X - sourcePos.X), (double)(targetPos.Y - sourcePos.Y), (double)(targetPos.Z - sourcePos.Z) };
	velocity *= speed / targetPos.DistanceFrom(sourcePos);
	return velocity;
}

VelocityClass Helpers_DP::RecalculateVelocityClass(BulletClass* pBullet)
{
	CoordStruct targetPos = pBullet->TargetCoords;

	if (pBullet->Target)
	{
		targetPos = pBullet->Target->GetCoords();
	}

	return Helpers_DP::RecalculateVelocityClass(pBullet, targetPos);
}

VelocityClass Helpers_DP::RecalculateVelocityClass(BulletClass* pBullet, CoordStruct const& targetPos)
{
	return Helpers_DP::RecalculateVelocityClass(pBullet, pBullet->GetCoords(), targetPos);
}

VelocityClass Helpers_DP::RecalculateVelocityClass(BulletClass* pBullet, CoordStruct const& sourcePos, CoordStruct const& targetPos)
{
	VelocityClass velocity { (double)(targetPos.X - sourcePos.X), (double)(targetPos.Y - sourcePos.Y), (double)(targetPos.Z - sourcePos.Z) };
	velocity *= pBullet->Speed / targetPos.DistanceFrom(sourcePos);
	pBullet->Velocity = velocity;
	pBullet->SourceCoords = sourcePos;
	pBullet->TargetCoords = targetPos;
	return velocity;
}

DirStruct Helpers_DP::Point2Dir(CoordStruct& sourcePos, CoordStruct& targetPos)
{
	// get angle
	double radians = std::atan2(static_cast<double>(sourcePos.Y - targetPos.Y), static_cast<double>(targetPos.X - sourcePos.X));
	// Magic form tomsons26
	radians -= Math::deg2rad_Alternate(90);
	return DirStruct(static_cast<short>(radians / Math::BINARY_ANGLE_MAGIC_ALTERNATE));
}

Vector3D<float> Helpers_DP::GetFLHAbsoluteOffset(CoordStruct& flh, DirStruct& dir, const CoordStruct& turretOffset)
{
	if (flh)
	{
		Matrix3D matrix3D {  };
		matrix3D.MakeIdentity();
		matrix3D.Translate(static_cast<float>(turretOffset.X), static_cast<float>(turretOffset.Y), static_cast<float>(turretOffset.Z));
		matrix3D.RotateZ(static_cast<float>(dir.GetRadian()));
		return Helpers_DP::GetFLHOffset(matrix3D, flh);
	}

	return Vector3D<float>::Empty;
}

VelocityClass Helpers_DP::GetVelocityClass(CoordStruct sourcePos, CoordStruct targetPos)
{
	CoordStruct bulletFLH { 1, 0, 0 };
	DirStruct bulletDir = Helpers_DP::Point2Dir(sourcePos, targetPos);
	const Vector3D<float> bulletV = Helpers_DP::GetFLHAbsoluteOffset(bulletFLH, bulletDir, CoordStruct::Empty);
	return { static_cast<double>(bulletV.X) , static_cast<double>(bulletV.Y) , static_cast<double>(bulletV.Z) };
}

CoordStruct Helpers_DP::GetFLHAbsoluteCoords(CoordStruct source, CoordStruct& flh, DirStruct& dir, const CoordStruct& turretOffset)
{
	if (flh)
	{
		Vector3D<float> offset = Helpers_DP::GetFLHAbsoluteOffset(flh, dir, turretOffset);
		source += { static_cast<int>(offset.X), static_cast<int>(offset.Y), static_cast<int>(offset.Z) };
		//source += { std::lround(offset.X), std::lround(offset.Y), std::lround(offset.Z) };
	}

	return source;
}

DirStruct Helpers_DP::Facing(BulletClass* pBullet, CoordStruct& location)
{
	CoordStruct velocity = CoordStruct { (int)pBullet->Velocity.X, (int)pBullet->Velocity.Y, (int)pBullet->Velocity.Z };
	CoordStruct forwardLocation = location + velocity;
	return Helpers_DP::Point2Dir(location, forwardLocation);
}

DirStruct Helpers_DP::Facing(VoxelAnimClass* pVoxelAnim, CoordStruct& location)
{
	auto const& pBounce = pVoxelAnim->Bounce;
	CoordStruct velocity = CoordStruct { (int)pBounce.Velocity.X, (int)pBounce.Velocity.Y, (int)pBounce.Velocity.Z };
	CoordStruct forwardLocation = location + velocity;
	return Helpers_DP::Point2Dir(location, forwardLocation);
}

DirStruct Helpers_DP::Facing(AnimClass* pAnim, CoordStruct& location)
{
	auto const& pBounce = pAnim->Bounce;
	CoordStruct velocity = CoordStruct { (int)pBounce.Velocity.X, (int)pBounce.Velocity.Y, (int)pBounce.Velocity.Z };
	CoordStruct forwardLocation = location + velocity;
	return Helpers_DP::Point2Dir(location, forwardLocation);
}

DirStruct Helpers_DP::Facing(BulletClass* pBullet)
{
	auto nLoc = pBullet->GetCoords();
	return Helpers_DP::Facing(pBullet, nLoc);
}

DirStruct Helpers_DP::Facing(VoxelAnimClass* pVoxelAnim)
{
	auto nLoc = pVoxelAnim->Bounce.GetCoords();
	return Helpers_DP::Facing(pVoxelAnim, nLoc);
}

DirStruct Helpers_DP::Facing(AnimClass* pAnim)
{
	auto nLoc = pAnim->Bounce.GetCoords();
	return Helpers_DP::Facing(pAnim, nLoc);
}

int Helpers_DP::ColorAdd2RGB565(ColorStruct colorAdd)
{
	return ((((colorAdd.R + 4)) / 255) << 5) +
		((((colorAdd.G + 2)) / 255) << 6) +
		((((colorAdd.B + 4)) / 255) << 5);
}

int Helpers_DP::Dir2FacingIndex(DirStruct& dir, int facing)
{
	size_t bits = static_cast<size_t>(std::round(std::sqrt(facing)));
	double face = static_cast<double>(dir.GetValue(bits));
	auto nDivider = static_cast<int>(bits);
	auto nDivider_shrOne = (1 << nDivider);
	double x = (face / nDivider_shrOne) * facing;
	return static_cast<int>(std::round(x));
}

int Helpers_DP::Dir2FrameIndex(DirStruct& dir, int facing)
{
	int index = Dir2FacingIndex(dir, facing);
	index = (int)(facing / 8) + index;
	if (index >= facing)
	{
		index -= facing;
	}

	return index;
}

double Helpers_DP::GetROFMult(TechnoClass const* pTech)
{
	return TechnoExt::GetROFMult(pTech);
}

double Helpers_DP::GetDamageMult(TechnoClass* pTechno)
{
	if (!pTechno || !pTechno->IsAlive)
		return 1.0;

	return TechnoExt::GetDamageMult(pTechno);
}

DirStruct Helpers_DP::DirNormalized(int index, int facing)
{
	double radians = Math::deg2rad_Alternate((double)(-360 / facing * index));
	return DirStruct(static_cast<short>(radians / Math::BINARY_ANGLE_MAGIC_ALTERNATE));
}

CoordStruct Helpers_DP::OneCellOffsetToTarget(CoordStruct& sourcePos, CoordStruct& targetPos)
{
	const double angle = std::atan2(static_cast<double>(targetPos.Y - sourcePos.Y), static_cast<double>(targetPos.X - sourcePos.X));
	int y = static_cast<int>(256.0 * std::tan(angle));
	int x = static_cast<int>(256.0 / std::tan(angle));

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
		if (std::abs(x) <= 256)
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
			if (std::abs(angle) < 0.5 * Math::Pi)
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

CoordStruct Helpers_DP::GetFLHAbsoluteCoords(BulletClass* pBullet, CoordStruct& flh, int flipY)
{
	CoordStruct location = pBullet->GetCoords();
	DirStruct bulletFacing = Facing(pBullet, location);

	CoordStruct tempFLH = flh;
	tempFLH.Y *= flipY;
	return GetFLHAbsoluteCoords(location, tempFLH, bulletFacing);
}

CoordStruct Helpers_DP::GetFLHAbsoluteCoords(AnimClass* pAnim, CoordStruct& flh, int flipY)
{
	CoordStruct location = pAnim->Bounce.GetCoords();
	DirStruct bulletFacing = Facing(pAnim, location);

	CoordStruct tempFLH = flh;
	tempFLH.Y *= flipY;
	return GetFLHAbsoluteCoords(location, tempFLH, bulletFacing);

}

CoordStruct Helpers_DP::GetFLHAbsoluteCoords(VoxelAnimClass* pVoxelAnim, CoordStruct& flh, int flipY)
{
	CoordStruct location = pVoxelAnim->Bounce.GetCoords();
	DirStruct bulletFacing = Facing(pVoxelAnim, location);

	CoordStruct tempFLH = flh;
	tempFLH.Y *= flipY;
	return GetFLHAbsoluteCoords(location, tempFLH, bulletFacing);
}

CoordStruct Helpers_DP::GetFLHAbsoluteCoords(TechnoClass* pTechno, const CoordStruct& flh, bool isOnTurret, int flipY, CoordStruct& turretOffset, bool nextFrame)
{
	if (!pTechno)
		return CoordStruct::Empty;

	auto const nCoord = pTechno->GetCoords();
	Vector3D<float> res = { static_cast<float>(nCoord.X), static_cast<float>(nCoord.Y), static_cast<float>(nCoord.Z) };

	CoordStruct sourceOffset = turretOffset;
	CoordStruct tempFLH = flh;

	if (nextFrame && !Is_Building(pTechno))
	{
		if (FootClass* pFoot = (FootClass*)pTechno)
		{
			CoordStruct nBuffer { 0,0,0 };
			int speed = 0;
			if (pFoot->Locomotor.get()->Is_Moving() && (speed = pFoot->GetCurrentSpeed()) > 0)
			{
				nBuffer.X = speed;
				sourceOffset += nBuffer;
			}
		}
	}
	else
	{
		if (Is_Building(pTechno))
		{
			tempFLH.Z += Unsorted::LevelHeight;
		}
	}

	if (flh)
	{
		Matrix3D matrix3D = GetMatrix3D(pTechno);
		matrix3D.Translate(static_cast<float>(turretOffset.X), static_cast<float>(turretOffset.Y), static_cast<float>(turretOffset.Z));
		RotateMatrix3D(matrix3D, pTechno, isOnTurret, nextFrame);
		tempFLH.Y *= flipY;
		Vector3D<float> offset = GetFLHOffset(matrix3D, tempFLH);
		// Step 5: offset techno location
		res += offset;
	}

	return { static_cast<int>(res.X), static_cast<int>(res.Y), static_cast<int>(res.Z) };
}

Vector3D<float> Helpers_DP::GetFLHOffset(Matrix3D& matrix3D, CoordStruct& flh)
{
	matrix3D.Translate(static_cast<float>(flh.X), static_cast<float>(flh.Y), static_cast<float>(flh.Z));
	Vector3D<float> result;
	Matrix3D::MatrixMultiply(result, &matrix3D, Vector3D<float>::Empty);
	result.Y *= -1;
	return result;
}

void Helpers_DP::RotateMatrix3D(Matrix3D& matrix3D, TechnoClass* pTechno, bool isOnTurret, bool nextFrame)
{
	if (isOnTurret)
	{
		if (pTechno->HasTurret())
		{
			DirStruct turretDir = nextFrame ? pTechno->SecondaryFacing.Next() : pTechno->SecondaryFacing.Current();

			if (Is_Building(pTechno))
			{
				double turretRad = turretDir.GetRadian();
				matrix3D.RotateZ(static_cast<float>(turretRad));
			}
			else
			{
				matrix3D.RotateZ(-matrix3D.GetZRotation());
				matrix3D.RotateZ(static_cast<float>(turretDir.GetRadian()));
			}
		}
	}
	else if (nextFrame)
	{
		matrix3D.RotateZ(-matrix3D.GetZRotation());
		matrix3D.RotateZ(static_cast<float>(pTechno->PrimaryFacing.Next().GetRadian()));
	}
}

Matrix3D Helpers_DP::GetMatrix3D(TechnoClass* pTechno)
{
	// Step 1: get body transform matrix
	Matrix3D matrix3D { };

	if (auto const pFoot = abstract_cast<FootClass*>(pTechno))
	{
		if (auto const pLoco = pFoot->Locomotor.get())
		{
			pLoco->Draw_Matrix(&matrix3D, nullptr);
			return matrix3D;
		}
	}

	matrix3D.MakeIdentity();
	return matrix3D;
}

Vector3D<float> Helpers_DP::ToVector3D(DirStruct& dir)
{
	double rad = -dir.GetRadian();
	return { static_cast<float>(std::cos(rad)), static_cast<float>(std::sin(rad)), 0.0f };
}

Vector3D<float> Helpers_DP::GetForwardVector(TechnoClass* pTechno, bool getTurret)
{
	if (getTurret)
	{
		FacingClass facing { pTechno->SecondaryFacing };
		auto nDir = facing.Current();
		return ToVector3D(nDir);

	}
	else
	{
		FacingClass facing { pTechno->PrimaryFacing };
		auto nDir = facing.Current();
		return ToVector3D(nDir);
	}
}

CoordStruct Helpers_DP::GetFLH(CoordStruct& source, CoordStruct& flh, DirStruct& dir, bool flip)
{
	if (flh)
	{
		double radians = dir.GetRadian();

		double rF = flh.X;
		double xF = rF * std::cos(-radians);
		double yF = rF * std::sin(-radians);
		CoordStruct offsetF = { static_cast<int>(xF),static_cast<int>(yF), 0 };

		double rL = flip ? flh.Y : -flh.Y;
		double xL = rL * std::sin(radians);
		double yL = rL * std::cos(radians);
		CoordStruct offsetL = { static_cast<int>(xL), static_cast<int>(yL), 0 };

		CoordStruct nZFLHBuff { 0, 0, flh.Z };
		return source + offsetF + offsetL + nZFLHBuff;
	}

	return source;
}

CoordStruct Helpers_DP::GetFLHAbsoluteCoords(ObjectClass* pObject, CoordStruct& flh, bool isOnTurret, int flipY)
{
	if (pObject->AbstractFlags & AbstractFlags::Techno)
	{
		return GetFLHAbsoluteCoords(static_cast<TechnoClass*>(pObject), flh, isOnTurret, flipY);
	}
	else
	{

		switch (GetVtableAddr(pObject))
		{
		case BulletClass::vtable:
			return GetFLHAbsoluteCoords(static_cast<BulletClass*>(pObject), flh, isOnTurret, flipY);
		case AnimClass::vtable:
			return  GetFLHAbsoluteCoords(static_cast<AnimClass*>(pObject), flh, isOnTurret, flipY);
		case VoxelAnimClass::vtable:
			return  GetFLHAbsoluteCoords(static_cast<VoxelAnimClass*>(pObject), flh, isOnTurret, flipY);
		}

	}

	return CoordStruct::Empty;
}

LocationMark Helpers_DP::GetRelativeLocation(ObjectClass* pOwner, OffsetData data, CoordStruct offset)
{
	if (!offset) {
		offset = data.Offset;
	}

	if (data.IsOnWorld)
	{
		DirStruct targetDir = DirStruct { 0 };
		CoordStruct targetPos = Helpers_DP::GetFLHAbsoluteCoords(pOwner->Location, offset, targetDir);
		return { targetDir , targetPos };
	}
	else
	{
		if (pOwner->AbstractFlags & AbstractFlags::Techno)
		{
			auto pTech = static_cast<TechnoClass*>(pOwner);
			DirStruct targetDir = Helpers_DP::GetDirectionRelative(pTech, data.Dir, data.IsOnturret);
			CoordStruct targetPos = Helpers_DP::GetFLHAbsoluteCoords(pTech, offset, data.IsOnturret);
			return { targetDir , targetPos };
		}
		else
		{
			if (Is_Bullet(pOwner))
			{
				auto pBullet = static_cast<BulletClass*>(pOwner);
				// 增加抛射体偏移值取下一帧所在实际位置
				CoordStruct velocity {(int)pBullet->Velocity.X , (int)pBullet->Velocity.Y , (int)pBullet->Velocity.Z };
				CoordStruct sourcePos = pOwner->Location + velocity;
				// 获取面向
				DirStruct targetDir = Helpers_DP::Point2Dir(sourcePos, pBullet->TargetCoords);
				CoordStruct targetPos = Helpers_DP::GetFLHAbsoluteCoords(sourcePos, offset, targetDir);
				return { targetDir , targetPos };
			}
		}
	}

	return {};
}

std::optional<DirStruct> Helpers_DP::GetRelativeDir(ObjectClass* pOwner, int dir, bool isOnTurret, bool isOnWorld)
{
	if (!isOnWorld)
	{
		// 绑定世界坐标，朝向固定北向
		return DirStruct {};
	}
	else
	{
		if (pOwner->AbstractFlags & AbstractFlags::Techno) {
			return Helpers_DP::GetDirectionRelative(static_cast<TechnoClass*>(pOwner),dir,isOnTurret);
		}

		switch (GetVtableAddr(pOwner))
		{
		case AnimClass::vtable:
		{
			//TODO
			break;
		}
		case VoxelAnimClass::vtable:
		{
			//TODO
			break;
		}
		case BulletClass::vtable:
		{
			// 增加抛射体偏移值取下一帧所在实际位置
			CoordStruct sourcePos = pOwner->Location;
			auto const pBullet = static_cast<BulletClass*>(pOwner);
			CoordStruct nvel { (int)pBullet->Velocity.X ,(int)pBullet->Velocity.Y , (int)pBullet->Velocity.Z };
			sourcePos += nvel;
			// 获取面向
			return Helpers_DP::Point2Dir(sourcePos, pBullet->TargetCoords);
		}
		}
	}

	return std::nullopt;
}

DirStruct Helpers_DP::GetDirectionRelative(TechnoClass* pMaster, int dir, bool isOnTurret)
{
	// turn offset
	DirStruct targetDir = Helpers_DP::DirNormalized(dir, 16);

	if (pMaster->AbstractFlags & AbstractFlags::Foot)
	{
		auto const pFoot = static_cast<FootClass*>(pMaster);
		double targetRad = targetDir.GetRadian();
		DirStruct sourceDir = pMaster->PrimaryFacing.Current();

		if (auto const pLoco = locomotion_cast<JumpjetLocomotionClass*>(pFoot->Locomotor)) {
			sourceDir = pLoco->Facing.Current();
		}

		if (isOnTurret || Is_Aircraft(pFoot))
		{
			sourceDir = pMaster->GetRealFacing();
		}

		double sourceRad = sourceDir.GetRadian();

		float angle = (float)(sourceRad - targetRad);
		targetDir = Helpers_DP::Radians2Dir(angle);
	}

	return targetDir;
}

void Helpers_DP::ForceStopMoving(ILocomotion* loco)
{
	loco->Stop_Moving();
	loco->Mark_All_Occupation_Bits(0);

	switch (VTable::Get(loco)) // :p - Otamaa
	{
	case DriveLocomotionClass::ILoco_vtable:
	{
		auto pLoco = static_cast<DriveLocomotionClass*>(loco);
		pLoco->Destination = CoordStruct::Empty;
		pLoco->HeadToCoord = CoordStruct::Empty;
		pLoco->IsDriving = false;
		break;
	}
	case ShipLocomotionClass::ILoco_vtable:
	{
		auto pLoco = static_cast<ShipLocomotionClass*>(loco);
		pLoco->Destination = CoordStruct::Empty;
		pLoco->HeadToCoord = CoordStruct::Empty;
		pLoco->IsDriving = false;
		break;
	}
	case WalkLocomotionClass::ILoco_vtable:
	{
		auto pLoco = static_cast<WalkLocomotionClass*>(loco);
		pLoco->MovingDestination = CoordStruct::Empty;
		pLoco->CoordHeadTo = CoordStruct::Empty;
		pLoco->IsMoving = false;
		pLoco->IsReallyMoving = false;
		break;
	}
	case MechLocomotionClass::ILoco_vtable:
	{
		auto pLoco = static_cast<MechLocomotionClass*>(loco);
		pLoco->MovingDestination = CoordStruct::Empty;
		pLoco->CoordHeadTo = CoordStruct::Empty;
		pLoco->IsMoving = false;
		break;
	}
	}
}

void Helpers_DP::ForceStopMoving(FootClass* pFoot)
{
	auto loco = pFoot->Locomotor.get();

	loco->Mark_All_Occupation_Bits(0);

	if (loco->Apparent_Speed() > 0)
	{
		pFoot->SetDestination(nullptr ,true);
		pFoot->Destination = nullptr;
		Helpers_DP::ForceStopMoving(loco);
	}
}

bool Helpers_DP::CanDamageMe(TechnoClass* pTechno, int damage, int distanceFromEpicenter, WarheadTypeClass* pWH, int& realDamage, bool effectsRequireDamage)
{
	// 计算实际伤害
	auto const Armor = pTechno->GetTechnoType()->Armor;
	realDamage = MapClass::Instance->GetTotalDamage(damage,pWH, Armor,distanceFromEpicenter);
	auto const data = WarheadTypeExt::ExtMap.Find(pWH);

	if (damage == 0)
	{
		return data->AllowZeroDamage;
	}
	else
	{
		if (data->EffectsRequireVerses)
		{
			if(std::abs(
				data->GetVerses(Armor).Verses
				//GeneralUtils::GetWarheadVersusArmor(pWH , Armor)
				) < 0.001)
				return false;

			if (effectsRequireDamage || data->EffectsRequireDamage) {
				return realDamage != 0;
			}
		}
	}

	return true;
}

CoordStruct Helpers_DP::RandomOffset(int min, int max)
{
	const double r = ScenarioClass::Instance->Random.RandomRanged(min, max);

	if (r > 0) {
		const double theta = ScenarioClass::Instance->Random.RandomDouble() * 2 * Math::PI;
		return { (int)(r * std::cos(theta)) ,(int)(r * std::sin(theta)) , 0 };
	}

	return CoordStruct::Empty;
}

CoordStruct Helpers_DP::RandomOffset(double maxSpread, double minSpread)
{
	const int min = (int)((minSpread <= 0 ? 0 : minSpread) * 256);
	const int max = (int)((maxSpread > 0 ? maxSpread : 1) * 256);
	return Helpers_DP::RandomOffset(min, max);
}

CoordStruct Helpers_DP::GetInaccurateOffset(float scatterMin, float scatterMax)
{
	int min = (int)(scatterMin * 256);
	int max = scatterMax > 0 ? (int)(scatterMax * 256) : RulesClass::Instance->BallisticScatter;
	if (min > max)
	{
		int temp = min;
		min = max;
		max = temp;
	}
	
	return Helpers_DP::RandomOffset(min, max);
}

VelocityClass Helpers_DP::GetBulletArcingVelocity(const CoordStruct& sourcePos, CoordStruct& targetPos,
			double speed, int gravity, bool lobber, bool inaccurate, float scatterMin, float scatterMax,
			int zOffset, ArcingVelocityData& outData)
{
	// 不精确
	if (inaccurate) {
		targetPos += GetInaccurateOffset(scatterMin, scatterMax);
	}

	// 不潜地
	outData.m_TargetCell = MapClass::Instance->TryGetCellAt(targetPos);
	if (outData.m_TargetCell) {
		targetPos.Z = outData.m_TargetCell->GetCoordsWithBridge().Z;
	}

	// 重算抛物线弹道
	if (gravity == 0) {
		gravity = RulesClass::Instance->Gravity;
	}

	CoordStruct tempSourcePos = sourcePos;
	CoordStruct tempTargetPos = targetPos;
	int zDiff = tempTargetPos.Z - tempSourcePos.Z + zOffset; // 修正高度差
	tempTargetPos.Z = 0;
	tempSourcePos.Z = 0;
	outData.m_StraightDistance = tempTargetPos.DistanceFrom(tempSourcePos);

	// Logger.Log("位置和目标的水平距离{0}", straightDistance);
	outData.m_RealSpeed = speed;
	if (outData.m_StraightDistance == 0.0 || isnan(outData.m_StraightDistance))
	{
		// 直上直下
		return  { 0.0 , 0.0 , (double)gravity };
	}

	if (outData.m_RealSpeed == 0.0) {
		outData.m_RealSpeed = std::sqrt(outData.m_StraightDistance * gravity * 1.2);
	}

	// 高抛弹道
	if (lobber) {
		outData.m_RealSpeed = (int)(outData.m_RealSpeed * 0.5);
	}

	double vZ = (zDiff * outData.m_RealSpeed) / outData.m_StraightDistance + 0.5 * gravity * outData.m_StraightDistance / outData.m_RealSpeed;
	VelocityClass v { (double)(tempTargetPos.X - tempSourcePos.X), (double)(tempTargetPos.Y - tempSourcePos.Y), 0.0 };
	v *= outData.m_RealSpeed / outData.m_StraightDistance;
	v.Z = vZ;
	return v;
}

CoordStruct Helpers_DP::GetFLHAbsoluteCoords(TechnoClass* pTechno, CoordStruct& flh, bool isOnTurret, int flipY, bool nextFrame)
{
	CoordStruct turretOffset {};
	auto const pType = pTechno->GetTechnoType();

	if (isOnTurret)
	{
		const auto& nTurretOffset = TechnoTypeExt::ExtMap.Find(pType)->TurretOffset.Get();
		turretOffset.X = nTurretOffset.X;
		turretOffset.Y = nTurretOffset.Y;
		turretOffset.Z = nTurretOffset.Z;
	}

	return GetFLHAbsoluteCoords(pTechno, flh, isOnTurret, flipY, turretOffset, nextFrame);
}

TechnoClass* Helpers_DP::CreateAndPutTechno(TechnoTypeClass* pType, HouseClass* pHouse, CoordStruct& location, CellClass* pCell , bool bPathfinding)
{
	if (pType)
	{
		auto const pTechno = generic_cast<TechnoClass*>(pType->CreateObject(pHouse));
		bool UnlimboSuccess = false;

		if (!pCell && location != CoordStruct::Empty)
			pCell = MapClass::Instance->GetCellAt(location);

		if (pCell)
		{
			const auto occFlags = pCell->OccupationFlags;

			if(!bPathfinding) {
				pTechno->OnBridge = pCell->ContainsBridge();
				++Unsorted::IKnowWhatImDoing;
				UnlimboSuccess = pTechno->Unlimbo(pCell->GetCoordsWithBridge(), DirType::East);
				--Unsorted::IKnowWhatImDoing;
			} else {

				if (Is_BuildingType(pType)) {
					if (!pCell->CanThisExistHere(pType->SpeedType, static_cast<BuildingTypeClass*>(pType), pHouse)) {
						location = MapClass::Instance->GetRandomCoordsNear(location, 0, false);
						pCell = MapClass::Instance->GetCellAt(location);
					}
				}

				pTechno->OnBridge = pCell->ContainsBridge();
				UnlimboSuccess = pTechno->Unlimbo(pCell->GetCoordsWithBridge(), DirType::East);
			}

			if (UnlimboSuccess)
			{
				pCell->OccupationFlags = occFlags;
				pTechno->SetLocation(location);
				return pTechno;
			}
			else
			{
				Debug::Log("Gifbox - Failed To Place Techno [%s] ! \n", pType->ID);

				if (pTechno)
				{
					TechnoExt::HandleRemove(pTechno);
				}
			}
		}
	}

	return nullptr;
}

void Helpers_DP::FireWeaponTo(TechnoClass* pShooter, TechnoClass* pAttacker, AbstractClass* pTarget, WeaponTypeClass* pWeapon, const CoordStruct& flh, FireBulletToTarget callback,const CoordStruct& bulletSourcePos, bool radialFire, int splitAngle)
{
	if (!pWeapon)
		return;

	if (!pTarget)
		return;

	CoordStruct targetPos {};
	if (auto const pFoot = generic_cast<FootClass*>(pTarget))
		targetPos = CellClass::Cell2Coord(pFoot->GetDestinationMapCoords());
	else
		targetPos = pTarget->GetCoords();

	if(auto const pCell  = MapClass::Instance->GetCellAt(targetPos))
		targetPos.Z = pCell->GetFloorHeight(Point2D::Empty);

	// radial fire
	int burst = pWeapon->Burst;
	RadialFireHelper radialFireHelper { pShooter, burst, splitAngle };
	int flipY = -1;

	for (int i = 0; i < burst; i++)
	{
		VelocityClass bulletVelocity { };
		if (radialFire) {
			flipY = (i < burst / 2.0f) ? -1 : 1;
			bulletVelocity = radialFireHelper.GetBulletVelocity(i);
		} else {
			flipY *= -1;
		}

		CoordStruct sourcePos = bulletSourcePos;

		if (!bulletSourcePos)
		{
			CoordStruct nFLh_ = flh;
			sourcePos = GetFLHAbsoluteCoords(pShooter, nFLh_, true, flipY);
		}

		if (!bulletVelocity.IsValid())
		{
			bulletVelocity = GetBulletVelocity(sourcePos, targetPos);
		}

		auto pBullet = FireBulletTo(pAttacker, pTarget, pWeapon, sourcePos, targetPos, bulletVelocity);

		if (callback && pBullet) {
			callback(i, burst, pBullet, pTarget);
		}

	}
}

BulletClass* Helpers_DP::FireBulletTo(TechnoClass* pAttacker, AbstractClass* pTarget, WeaponTypeClass* pWeapon, CoordStruct& sourcePos, CoordStruct& targetPos, VelocityClass& bulletVelocity)
{
	if (!pWeapon)
		return nullptr;

	if (!pTarget || !pAttacker->IsAlive)
		return nullptr;

	// Fire weapon
	auto const pBullet = FireBullet(pAttacker, pTarget, pWeapon, sourcePos, targetPos, bulletVelocity);
	// Draw bullet effect
	DrawBulletEffect(pWeapon, sourcePos, targetPos, pAttacker, pTarget);
	// Draw particle system
	AttachedParticleSystem(pWeapon, sourcePos, pTarget, pAttacker, targetPos);
	// Play report sound
	PlayReportSound(pWeapon, sourcePos ,pAttacker);
	// Draw weapon anim
	DrawWeaponAnim(pWeapon, sourcePos, targetPos, pAttacker, pTarget);
	return pBullet;
}

void Helpers_DP::PlayReportSound(WeaponTypeClass* pWeapon, CoordStruct& sourcePos, TechnoClass* pTechno)
{
	if (pWeapon->Report.Count > 0)
	{
		const int nResult = pTechno ? pTechno->weapon_sound_randomnumber_3C8 % pWeapon->Report.Count
			: Random2Class::Global->RandomFromMax(pWeapon->Report.Count - 1);
		VocClass::PlayIndexAtPos(pWeapon->Report.GetItem(nResult), sourcePos, nullptr);
	}
}

BulletClass* Helpers_DP::FireBullet(TechnoClass* pAttacker, AbstractClass* pTarget, WeaponTypeClass* pWeapon, CoordStruct& sourcePos, CoordStruct& targetPos, VelocityClass& bulletVelocity)
{
	if (!pWeapon)
		return nullptr;

	double fireMult = 1;

	if (pAttacker && pAttacker->IsAlive && !pAttacker->IsCrashing && !pAttacker->IsSinking)
	{
		// check spawner
		auto pSpawn = pAttacker->SpawnManager;
		if (pWeapon->Spawner && pSpawn)
		{
			pSpawn->SetTarget(pTarget);
			return nullptr;
		}

		// check Abilities FIREPOWER
		fireMult = GetDamageMult(pAttacker);
	}

	int damage = static_cast<int>(pWeapon->Damage * fireMult);
	auto pWH = pWeapon->Warhead;
	int speed = pWeapon->GetWeaponSpeed(sourcePos, targetPos);
	bool bright = pWeapon->Bright || pWH->Bright;

	auto pBulletTypeExt = BulletTypeExt::ExtMap.Find(pWeapon->Projectile);
	auto pExt = WeaponTypeExt::ExtMap.Find(pWeapon);

	BulletClass* pBullet = pBulletTypeExt->CreateBullet(pTarget, pAttacker, damage, pWH, speed, pExt->GetProjectileRange(), bright, true);

	if (!pBullet)
		return nullptr;

	pBullet->SetWeaponType(pWeapon);
	pBullet->MoveTo(sourcePos, bulletVelocity);

	if (pWeapon->Projectile->Inviso && !pWeapon->Projectile->Airburst)
	{
		pBullet->Detonate(targetPos);
		//GameDelete<true,false>(pBullet);
		pBullet->UnInit();

		return nullptr;
	}

	return pBullet;
}

void Helpers_DP::DrawWeaponAnim(WeaponTypeClass* pWeapon, CoordStruct& sourcePos, CoordStruct& targetPos, TechnoClass* pOwner, AbstractClass* pTarget)
{ 
	if (!pWeapon)
		return;

	// Anim
	if (pWeapon->Anim.Count > 0)
	{
		int facing = pWeapon->Anim.Count;
		int index = 0;
		if (facing % 8 == 0)
		{
			auto nDir = Point2Dir(sourcePos, targetPos);
			index = Dir2FacingIndex(nDir, facing);
			index = (int)(facing / 8) + index;
			if (index >= facing)
			{
				index = 0;
			}
		}

		if (auto pAnimType = pWeapon->Anim.GetItem(index))
		{
			if (auto pAnim = GameCreate<AnimClass>(pAnimType, sourcePos))
			{
				AnimExt::SetAnimOwnerHouseKind(pAnim, pOwner ? pOwner->GetOwningHouse() : nullptr, pTarget ? pTarget->GetOwningHouse() : nullptr, pOwner, false);
			}
		}
	}
}