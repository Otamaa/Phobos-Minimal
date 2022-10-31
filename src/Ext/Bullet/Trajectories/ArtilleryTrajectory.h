#pragma once

#include "PhobosTrajectory.h"

class ArtilleryTrajectoryType final : public PhobosTrajectoryType
{
public:
	Valueable<double> MaxHeight;

	ArtilleryTrajectoryType() : PhobosTrajectoryType(TrajectoryFlag::Artillery)
		, MaxHeight { 1500.0 }
	{ }

	virtual ~ArtilleryTrajectoryType() = default;
	virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
	virtual bool Read(CCINIClass* const pINI, const char* pSection) override;
};

class ArtilleryTrajectory final : public PhobosTrajectory
{
public:
	ArtilleryTrajectory() : PhobosTrajectory { TrajectoryFlag::Artillery }
		, InitialTargetLocation { CoordStruct::Empty }
		, InitialSourceLocation { CoordStruct::Empty }
		, MaxHeight { 0.0 }
	{}

	ArtilleryTrajectory(PhobosTrajectoryType* pType) : PhobosTrajectory { TrajectoryFlag::Artillery, pType }
		, InitialTargetLocation { CoordStruct::Empty }
		, InitialSourceLocation { CoordStruct::Empty }
		, MaxHeight { 0.0 }
	{}

	virtual ~ArtilleryTrajectory() = default;
	virtual void InvalidatePointer(void* ptr, bool bRemoved) { }
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual ArtilleryTrajectoryType* GetTrajectoryType() const { return reinterpret_cast<ArtilleryTrajectoryType*>(PhobosTrajectory::GetTrajectoryType()); }

	virtual void OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, VelocityClass* pVelocity) override;
	virtual bool OnAI(BulletClass* pBullet) override;
	virtual void OnAIPreDetonate(BulletClass* pBullet) override;
	virtual void OnAIVelocity(BulletClass* pBullet, VelocityClass* pSpeed, VelocityClass* pPosition) override;
	virtual TrajectoryCheckReturnType OnAITargetCoordCheck(BulletClass* pBullet, CoordStruct coords) override;
	virtual TrajectoryCheckReturnType OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno) override;

	CoordStruct InitialTargetLocation;
	CoordStruct InitialSourceLocation;
	double MaxHeight;
};