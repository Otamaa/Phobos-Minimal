#pragma once

#include "PhobosTrajectory.h"

class MeteorTrajectoryType final : public PhobosTrajectoryType
{
public:

	Valueable<double> Height { 10000.0 };
	Valueable<double> Range { 30.0 };

	MeteorTrajectoryType() : PhobosTrajectoryType { TrajectoryFlag::Meteor } { }
	virtual ~MeteorTrajectoryType() = default;

	virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual bool Read(CCINIClass* const pINI, const char* pSection) override;


};

class MeteorTrajectory final : public PhobosTrajectory
{
public:

	MeteorTrajectory() : PhobosTrajectory { TrajectoryFlag::Meteor } { }

	MeteorTrajectory(BulletClass* pBullet, PhobosTrajectoryType* pType) : 
		PhobosTrajectory { TrajectoryFlag::Meteor , pBullet , pType }
	{ }

	virtual ~MeteorTrajectory() = default;
	virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual MeteorTrajectoryType* GetTrajectoryType() const { return reinterpret_cast<MeteorTrajectoryType*>(PhobosTrajectory::GetTrajectoryType()); }


	virtual void OnUnlimbo(CoordStruct* pCoord, VelocityClass* pVelocity) override;
	virtual bool OnAI() override;
	virtual void OnAIPreDetonate() override;
	virtual void OnAIVelocity(VelocityClass* pSpeed, VelocityClass* pPosition) override;
	virtual TrajectoryCheckReturnType OnAITargetCoordCheck(CoordStruct& coords) override;
	virtual TrajectoryCheckReturnType OnAITechnoCheck(TechnoClass* pTechno) override;

};
#pragma once
