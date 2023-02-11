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

	VerticalTrajectory(BulletClass* pBullet, PhobosTrajectoryType* pType) : PhobosTrajectory { TrajectoryFlag::Vertical , pBullet , pType }
		, IsFalling { false }
		, Height { 0.0 }
	{ }

	virtual ~VerticalTrajectory() = default;
	virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual VerticalTrajectoryType* GetTrajectoryType() const { return reinterpret_cast<VerticalTrajectoryType*>(PhobosTrajectory::GetTrajectoryType()); }

	virtual void OnUnlimbo(CoordStruct* pCoord, VelocityClass* pVelocity) override;
	virtual bool OnAI() override;
	virtual void OnAIPreDetonate() override;
	virtual void OnAIVelocity(VelocityClass* pSpeed, VelocityClass* pPosition) override;
	virtual TrajectoryCheckReturnType OnAITargetCoordCheck(CoordStruct& coords) override;
	virtual TrajectoryCheckReturnType OnAITechnoCheck(TechnoClass* pTechno) override;

};
#pragma once
