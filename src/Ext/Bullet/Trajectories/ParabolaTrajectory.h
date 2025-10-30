#pragma once
#include "PhobosTrajectory.h"

enum class ParabolaFireMode : size_t
{
	Speed = 0,
	Height = 1,
	Angle = 2,
	SpeedAndHeight = 3,
	HeightAndAngle = 4,
	SpeedAndAngle = 5,

	count
};


class ParabolaTrajectoryType final : public PhobosTrajectoryType
{
public:
	Valueable<Leptons> TargetSnapDistance { Leptons(128) };
	Valueable<ParabolaFireMode> OpenFireMode { ParabolaFireMode::Speed };
	Valueable<int> ThrowHeight { 600 };
	Valueable<double> LaunchAngle { 30.0 };
	Valueable<bool> LeadTimeCalculate { false };
	Valueable<bool> LeadTimeSimplify { false };
	Valueable<double> LeadTimeMultiplier { 1.0 };
	Valueable<double> DetonationAngle { -90.0 };
	Valueable<int> DetonationHeight { -1 };
	Valueable<int> BounceTimes { 0 };
	Valueable<bool> BounceOnWater { false };
	Valueable<bool> BounceDetonate { false };
	Valueable<double> BounceAttenuation { 0.8 };
	Valueable<double> BounceCoefficient { 0.8 };
	Valueable<CoordStruct> OffsetCoord {};
	Valueable<int> RotateCoord {};
	Valueable<bool> MirrorCoord { true };
	Valueable<bool> UseDisperseBurst { false };
	Valueable<CoordStruct> AxisOfRotation { { 0, 0, 1 } };

	ParabolaTrajectoryType() : PhobosTrajectoryType(TrajectoryFlag::Parabola) { }
	virtual ~ParabolaTrajectoryType() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override { }
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
	virtual bool Read(CCINIClass* const pINI, const char* pSection) override;
	virtual const char* Name() { return "ParabolaTrajectoryType"; }
};

class ParabolaTrajectory final : public PhobosTrajectory
{
public:
	int BounceTimes {};
	CoordStruct OffsetCoord {};
	bool ShouldDetonate {};
	bool ShouldBounce {};
	bool NeedExtraCheck {};
	CoordStruct LastTargetCoord {};
	int CurrentBurst {};
	int CountOfBurst {};
	CDTimerClass WaitOneFrame {};
	VelocityClass LastVelocity {};

	ParabolaTrajectory() : PhobosTrajectory { TrajectoryFlag::Parabola } { }
	ParabolaTrajectory(BulletClass* pBullet, PhobosTrajectoryType* pType) :
		PhobosTrajectory { TrajectoryFlag::Parabola, pBullet ,pType }
	{
	}

	virtual ~ParabolaTrajectory() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) { }
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual ParabolaTrajectoryType* GetTrajectoryType() const { return reinterpret_cast<ParabolaTrajectoryType*>(PhobosTrajectory::GetTrajectoryType()); }

	virtual void OnUnlimbo(CoordStruct* pCoord, VelocityClass* pVelocity) override;

	virtual bool OnAI() override;
	virtual void OnAIPreDetonate() override;
	virtual void OnAIVelocity(VelocityClass* pSpeed, VelocityClass* pPosition) override;

	virtual TrajectoryCheckReturnType OnAITargetCoordCheck(CoordStruct& coords) override;
	virtual TrajectoryCheckReturnType OnAITechnoCheck(TechnoClass* pTechno) override;

	virtual const char* Name() { return "ParabolaTrajectory"; }
private:

	void PrepareForOpenFire();
	bool BulletPrepareCheck();
	void CalculateBulletVelocityRightNow(CoordStruct* pSourceCoords, double gravity);
	void CalculateBulletVelocityLeadTime(CoordStruct* pSourceCoords, double gravity);
	void CheckIfNeedExtraCheck();
	double SearchVelocity(double horizontalDistance, int distanceCoordsZ, double radian, double gravity);
	double CheckVelocityEquation(double horizontalDistance, int distanceCoordsZ, double velocity, double radian, double gravity);
	double SolveFixedSpeedMeetTime(CoordStruct* pSourceCrd, CoordStruct* pTargetCrd, CoordStruct* pOffsetCrd, double horizontalSpeed);
	double SearchFixedHeightMeetTime(CoordStruct* pSourceCrd, CoordStruct* pTargetCrd, CoordStruct* pOffsetCrd, double gravity);
	double CheckFixedHeightEquation(CoordStruct* pSourceCrd, CoordStruct* pTargetCrd, CoordStruct* pOffsetCrd, double meetTime, double gravity);
	double SearchFixedAngleMeetTime(CoordStruct* pSourceCrd, CoordStruct* pTargetCrd, CoordStruct* pOffsetCrd, double radian, double gravity);
	double CheckFixedAngleEquation(CoordStruct* pSourceCrd, CoordStruct* pTargetCrd, CoordStruct* pOffsetCrd, double meetTime, double radian, double gravity);
	bool CalculateBulletVelocityAfterBounce(CellClass* pCell, double gravity);
	VelocityClass GetGroundNormalVector(CellClass* pCell);
	bool CheckBulletHitCliff(short X, short Y, int bulletHeight, int lastCellHeight);
	bool BulletDetonatePreCheck();
	bool BulletDetonateLastCheck(double gravity);
	void BulletDetonateEffectuate(double velocityMult);

};