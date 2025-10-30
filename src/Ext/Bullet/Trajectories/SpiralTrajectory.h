#pragma once

#include "PhobosTrajectory.h"

class SpiralTrajectoryType final : public PhobosTrajectoryType
{
public:

	Valueable<double> MaxRadius { 128 };
	Valueable<double> Length { 256 };
	Valueable<double> Angel { 30 };

	SpiralTrajectoryType() : PhobosTrajectoryType { TrajectoryFlag::Spiral }
	{}

	virtual ~SpiralTrajectoryType() = default;
	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override { }
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual bool Read(CCINIClass* const pINI, const char* pSection) override;
	virtual const char* Name() { return "SpiralTrajectoryType"; }
};

class SpiralTrajectory final : public PhobosTrajectory
{
public:

	CoordStruct CenterLocation { CoordStruct::Empty };
	double DirectionAngel { 0.0 };
	double CurrentRadius { 0.0 };
	double CurrentAngel { 0.0 };
	bool close { false };

	SpiralTrajectory() : PhobosTrajectory { TrajectoryFlag::Spiral }
	{}

	SpiralTrajectory(BulletClass* pBullet, PhobosTrajectoryType* pType) :
		PhobosTrajectory { TrajectoryFlag::Spiral , pBullet , pType }
	{}

	virtual ~SpiralTrajectory() = default;
	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override { }
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual SpiralTrajectoryType* GetTrajectoryType() const { return reinterpret_cast<SpiralTrajectoryType*>(PhobosTrajectory::GetTrajectoryType()); }

	virtual void OnUnlimbo(CoordStruct* pCoord, VelocityClass* pVelocity) override;
	virtual bool OnAI() override;
	virtual void OnAIPreDetonate() override;
	virtual void OnAIVelocity(VelocityClass* pSpeed, VelocityClass* pPosition) override;
	virtual TrajectoryCheckReturnType OnAITargetCoordCheck(CoordStruct& coords) override;
	virtual TrajectoryCheckReturnType OnAITechnoCheck(TechnoClass* pTechno) override;

	virtual const char* Name() { return "SpiralTrajectory"; }
};
