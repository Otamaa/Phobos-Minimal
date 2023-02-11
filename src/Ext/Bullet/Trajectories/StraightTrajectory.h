#pragma once

#include "PhobosTrajectory.h"

class StraightTrajectoryType final : public PhobosTrajectoryType
{
public:
	Valueable<bool> SnapOnTarget;
	Valueable<Leptons> SnapThreshold;
	Valueable<bool> PassThrough;

	StraightTrajectoryType() : PhobosTrajectoryType {TrajectoryFlag::Straight}
		, SnapOnTarget { true }
		, SnapThreshold {}
		, PassThrough { false }
	{
		SnapThreshold = Leptons(Unsorted::LeptonsPerCell);
	}

	virtual ~StraightTrajectoryType() = default;
	virtual void InvalidatePointer(void* ptr, bool bRemoved) override  { }
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual bool Read(CCINIClass* const pINI, const char* pSection) override;
};

class StraightTrajectory final : public PhobosTrajectory
{
public:


	StraightTrajectory() : PhobosTrajectory { TrajectoryFlag::Straight }
	{ }

	StraightTrajectory(BulletClass* pBullet, PhobosTrajectoryType* pType) : PhobosTrajectory {TrajectoryFlag::Straight , pBullet , pType }
	{ }

	virtual ~StraightTrajectory() override = default;
	virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual StraightTrajectoryType* GetTrajectoryType() const { return reinterpret_cast<StraightTrajectoryType*>(PhobosTrajectory::GetTrajectoryType()); }

	virtual void OnUnlimbo(CoordStruct* pCoord, VelocityClass* pVelocity) override;
	virtual bool OnAI() override;
	virtual void OnAIPreDetonate() override;
	virtual void OnAIVelocity(VelocityClass* pSpeed, VelocityClass* pPosition) override;
	virtual TrajectoryCheckReturnType OnAITargetCoordCheck(CoordStruct& coords) override;
	virtual TrajectoryCheckReturnType OnAITechnoCheck(TechnoClass* pTechno) override;
};