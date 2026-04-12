#include "BounceTrajectory.h"

#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>

#include <WeaponTypeClass.h>

namespace Utils
{
	void DetonateBullet(WeaponTypeClass* pWeapon, BulletClass* pFrom, CoordStruct const& nCoord)
	{
		CoordStruct coords = nCoord;
		auto payback = pFrom->Owner;

		if (!flag_cast_to<TechnoClass*>(pFrom->Owner))
		{
			payback = nullptr;
		}

		if (auto v9 = pWeapon->Projectile->CreateBullet(pFrom->GetCell(), payback, pWeapon->Damage, pWeapon->Warhead, 0, pWeapon->Bright))
		{
			v9->WeaponType = pWeapon;
			if (pWeapon->Projectile->ShrapnelWeapon)
			{
				v9->SetLocation(coords);
			}

			v9->Limbo();
			v9->Detonate(coords);
			v9->UnInit();
		}
	}
}


bool BounceTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return PhobosTrajectoryType::Load(Stm, RegisterForChange) && Serialize(Stm);
}

bool BounceTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	return PhobosTrajectoryType::Save(Stm) 
 	&& const_cast<BounceTrajectoryType*>(this)->Serialize(Stm);
}

bool BounceTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	if (!this->PhobosTrajectoryType::Read(pINI, pSection))
		return false;

	INI_EX exINI { pINI };

  Valueable<int> nBounceAmount { 0 };
    nBounceAmount.Read(exINI, pSection, "Trajectory.Bounce.Amount");
    this->BounceAmount = Math::abs(nBounceAmount.Get());
 
    this->BounceWeapon.Read(exINI, pSection, "Trajectory.Bounce.Weapon");
    this->BounceMinVelocity.Read(exINI, pSection, "Trajectory.Bounce.MinVelocity");
    this->BounceOnTerrain.Read(exINI, pSection, "Trajectory.Bounce.OnTerrain");	
	
	return true;
}

bool BounceTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
    return PhobosTrajectory::Load(Stm, RegisterForChange) &&
        Stm.Process(this->IsBouncing, false)
           .Process(this->BounceLeft, false)
           .Process(this->LastObject, true);
}
 
bool BounceTrajectory::Save(PhobosStreamWriter& Stm) const
{
    return PhobosTrajectory::Save(Stm) &&
        Stm.Process(this->IsBouncing, false)
           .Process(this->BounceLeft, false)
           .Process(this->LastObject, true);
}

void BounceTrajectory::InvalidatePointer(AbstractClass* ptr, bool bRemoved)
{
    if (this->LastObject == ptr && bRemoved)
        this->LastObject = nullptr;
}

// Do some math here to set the initial speed of your proj
// Also set some needed properties here
void BounceTrajectory::OnUnlimbo(CoordStruct* pCoord, VelocityClass* pVelocity)
{
    auto const pType   = this->GetTrajectoryType();
    auto const pBullet = this->AttachedTo;
 
    this->SetInaccurate();
    this->DetonationDistance = pType->DetonationDistance.Get(Leptons(102));
    this->BounceLeft         = pType->BounceAmount;
    this->IsBouncing         = false;
    this->LastObject         = nullptr;
 
    // Normalize the launch velocity to the trajectory's configured speed.
    // The weapon's launch math has already aimed it at the target — we just
    // rescale to the right magnitude.
    double velocityLength = pBullet->Velocity.Length();
    if (velocityLength > 1e-10)
    {
        pBullet->Velocity *= (this->GetTrajectorySpeed() / velocityLength);
    }
    else
    {
        // Degenerate launch — pick an arbitrary forward direction
        pBullet->Velocity.X = this->GetTrajectorySpeed();
        pBullet->Velocity.Y = 0.0;
        pBullet->Velocity.Z = 0.0;
    }
}

// Some early checks on each game frame here.
// Return true to detonate the bullet immediately afterwards.
bool BounceTrajectory::OnAI() { return false; }

void BounceTrajectory::OnAIPreDetonate() { }

// Where you update the speed and position
// pSpeed: The speed of this proj in the next frame
// pPosition: Current position of the proj, and in the next frame it will be *pSpeed + *pPosition
void BounceTrajectory::OnAIVelocity(VelocityClass* pSpeed, VelocityClass* pPosition) { }

// Where additional checks based on bullet reaching its target coordinate can be done.
// Vanilla code will do additional checks regarding buildings on target coordinate and Vertical projectiles and will detonate the projectile if they pass.
// Return value determines what is done regards to the game checks: they can be skipped, executed as normal or treated as if the condition is already satisfied.
TrajectoryCheckReturnType BounceTrajectory::OnAITargetCoordCheck(CoordStruct& coords)
{
	 auto const pType = this->GetTrajectoryType();
 
    // If the bullet still has bounces left (or hasn't started bouncing yet),
    // skip the target-coord auto-detonation. Once bounces are exhausted, fall
    // through to vanilla behavior so the bullet can detonate normally.
    if (pType->BounceAmount > 0 && (!this->IsBouncing || this->BounceLeft > 0))
        return TrajectoryCheckReturnType::SkipGameCheck;
 
    return TrajectoryCheckReturnType::ExecuteGameCheck;
}

// Where additional checks based on a TechnoClass instance in same cell as the bullet can be done.
// Vanilla code will do additional trajectory alterations here if there is an enemy techno in the cell.
// Return value determines what is done regards to the game checks: they can be skipped, executed as normal or treated as if the condition is already satisfied.
// pTechno: TechnoClass instance in same cell as the bullet.
TrajectoryCheckReturnType BounceTrajectory::OnAITechnoCheck(TechnoClass* pTechno)
{
	   auto const pType = this->GetTrajectoryType();
 
    // Same gating: don't auto-detonate on enemy techno collision until bounces
    // are exhausted. The actual reflection happens in OnBounceCheck.
    if (pType->BounceAmount > 0 && (!this->IsBouncing || this->BounceLeft > 0))
        return TrajectoryCheckReturnType::SkipGameCheck;
 
    return TrajectoryCheckReturnType::ExecuteGameCheck;
}

// ============================================================================
// OnBounceCheck — the main reflection entry point
//
// Called from FinalizeBulletMotion before the explode flag is committed.
// Looks up the obstacle at the impact coord, decides whether to bounce or
// detonate, and applies reflection if bouncing.
// ============================================================================
 
BounceCheckResult BounceTrajectory::OnBounceCheck(CoordStruct const& impactCoord,
                                                    bool& force_detonate)
{
    auto const pBullet   = this->AttachedTo;
    auto const pType     = this->GetTrajectoryType();
    auto const pBulletType = pBullet->Type;
 
    if (pType->BounceAmount <= 0)
        return BounceCheckResult::NotHandled;
 
    // Velocity decay termination — too slow to keep bouncing
    if (pBullet->Velocity.Length() < pType->BounceMinVelocity.Get())
    {
        force_detonate = true;
        return BounceCheckResult::BouncedDetonate;
    }
 
    // Out-of-map termination
    if (!MapClass::Instance->IsValid(impactCoord))
    {
        force_detonate = true;
        return BounceCheckResult::BouncedDetonate;
    }
 
    auto pCell = MapClass::Instance->GetCellAt(impactCoord);
    if (!pCell)
    {
        force_detonate = true;
        return BounceCheckResult::BouncedDetonate;
    }
 
    bool bAlt = (static_cast<unsigned int>(pCell->Flags) >> 8) & 1;
    auto pObj = pCell->FindTechnoNearestTo(
        Point2D { impactCoord.X, impactCoord.Y }, bAlt, nullptr);
 
    return TryBounce(impactCoord, pObj, force_detonate);
}
 
// ============================================================================
// TryBounce — does the actual bounce/detonate decision
// ============================================================================
 
BounceCheckResult BounceTrajectory::TryBounce(CoordStruct const& impactCoord,
                                                AbstractClass* pObstacle,
                                                bool& force_detonate)
{
    auto const pBullet     = this->AttachedTo;
    auto const pType       = this->GetTrajectoryType();
    auto const pBulletType = pBullet->Type;
    auto const pBulletExt  = BulletTypeExtContainer::Instance.Find(pBulletType);
 
    // ------------------------------------------------------------------------
    // No obstacle case — bullet hit the ground or empty terrain.
    // Anti-double-bounce: if we just bounced off the same object (or absence
    // of object) last tick, treat this as a real impact and detonate.
    // ------------------------------------------------------------------------
    if (!pObstacle)
    {
        if (this->LastObject == nullptr && this->IsBouncing)
        {
            // Already bouncing on empty terrain — count down
            if (--this->BounceLeft <= 0)
            {
                force_detonate = true;
                return BounceCheckResult::BouncedDetonate;
            }
        }
 
        this->LastObject = nullptr;
 
        // Floor bounce — reflect Z velocity and apply elasticity
        ReflectVelocityOffFloor();
        pBullet->Velocity *= pBulletType->Elasticity;
 
        // Optional bounce weapon
        if (auto pWeapon = pType->BounceWeapon.Get())
            Utils::DetonateBullet(pWeapon, pBullet, impactCoord);
 
        // First bounce is free (initialize counter)
        if (!this->IsBouncing)
        {
            this->BounceLeft = pType->BounceAmount;
            this->IsBouncing = true;
        }
 
        return BounceCheckResult::BouncedKeepFlying;
    }
 
    // ------------------------------------------------------------------------
    // Object case — check terrain filtering and proximity gates
    // ------------------------------------------------------------------------
    bool is_terrain = (pObstacle->WhatAmI() == AbstractType::Terrain);
    bool is_building = (pObstacle->WhatAmI() == AbstractType::Building);
	auto pObject = flag_cast_to<ObjectClass*>(pObstacle);

    // Skip terrain bounces if disabled by BulletType
    if (is_terrain && !pBulletExt->BounceOnTerrain.Get())
    {
        if (this->IsBouncing && --this->BounceLeft <= 0)
        {
            force_detonate = true;
            return BounceCheckResult::BouncedDetonate;
        }
        return BounceCheckResult::NotHandled;
    }
 
    // Proximity gate — only bounce if we're actually close to the object
    if (!is_building && pObject && impactCoord.DistanceFrom(pObject->Location) >= 128.0)
    {
        if (this->IsBouncing && --this->BounceLeft <= 0)
        {
            force_detonate = true;
            return BounceCheckResult::BouncedDetonate;
        }
        return BounceCheckResult::NotHandled;
    }
 
    // ------------------------------------------------------------------------
    // Anti-double-bounce: don't bounce twice off the same object back-to-back.
    // The original hook had this inverted — fixed here.
    // ------------------------------------------------------------------------
    if (this->LastObject == pObstacle)
    {
        // Same object as last tick — treat as stuck, force detonate
        if (this->IsBouncing && --this->BounceLeft <= 0)
        {
            force_detonate = true;
            return BounceCheckResult::BouncedDetonate;
        }
        // Even if bounces remain, refuse to bounce off the same object again
        return BounceCheckResult::NotHandled;
    }
 
    this->LastObject = pObstacle;
 
    // ------------------------------------------------------------------------
    // Apply reflection
    // ------------------------------------------------------------------------
    ReflectVelocityOff(pObstacle, impactCoord);
    pBullet->Velocity *= pBulletType->Elasticity;
 
    // Optional bounce weapon at impact point
    if (auto pWeapon = pType->BounceWeapon.Get())
        Utils::DetonateBullet(pWeapon, pBullet, impactCoord);
 
    // First bounce is free (initialize counter)
    if (!this->IsBouncing)
    {
        this->BounceLeft = pType->BounceAmount;
        this->IsBouncing = true;
        return BounceCheckResult::BouncedKeepFlying;
    }
 
    if (this->BounceLeft > 0)
        return BounceCheckResult::BouncedKeepFlying;
 
    // Bounces exhausted
    force_detonate = true;
    return BounceCheckResult::BouncedDetonate;
}
 
// ============================================================================
// ReflectVelocityOff — proper face-normal reflection
//
// Determines which face of the obstacle was hit by comparing the impact point
// to the obstacle's bounding box, then reflects the velocity component along
// the surface normal (the standard r = v - 2(v·n)n formula, simplified for
// axis-aligned faces).
//
// For buildings, uses the foundation footprint as the bounding box.
// For other techno, uses a fixed 128-lepton box around the object center.
// ============================================================================
 
void BounceTrajectory::ReflectVelocityOff(AbstractClass* pObstacle,
                                            CoordStruct const& impactCoord)
{
    auto const pBullet = this->AttachedTo;
 
    // Determine the obstacle's bounding box (axis-aligned)
    int min_x, max_x, min_y, max_y;
 
    if (auto pBuilding = cast_to<BuildingClass*>(pObstacle)) {
        auto bldCoord = pBuilding->GetCoords();
        auto pBldType = pBuilding->Type;
        int width  = pBldType->GetFoundationWidth()  * 256;
        int height = pBldType->GetFoundationHeight(false) * 256;
        min_x = bldCoord.X;
        max_x = bldCoord.X + width;
        min_y = bldCoord.Y;
        max_y = bldCoord.Y + height;
    } else if(auto pObject = flag_cast_to<ObjectClass*>(pObstacle)) {
        // Generic 128-lepton box around object center
        auto loc = pObject->Location;
        min_x = loc.X - 128;
        max_x = loc.X + 128;
        min_y = loc.Y - 128;
        max_y = loc.Y + 128;
    } else{
		return;
	}
 
    // Compute penetration depth on each axis (how far inside the box the
    // impact is from each face). The face with the smallest penetration is
    // the one we hit.
    int pen_left   = impactCoord.X - min_x;       // distance from left face
    int pen_right  = max_x - impactCoord.X;       // distance from right face
    int pen_bottom = impactCoord.Y - min_y;       // distance from bottom face
    int pen_top    = max_y - impactCoord.Y;       // distance from top face
 
    // Find the smallest positive penetration
    int min_pen = pen_left;
    char hit_face = 'L';
 
    if (pen_right < min_pen)  { min_pen = pen_right;  hit_face = 'R'; }
    if (pen_bottom < min_pen) { min_pen = pen_bottom; hit_face = 'B'; }
    if (pen_top < min_pen)    { min_pen = pen_top;    hit_face = 'T'; }
 
    // Reflect the velocity component along the face normal. For axis-aligned
    // boxes this is just a sign flip on the appropriate component, BUT only
    // if the bullet is actually moving INTO the face (sign check on velocity).
    switch (hit_face)
    {
    case 'L':
        // Hit left face (normal = -X). Reflect X if moving in +X direction.
        if (pBullet->Velocity.X > 0.0)
            pBullet->Velocity.X = -pBullet->Velocity.X;
        break;
    case 'R':
        // Hit right face (normal = +X). Reflect X if moving in -X direction.
        if (pBullet->Velocity.X < 0.0)
            pBullet->Velocity.X = -pBullet->Velocity.X;
        break;
    case 'B':
        // Hit bottom face (normal = -Y). Reflect Y if moving in +Y direction.
        if (pBullet->Velocity.Y > 0.0)
            pBullet->Velocity.Y = -pBullet->Velocity.Y;
        break;
    case 'T':
        // Hit top face (normal = +Y). Reflect Y if moving in -Y direction.
        if (pBullet->Velocity.Y < 0.0)
            pBullet->Velocity.Y = -pBullet->Velocity.Y;
        break;
    }
 
    // Always reflect Z component on impact (bullet bounces upward off
    // anything it hits). This makes the bounce visible vertically.
    if (pBullet->Velocity.Z < 0.0)
        pBullet->Velocity.Z = -pBullet->Velocity.Z * 0.5;
}
 
// ============================================================================
// ReflectVelocityOffFloor — Z-only reflection for ground hits
// ============================================================================
 
void BounceTrajectory::ReflectVelocityOffFloor()
{
    auto const pBullet = this->AttachedTo;
    if (pBullet->Velocity.Z < 0.0)
        pBullet->Velocity.Z = -pBullet->Velocity.Z;
}
