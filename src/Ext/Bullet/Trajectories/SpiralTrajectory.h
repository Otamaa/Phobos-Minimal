#pragma once

#include "PhobosTrajectory.h"

class SpiralTrajectoryType final : public PhobosTrajectoryType
{
public:

	Valueable<double> MaxRadius;
	Valueable<double> Length;
	Valueable<double> Angel;

	SpiralTrajectoryType() : PhobosTrajectoryType { TrajectoryFlag::Spiral }
		, MaxRadius { 128 }
		, Length { 256 }
		, Angel { 30 }
	{}

	virtual ~SpiralTrajectoryType() = default;
	virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual bool Read(CCINIClass* const pINI, const char* pSection) override;

};

class SpiralTrajectory final : public PhobosTrajectory
{
public:

	CoordStruct CenterLocation;
	double DirectionAngel;
	double CurrentRadius;
	double CurrentAngel;
	bool close;

	SpiralTrajectory() : PhobosTrajectory { TrajectoryFlag::Spiral }
		, CenterLocation { CoordStruct::Empty }
		, DirectionAngel { 0.0 }
		, CurrentRadius { 0.0 }
		, CurrentAngel { 0.0 }
		, close { false }
	{}

	SpiralTrajectory(BulletClass* pBullet, PhobosTrajectoryType* pType) : PhobosTrajectory { TrajectoryFlag::Spiral , pBullet , pType }
		, CenterLocation { CoordStruct::Empty }
		, DirectionAngel { 0.0 }
		, CurrentRadius { 0.0 }
		, CurrentAngel { 0.0 }
		, close { false }
	{}

	virtual ~SpiralTrajectory() = default;
	virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual SpiralTrajectoryType* GetTrajectoryType() const { return reinterpret_cast<SpiralTrajectoryType*>(PhobosTrajectory::GetTrajectoryType()); }

	virtual void OnUnlimbo(CoordStruct* pCoord, VelocityClass* pVelocity) override;
	virtual bool OnAI() override;
	virtual void OnAIPreDetonate() override;
	virtual void OnAIVelocity(VelocityClass* pSpeed, VelocityClass* pPosition) override;
	virtual TrajectoryCheckReturnType OnAITargetCoordCheck(CoordStruct& coords) override;
	virtual TrajectoryCheckReturnType OnAITechnoCheck(TechnoClass* pTechno) override;
};
