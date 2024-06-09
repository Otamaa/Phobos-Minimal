#pragma once

#include "PhobosTrajectory.h"

class BombardTrajectoryType final : public PhobosTrajectoryType
{
public:

	Valueable<double> Height { 0.0 };
	Valueable<double> FallPercent { 1.0 };
	Valueable<double> FallPercentShift { 0.0 };
	Valueable<Leptons> FallScatterRange {};
	Valueable<double> FallSpeed { 0.0 };
	Valueable<Leptons> TargetSnapDistance { Leptons(128) };
	Valueable<bool> FreeFallOnTarget { true };
	Valueable<bool> NoLaunch { false };
	Valueable<AnimTypeClass*> TurningPointAnim { nullptr };

	BombardTrajectoryType() : PhobosTrajectoryType { TrajectoryFlag::Bombard } { }
	virtual ~BombardTrajectoryType() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override  { }
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual bool Read(CCINIClass* const pINI, const char* pSection) override;
};

class BombardTrajectory final : public PhobosTrajectory
{
public:

	bool IsFalling { false };
	double Height { 0.0 };

	BombardTrajectory() : PhobosTrajectory { TrajectoryFlag::Bombard } {}
	BombardTrajectory(BulletClass* pBullet , PhobosTrajectoryType* pType) :
		PhobosTrajectory { TrajectoryFlag::Bombard , pBullet,  pType }
	{}
	virtual ~BombardTrajectory() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override { }
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual BombardTrajectoryType* GetTrajectoryType() const { return reinterpret_cast<BombardTrajectoryType*>(PhobosTrajectory::GetTrajectoryType()); }

	virtual void OnUnlimbo(CoordStruct* pCoord, VelocityClass* pVelocity) override;
	virtual bool OnAI() override;
	virtual void OnAIPreDetonate() override;
	virtual void OnAIVelocity(VelocityClass* pSpeed, VelocityClass* pPosition) override;
	virtual TrajectoryCheckReturnType OnAITargetCoordCheck(CoordStruct& coords) override;
	virtual TrajectoryCheckReturnType OnAITechnoCheck(TechnoClass* pTechno) override;

	void ApplyTurningPointAnim(CoordStruct& Position);
};