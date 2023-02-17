#include "Body.h"

#ifdef COMPILE_PORTED_DP_FEATURES
#include <Misc/DynamicPatcher/Trails/TrailsManager.h>
#endif
/*
static CoordStruct GetFLHAbsoluteCoords(CoordStruct nFLH, CoordStruct nCurLoc)
{
	Matrix3D mtx;
	mtx.MakeIdentity();
	mtx.Translate((float)nFLH.X, (float)nFLH.Y, (float)nFLH.Z);
	Vector3D<float> result = Matrix3D::MatrixMultiply(mtx, Vector3D<float>::Empty);
	result.Y *= -1;
	nCurLoc += { (int)result.X, (int)result.Y, (int)result.Z };
	return nCurLoc;
}
*/
//#ifdef COMPILE_PORTED_DP_FEATURES
DEFINE_HOOK(0x62CE86, ParticleClass_AI, 0x7) // F , this is the end, here's the beginning: 0x62CE49 0x6
{
	GET(ParticleClass*, pThis, ESI);

	const auto pParticleExt = ParticleExt::ExtMap.Find(pThis);

	if (!pParticleExt)
		Debug::Log("Particle[%x - %s] , Without Ext , Returning ! \n", pThis, pThis->get_ID());

	// TODO: Check this - Morton
	// LaserTrails update routine is in BulletClass::AI hook because BulletClass::Draw
	// doesn't run when the object is off-screen which leads to visual bugs - Kerbiter

	if (pParticleExt->LaserTrails.size())
	{
		const CoordStruct location = pThis->GetCoords();
		const CoordStruct drawnCoords = location;
		for (auto const& trail : pParticleExt->LaserTrails)
		{
			// Left this here for now - Morton
			// We insert initial position so the first frame of trail doesn't get skipped - Kerbiter
			// TODO move hack to BulletClass creation
			if (!trail->LastLocation.isset())
				trail->LastLocation = location;

			//trail->Update(GetFLHAbsoluteCoords(trail->FLH, drawnCoords));
			trail->Update(drawnCoords);
			trail->Visible = pThis->IsOnMyView();
		}
	}
#ifdef COMPILE_PORTED_DP_FEATURES
	TrailsManager::AI(pThis);
#endif
	return 0;
}
//#endif