#pragma once

#include "PhobosTrajectory.h"
class WeaponTypeClass;
class BounceTrajectoryType final : public PhobosTrajectoryType
{
public:

	int BounceAmount;
	Valueable<WeaponTypeClass*> BounceWeapon;

	BounceTrajectoryType() : PhobosTrajectoryType { TrajectoryFlag::Bounce }
		, BounceAmount { 0 }
		, BounceWeapon { nullptr }
	{ }

	virtual ~BounceTrajectoryType() = default;
	virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual bool Read(CCINIClass* const pINI, const char* pSection) override;

};

class BounceTrajectory final : public PhobosTrajectory
{
public:

	bool IsBouncing;
	int BounceLeft;

	BounceTrajectory() : PhobosTrajectory { TrajectoryFlag::Bounce }
		, IsBouncing { false }
		, BounceLeft { 0 }
	{}

	BounceTrajectory(PhobosTrajectoryType* pType) : PhobosTrajectory { TrajectoryFlag::Bounce , pType }
		, IsBouncing { false }
		, BounceLeft { 0 }
	{}

	virtual ~BounceTrajectory() override = default;
	virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual BounceTrajectoryType* GetTrajectoryType() const { return reinterpret_cast<BounceTrajectoryType*>(PhobosTrajectory::GetTrajectoryType()); }

	virtual void OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, VelocityClass* pVelocity) override;
	virtual bool OnAI(BulletClass* pBullet) override;
	virtual void OnAIPreDetonate(BulletClass* pBullet) override;
	virtual void OnAIVelocity(BulletClass* pBullet, VelocityClass* pSpeed, VelocityClass* pPosition) override;
	virtual TrajectoryCheckReturnType OnAITargetCoordCheck(BulletClass* pBullet, CoordStruct& coords) override;
	virtual TrajectoryCheckReturnType OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno) override;

};