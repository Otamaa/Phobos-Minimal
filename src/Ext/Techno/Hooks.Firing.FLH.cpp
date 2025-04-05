 #include "Body.h"

#include <Ext/TechnoType/Body.h>

#include <InfantryClass.h>

CoordStruct GetFLHAbsoluteCoords(TechnoClass* pThis, const CoordStruct& pCoord, bool isOnTurret)
{
	auto const pType = pThis->GetTechnoType();
	Matrix3D mtx = TechnoExtData::GetTransform(pThis);
	auto pFoot = flag_cast_to<FootClass*>(pThis);

	// Steps 2-3: turret offset and rotation
	if (isOnTurret && pThis->HasTurret())
	{
		TechnoTypeExtContainer::Instance.Find(pType)->ApplyTurretOffset(&mtx, 1.0);
		double turretRad = pThis->TurretFacing().GetRadian<32>();
		double bodyRad = pThis->PrimaryFacing.Current().GetRadian<32>();
		// For BuildingClass turret facing is equal to primary facing

		float angle = pFoot ? (float)(turretRad - bodyRad) : (float)(turretRad);
		mtx.RotateZ(angle);
	}

	// Step 4: apply FLH offset
	mtx.Translate(static_cast<float>(pCoord.X), static_cast<float>(pCoord.Y), static_cast<float>(pCoord.Z));

	Vector3D<float> result {};
	Matrix3D::MatrixMultiply(&result , &mtx, &Vector3D<float>::Empty);
	// Resulting coords are mirrored along X axis, so we mirror it back
	result.Y *= -1;

	// Step 5: apply as an offset to global object coords
	CoordStruct location = pThis->GetRenderCoords();
	location += CoordStruct { static_cast<int>(result.X), static_cast<int>(result.Y), static_cast<int>(result.Z) };
	// += { std::lround(result.X), std::lround(result.Y), std::lround(result.Z) };

	return location;
}

ASMJIT_PATCH(0x6F3AEB, TechnoClass_GetFLH, 0x6)
{
	GET(TechnoClass*, pThis, EBX);
	GET(TechnoTypeClass*, pType, EAX);
	GET(int, weaponIndex, ESI);
	GET_STACK(CoordStruct*, pCoords, STACK_OFFSET(0xD8, 0x4));

	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	auto const pExt = TechnoExtContainer::Instance.Find(pThis);
	bool allowOnTurret = true;
	bool useBurstMirroring = true;
	CoordStruct flh = CoordStruct::Empty;

	if (weaponIndex >= 0)
	{
		auto[found , _flh] =
		!pExt->CustomFiringOffset.has_value() ?
	 	TechnoExtData::GetBurstFLH(pThis, weaponIndex) :
	 	std::make_pair(true , pExt->CustomFiringOffset.value());

		if (!found) {

			if (pThis->WhatAmI() == InfantryClass::AbsID) {
				auto res = TechnoExtData::GetInfantryFLH(reinterpret_cast<InfantryClass*>(pThis), weaponIndex);
				found = res.first;
				_flh = res.second;
			}
				
			if (!found) {
				_flh = pThis->GetWeapon(weaponIndex)->FLH;
			}

			flh = _flh;
		} else {
			useBurstMirroring = false;
		}
	}
	else
	{
		int index = -weaponIndex - 1;
		useBurstMirroring = false;

		if ((size_t)index < pTypeExt->AlternateFLHs.size())
			flh = pTypeExt->AlternateFLHs[index];

		if (!pTypeExt->AlternateFLH_OnTurret)
			allowOnTurret = false;
	}

	if (useBurstMirroring && pThis->CurrentBurstIndex % 2 != 0)
		flh.Y = -flh.Y;

	*pCoords = GetFLHAbsoluteCoords(pThis, flh, allowOnTurret);
	R->EAX(pCoords);

	return 0x6F3D50;
}
