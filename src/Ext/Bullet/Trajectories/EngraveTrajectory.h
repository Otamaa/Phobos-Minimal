#pragma once
#include "PhobosTrajectory.h"

class EngraveTrajectoryType : public PhobosTrajectoryType
{
public:

	Valueable<Point2D> SourceCoord { { 0, 0 } };
	Valueable<Point2D> TargetCoord { { 0, 0 } };
	Valueable<bool> MirrorCoord { true };
	Valueable<bool> IsSupported { false };
	Valueable<bool> IsHouseColor { false };
	Valueable<bool> IsSingleColor { false };
	Valueable<ColorStruct> LaserInnerColor { { 0, 0, 0 } };
	Valueable<ColorStruct> LaserOuterColor { { 0, 0, 0 } };
	Valueable<ColorStruct> LaserOuterSpread { { 0, 0, 0 } };
	Valueable<int> LaserThickness { 3 };
	Valueable<int> LaserDuration { 1 };
	Valueable<int> LaserDelay { 1 };
	Valueable<int> DamageDelay { 10 };
	Valueable<int> Duration { 0 };

	EngraveTrajectoryType(TrajectoryFlag variant) : PhobosTrajectoryType { variant }
	{ }

	EngraveTrajectoryType() : PhobosTrajectoryType { TrajectoryFlag::Engrave }
	{ }

	virtual ~EngraveTrajectoryType() = default;
	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override { };
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual bool Read(CCINIClass* const pINI, const char* pSection) override;
};

class EngraveTrajectory : public PhobosTrajectory
{
public:

	Point2D SourceCoord {};
	Point2D TargetCoord {};

	int LaserTimer { 0 };
	int DamageTimer { 0 };
	int SourceHeight { 0 };
	int Duration { 0 };

	bool SetItsLocation { false };
	bool TechnoInLimbo { false };

	double FirepowerMult { 1.0 };
	CoordStruct FLHCoord { };

	EngraveTrajectory(TrajectoryFlag varian) : PhobosTrajectory { varian }
	{ }

	EngraveTrajectory() : PhobosTrajectory { TrajectoryFlag::Engrave }
	{ }

	EngraveTrajectory(TrajectoryFlag varian, BulletClass* pBullet, PhobosTrajectoryType* pType) :
		PhobosTrajectory { varian , pBullet , pType }
	{
	}

	EngraveTrajectory(BulletClass* pBullet, PhobosTrajectoryType* pType) :
		PhobosTrajectory { TrajectoryFlag::Engrave , pBullet , pType }
	{
	}

	virtual ~EngraveTrajectory() override = default;
	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override { }
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual EngraveTrajectoryType* GetTrajectoryType() const { return reinterpret_cast<EngraveTrajectoryType*>(PhobosTrajectory::GetTrajectoryType()); }

	virtual void OnUnlimbo(CoordStruct* pCoord, VelocityClass* pVelocity) override;
	virtual bool OnAI() override;
	virtual void OnAIPreDetonate() override;
	virtual void OnAIVelocity(VelocityClass* pSpeed, VelocityClass* pPosition) override;
	virtual TrajectoryCheckReturnType OnAITargetCoordCheck(CoordStruct& coords) override;
	virtual TrajectoryCheckReturnType OnAITechnoCheck(TechnoClass* pTechno) override;

private:
	int GetFloorCoordHeight(CoordStruct Coord) const;

	template <typename T>
	void Serialize(T& Stm)
	{
		Stm
			.Process(this->SourceCoord)
			.Process(this->TargetCoord)
			.Process(this->LaserTimer)
			.Process(this->DamageTimer)
			.Process(this->SourceHeight)
			.Process(this->Duration)
			.Process(this->SetItsLocation)
			.Process(this->TechnoInLimbo)
			.Process(this->FirepowerMult)
			.Process(this->FLHCoord)
			;
	};

};