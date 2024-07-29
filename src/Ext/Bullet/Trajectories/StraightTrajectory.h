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
};

class StraightVariantBTrajectoryType final : public StraightTrajectoryType
{
public:
	StraightVariantBTrajectoryType() : StraightTrajectoryType { TrajectoryFlag::StraightVariantB } { }
	virtual ~StraightVariantBTrajectoryType() = default;
};

class StraightVariantCTrajectoryType final : public StraightTrajectoryType
{
public:
	Valueable<bool> PassDetonate { false };
	Valueable<int> PassDetonateDelay { 1 };
	Valueable<int> PassDetonateTimer { 0 };
	Valueable<bool> PassDetonateLocal { false };
	Valueable<bool> LeadTimeCalculate { false };
	Valueable<CoordStruct> OffsetCoord {};
	Valueable<bool> MirrorCoord { true };
	Valueable<int> ProximityImpact { 0 };
	Valueable<double> ProximityRadius { 0.7 };
	Valueable<double> ProximityAllies { 0.0 };
	Valueable<bool> ThroughVehicles { true };
	Valueable<bool> ThroughBuilding { true };
	Nullable<WarheadTypeClass*> StraightWarhead {};
	Valueable<int> StraightDamage { 0 };
	Valueable<bool> SubjectToGround { false };
	Valueable<int> ConfineAtHeight { 0 };
	Valueable<double> EdgeAttenuation { 1.0 };

	StraightVariantCTrajectoryType() : StraightTrajectoryType { TrajectoryFlag::StraightVariantC } { }
	virtual ~StraightVariantCTrajectoryType() = default;

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual bool Read(CCINIClass* const pINI, const char* pSection) override;
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

private:
	int GetVelocityZ(CoordStruct& source) const;
};

class StraightTrajectoryVarianC : public StraightTrajectory
{
public:

	int PassDetonateTimer { 0 };
	int WaitOneFrame { 0 };
	int ProximityImpact { 0 };
	int CheckTimesLimit { 1 };

	CoordStruct OffsetCoord {};
	CoordStruct LastTargetCoord {};

	std::vector<TechnoClass*> LastCasualty {};

	double FirepowerMult { 1.0 };

	bool ThroughVehicles {};
	bool ThroughBuilding {};

	TechnoClass* ExtraCheck1 { nullptr };
	TechnoClass* ExtraCheck2 { nullptr };
	TechnoClass* ExtraCheck3 { nullptr };

	StraightTrajectoryVarianC() : StraightTrajectory { TrajectoryFlag::StraightVariantC }
	{ }

	StraightTrajectoryVarianC(BulletClass* pBullet, PhobosTrajectoryType* pType) :
		StraightTrajectory { TrajectoryFlag::StraightVariantC, pBullet , pType }
	{
	}

	virtual ~StraightTrajectoryVarianC() override = default;

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual StraightVariantCTrajectoryType* GetTrajectoryType() const { return reinterpret_cast<StraightVariantCTrajectoryType*>(PhobosTrajectory::GetTrajectoryType()); }

	virtual void OnUnlimbo(CoordStruct* pCoord, VelocityClass* pVelocity) override;
	virtual bool OnAI() override;
	virtual void OnAIPreDetonate() override;
	virtual void OnAIVelocity(VelocityClass* pSpeed, VelocityClass* pPosition) override;
	virtual TrajectoryCheckReturnType OnAITargetCoordCheck(CoordStruct& coords) override {
		return TrajectoryCheckReturnType::SkipGameCheck;
	}

	virtual TrajectoryCheckReturnType OnAITechnoCheck(TechnoClass* pTechno) override;
	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override
	{
		AnnounceInvalidPointer(LastCasualty, ptr, bRemoved);
		AnnounceInvalidPointer(ExtraCheck1, ptr, bRemoved);
		AnnounceInvalidPointer(ExtraCheck2, ptr, bRemoved);
		AnnounceInvalidPointer(ExtraCheck3, ptr, bRemoved);
	}

private:
	void PrepareForOpenFire();
	int GetVelocityZ() const;
	bool CalculateBulletVelocity(double StraightSpeed);
	bool BulletDetonatePreCheck(HouseClass* pOwner);
	void BulletDetonateLastCheck(HouseClass* pOwner, double StraightSpeed);
	void PassWithDetonateAt(HouseClass* pOwner);
	void PrepareForDetonateAt(HouseClass* pOwner);
	std::vector<CellClass*> GetCellsInPassThrough();
	std::vector<CellClass*> GetCellsInProximityRadius();
	std::vector<CellStruct> GetCellsInRectangle(CellStruct bStaCell, CellStruct lMidCell, CellStruct rMidCell, CellStruct tEndCell);
	double GetExtraDamageMultiplier(TechnoClass* pTechno, HouseClass* pOwner, bool Self) const;
	bool PassAndConfineAtHeight(double StraightSpeed);
};
