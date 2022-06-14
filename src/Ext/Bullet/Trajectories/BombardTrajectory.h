#pragma once

#include "PhobosTrajectory.h"

class BombardTrajectoryType final : public PhobosTrajectoryType
{
public:

	double Height;

	BombardTrajectoryType() : PhobosTrajectoryType { TrajectoryFlag::Bombard }
		, Height { 0.0 }
	{ }

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual void Read(CCINIClass* const pINI, const char* pSection) override;

};

class BombardTrajectory final : public PhobosTrajectory
{
public:

	bool IsFalling;
	double Height;

	BombardTrajectory() : PhobosTrajectory { TrajectoryFlag::Bombard }
		, IsFalling { false }
		, Height { 0.0 }
	{}

	BombardTrajectory(PhobosTrajectoryType* pType, const BulletExt::ExtData* pData) : PhobosTrajectory { TrajectoryFlag::Bombard  , pData }
		, IsFalling { false }
		, Height { 0.0 }
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