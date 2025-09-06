#pragma once

#include "PhobosTrajectory.h"

class StraightTrajectoryType : public PhobosTrajectoryType
{
public:
	Valueable<bool> SnapOnTarget { true };
	Nullable<Leptons> SnapThreshold { Leptons(Unsorted::LeptonsPerCell) };
	Valueable<Leptons> TargetSnapDistance { Leptons(0) };
	Valueable<bool> PassThrough { false };
	Valueable<bool> DetonationDistance_ApplyRangeModifiers { false };

	StraightTrajectoryType(TrajectoryFlag variant) : PhobosTrajectoryType { variant }
	{ }

	StraightTrajectoryType() : PhobosTrajectoryType {TrajectoryFlag::Straight}
	{ }

	virtual ~StraightTrajectoryType() = default;
	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override  { }
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual bool Read(CCINIClass* const pINI, const char* pSection) override;

	virtual const char* Name() { return "StraightTrajectoryType"; }
};

class StraightVariantBTrajectoryType final : public StraightTrajectoryType
{
public:
	StraightVariantBTrajectoryType() : StraightTrajectoryType { TrajectoryFlag::StraightVariantB } { }
	virtual ~StraightVariantBTrajectoryType() = default;
	virtual const char* Name() { return "StraightVariantBTrajectoryType"; }
};

class StraightTrajectory : public PhobosTrajectory
{
public:

	int FirerZPosition { 0 };
	int TargetZPosition { 0 };

	StraightTrajectory(TrajectoryFlag varian) : PhobosTrajectory { varian }
	{ }

	StraightTrajectory() : PhobosTrajectory { TrajectoryFlag::Straight }
	{ }

	StraightTrajectory(TrajectoryFlag varian , BulletClass* pBullet, PhobosTrajectoryType* pType) :
		PhobosTrajectory { varian , pBullet , pType } {
	}

	StraightTrajectory(BulletClass* pBullet, PhobosTrajectoryType* pType) :
		PhobosTrajectory {TrajectoryFlag::Straight , pBullet , pType }
	{ }

	virtual ~StraightTrajectory() override = default;
	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override { }
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual StraightTrajectoryType* GetTrajectoryType() const { return reinterpret_cast<StraightTrajectoryType*>(PhobosTrajectory::GetTrajectoryType()); }

	virtual void OnUnlimbo(CoordStruct* pCoord, VelocityClass* pVelocity) override;
	virtual bool OnAI() override;
	virtual void OnAIPreDetonate() override;
	virtual void OnAIVelocity(VelocityClass* pSpeed, VelocityClass* pPosition) override;
	virtual TrajectoryCheckReturnType OnAITargetCoordCheck(CoordStruct& coords) override;
	virtual TrajectoryCheckReturnType OnAITechnoCheck(TechnoClass* pTechno) override;

	virtual const char* Name() { return "StraightTrajectory"; }
private:
	int GetVelocityZ(CoordStruct& source) const;
	int GetFirerZPosition() const;
	CoordStruct GetTargetPosition() const;
	bool ElevationDetonationCheck() const;
};

class StraightTrajectoryVarianB : public StraightTrajectory
{
public:

	StraightTrajectoryVarianB() : StraightTrajectory { TrajectoryFlag::StraightVariantB }
	{ }

	StraightTrajectoryVarianB(BulletClass* pBullet, PhobosTrajectoryType* pType) :
		StraightTrajectory { TrajectoryFlag::StraightVariantB, pBullet , pType }
	{ }

	virtual ~StraightTrajectoryVarianB() override = default;

	//virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	//virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual StraightVariantBTrajectoryType* GetTrajectoryType() const { return reinterpret_cast<StraightVariantBTrajectoryType*>(PhobosTrajectory::GetTrajectoryType()); }

	virtual void OnUnlimbo(CoordStruct* pCoord, VelocityClass* pVelocity) override;
	virtual bool OnAI() override;
	virtual void OnAIPreDetonate() override;
	virtual void OnAIVelocity(VelocityClass* pSpeed, VelocityClass* pPosition) override;
	virtual TrajectoryCheckReturnType OnAITargetCoordCheck(CoordStruct& coords) override;
	virtual const char* Name() { return "StraightTrajectoryVarianB"; }
private:
	int GetVelocityZ(CoordStruct& source) const;
};
