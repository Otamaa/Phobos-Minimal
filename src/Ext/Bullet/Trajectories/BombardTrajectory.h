#pragma once

#include "PhobosTrajectory.h"

class BombardTrajectoryType final : public PhobosTrajectoryType
{
public:

	Valueable<double> Height;
	Valueable<bool> Anti;
	
	BombardTrajectoryType() : PhobosTrajectoryType { TrajectoryFlag::Bombard }
		, Height { 0.0 }
		, Anti { false }
	{ }

	virtual ~BombardTrajectoryType() = default;
	virtual void InvalidatePointer(void* ptr, bool bRemoved) override  { }
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual bool Read(CCINIClass* const pINI, const char* pSection) override;

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

	BombardTrajectory(PhobosTrajectoryType* pType) : PhobosTrajectory { TrajectoryFlag::Bombard , pType }
		, IsFalling { false }
		, Height { 0.0 }
	{}

	virtual ~BombardTrajectory() = default;
	virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual BombardTrajectoryType* GetTrajectoryType() const { return reinterpret_cast<BombardTrajectoryType*>(PhobosTrajectory::GetTrajectoryType()); }

	virtual void OnUnlimbo(BulletClass* pBullet,CoordStruct* pCoord, VelocityClass* pVelocity) override;
	virtual bool OnAI(BulletClass* pBullet) override;
	virtual void OnAIPreDetonate(BulletClass* pBullet) override;
	virtual void OnAIVelocity(BulletClass* pBullet, VelocityClass* pSpeed, VelocityClass* pPosition) override;
	virtual TrajectoryCheckReturnType OnAITargetCoordCheck(BulletClass* pBullet, CoordStruct& coords) override;
	virtual TrajectoryCheckReturnType OnAITechnoCheck(BulletClass* pBullet,TechnoClass* pTechno) override;

};