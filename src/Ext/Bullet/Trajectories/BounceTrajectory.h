#pragma once

#include "PhobosTrajectory.h"
class WeaponTypeClass;
class BounceTrajectoryType final : public PhobosTrajectoryType
{
public:

	int BounceAmount { 0 };
	Valueable<WeaponTypeClass*> BounceWeapon { nullptr };

	BounceTrajectoryType() : PhobosTrajectoryType { TrajectoryFlag::Bounce } { }
	virtual ~BounceTrajectoryType() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override { }
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual bool Read(CCINIClass* const pINI, const char* pSection) override;
	virtual const char* Name() { return "BounceTrajectoryType"; }
};

class BounceTrajectory final : public PhobosTrajectory
{
public:

	bool IsBouncing { false };
	int BounceLeft { 0 };

	BounceTrajectory() : PhobosTrajectory { TrajectoryFlag::Bounce } {}
	BounceTrajectory(BulletClass* pBullet , PhobosTrajectoryType* pType) :
		PhobosTrajectory { TrajectoryFlag::Bounce , pBullet , pType }
	{}
	virtual ~BounceTrajectory() override = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override { }
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual BounceTrajectoryType* GetTrajectoryType() const { return reinterpret_cast<BounceTrajectoryType*>(PhobosTrajectory::GetTrajectoryType()); }

	virtual void OnUnlimbo(CoordStruct* pCoord, VelocityClass* pVelocity) override;
	virtual bool OnAI() override;
	virtual void OnAIPreDetonate() override;
	virtual void OnAIVelocity(VelocityClass* pSpeed, VelocityClass* pPosition) override;
	virtual TrajectoryCheckReturnType OnAITargetCoordCheck(CoordStruct& coords) override;
	virtual TrajectoryCheckReturnType OnAITechnoCheck(TechnoClass* pTechno) override;
	virtual const char* Name() { return "BounceTrajectory"; }
};