#pragma once

#include "PhobosTrajectory.h"

class WaveTrajectoryType final : public PhobosTrajectoryType
{
public:

	Valueable<int> MaxHeight { 2000 };
	Valueable<int> MinHeight { 0 };

	WaveTrajectoryType() : PhobosTrajectoryType { TrajectoryFlag::Wave }
	{ }

	virtual ~WaveTrajectoryType() = default;
	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override { }
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual bool Read(CCINIClass* const pINI, const char* pSection) override;
	virtual const char* Name() { return "WaveTrajectoryType"; }
};

class WaveTrajectory final : public PhobosTrajectory
{
public:

	bool Fallen { false };

	WaveTrajectory() : PhobosTrajectory { TrajectoryFlag::Wave }
	{ }

	WaveTrajectory(BulletClass* pBullet, PhobosTrajectoryType* pType) :
		PhobosTrajectory { TrajectoryFlag::Wave , pBullet , pType }
	{ }

	virtual ~WaveTrajectory() = default;
	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override { }
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual WaveTrajectoryType* GetTrajectoryType() const { return reinterpret_cast<WaveTrajectoryType*>(PhobosTrajectory::GetTrajectoryType()); }

	virtual void OnUnlimbo(CoordStruct* pCoord, VelocityClass* pVelocity) override;
	virtual bool OnAI() override;
	virtual void OnAIPreDetonate() override;
	virtual void OnAIVelocity(VelocityClass* pSpeed, VelocityClass* pPosition) override;
	virtual TrajectoryCheckReturnType OnAITargetCoordCheck(CoordStruct& coords) override;
	virtual TrajectoryCheckReturnType OnAITechnoCheck(TechnoClass* pTechno) override;
	virtual const char* Name() { return "WaveTrajectory"; }
};
