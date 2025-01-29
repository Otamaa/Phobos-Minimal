#pragma once
#include "PhobosTrajectory.h"

class DisperseTrajectoryType : public PhobosTrajectoryType
{
public:

	Valueable<bool> UniqueCurve { false };
	Valueable<CoordStruct> PreAimCoord {};
	Valueable<double> RotateCoord {};
	Valueable<bool> MirrorCoord { true };
	Valueable<bool> FacingCoord {};
	Valueable<bool> ReduceCoord { true };
	Valueable<bool> UseDisperseBurst {};
	Valueable<CoordStruct> AxisOfRotation { { 0, 0, 1 } };
	Valueable<double> LaunchSpeed {};
	Valueable<double> Acceleration { 10.0 };
	Valueable<double> ROT { 30.0 };
	Valueable<bool> LockDirection {};
	Valueable<bool> CruiseEnable {};
	Valueable<Leptons> CruiseUnableRange { Leptons(1280) };
	Valueable<int> CruiseAltitude { 800 };
	Valueable<bool> CruiseAlongLevel {};
	Valueable<bool> LeadTimeCalculate { true };
	Valueable<Leptons> TargetSnapDistance { Leptons(128) };
	Valueable<bool> RetargetAllies {};
	Valueable<double> RetargetRadius {};
	Valueable<bool> SuicideShortOfROT { true };
	Valueable<double> SuicideAboveRange {};
	Valueable<bool> SuicideIfNoWeapon { true };
	ValueableVector<WeaponTypeClass*> Weapons {};
	ValueableVector<int> WeaponBurst {};
	Valueable<int> WeaponCount {};
	Valueable<int> WeaponDelay { 1 };
	Valueable<int> WeaponInitialDelay {};
	Valueable<Leptons> WeaponEffectiveRange {};
	Valueable<bool> WeaponSeparate {};
	Valueable<bool> WeaponRetarget {};
	Valueable<bool> WeaponLocation {};
	Valueable<bool> WeaponTendency {};
	Valueable<bool> WeaponHolistic {};
	Valueable<bool> WeaponMarginal {};
	Valueable<bool> WeaponToAllies {};
	Valueable<bool> WeaponDoRepeat {};

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
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(this->UniqueCurve)
			.Process(this->PreAimCoord)
			.Process(this->RotateCoord)
			.Process(this->MirrorCoord)
			.Process(this->FacingCoord)
			.Process(this->ReduceCoord)
			.Process(this->UseDisperseBurst)
			.Process(this->AxisOfRotation)
			.Process(this->LaunchSpeed)
			.Process(this->Acceleration)
			.Process(this->ROT)
			.Process(this->LockDirection)
			.Process(this->CruiseEnable)
			.Process(this->CruiseUnableRange)
			.Process(this->CruiseAltitude)
			.Process(this->CruiseAlongLevel)
			.Process(this->LeadTimeCalculate)
			.Process(this->TargetSnapDistance)
			.Process(this->RetargetRadius)
			.Process(this->RetargetAllies)
			.Process(this->SuicideShortOfROT)
			.Process(this->SuicideAboveRange)
			.Process(this->SuicideIfNoWeapon)
			.Process(this->Weapons)
			.Process(this->WeaponBurst)
			.Process(this->WeaponCount)
			.Process(this->WeaponDelay)
			.Process(this->WeaponInitialDelay)
			.Process(this->WeaponEffectiveRange)
			.Process(this->WeaponSeparate)
			.Process(this->WeaponRetarget)
			.Process(this->WeaponLocation)
			.Process(this->WeaponTendency)
			.Process(this->WeaponHolistic)
			.Process(this->WeaponMarginal)
			.Process(this->WeaponToAllies)
			.Process(this->WeaponDoRepeat)
			.Success()
			;
	};
};

class DisperseTrajectory : public PhobosTrajectory
{
public:

	double Speed {};
	CoordStruct PreAimCoord {};
	bool UseDisperseBurst {};
	bool CruiseEnable {};
	double SuicideAboveRange {};
	int WeaponCount {};
	CDTimerClass WeaponTimer {};
	bool InStraight {};
	bool Accelerate { true };
	bool TargetInTheAir {};
	bool TargetIsTechno {};
	int OriginalDistance {};
	int CurrentBurst {};
	int ThisWeaponIndex {};
	CoordStruct LastTargetCoord {};
	double PreAimDistance {};
	double LastReviseMult {};
	double FirepowerMult { 1.0 };

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
	void InitializeBulletNotCurve(bool facing);
	inline VelocityClass RotateAboutTheAxis(VelocityClass theSpeed, VelocityClass theAxis, double theRadian);
	bool CalculateBulletVelocity(double trajectorySpeed);
	bool BulletRetargetTechno();
	inline bool CheckTechnoIsInvalid(TechnoClass* pTechno);
	inline bool CheckWeaponCanTarget(WeaponTypeClass* pWeapon, TechnoClass* pFirer, TechnoClass* pTarget);
	bool CurveVelocityChange();
	bool NotCurveVelocityChange();
	bool StandardVelocityChange();
	bool ChangeBulletVelocity(CoordStruct targetLocation, double turningRadius, bool curve);
	bool PrepareDisperseWeapon();
	void CreateDisperseBullets(WeaponTypeClass* pWeapon, AbstractClass* pTarget, HouseClass* pOwner, int curBurst, int maxBurst);
	void DisperseBurstSubstitution(CoordStruct axis, double rotateCoord, int curBurst, int maxBurst, bool mirror);
	
	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(this->Speed)
			.Process(this->PreAimCoord)
			.Process(this->UseDisperseBurst)
			.Process(this->CruiseEnable)
			.Process(this->SuicideAboveRange)
			.Process(this->WeaponCount)
			.Process(this->WeaponTimer)
			.Process(this->InStraight)
			.Process(this->Accelerate)
			.Process(this->TargetInTheAir)
			.Process(this->TargetIsTechno)
			.Process(this->OriginalDistance)
			.Process(this->CurrentBurst)
			.Process(this->ThisWeaponIndex)
			.Process(this->LastTargetCoord)
			.Process(this->PreAimDistance)
			.Process(this->LastReviseMult)
			.Process(this->FirepowerMult)
			.Success()
			;
	};
};