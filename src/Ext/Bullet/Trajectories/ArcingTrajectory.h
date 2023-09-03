#pragma once

#include "PhobosTrajectory.h"

class ArcingTrajectoryType final : public PhobosTrajectoryType
{
public:
	Valueable<double> Elevation { 0.0 };
	Valueable<bool> Lobber { false };

	ArcingTrajectoryType() : PhobosTrajectoryType(TrajectoryFlag::Arcing) { }
	virtual ~ArcingTrajectoryType() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override { }
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
	virtual bool Read(CCINIClass* const pINI, const char* pSection) override;
};

class ArcingTrajectory final : public PhobosTrajectory
{
public:

	bool OverRange { false };

	ArcingTrajectory() : PhobosTrajectory { TrajectoryFlag::Arcing } { }
	ArcingTrajectory(BulletClass* pBullet, PhobosTrajectoryType* pType) :
		PhobosTrajectory { TrajectoryFlag::Arcing, pBullet ,pType }
	{ }

	virtual ~ArcingTrajectory() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) { }
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual ArcingTrajectoryType* GetTrajectoryType() const { return reinterpret_cast<ArcingTrajectoryType*>(PhobosTrajectory::GetTrajectoryType()); }

	virtual void OnUnlimbo(CoordStruct* pCoord, VelocityClass* pVelocity) override;

	virtual bool OnAI() override;
	virtual void OnAIPreDetonate() override;
	virtual void OnAIVelocity(VelocityClass* pSpeed, VelocityClass* pPosition) override;

	virtual TrajectoryCheckReturnType OnAITargetCoordCheck(CoordStruct& coords) override;
	virtual TrajectoryCheckReturnType OnAITechnoCheck(TechnoClass* pTechno) override;

};