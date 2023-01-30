#pragma once

#include "PhobosTrajectory.h"

class WaveTrajectoryType final : public PhobosTrajectoryType
{
public:

	Valueable<int> MaxHeight;
	Valueable<int> MinHeight;

	WaveTrajectoryType() : PhobosTrajectoryType { TrajectoryFlag::Wave }
		, MaxHeight { 2000 }
		, MinHeight { 0 }
	{ }

	virtual ~WaveTrajectoryType() = default;
	virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual bool Read(CCINIClass* const pINI, const char* pSection) override;

};

class WaveTrajectory final : public PhobosTrajectory
{
public:

	bool Fallen;

	WaveTrajectory() : PhobosTrajectory { TrajectoryFlag::Wave }
		, Fallen { false }
	{ }

	WaveTrajectory(PhobosTrajectoryType* pType) : PhobosTrajectory { TrajectoryFlag::Wave , pType }
		, Fallen { false }
	{ }

	virtual ~WaveTrajectory() = default;
	virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual WaveTrajectoryType* GetTrajectoryType() const { return reinterpret_cast<WaveTrajectoryType*>(PhobosTrajectory::GetTrajectoryType()); }

	virtual void OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, VelocityClass* pVelocity) override;
	virtual bool OnAI(BulletClass* pBullet) override;
	virtual void OnAIPreDetonate(BulletClass* pBullet) override;
	virtual void OnAIVelocity(BulletClass* pBullet, VelocityClass* pSpeed, VelocityClass* pPosition) override;
	virtual TrajectoryCheckReturnType OnAITargetCoordCheck(BulletClass* pBullet, CoordStruct& coords) override;
	virtual TrajectoryCheckReturnType OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno) override;

};
