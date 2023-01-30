#pragma once

#include "PhobosTrajectory.h"

class VerticalTrajectoryType final : public PhobosTrajectoryType
{
public:

	Valueable<double> Height;

	VerticalTrajectoryType() : PhobosTrajectoryType { TrajectoryFlag::Vertical }
		, Height { 10000.0 }
	{ }

	virtual ~VerticalTrajectoryType() = default;
	virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual bool Read(CCINIClass* const pINI, const char* pSection) override;


};

class VerticalTrajectory final : public PhobosTrajectory
{
public:

	bool IsFalling;
	double Height;

	VerticalTrajectory() : PhobosTrajectory { TrajectoryFlag::Vertical }
		, IsFalling { false }
		, Height { 0.0 }
	{ }

	VerticalTrajectory(PhobosTrajectoryType* pType) : PhobosTrajectory { TrajectoryFlag::Vertical , pType }
		, IsFalling { false }
		, Height { 0.0 }
	{ }

	virtual ~VerticalTrajectory() = default;
	virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual VerticalTrajectoryType* GetTrajectoryType() const { return reinterpret_cast<VerticalTrajectoryType*>(PhobosTrajectory::GetTrajectoryType()); }

	virtual void OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, VelocityClass* pVelocity) override;
	virtual bool OnAI(BulletClass* pBullet) override;
	virtual void OnAIPreDetonate(BulletClass* pBullet) override;
	virtual void OnAIVelocity(BulletClass* pBullet, VelocityClass* pSpeed, VelocityClass* pPosition) override;
	virtual TrajectoryCheckReturnType OnAITargetCoordCheck(BulletClass* pBullet, CoordStruct& coords) override;
	virtual TrajectoryCheckReturnType OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno) override;

};
#pragma once
