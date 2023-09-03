#pragma once

#include "PhobosTrajectory.h"

class ArtilleryTrajectoryType final : public PhobosTrajectoryType
{
public:
	Valueable<double> MaxHeight { 2000.0 };
	Valueable<bool> DistanceToHeight { true };
	Valueable<double> DistanceToHeight_Multiplier { 0.2 };

	ArtilleryTrajectoryType() : PhobosTrajectoryType(TrajectoryFlag::Artillery){ }
	virtual ~ArtilleryTrajectoryType() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override { }
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
	virtual bool Read(CCINIClass* const pINI, const char* pSection) override;
};

class ArtilleryTrajectory final : public PhobosTrajectory
{
public:

	CoordStruct InitialTargetLocation { CoordStruct::Empty };
	CoordStruct InitialSourceLocation { CoordStruct::Empty };
	CoordStruct CenterLocation { CoordStruct::Empty };
	double Height { 0 };
	bool Init { false };

	ArtilleryTrajectory() : PhobosTrajectory { TrajectoryFlag::Artillery } {}
	ArtilleryTrajectory(BulletClass* pBullet , PhobosTrajectoryType* pType) : 
		PhobosTrajectory { TrajectoryFlag::Artillery, pBullet ,pType } 
	{}
	virtual ~ArtilleryTrajectory() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) {}
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual ArtilleryTrajectoryType* GetTrajectoryType() const { return reinterpret_cast<ArtilleryTrajectoryType*>(PhobosTrajectory::GetTrajectoryType()); }

	virtual void OnUnlimbo(CoordStruct* pCoord, VelocityClass* pVelocity) override;
	virtual bool OnAI() override;
	virtual void OnAIPreDetonate() override;
	virtual void OnAIVelocity(VelocityClass* pSpeed, VelocityClass* pPosition) override;
	virtual TrajectoryCheckReturnType OnAITargetCoordCheck(CoordStruct& coords) override;
	virtual TrajectoryCheckReturnType OnAITechnoCheck(TechnoClass* pTechno) override;

};