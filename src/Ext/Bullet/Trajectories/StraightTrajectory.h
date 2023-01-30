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

	//bool SnapOnTarget;
	//Leptons SnapThreshold;
	//bool PassThrough;

	StraightTrajectory() : PhobosTrajectory { TrajectoryFlag::Straight }
		//, SnapOnTarget { true }
		//, SnapThreshold { 0 }
		//, PassThrough { false }
	{ }

	StraightTrajectory(PhobosTrajectoryType* pType) : PhobosTrajectory {TrajectoryFlag::Straight , pType }
		//, SnapOnTarget { true }
		//, SnapThreshold { 0 }
		//, PassThrough { false }
	{}

	virtual ~StraightTrajectory() override = default;
	virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual StraightTrajectoryType* GetTrajectoryType() const { return reinterpret_cast<StraightTrajectoryType*>(PhobosTrajectory::GetTrajectoryType()); }

	virtual void OnUnlimbo(BulletClass* pBullet,CoordStruct* pCoord, VelocityClass* pVelocity) override;
	virtual bool OnAI(BulletClass* pBullet) override;
	virtual void OnAIPreDetonate(BulletClass* pBullet) override;
	virtual void OnAIVelocity(BulletClass* pBullet, VelocityClass* pSpeed, VelocityClass* pPosition) override;
	virtual TrajectoryCheckReturnType OnAITargetCoordCheck(BulletClass* pBullet, CoordStruct& coords) override;
	virtual TrajectoryCheckReturnType OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno) override;
};