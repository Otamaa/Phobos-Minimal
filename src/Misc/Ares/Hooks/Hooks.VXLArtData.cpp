#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>

#include "Header.h"

#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

#include <Utilities/Cast.h>

#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WarheadType/Body.h>

#include <WWKeyboardClass.h>
#include <Conversions.h>

DEFINE_HOOK(0x5F8277, ObjectTypeClass_Load3DArt_NoSpawnAlt1, 7)
{
	REF_STACK(bool, bLoadFailed, 0x13);
	GET(ObjectTypeClass*, pThis, ESI);

	const auto pType = specific_cast<UnitTypeClass*>(pThis);

	if (!pType)
		return 0x5F8640;

	if (pType->NoSpawnAlt)
	{
		auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

		std::string _buffer = pThis->ImageFile;
		_buffer += "WO";
		ImageStatusses nPairStatus = ImageStatusses::ReadVoxel(_buffer.c_str(), 1);
		nPairStatus.swap(pTypeExt->SpawnAltData);

		if (!nPairStatus.Loaded)
		{
			Debug::Log("%s Techno NoSpawnAlt Image[%s] cannot be loaded ,returning load failed ! \n", pThis->ID, _buffer.c_str());
			bLoadFailed = true;
		}
	}

	return 0x5F8287;
}

//ObjectTypeClass_Load3DArt_NoSpawnAlt2
DEFINE_JUMP(LJMP, 0x5F848C, 0x5F8844);

DEFINE_HOOK(0x5F887B, ObjectTypeClass_Load3DArt_Barrels, 6)
{
	GET(TechnoTypeClass*, pThis, ESI);

	auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis);

	const int nRemaining = (pThis->TurretCount - TechnoTypeClass::MaxWeapons) < 0 ?
		0 : (pThis->TurretCount - TechnoTypeClass::MaxWeapons);

	pTypeExt->BarrelImageData.resize(nRemaining);

	if (pThis->TurretCount <= 0)
		return 0x5F8A60;

	for (int i = 0; ; ++i)
	{
		if (i >= pThis->TurretCount)
			return 0x5F8A60;

		std::string _buffer = pThis->ImageFile;
		_buffer += !i ? "BARL" : (std::string("BARL") + std::to_string(i));

		//if (i > (pTypeExt->TurretImageData.size() + TechnoTypeClass::MaxWeapons)) {
		//	Debug::Log("Reading Barrel [%d] for [%s] Which is More than array size ! \n", i, pThis->ImageFile);
		//	return 0x5F8844;
		//}

		auto &nArr = i < TechnoTypeClass::MaxWeapons ?
			pThis->ChargerBarrels[i] :
			pTypeExt->BarrelImageData[i - TechnoTypeClass::MaxWeapons];

		ImageStatusses nPairStatus = ImageStatusses::ReadVoxel(_buffer.c_str(), 1);
		nPairStatus.swap(nArr);

		if (!nPairStatus.Loaded)
		{
			Debug::Log("%s Techno Barrel [%s] at[%d] cannot be loaded , breaking the loop ! \n", pThis->ID, _buffer.c_str(), i);
			break;
		}
	}

	return 0x5F8A6A;
}

DEFINE_HOOK(0x5F865F, ObjectTypeClass_Load3DArt_Turrets, 6)
{
	GET(TechnoTypeClass*, pThis, ESI);

	auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis);

	const int nRemaining = (pThis->TurretCount - TechnoTypeClass::MaxWeapons) < 0 ?
		0 : (pThis->TurretCount - TechnoTypeClass::MaxWeapons);

	pTypeExt->TurretImageData.resize(nRemaining);

	if (pThis->TurretCount <= 0)
		return 0x5F8844;

	for (int i = 0; ; ++i)
	{
		if (i >= pThis->TurretCount)
			return 0x5F8844;

		std::string _buffer = pThis->ImageFile;
		_buffer += !i ? "TUR" : (std::string("TUR") + std::to_string(i));

		//if (i > (pTypeExt->TurretImageData.size() + TechnoTypeClass::MaxWeapons)) {
		//	Debug::Log("Reading Turrent [%d] for [%s] Which is More than array size ! \n", i, pThis->ImageFile);
		//	return 0x5F8844;
		//}

		auto& nArr = i < TechnoTypeClass::MaxWeapons ?
			pThis->ChargerTurrets[i] :
			pTypeExt->TurretImageData[i - TechnoTypeClass::MaxWeapons];
			;

		ImageStatusses nPairStatus = ImageStatusses::ReadVoxel(_buffer.c_str(), 1);
		nPairStatus.swap(nArr);

		if (!nPairStatus.Loaded)
		{
			Debug::Log("%s Techno Turret [%s] at[%d] cannot be loaded , breaking the loop ! \n", pThis->ID, _buffer.c_str(), i);
			break;
		}

	}

	return 0x5F868C;
}

DEFINE_HOOK(0x73B90E, UnitClass_DrawVXL_Barrels1, 7)
{
	GET(UnitTypeClass*, pUnit, EBX);
	GET(int, nIdx, EAX);

	R->Stack(0x2C, R->ESI());
	const auto pData = TechnoTypeExtData::GetBarrelsVoxel(pUnit, nIdx);
	return (!pData->VXL || !pData->HVA) ? 0x73B94A : 0x73B928;
}

DEFINE_HOOK(0x73BCCD, UnitClass_DrawVXL_Barrels2, 7)
{
	GET(UnitTypeClass*, pUnit, EBX);
	GET(int, nIdx, ECX);
	R->EDX(TechnoTypeExtData::GetBarrelsVoxel(pUnit, nIdx));
	return 0x73BCD4;
}

DEFINE_HOOK(0x73BD6A, UnitClass_DrawVXL_Barrels3, 7)
{
	GET(UnitTypeClass*, pUnit, EBX);
	GET(int, nIdx, ESI);
	R->ECX(TechnoTypeExtData::GetBarrelsVoxel(pUnit, nIdx));
	return 0x73BD71;
}

DEFINE_HOOK(0x73BD15, UnitClass_DrawVXL_Turrets, 7)
{
	GET(UnitTypeClass*, pUnit, EBX);
	GET(int, nIdx, ESI);
	R->ECX(TechnoTypeExtData::GetTurretsVoxel(pUnit, nIdx));
	return 0x73BD1C;
}

DEFINE_HOOK(0x5F8084, ObjectTypeClass_UnloadTurretArt, 6)
{
	GET(ObjectTypeClass*, pThis, ECX);

	const auto pThisTech = VTable::Get(pThis);

	if (!pThis ||
		pThisTech != UnitTypeClass::vtable
		&& pThisTech != AircraftTypeClass::vtable
		&& pThisTech != BuildingTypeClass::vtable
		&& pThisTech != InfantryTypeClass::vtable
		)
		return 0;

	auto pTypeExt = TechnoTypeExtContainer::Instance.Find((TechnoTypeClass*)pThis);

	pTypeExt->BarrelImageData.clear();
	pTypeExt->TurretImageData.clear();

	return 0;
}

DEFINE_HOOK(0x73B6E3, UnitClass_DrawVXL_NoSpawnAlt, 6)
{
	GET(UnitTypeClass*, pType, EBX);
	R->EDX(&TechnoTypeExtContainer::Instance.Find(pType)->SpawnAltData);
	return 0x73B6E9;
}

#ifdef FUCKEDUP
DEFINE_HOOK(0x4DB157, FootClass_DrawVoxelShadow_TurretShadow, 0x8)
{
	using VoxelShadowIdx = IndexClass<ShadowVoxelIndexKey, VoxelCacheStruct*>;

	GET(FootClass*, pThis, ESI);
	GET_STACK(Point2D, pos, STACK_OFFSET(0x18, 0x28));
	GET_STACK(Surface*, pSurface, STACK_OFFSET(0x18, 0x24));
	GET_STACK(bool, a9, STACK_OFFSET(0x18, 0x20));
	GET_STACK(Matrix3D*, pMatrix, STACK_OFFSET(0x18, 0x1C));
	GET_STACK(RectangleStruct*, bound, STACK_OFFSET(0x18, 0x14));
	GET_STACK(Point2D, a3, STACK_OFFSET(0x18, -0x10));
	GET_STACK(VoxelShadowIdx*, shadow_cache, STACK_OFFSET(0x18, 0x10));
	GET_STACK(VoxelIndexKey, index_key, STACK_OFFSET(0x18, 0xC));
	GET_STACK(int, shadow_index, STACK_OFFSET(0x18, 0x8));
	GET_STACK(VoxelStruct*, main_vxl, STACK_OFFSET(0x18, 0x4));

	if (!pThis->IsAlive)
		return 0x0;

	auto pType = TechnoExt_ExtData::GetImage(pThis);
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if(pTypeExt->TurretShadow.Get(RulesExtData::Instance()->DrawTurretShadow))
	{

		const auto tur = pType->Gunner || pType->IsChargeTurret
		? TechnoTypeExtData::GetTurretsVoxel(pType, pThis->CurrentTurretNumber)
		: &pType->TurretVoxel;

		if (tur && tur->VXL && tur->HVA)
		{
			Matrix3D mtx = Matrix3D::GetIdentity();
			pTypeExt->ApplyTurretOffset(&mtx, Game::Pixel_Per_Lepton());
			mtx.TranslateZ(-tur->HVA->Matrixes[0].GetZVal());
			mtx.RotateZ(static_cast<float>(pThis->SecondaryFacing.Current().GetRadian<32>() - pThis->PrimaryFacing.Current().GetRadian<32>()));			mtx = (*pMatrix) * mtx;
			mtx = *pMatrix * mtx;

			pThis->DrawVoxelShadow(tur, 0, index_key, nullptr, bound, &a3, &mtx, a9, pSurface, pos);

			const auto bar = pType->ChargerBarrels ?
				TechnoTypeExtData::GetBarrelsVoxel(pType, pThis->CurrentTurretNumber)
				: &pType->BarrelVoxel;

			if (bar && bar->VXL && bar->HVA)
				pThis->DrawVoxelShadow(bar, 0, index_key, nullptr, bound, &a3, &mtx, a9, pSurface, pos);
		}
	}

	if (pTypeExt->ShadowIndices.empty())
	{
		pThis->DrawVoxelShadow(main_vxl, shadow_index, index_key, shadow_cache, bound, &a3, pMatrix, a9, pSurface, pos);
	}
	else
	{
		for (const auto& index : pTypeExt->ShadowIndices)
		{
			//Matrix3D copy_ = *pMatrix;
			//copy_.TranslateZ(-pVXL->HVA->Matrixes[index].GetZVal());
			//Matrix3D::MatrixMultiply(&copy_, &Game::VoxelDefaultMatrix(), &copy_);
			pThis->DrawVoxelShadow(main_vxl, index, index_key, shadow_cache, bound, &a3, pMatrix, a9, pSurface, pos);
		}
	}

	return 0x4DB195;
}
#else
DEFINE_HOOK(0x4DB157, FootClass_DrawVoxelShadow_TurretShadow, 0x8)
{
	using VoxelShadowIdx = IndexClass<ShadowVoxelIndexKey, VoxelCacheStruct*>;
	GET(FootClass*, pThis, ESI);
	GET_STACK(Point2D, pos, STACK_OFFSET(0x18, 0x28));
	GET_STACK(Surface*, pSurface, STACK_OFFSET(0x18, 0x24));
	GET_STACK(bool, a9, STACK_OFFSET(0x18, 0x20));
	GET_STACK(Matrix3D*, pMatrix, STACK_OFFSET(0x18, 0x1C));
	GET_STACK(RectangleStruct*, bound, STACK_OFFSET(0x18, 0x14));
	GET_STACK(Point2D, a3, STACK_OFFSET(0x18, -0x10));
	GET_STACK(VoxelShadowIdx*, shadow_cache, STACK_OFFSET(0x18, 0x10));
	GET_STACK(VoxelIndexKey, index_key, STACK_OFFSET(0x18, 0xC));
	GET_STACK(int, shadow_index, STACK_OFFSET(0x18, 0x8));
	GET_STACK(VoxelStruct*, main_vxl, STACK_OFFSET(0x18, 0x4));

	if (!pThis->IsAlive)
		return 0x0;

	auto pType = TechnoExt_ExtData::GetImage(pThis);
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	const auto tur = pType->Gunner || pType->IsChargeTurret
		? TechnoTypeExtData::GetTurretsVoxel(pType, pThis->CurrentTurretNumber)
		: &pType->TurretVoxel;

	if (pTypeExt->TurretShadow.Get(RulesExtData::Instance()->DrawTurretShadow) && tur->VXL && tur->HVA)
	{
		Matrix3D mtx {};
		pThis->Locomotor.GetInterfacePtr()->Shadow_Matrix(&mtx, nullptr);
		pTypeExt->ApplyTurretOffset(&mtx, *reinterpret_cast<double*>(0xB1D008));
		mtx.TranslateZ(-tur->HVA->Matrixes[0].GetZVal());

		if (pType->Turret)
		{
			mtx.RotateZ((float)(pThis->SecondaryFacing.Current().GetRadian<32>() - pThis->PrimaryFacing.Current().GetRadian<32>()));
		}

		mtx = Game::VoxelDefaultMatrix() * mtx;

		pThis->DrawVoxelShadow(tur, 0, index_key, 0, bound, &a3, &mtx, a9, pSurface, pos);

		const auto bar = pType->ChargerBarrels ?
			TechnoTypeExtData::GetBarrelsVoxel(pType, pThis->CurrentTurretNumber)
			: &pType->BarrelVoxel;

		if (bar->VXL && bar->HVA)
			pThis->DrawVoxelShadow(bar, 0, index_key, 0, bound, &a3, &mtx, a9, pSurface, pos);
	}

	if (pTypeExt->ShadowIndices.empty())
	{
		pThis->DrawVoxelShadow(main_vxl, shadow_index, index_key, shadow_cache, bound, &a3, pMatrix, a9, pSurface, pos);
	}
	else
	{
		for (const auto& index : pTypeExt->ShadowIndices)
		{
			//Matrix3D copy_ = *pMatrix;
			//copy_.TranslateZ(-pVXL->HVA->Matrixes[index].GetZVal());
			//Matrix3D::MatrixMultiply(&copy_, &Game::VoxelDefaultMatrix(), &copy_);
			pThis->DrawVoxelShadow(main_vxl, index, index_key, shadow_cache, bound, &a3, pMatrix, a9, pSurface, pos);
		}
	}

	return 0x4DB195;
}
#endif

DEFINE_HOOK(0x73B4A0, UnitClass_DrawVXL_WaterType, 9)
{
	R->ESI(0);
	GET(UnitClass*, U, EBP);

	ObjectTypeClass* Image = U->Type;

	if (UnitTypeClass* const pCustomType = TechnoExt_ExtData::GetUnitTypeImage(U))
	{
		Image = pCustomType;
	}

	if (U->Deployed && U->Type->UnloadingClass)
	{
		Image = U->Type->UnloadingClass;
	}

	if (!U->IsClearlyVisibleTo(HouseClass::CurrentPlayer))
	{
		Image = U->GetDisguise(true);
	}

	R->EBX<ObjectTypeClass*>(Image);
	return 0x73B4DA;
}

DEFINE_HOOK(0x715320, TechnoTypeClass_LoadFromINI_EarlyReader, 6)
{
	GET(CCINIClass*, pINI, EDI);
	GET(TechnoTypeClass*, pType, EBP);

	INI_EX exINI(pINI);
	TechnoTypeExtContainer::Instance.Find(pType)->WaterImage.Read(exINI, pType->ID, "WaterImage");

	return 0;
}

DEFINE_HOOK(0x73C485, UnitClass_DrawVXL_NoSpawnAlt_SkipShadow, 8)
{
	enum { DoNotDrawShadow = 0x73C5C9, ShadowAlreadyDrawn = 0x0 };

	GET(UnitClass*, pThis, EBP);
	auto const pSpawnManager = pThis->SpawnManager;

	if (pThis->Type->NoSpawnAlt
		&& pSpawnManager
		&& pSpawnManager->CountDockedSpawns() < pSpawnManager->SpawnCount
		)
	{
		if (TechnoTypeExtContainer::Instance.Find(pThis->Type)->NoShadowSpawnAlt.Get())
			return DoNotDrawShadow;
	}

	return ShadowAlreadyDrawn;
}