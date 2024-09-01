#pragma once
#include "PhobosTrajectory.h"

class DisperseTrajectoryType : public PhobosTrajectoryType
{
public:

	Valueable<bool> UniqueCurve { false };
	Valueable<CoordStruct> PreAimCoord { { 0, 0, 0 } };
	Valueable<double> LaunchSpeed { 0.0 };
	Valueable<double> Acceleration { 10.0 };
	Valueable<double> ROT { 10.0 };
	Valueable<bool> LockDirection { false };
	Valueable<bool> CruiseEnable { false };
	Valueable<Leptons> CruiseUnableRange { Leptons(128) };
	Valueable<bool> LeadTimeCalculate { false };
	Valueable<Leptons> TargetSnapDistance { Leptons(128) };
	Valueable<bool> RetargetAllies { false };
	Valueable<double> RetargetRadius { 0.0 };
	Valueable<double> SuicideAboveRange { 0.0 };
	Valueable<bool> SuicideIfNoWeapon { false };
	ValueableVector<WeaponTypeClass*> Weapon {};
	ValueableVector<int> WeaponBurst {};
	Valueable<int> WeaponCount { 0 };
	Valueable<int> WeaponDelay { 1 };
	Valueable<int> WeaponTimer { 0 };
	Valueable<Leptons> WeaponScope {};
	Valueable<bool> WeaponRetarget { false };
	Valueable<bool> WeaponLocation { false };
	Valueable<bool> WeaponTendency { false };
	Valueable<bool> WeaponToAllies { false };
	Valueable<bool> FacingCoord { false };

	DisperseTrajectoryType(TrajectoryFlag variant) : PhobosTrajectoryType { variant }
	{ }

	DisperseTrajectoryType() : PhobosTrajectoryType { TrajectoryFlag::Disperse }
	{ }

	virtual ~DisperseTrajectoryType() = default;
	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override { };
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual bool Read(CCINIClass* const pINI, const char* pSection) override;

private:
	template <typename T>
	void Serialize(T& Stm)
	{
		Stm
			.Process(UniqueCurve)
			.Process(PreAimCoord)
			.Process(LaunchSpeed)
			.Process(Acceleration)
			.Process(ROT)
			.Process(LockDirection)
			.Process(CruiseEnable)
			.Process(CruiseUnableRange)
			.Process(LeadTimeCalculate)
			.Process(TargetSnapDistance)
			.Process(RetargetAllies)
			.Process(RetargetRadius)
			.Process(SuicideAboveRange)
			.Process(SuicideIfNoWeapon)
			.Process(Weapon, true)
			.Process(WeaponBurst)
			.Process(WeaponCount)
			.Process(WeaponDelay)
			.Process(WeaponTimer)
			.Process(WeaponScope)
			.Process(WeaponRetarget)
			.Process(WeaponLocation)
			.Process(WeaponTendency)
			.Process(WeaponToAllies)
			.Process(FacingCoord)
			;
	};
};

class DisperseTrajectory : public PhobosTrajectory
{
public:

	double LaunchSpeed { 0.0 };
	double FirepowerMult { 1.0 };
	double SuicideAboveRange { 0.0 };
	double LastReviseMult { 0.0 };

	bool TargetInAir { false };
	bool Accelerate { true };
	bool InStraight { false };

	int WeaponCount { 0 };
	int WeaponTimer { 0 };
	int FinalHeight { 0 };

	CoordStruct LastTargetCoord { };

	DisperseTrajectory(TrajectoryFlag varian) : PhobosTrajectory { varian }
	{ }

	DisperseTrajectory() : PhobosTrajectory { TrajectoryFlag::Disperse }
	{ }

	DisperseTrajectory(TrajectoryFlag varian, BulletClass* pBullet, PhobosTrajectoryType* pType) :
		PhobosTrajectory { varian , pBullet , pType }
	{
	}

	DisperseTrajectory(BulletClass* pBullet, PhobosTrajectoryType* pType) :
		PhobosTrajectory { TrajectoryFlag::Disperse , pBullet , pType }
	{
	}

	virtual ~DisperseTrajectory() override = default;
	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override { };
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual DisperseTrajectoryType* GetTrajectoryType() const { return reinterpret_cast<DisperseTrajectoryType*>(PhobosTrajectory::GetTrajectoryType()); }

	virtual void OnUnlimbo(CoordStruct* pCoord, VelocityClass* pVelocity) override;
	virtual bool OnAI() override;
	virtual void OnAIPreDetonate() override;
	virtual void OnAIVelocity(VelocityClass* pSpeed, VelocityClass* pPosition) override;
	virtual TrajectoryCheckReturnType OnAITargetCoordCheck(CoordStruct& coords) override;
	virtual TrajectoryCheckReturnType OnAITechnoCheck(TechnoClass* pTechno) override;

private:
	bool CalculateBulletVelocity(double StraightSpeed) const;
	bool BulletDetonatePreCheck();
	bool BulletRetargetTechno(HouseClass* pOwner);
	bool CurveVelocityChange();
	bool StandardVelocityChange();
	bool ChangeBulletVelocity(CoordStruct TargetLocation, double TurningRadius, bool Curve);
	bool PrepareDisperseWeapon(HouseClass* pOwner);
	std::vector<TechnoClass*> GetValidTechnosInSame(std::vector<TechnoClass*>& Technos, HouseClass* pOwner, WarheadTypeClass* pWH, bool Mode) const;
	void CreateDisperseBullets(WeaponTypeClass* pWeapon, AbstractClass* BulletTarget, HouseClass* pOwner) const;

	template <typename T>
	void Serialize(T& Stm)
	{
		Stm
			.Process(LaunchSpeed)
			.Process(FirepowerMult)
			.Process(SuicideAboveRange)
			.Process(LastReviseMult)
			.Process(TargetInAir)
			.Process(Accelerate)
			.Process(InStraight)
			.Process(WeaponCount)
			.Process(WeaponTimer)
			.Process(FinalHeight)
			.Process(LastTargetCoord)
			;
	};
};