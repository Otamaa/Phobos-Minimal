#pragma once

#include "PhobosTrajectory.h"

class MeteorTrajectoryType final : public PhobosTrajectoryType
{
public:

	Valueable<double> Height;
	Valueable<double> Range;

	MeteorTrajectoryType() : PhobosTrajectoryType { TrajectoryFlag::Meteor }
		, Height { 10000.0 }
		, Range { 30.0 }
	{ }

	virtual ~MeteorTrajectoryType() = default;
	virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual bool Read(CCINIClass* const pINI, const char* pSection) override;


};

class MeteorTrajectory final : public PhobosTrajectory
{
public:

	MeteorTrajectory() : PhobosTrajectory { TrajectoryFlag::Meteor }
	{ }

	MeteorTrajectory(PhobosTrajectoryType* pType) : PhobosTrajectory { TrajectoryFlag::Meteor , pType }
	{ }

	virtual ~MeteorTrajectory() = default;
	virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual MeteorTrajectoryType* GetTrajectoryType() const { return reinterpret_cast<MeteorTrajectoryType*>(PhobosTrajectory::GetTrajectoryType()); }


	virtual void OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, VelocityClass* pVelocity) override;
	virtual bool OnAI(BulletClass* pBullet) override;
	virtual void OnAIPreDetonate(BulletClass* pBullet) override;
	virtual void OnAIVelocity(BulletClass* pBullet, VelocityClass* pSpeed, VelocityClass* pPosition) override;
	virtual TrajectoryCheckReturnType OnAITargetCoordCheck(BulletClass* pBullet, CoordStruct& coords) override;
	virtual TrajectoryCheckReturnType OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno) override;

};
#pragma once
