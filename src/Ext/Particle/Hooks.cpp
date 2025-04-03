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


ASMJIT_PATCH(0x62CE86, ParticleClass_AI, 0x7) // F , this is the end, here's the beginning: 0x62CE49 0x6
{
	GET(FakeParticleClass*, pThis, ESI);

	const auto pParticleExt = pThis->_GetExtData();

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

	TrailsManager::AI(pThis->_AsParticle());

	return 0;
}
//#endif

ASMJIT_PATCH(0x62D301, ParticleClass_SmokeDirection_AI_WinDirMult, 0x6)
{
	GET(int, facing, EAX);
	GET(FakeParticleClass*, pThis, ESI);
	const auto& mult = pThis->_GetTypeExtData()->WindMult[facing];
	R->ECX(mult.X);
	R->EAX(mult.Y);
	return 0x62D30D;
}

ASMJIT_PATCH(0x62D44A, ParticleClass_GasDirection_AI_WinDirMult, 0x6)
{
	GET(int, facing, EAX);
	GET(FakeParticleClass*, pThis, ESI);
	const auto& mult = pThis->_GetTypeExtData()->WindMult[facing];
	R->ECX(mult.X);
	R->EAX(mult.Y);
	return 0x62D456;
}

//ASMJIT_PATCH(0x62BD69, ParticleClass_ProcessGasBehaviour_NoWind, 6)
//{
//	GET(ParticleClass* const, pThis, EBP);
//
//	return pThis->Type->WindEffect == -1
//		? 0x62C200 : 0x0;
//}
//
//ASMJIT_PATCH(0x62C361, ParticleClass_ProcessGasBehaviour_DisOnWater, 6)
//{
//	GET(ParticleClass*, pThis, EBP);
//	GET(ParticleTypeClass* const, pType, EDI);
//
//	const auto pTypeExt = ParticleTypeExtContainer::Instance.Find(pType);
//
//	if (pType->WindEffect == -1)
//	{
//		const auto pCell = pThis->GetCell();
//		if (pCell->ContainsBridge() || (pCell->LandType != LandType::Beach && pCell->LandType != LandType::Water))
//			return 0;
//	}
//	else
//	{
//
//		if (!pTypeExt->DeleteWhenReachWater)
//			return 0;
//
//		const auto pCell = pThis->GetCell();
//		if (pCell->ContainsBridge() || (pCell->LandType != LandType::Beach && pCell->LandType != LandType::Water))
//			return 0;
//	}
//
//	pThis->hasremaining = 1;
//	//GameDelete<true,false>(pThis);
//	pThis->UnInit();
//	return 0x62C394;
//}
//
ASMJIT_PATCH(0x62BE2B, ParticleClass_Gas_AI_DriftSpeed, 0x5)
{

	GET(FakeParticleClass*, pParticle, EBP);
	GET(int, value_X, EBX);
	GET(int, value_Y, EDI);
	const auto pExt = pParticle->_GetTypeExtData();
	const auto maxDriftSpeedX = &pExt->Gas_DriftSpeedX.Get();
	const auto minDriftSpeedY = &pExt->Gas_DriftSpeedY.Get();

	if (value_X >= maxDriftSpeedX->Y) {
		if (value_X > maxDriftSpeedX->X) {
			pParticle->GasVelocity.X = maxDriftSpeedX->X;
		}
	}
	else {
		pParticle->GasVelocity.X = maxDriftSpeedX->Y;
	}

	if (value_Y >= minDriftSpeedY->Y) {
		if (value_Y > minDriftSpeedY->X) {
			pParticle->GasVelocity.X = minDriftSpeedY->X;
		}
	} else {
		pParticle->GasVelocity.X = minDriftSpeedY->Y;
	}

	return 0x62BE60;
}

//ASMJIT_PATCH(0x62CD2F, ParticleClass_Update_Fire, 8)
//{
//
//}
