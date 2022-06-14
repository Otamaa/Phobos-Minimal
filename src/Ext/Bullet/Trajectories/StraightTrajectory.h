#pragma once

#include "PhobosTrajectory.h"

class StraightTrajectoryType final : public PhobosTrajectoryType
{
public:
	bool SnapOnTarget;
	Leptons DetonationDistance;

	StraightTrajectoryType() : PhobosTrajectoryType {TrajectoryFlag::Straight}
		, SnapOnTarget { true }
		, DetonationDistance { 100 }
	{ }

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual void Read(CCINIClass* const pINI, const char* pSection) override;
};

class StraightTrajectory final : public PhobosTrajectory
{
public:

	bool SnapOnTarget;
	Leptons DetonationDistance;
	StraightTrajectory() : PhobosTrajectory { TrajectoryFlag::Straight }
		, SnapOnTarget { true }
		, DetonationDistance { 100 }
	{ }

	StraightTrajectory(PhobosTrajectoryType* pType, const BulletExt::ExtData* pData) : PhobosTrajectory {TrajectoryFlag::Straight , pData }
	{}

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual void OnUnlimbo(CoordStruct* pCoord, BulletVelocity* pVelocity) override;
	virtual bool OnAI() override;
	virtual void OnAIPreDetonate() override;
	virtual void OnAIVelocity(BulletVelocity* pSpeed, BulletVelocity* pPosition) override;
	virtual TrajectoryCheckReturnType OnAITargetCoordCheck(CoordStruct coords) override;
	virtual TrajectoryCheckReturnType OnAITechnoCheck(TechnoClass* pTechno) override;
};