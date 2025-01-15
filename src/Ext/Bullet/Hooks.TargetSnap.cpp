#include "Body.h"
#include "Trajectories/PhobosTrajectory.h"

DEFINE_HOOK(0x467CCA, BulletClass_AI_TargetSnapChecks, 0x6) //was C
{
	enum { SkipAirburstCheck = 0x467CDE, SkipSnapFunc = 0x467E53 };

	GET(FakeBulletClass*, pThis, EBP);

	retfunc_fixed nRet(R, SkipAirburstCheck , pThis->Type);

	// Do not require Airburst=no to check target snapping for Inviso / Trajectory=Straight projectiles
	if (pThis->Type->Inviso)
	{
		return nRet();
	}
	else
	{
		auto const pExt = pThis->_GetExtData();

		if (pExt->Trajectory && PhobosTrajectory::CanSnap(pExt->Trajectory))
		{
			return nRet();
		}
	}

	return 0;
}

DEFINE_HOOK(0x468E61, BulletClass_Explode_TargetSnapChecks1, 0x6) //was C
{
	enum { SkipAirburstChecks = 0x468E7B, SkipCoordFunc = 0x468E9F };

	GET(FakeBulletClass*, pThis, ESI);

	retfunc_fixed nRet(R, SkipAirburstChecks, pThis->Type);

	// Do not require Airburst=no to check target snapping for Inviso / Trajectory=Straight projectiles
	if (pThis->Type->Inviso)
	{
		return nRet();
	}
	else if (pThis->Type->Arcing || pThis->Type->ROT > 0)
	{
		return 0x0;
	}
	else
	{
		auto const pExt = pThis->_GetExtData();

		if (pExt->Trajectory
			&& PhobosTrajectory::CanSnap(pExt->Trajectory)
			&& !pExt->SnappedToTarget
		) {
			return nRet();
		}
	}

	return 0x0;
}

DEFINE_HOOK(0x468E9F, BulletClass_Explode_TargetSnapChecks2, 0x6) //was C
{
	enum { SkipInitialChecks = 0x468EC7, SkipSetCoordinate = 0x468F23 };

	GET(FakeBulletClass*, pThis, ESI);

	// Do not require EMEffect=no & Airburst=no to check target coordinate snapping for Inviso projectiles.
	if (pThis->Type->Inviso)
	{
		R->EAX(pThis->Type);
		return SkipInitialChecks;
	}
	else if (pThis->Type->Arcing || pThis->Type->ROT > 0)
	{
		return 0;
	}

	// Do not force Trajectory=Straight projectiles to detonate at target coordinates under certain circumstances.
	// Fixes issues with walls etc.
	auto const pExt = pThis->_GetExtData();
	if (pExt->Trajectory
		&& PhobosTrajectory::CanSnap(pExt->Trajectory)
		&& !pExt->SnappedToTarget)
	{
		return SkipSetCoordinate;
	}

	return 0;
}

DEFINE_HOOK(0x468D3F, BulletClass_ShouldExplode_AirTarget, 0x8)
{
	enum { DontExplode = 0x468D73 , Contine = 0x0 };

	GET(FakeBulletClass*, pThis, ESI);
	auto const pExt = pThis->_GetExtData();

	if (pExt->Trajectory && PhobosTrajectory::CanSnap(pExt->Trajectory)) {
		// Straight trajectory has its own proximity checks.
		return DontExplode;
	}

	return Contine;
}