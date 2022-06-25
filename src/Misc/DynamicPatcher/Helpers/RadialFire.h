#pragma once
#include <Utilities/TemplateDef.h>

#include <YRMath.h>

struct RadialFireHelper
{
	int Burst;
	double Degrees = 0;
	int Delta = 0;
	float DeltaZ = 0;

	RadialFireHelper(TechnoClass* pTechno, int burst, int splitAngle)
		: Burst { burst }
	{
		DirStruct dir = pTechno->PrimaryFacing.target();
		if (pTechno->HasTurret())
			dir = pTechno->TurretFacing();

		InitData(dir, splitAngle);
	}

	RadialFireHelper(DirStruct dir, int burst, int splitAngle)
		: Burst { burst }
	{ InitData(dir, splitAngle); }

	void InitData(DirStruct dir, int splitAngle)
	{
		Degrees = Math::rad2deg(dir.radians()) + splitAngle;
		Delta = splitAngle / (Burst + 1);
		DeltaZ = 1.0f / (Burst / 2.0f + 1);
	}

	VelocityClass GetBulletVelocity(int index)
	{
		int z = 0;
		float temp = Burst / 2.0f;
		if (index - temp < 0)
			z = index;
		else
			z = abs(index - Burst + 1);

		double angle = Degrees + Delta * (index + 1);
		DirStruct targetDir = DirStruct(Math::deg2rad(angle));
		Matrix3D matrix3D = Matrix3D { };
		matrix3D.MakeIdentity();
		matrix3D.RotateZ(static_cast<float>(targetDir.radians()));
		matrix3D.Translate(1, 0, 0);
		auto offset = Matrix3D::MatrixMultiply(matrix3D, Vector3D<float>::Empty);
		return { offset.X, -offset.Y, DeltaZ * z };
	}

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ Debug::Log("Loading Element From RadialFire ! \n");  return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm)
	{ return Serialize(Stm); }

private:
	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(Burst)
			.Process(Degrees)
			.Process(Delta)
			.Process(DeltaZ)
			.Success()
			;
	}

};