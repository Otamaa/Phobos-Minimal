#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

#include <HoverLocomotionClass.h>

#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WarheadType/Body.h>

#include <WWKeyboardClass.h>
#include <Conversions.h>

#include <Misc/AresData.h>

DEFINE_OVERRIDE_HOOK(0x5F8277, ObjectTypeClass_Load3DArt_NoSpawnAlt1, 7)
{
	REF_STACK(bool, bLoadFailed, 0x13);
	GET(ObjectTypeClass*, pThis, ESI);

	if (!pThis || 
		(!Is_AircraftType(pThis) && 
			!Is_UnitType(pThis) && 
			!Is_InfantryType(pThis) && 
			!Is_BuildingType(pThis))
		)
		return 0x5F8640;

	TechnoTypeClass* const pType = reinterpret_cast<TechnoTypeClass*>(pThis);

	if (pType->NoSpawnAlt)
	{
		auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

		char Buffer[0x40];
		auto pKey = "%sWO";
		IMPL_SNPRNINTF(Buffer, sizeof(Buffer), pKey, pThis->ImageFile);
		ImageStatusses nPairStatus {};
		ImageStatusses::ReadVoxel(nPairStatus, Buffer, 1);

		if (pTypeExt->SpawnAltData.VXL != nPairStatus.Images.VXL)
		{
			std::swap(pTypeExt->SpawnAltData.VXL, nPairStatus.Images.VXL);
		}

		if (pTypeExt->SpawnAltData.HVA != nPairStatus.Images.HVA)
		{
			std::swap(pTypeExt->SpawnAltData.HVA, nPairStatus.Images.HVA);
		}

		//clean up
		GameDelete<true, true>(nPairStatus.Images.HVA);
		GameDelete<true, true>(nPairStatus.Images.VXL);

		if (!nPairStatus.Loaded)
		{
			Debug::Log("%s Techno NoSpawnAlt Image[%s] cannot be loaded ,returning load failed ! \n", pThis->ID, Buffer);
			bLoadFailed = true;
		}
	}

	return 0x5F8287;
}

DEFINE_OVERRIDE_SKIP_HOOK(0x5F848C, ObjectTypeClass_Load3DArt_NoSpawnAlt2, 6, 5F8844)

DEFINE_OVERRIDE_HOOK(0x5F887B, ObjectTypeClass_Load3DArt_Barrels, 6)
{
	GET(TechnoTypeClass*, pThis, ESI);

	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis);

	const auto nRemaining = pThis->TurretCount - TechnoTypeClass::MaxWeapons;
	if (nRemaining > 0)
	{
		TechnoTypeExt::ClearImageData(pTypeExt->BarrelImageData);

		if (nRemaining > pTypeExt->BarrelImageData.size())
			pTypeExt->BarrelImageData.resize((size_t)nRemaining + 2);
	}

	if (pThis->TurretCount <= 0)
		return 0x5F8A60;

	char Buffer[0x40];
	for (int i = 0; ; ++i)
	{
		const auto pKey = !i ? "%sBARL" : "%sBARL%d";
		_snprintf(Buffer, sizeof(Buffer), pKey, pThis->ImageFile, i);

		//if (i > (pTypeExt->TurretImageData.size() + TechnoTypeClass::MaxWeapons)) {
		//	Debug::Log("Reading Barrel [%d] for [%s] Which is More than array size ! \n", i, pThis->ImageFile);
		//	return 0x5F8844;
		//}

		auto& nArr = i >= TechnoTypeClass::MaxWeapons ?
			pTypeExt->BarrelImageData[i - TechnoTypeClass::MaxWeapons] : pThis->ChargerBarrels[i];

		ImageStatusses nPairStatus {};
		ImageStatusses::ReadVoxel(nPairStatus, Buffer, 1);

		if (nArr.VXL != nPairStatus.Images.VXL)
		{
			std::swap(nArr.VXL, nPairStatus.Images.VXL);
		}

		if (nArr.HVA != nPairStatus.Images.HVA)
		{
			std::swap(nArr.HVA, nPairStatus.Images.HVA);
		}

		//clean up
		GameDelete<true, true>(nPairStatus.Images.HVA);
		GameDelete<true, true>(nPairStatus.Images.VXL);

		if (!nPairStatus.Loaded)
		{
			Debug::Log("%s Techno Barrel [%s] at[%d] cannot be loaded , breaking the loop ! \n", pThis->ID, Buffer, i);
			break;
		}

		if (i >= pThis->TurretCount)
			return 0x5F8A60;
	}

	return 0x5F8A6A;
}

DEFINE_OVERRIDE_HOOK(0x5F865F, ObjectTypeClass_Load3DArt_Turrets, 6)
{
	GET(TechnoTypeClass*, pThis, ESI);

	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis);

	const auto nRemaining = pThis->TurretCount - TechnoTypeClass::MaxWeapons;
	if (nRemaining > 0)
	{
		TechnoTypeExt::ClearImageData(pTypeExt->BarrelImageData);

		if (nRemaining > pTypeExt->TurretImageData.size())
			pTypeExt->TurretImageData.resize((size_t)nRemaining + 2);
	}

	if (pThis->TurretCount <= 0)
		return 0x5F8844;

	char Buffer[0x40];
	for (int i = 0; ; ++i)
	{
		const auto pKey = !i ? "%sTUR" : "%sTUR%d";
		_snprintf(Buffer, sizeof(Buffer), pKey, pThis->ImageFile, i);

		//if (i > (pTypeExt->TurretImageData.size() + TechnoTypeClass::MaxWeapons)) {
		//	Debug::Log("Reading Turrent [%d] for [%s] Which is More than array size ! \n", i, pThis->ImageFile);
		//	return 0x5F8844;
		//}

		auto& nArr = i >= TechnoTypeClass::MaxWeapons ?
			pTypeExt->TurretImageData[i - TechnoTypeClass::MaxWeapons] : pThis->ChargerTurrets[i];

		ImageStatusses nPairStatus {};
		ImageStatusses::ReadVoxel(nPairStatus, Buffer, 1);

		if (nArr.VXL != nPairStatus.Images.VXL)
		{
			std::swap(nArr.VXL, nPairStatus.Images.VXL);
		}

		if (nArr.HVA != nPairStatus.Images.HVA)
		{
			std::swap(nArr.HVA, nPairStatus.Images.HVA);
		}

		//clean up
		GameDelete<true, true>(nPairStatus.Images.HVA);
		GameDelete<true, true>(nPairStatus.Images.VXL);

		if (!nPairStatus.Loaded)
		{
			Debug::Log("%s Techno Turret [%s] at[%d] cannot be loaded , breaking the loop ! \n", pThis->ID, Buffer, i);
			break;
		}

		if (i >= pThis->TurretCount)
			return 0x5F8844;
	}

	return 0x5F868C;
}

DEFINE_OVERRIDE_HOOK(0x73B90E, UnitClass_DrawVXL_Barrels1, 7)
{
	GET(UnitTypeClass*, pUnit, EBX);
	GET(int, nIdx, EAX);

	R->Stack(0x2C, R->ESI());
	const auto pData = TechnoTypeExt::GetBarrelsVoxelData(pUnit, nIdx);
	return (!pData->VXL || !pData->HVA) ? 0x73B94A : 0x73B928;
}

DEFINE_OVERRIDE_HOOK(0x73BCCD, UnitClass_DrawVXL_Barrels2, 7)
{
	GET(UnitTypeClass*, pUnit, EBX);
	GET(int, nIdx, ECX);
	R->EDX(TechnoTypeExt::GetBarrelsVoxelData(pUnit, nIdx));
	return 0x73BCD4;
}

DEFINE_OVERRIDE_HOOK(0x73BD6A, UnitClass_DrawVXL_Barrels3, 7)
{
	GET(UnitTypeClass*, pUnit, EBX);
	GET(int, nIdx, ESI);
	R->ECX(TechnoTypeExt::GetBarrelsVoxelData(pUnit, nIdx));
	return 0x73BD71;
}

DEFINE_OVERRIDE_HOOK(0x73BD15, UnitClass_DrawVXL_Turrets, 7)
{
	GET(UnitTypeClass*, pUnit, EBX);
	GET(int, nIdx, ESI);
	R->ECX(TechnoTypeExt::GetTurretVoxelData(pUnit, nIdx));
	return 0x73BD1C;
}

DEFINE_OVERRIDE_HOOK(0x5F8084, ObjectTypeClass_UnloadTurretArt, 6)
{
	GET(ObjectTypeClass*, pThis, ESI);

	if (!pThis || (!Is_AircraftType(pThis) && !Is_UnitType(pThis) && !Is_InfantryType(pThis) && !Is_BuildingType(pThis)))
		return 0;

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(static_cast<TechnoTypeClass*>(pThis));
	TechnoTypeExt::ClearImageData(pTypeExt->BarrelImageData);
	TechnoTypeExt::ClearImageData(pTypeExt->TurretImageData);

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x73B6E3, UnitClass_DrawVXL_NoSpawnAlt, 6)
{
	GET(UnitTypeClass*, pType, EBX);
	R->EDX(&TechnoTypeExt::ExtMap.Find(pType)->SpawnAltData);
	return 0x73B6E9;
}
