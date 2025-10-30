#pragma once

#include "PhobosTrajectory.h"


class StraightVariantCTrajectoryType final : public PhobosTrajectoryType
{
public:
	Valueable<Leptons> TargetSnapDistance { Leptons(128) };
	Valueable<bool> ApplyRangeModifiers { false };
	Valueable<bool> PassThrough { false };
	Valueable<bool> PassDetonate { false };
	Valueable<WarheadTypeClass*> PassDetonateWarhead {};
	Valueable<int> PassDetonateDamage { 0 };
	Valueable<int> PassDetonateDelay { 1 };
	Valueable<int> PassDetonateInitialDelay { 0 };
	Valueable<bool> PassDetonateLocal { false };
	Valueable<bool> LeadTimeCalculate { false };
	Valueable<CoordStruct> OffsetCoord {};
	Valueable<double> RotateCoord {};
	Valueable<bool> MirrorCoord { true };
	Valueable<bool> UseDisperseBurst { false };
	Valueable<CoordStruct> AxisOfRotation { { 0, 0, 1 } };
	Valueable<int> ProximityImpact {};
	Valueable<WarheadTypeClass*> ProximityWarhead {};
	Valueable<int> ProximityDamage {};
	Valueable<Leptons> ProximityRadius { Leptons(179) };
	Valueable<bool> ProximityDirect { false };
	Valueable<bool> ProximityMedial { false };
	Valueable<bool> ProximityAllies { false };
	Valueable<bool> ProximityFlight { false };
	Valueable<bool> ThroughVehicles { true };
	Valueable<bool> ThroughBuilding { true };
	Valueable<bool> SubjectToGround { false };
	Valueable<int> ConfineAtHeight {};
	Valueable<double> EdgeAttenuation { 1.0 };
	Valueable<double> CountAttenuation { 1.0 };

	StraightVariantCTrajectoryType() : PhobosTrajectoryType { TrajectoryFlag::StraightVariantC } { }
	virtual ~StraightVariantCTrajectoryType() = default;
	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override { }
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual bool Read(CCINIClass* const pINI, const char* pSection) override;

	virtual const char* Name() { return "StraightVariantCTrajectoryType"; }
};


class StraightTrajectoryVarianC : public PhobosTrajectory
{
public:

	double TrajectorySpeed {};
	int PassDetonateDamage {};
	CDTimerClass PassDetonateTimer {};
	CoordStruct OffsetCoord {};
	bool UseDisperseBurst {};
	int ProximityImpact {};
	int ProximityDamage {};
	int RemainingDistance { 1 };
	TechnoClass* ExtraCheck { nullptr }; // No taken out for use in next frame
	std::map<int, int> TheCasualty {}; // Only for recording existence
	double FirepowerMult { 1.0 };
	int AttenuationRange {};
	CoordStruct LastTargetCoord {};
	int CurrentBurst {};
	int CountOfBurst {};
	int WaitOneFrame {};

	StraightTrajectoryVarianC() : PhobosTrajectory { TrajectoryFlag::StraightVariantC }
	{ }

	StraightTrajectoryVarianC(BulletClass* pBullet, PhobosTrajectoryType* pType) :
		PhobosTrajectory { TrajectoryFlag::StraightVariantC, pBullet , pType }
	{ }

	virtual ~StraightTrajectoryVarianC() override = default;

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual StraightVariantCTrajectoryType* GetTrajectoryType() const { return reinterpret_cast<StraightVariantCTrajectoryType*>(PhobosTrajectory::GetTrajectoryType()); }

	virtual void OnUnlimbo(CoordStruct* pCoord, VelocityClass* pVelocity) override;
	virtual bool OnAI() override;
	virtual void OnAIPreDetonate() override;
	virtual void OnAIVelocity(VelocityClass* pSpeed, VelocityClass* pPosition) override;
	virtual TrajectoryCheckReturnType OnAITargetCoordCheck(CoordStruct& coords) override
	{
		return TrajectoryCheckReturnType::SkipGameCheck;
	}

	virtual TrajectoryCheckReturnType OnAITechnoCheck(TechnoClass* pTechno) override;
	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override
	{
		AnnounceInvalidPointer(ExtraCheck, ptr, bRemoved);
	}

	virtual const char* Name() { return "StraightTrajectoryVarianC"; }

private:
	void PrepareForOpenFire();
	int GetVelocityZ();
	bool CalculateBulletVelocity();
	bool BulletPrepareCheck();
	bool BulletDetonatePreCheck();
	void BulletDetonateVelocityCheck(HouseClass* pOwner);
	void BulletDetonateLastCheck(HouseClass* pOwner);
	bool CheckThroughAndSubjectInCell(CellClass* pCell, HouseClass* pOwner);
	void CalculateNewDamage();
	void PassWithDetonateAt(HouseClass* pOwner);
	void PrepareForDetonateAt(HouseClass* pOwner);
	std::vector<CellClass*> GetCellsInProximityRadius();
	std::vector<CellStruct> GetCellsInRectangle(CellStruct bottomStaCell, CellStruct leftMidCell, CellStruct rightMidCell, CellStruct topEndCell);
	int GetTheTrueDamage(int damage, TechnoClass* pTechno, bool self);
	double GetExtraDamageMultiplier(TechnoClass* pTechno);
	bool PassAndConfineAtHeight();
	int GetFirerZPosition();
	int GetTargetZPosition();
	bool ElevationDetonationCheck();
};
