#pragma once
#include <Utilities/TemplateDef.h>

#include <YRMath.h>

struct RadialFireHelper
{
	int Burst;
	double Degrees;
	int Delta;
	float DeltaZ;

	RadialFireHelper(TechnoClass* pTechno, int burst, int splitAngle)
		: Burst { burst }, Degrees { 0 } , Delta { 0 } , DeltaZ { 0.0f }
	{
		InitData(pTechno->HasTurret() ? pTechno->TurretFacing() : pTechno->PrimaryFacing.Desired(), splitAngle);
	}

	RadialFireHelper(const DirStruct& dir, int burst, int splitAngle)
		: Burst { burst }, Degrees { 0 }, Delta { 0 }, DeltaZ { 0.0f }
	{ InitData(dir, splitAngle); }

	RadialFireHelper(const RadialFireHelper& other) = default;
	RadialFireHelper& operator=(const RadialFireHelper& other) = default;

	~RadialFireHelper() = default;

	VelocityClass GetBulletVelocity(int index)
	{
		/*
		int z = 0;
		float temp = Burst / 2.0f;
		if (index - temp < 0)
			z = index;
		else
			z = abs(index - Burst + 1);

		const DirStruct targetDir = DirStruct(Degrees + Delta * (index + 1) * (Math::Pi / 180));
		Matrix3D matrix3D = Matrix3D::GetIdentity();
		matrix3D.RotateZ(static_cast<float>(targetDir.GetRadian()));
		matrix3D.Translate(1, 0, 0);
		Vector3D<float> offset {};
		Matrix3D::MatrixMultiply(&offset , &matrix3D, &Vector3D<float>::Empty);
		return { static_cast<double>(offset.X), static_cast<double>(-offset.Y), static_cast<double>(DeltaZ * z) };
		*/

		// Calculate Z offset
		int z = 0;
		float temp = Burst / 2.0f;
		if (index - temp < 0)
			z = index;
		else
			z = Math::abs(index - Burst + 1);

		// Direct trigonometric calculation - no matrix needed
		double targetAngle = Degrees + Delta * (index + 1) * (Math::Pi / 180.0);

		// Simple trig is 10x faster than matrix operations
		double cosAngle = Math::cos(targetAngle);
		double sinAngle = Math::sin(targetAngle);

		return { cosAngle, -sinAngle, static_cast<double>(DeltaZ * z) };
	}

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		// Debug::LogInfo("Loading Element From RadialFire ! ");
	 	return Serialize(Stm);
	 }

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
			//&& Stm.RegisterChange(this)
			;
	}

protected:
	void InitData(const DirStruct& dir, int splitAngle)
	{
		Degrees = dir.GetRadian() * (180.0 / Math::Pi) + splitAngle;
		Delta = splitAngle / (Burst + 1);
		DeltaZ = static_cast<float>(1.0 / (Burst / 2.0 + 1));
	}

};