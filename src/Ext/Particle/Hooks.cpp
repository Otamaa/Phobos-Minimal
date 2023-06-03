#include "Body.h"

#include <Misc/DynamicPatcher/Trails/TrailsManager.h>

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


DEFINE_HOOK(0x62CE86, ParticleClass_AI, 0x7) // F , this is the end, here's the beginning: 0x62CE49 0x6
{
	GET(ParticleClass*, pThis, ESI);

	const auto pParticleExt = ParticleExt::ExtMap.TryFind(pThis);

	if (!pParticleExt) {
		Debug::Log("Particle[%x - %s] , Without Ext , Returning ! \n", pThis, pThis->get_ID());
		return 0;
	}

	if (!pParticleExt->LaserTrails.empty())
	{
		const CoordStruct location = pThis->GetCoords();
		const CoordStruct drawnCoords = location;
		for (auto& trail : pParticleExt->LaserTrails)
		{
			if (!trail.LastLocation.isset())
				trail.LastLocation = location;

			//trail->Update(GetFLHAbsoluteCoords(trail->FLH, drawnCoords));
			trail.Update(drawnCoords);
			trail.Visible = pThis->IsOnMyView();
		}
	}

	TrailsManager::AI(pThis);

	return 0;
}
//#endif