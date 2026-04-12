#pragma once

#include "PhobosTrajectory.h"

// ============================================================================
// BounceTrajectory
//
// A projectile that reflects off obstacles instead of detonating on contact.
// On each impact:
//   - The velocity component along the surface normal is reflected
//   - The full velocity is scaled by BulletType->Elasticity (energy loss)
//   - Optionally fires BounceWeapon at the impact point
//   - Decrements BounceLeft; when it reaches 0, the next impact detonates
//
// The "first bounce is free" model is preserved from the original hooks:
// the first impact initializes BounceLeft from BounceAmount and lets the
// bullet keep flying. Subsequent impacts decrement.
//
// Termination conditions:
//   - BounceLeft reaches 0 AND another impact occurs
//   - Bullet leaves the map
//   - Velocity magnitude decays below BounceMinVelocity (default 8.0)
// ============================================================================

class WeaponTypeClass;

class BounceTrajectoryType final : public PhobosTrajectoryType
{
public:
    int                          BounceAmount      { 0 };
    Valueable<WeaponTypeClass*>  BounceWeapon      { nullptr };
    Valueable<double>            BounceMinVelocity { 8.0 };
    Valueable<bool>              BounceOnTerrain   { true };

    BounceTrajectoryType() : PhobosTrajectoryType { TrajectoryFlag::Bounce } { }
    virtual ~BounceTrajectoryType() = default;

    virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override { }
    virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
    virtual bool Save(PhobosStreamWriter& Stm) const override;

    virtual bool Read(CCINIClass* const pINI, const char* pSection) override;
    virtual const char* Name() { return "BounceTrajectoryType"; }

private:
	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(BounceAmount)
			.Process(BounceWeapon)
			.Process(BounceMinVelocity)
			.Process(BounceOnTerrain)

			;

	}
};

class BounceTrajectory final : public PhobosTrajectory
{
public:
    bool             IsBouncing { false };
    int              BounceLeft { 0 };
    AbstractClass*   LastObject { nullptr };  // anti-double-bounce tracker

    BounceTrajectory() : PhobosTrajectory { TrajectoryFlag::Bounce } {}
    BounceTrajectory(BulletClass* pBullet, PhobosTrajectoryType* pType)
        : PhobosTrajectory { TrajectoryFlag::Bounce, pBullet, pType } {}
    virtual ~BounceTrajectory() override = default;

    virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override;
    virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
    virtual bool Save(PhobosStreamWriter& Stm) const override;

    virtual BounceTrajectoryType* GetTrajectoryType() const
    {
        return reinterpret_cast<BounceTrajectoryType*>(PhobosTrajectory::GetTrajectoryType());
    }

    virtual void OnUnlimbo(CoordStruct* pCoord, VelocityClass* pVelocity) override;
    virtual bool OnAI() override;
    virtual void OnAIPreDetonate() override;
    virtual void OnAIVelocity(VelocityClass* pSpeed, VelocityClass* pPosition) override;
    virtual TrajectoryCheckReturnType OnAITargetCoordCheck(CoordStruct& coords) override;
    virtual TrajectoryCheckReturnType OnAITechnoCheck(TechnoClass* pTechno) override;

    // ------------------------------------------------------------------------
    // OnBounceCheck — new virtual added to PhobosTrajectory base
    //
    // Called from FinalizeBulletMotion *before* Is_Forced_To_Explode commits
    // the explode flag. Allows trajectories to intercept impact, perform
    // reflection, and tell the orchestrator whether to keep flying or detonate.
    // ------------------------------------------------------------------------
    virtual BounceCheckResult OnBounceCheck(CoordStruct const& impactCoord,
                                             bool& force_detonate) override;

    virtual const char* Name() { return "BounceTrajectory"; }

private:
    // Internal helpers used by OnBounceCheck
    BounceCheckResult TryBounce(CoordStruct const& impactCoord,
                                 AbstractClass* pObstacle,
                                 bool& force_detonate);

    // Computes a face-normal-based reflection of the bullet's velocity.
    // Determines which face was hit by comparing the impact point against
    // the obstacle's bounding box (or footprint, for buildings).
    void ReflectVelocityOff(AbstractClass* pObstacle, CoordStruct const& impactCoord);

    // Bounces vertically off the floor (Z component reflection).
    void ReflectVelocityOffFloor();
};

// ============================================================================
// Additions required in PhobosTrajectory.h base header:
// ============================================================================
//
//   enum class BounceCheckResult : unsigned char
//   {
//       NotHandled    = 0,  // not a bouncing trajectory; vanilla check runs
//       BouncedKeepFlying,  // reflection happened; bullet survives this tick
//       BouncedDetonate,    // last bounce consumed; detonate this tick
//   };
//
//   class PhobosTrajectory
//   {
//       // ... existing virtuals ...
//
//       virtual BounceCheckResult OnBounceCheck(CoordStruct const& /*impactCoord*/,
//                                                bool& /*force_detonate*/)
//       {
//           return BounceCheckResult::NotHandled;
//       }
//   };
//
// And in BulletTypeExt.h, add:
//   Valueable<bool> BounceOnTerrain { true };
//
// (Read in BulletTypeExt::Read with key "BounceOnTerrain", default true.)
// ============================================================================