#pragma once

#include "PhobosTrajectory.h"

class StraightTrajectoryType final : public PhobosTrajectoryType
{
public:
	Valueable<bool> SnapOnTarget { true };
	Nullable<Leptons> SnapThreshold { Leptons(Unsorted::LeptonsPerCell) };
	Valueable<Leptons> TargetSnapDistance { Leptons(0) };
	Valueable<bool> PassThrough { false };

	StraightTrajectoryType() : PhobosTrajectoryType {TrajectoryFlag::Straight}
	{ }

	virtual ~StraightTrajectoryType() = default;
	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override  { }
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual bool Read(CCINIClass* const pINI, const char* pSection) override;
};

class StraightTrajectory final : public PhobosTrajectory
{
public:

	int FirerZPosition { 0 };
	int TargetZPosition { 0 };

	StraightTrajectory() : PhobosTrajectory { TrajectoryFlag::Straight }
	{ }

	StraightTrajectory(BulletClass* pBullet, PhobosTrajectoryType* pType) :
		PhobosTrajectory {TrajectoryFlag::Straight , pBullet , pType }
	{ }

	virtual ~StraightTrajectory() override = default;
	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override { }
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual StraightTrajectoryType* GetTrajectoryType() const { return reinterpret_cast<StraightTrajectoryType*>(PhobosTrajectory::GetTrajectoryType()); }

	virtual void OnUnlimbo(CoordStruct* pCoord, VelocityClass* pVelocity) override;
	virtual bool OnAI() override;
	virtual void OnAIPreDetonate() override;
	virtual void OnAIVelocity(VelocityClass* pSpeed, VelocityClass* pPosition) override;
	virtual TrajectoryCheckReturnType OnAITargetCoordCheck(CoordStruct& coords) override;
	virtual TrajectoryCheckReturnType OnAITechnoCheck(TechnoClass* pTechno) override;

private:
	int GetVelocityZ() const;
	int GetFirerZPosition() const;
	CoordStruct GetTargetPosition() const;
	bool ElevationDetonationCheck() const;
};