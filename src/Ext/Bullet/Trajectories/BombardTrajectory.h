#pragma once

#include "PhobosTrajectory.h"

class BombardTrajectoryType final : public PhobosTrajectoryType
{
public:

	Valueable<double> Height {};
	Valueable<double> FallPercent { 1.0 };
	Valueable<double> FallPercentShift {};
	Valueable<Leptons> FallScatter_Max {};
	Valueable<Leptons> FallScatter_Min {};
	Valueable<bool> FallScatter_Linear {};
	Valueable<double> FallSpeed {};
	Valueable<Leptons> DetonationDistance { Leptons(102) };
	Valueable<int> DetonationHeight { -1 };
	Valueable<bool> EarlyDetonation {};
	Valueable<Leptons> TargetSnapDistance { Leptons(128) };
	Valueable<bool> FreeFallOnTarget { true };
	Valueable<bool> LeadTimeCalculate {};
	Valueable<bool> NoLaunch {};
	ValueableVector<AnimTypeClass*> TurningPointAnims {};
	Valueable<CoordStruct> OffsetCoord {};
	Valueable<double> RotateCoord {};
	Valueable<bool> MirrorCoord { true };
	Valueable<bool> UseDisperseBurst {};
	Valueable<CoordStruct> AxisOfRotation { { 0, 0, 1 } };
	Valueable<bool> SubjectToGround {};

	BombardTrajectoryType() : PhobosTrajectoryType { TrajectoryFlag::Bombard } { }
	virtual ~BombardTrajectoryType() = default;

	template<typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(this->Height)
			.Process(this->FallPercent)
			.Process(this->FallPercentShift)
			.Process(this->FallScatter_Max)
			.Process(this->FallScatter_Min)
			.Process(this->FallScatter_Linear)
			.Process(this->FallSpeed)
			.Process(this->DetonationDistance)
			.Process(this->DetonationHeight)
			.Process(this->EarlyDetonation)
			.Process(this->TargetSnapDistance)
			.Process(this->FreeFallOnTarget)
			.Process(this->LeadTimeCalculate)
			.Process(this->NoLaunch)
			.Process(this->TurningPointAnims)
			.Process(this->OffsetCoord)
			.Process(this->RotateCoord)
			.Process(this->MirrorCoord)
			.Process(this->UseDisperseBurst)
			.Process(this->AxisOfRotation)
			.Process(this->SubjectToGround)
			.Success()
			;
	}


	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override  { }
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual bool Read(CCINIClass* const pINI, const char* pSection) override;
};

class BombardTrajectory final : public PhobosTrajectory
{
public:

	double Height {};
	double FallPercent {};
	CoordStruct OffsetCoord {};
	bool UseDisperseBurst {};
	bool IsFalling {};
	bool ToFalling {};
	int RemainingDistance { 1 };
	CoordStruct LastTargetCoord {};
	CoordStruct InitialTargetCoord {};
	int CountOfBurst {};
	int CurrentBurst {};
	double RotateAngle {};
	int WaitOneFrame {};

	BombardTrajectory() : PhobosTrajectory { TrajectoryFlag::Bombard } {}
	BombardTrajectory(BulletClass* pBullet , PhobosTrajectoryType* pType) :
		PhobosTrajectory { TrajectoryFlag::Bombard , pBullet,  pType }
	{}
	virtual ~BombardTrajectory() = default;

	template<typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(this->Height)
			.Process(this->FallPercent)
			.Process(this->OffsetCoord)
			.Process(this->UseDisperseBurst)
			.Process(this->IsFalling)
			.Process(this->ToFalling)
			.Process(this->RemainingDistance)
			.Process(this->LastTargetCoord)
			.Process(this->InitialTargetCoord)
			.Process(this->CountOfBurst)
			.Process(this->CurrentBurst)
			.Process(this->RotateAngle)
			.Process(this->WaitOneFrame)
			.Success()
			;
	}

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override { }
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual BombardTrajectoryType* GetTrajectoryType() const { return reinterpret_cast<BombardTrajectoryType*>(PhobosTrajectory::GetTrajectoryType()); }

	virtual void OnUnlimbo(CoordStruct* pCoord, VelocityClass* pVelocity) override;
	virtual bool OnAI() override;
	virtual void OnAIPreDetonate() override;
	virtual void OnAIVelocity(VelocityClass* pSpeed, VelocityClass* pPosition) override;
	virtual TrajectoryCheckReturnType OnAITargetCoordCheck(CoordStruct& coords) override;
	virtual TrajectoryCheckReturnType OnAITechnoCheck(TechnoClass* pTechno) override;

	void PrepareForOpenFire();
	CoordStruct CalculateMiddleCoords();
	void CalculateTargetCoords();
	CoordStruct CalculateBulletLeadTime();
	void CalculateDisperseBurst();
	bool BulletPrepareCheck();
	bool BulletDetonatePreCheck();
	bool BulletDetonateRemainCheck(HouseClass* pOwner);
	void BulletVelocityChange();
	void RefreshBulletLineTrail();

	void CreateRandomAnim(CoordStruct coords, TechnoClass* pTechno = nullptr, HouseClass* pHouse = nullptr, bool invoker = false, bool ownedObject = false);
};
