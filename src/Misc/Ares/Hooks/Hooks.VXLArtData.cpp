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

#include <Locomotor/Cast.h>

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

#ifndef FUCKEDUP
int ChooseFrame(FootClass* pThis, int shadow_index_now, VoxelStruct* pVXL)
{
	auto pType = pThis->GetTechnoType();

	// Turret or Barrel
	if (pVXL != &pType->MainVoxel)
	{
		// verify just in case:
		auto who_are_you = reinterpret_cast<uintptr_t*>(reinterpret_cast<DWORD>(pVXL) - (offsetof(TechnoTypeClass, MainVoxel)));
		if (who_are_you[0] == UnitTypeClass::vtable)
			pType = reinterpret_cast<TechnoTypeClass*>(who_are_you);//you are someone else
		else {
				if((&TechnoTypeExtContainer::Instance.Find(pType)->SpawnAltData) != pVXL)
					return pThis->TurretAnimFrame % pVXL->HVA->FrameCount;
		}

		// you might also be SpawnAlt voxel, but I can't know
		// otherwise what would you expect me to do, shift back to ares typeext base and check if ownerobject is technotype?
	}

	// Main body sections
	const auto& shadowIndices = TechnoTypeExtContainer::Instance.Find(pType)->ShadowIndices;
	if (shadowIndices.empty()) {
		// Only ShadowIndex
		if (pType->ShadowIndex == shadow_index_now) {
			int shadow_index_frame = TechnoTypeExtContainer::Instance.Find(pType)->ShadowIndex_Frame;
			if (shadow_index_frame > -1)
				return shadow_index_frame % pVXL->HVA->FrameCount;
		} else {
			// WHO THE HELL ARE YOU???
			return 0;
		}
	} else {
		auto iter = shadowIndices.get_key_iterator(shadow_index_now);
		if(iter != shadowIndices.end()  && iter->second > -1)
			return iter->second % pVXL->HVA->FrameCount;
	}

	return pThis->WalkedFramesSoFar % pVXL->HVA->FrameCount;
}

Matrix3D* __fastcall BounceClass_ShadowMatrix(BounceClass* self, void*, Matrix3D* ret) {

	Matrix3D::FromQuaternion(ret, &self->CurrentAngle);
	*ret = Matrix3D { 1, 0, 0 , 0,	0, 0.25, -0.4330127018922194 , 0, 0, 0, 0 , 0 } * (*ret);
	return ret;
}
DEFINE_JUMP(CALL, 0x749CAC, GET_OFFSET(BounceClass_ShadowMatrix));

//the deeper part
DEFINE_HOOK(0x7072A1, suka707280_ChooseTheGoddamnMatrix, 0x7)
{
	//Debug::Log(__FUNCTION__" Exec\n");
	GET(FootClass*, pThis, EBX);//Maybe Techno later
	GET(VoxelStruct*, pVXL, EBP);
	GET_STACK(Matrix3D*, pMat, STACK_OFFSET(0xE8, 0xC));
	GET_STACK(int, shadow_index_now, STACK_OFFSET(0xE8, 0x18));// it's used later, otherwise I could have chosen the frame index earlier
	REF_STACK(Matrix3D, matRet, STACK_OFFSET(0xE8, -0x60));  //matRet is not initiated  ???

	int frameChoosen = ChooseFrame(pThis, shadow_index_now, pVXL);//Don't want to use goto

	matRet = (*pMat) * pVXL->HVA->Matrixes[shadow_index_now + pVXL->HVA->LayerCount * frameChoosen];
	{
		auto& arr = matRet.row;
		arr[0][2] = arr[1][2] = arr[2][2] = arr[2][1] = arr[2][0] = 0;
	}
	// A nasty temporary backward compatibility option
	// if (pVXL->HVA->LayerCount > 1 || pThis->GetTechnoType()->Turret) {
	// 	// NEEDS IMPROVEMENT : Choose the proper Z offset to shift the sections to the same level
	// 	matRet.TranslateZ(
	// 		-matRet.GetZVal()
	// 		- pVXL->VXL->TailerData->Bounds[0].Z
	// 	);
	// }

	// Recover vanilla instructions
	if (pThis->GetTechnoType()->UseBuffer)
		*reinterpret_cast<DWORD*>(0xB43180) = 1;

	REF_STACK(Matrix3D, b, STACK_OFFSET(0xE8, -0x90));
	b.MakeIdentity();// we don't do scaling here anymore

	return 0x707331;
}

//aircraft only
DEFINE_HOOK(0x4147F9, AircraftClass_Draw_Shadow, 0x6)
{
	//Debug::Log(__FUNCTION__" Exec\n");
	GET(AircraftClass*, pThis, EBP);
	GET(const int, height, EBX);
	REF_STACK(VoxelIndexKey, key, STACK_OFFSET(0xCC, -0xBC));
	REF_STACK(Point2D, flor, STACK_OFFSET(0xCC, -0xAC));
	GET_STACK(RectangleStruct*, bound, STACK_OFFSET(0xCC, 0x10));
	enum { FinishDrawing = 0x4148A5 };

	auto loco = pThis->Locomotor.GetInterfacePtr();
	if (pThis->Type->NoShadow || !loco->Is_To_Have_Shadow() || pThis->IsSinking)
		return FinishDrawing;

	const auto aTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);
	Matrix3D shadow_mtx {} ;
	loco->Shadow_Matrix(&shadow_mtx, &key);
	if(const auto flyloco = locomotion_cast<FlyLocomotionClass*>(pThis->Locomotor)) {
		const double baseScale_log = RulesExtData::Instance()->AirShadowBaseScale_log;

		if (RulesExtData::Instance()->HeightShadowScaling)
		{
			const double minScale = RulesExtData::Instance()->HeightShadowScaling_MinScale;
			const float cHeight = (float)aTypeExt->ShadowSizeCharacteristicHeight.Get(pThis->Type->GetFlightLevel());

			if (cHeight > 0)
			{
				shadow_mtx.Scale((float)std::max(GeneralUtils::Pade2_2(baseScale_log * height / cHeight), minScale));
				if (flyloco->FlightLevel > 0 || height > 0)
					key.Invalidate();
			}
		}
		else if (pThis->Type->ConsideredAircraft)
		{
			shadow_mtx.Scale((float)GeneralUtils::Pade2_2(baseScale_log));
		}

		double arf = pThis->AngleRotatedForwards;
		if (flyloco->CurrentSpeed > pThis->Type->PitchSpeed)
			arf += pThis->Type->PitchAngle;
		double ars = pThis->AngleRotatedSideways;
		if (key.Is_Valid_Key() && (std::abs(arf) > 0.005 || std::abs(ars) > 0.005))
			key.Invalidate();

		shadow_mtx.ScaleY((float)Math::cos(ars));
		shadow_mtx.ScaleX((float)Math::cos(arf));

	} else if (height > 0) {
		if(const auto flyloco = locomotion_cast<RocketLocomotionClass*>(pThis->Locomotor)){
			shadow_mtx.ScaleX((float)Math::cos(flyloco->CurrentPitch));
			key.Invalidate();
		}
	}

	shadow_mtx = Game::VoxelDefaultMatrix() * shadow_mtx;
	Point2D why = flor + loco->Shadow_Point();
	auto const main_vxl = &pThis->Type->MainVoxel;

	if (aTypeExt->ShadowIndices.empty())
	{
		auto const shadow_index = pThis->Type->ShadowIndex;
		if (shadow_index >= 0 && shadow_index < main_vxl->HVA->LayerCount)
			pThis->DrawVoxelShadow(main_vxl,
				shadow_index,
				key,
				&pThis->Type->VoxelCaches.Shadow,
				bound,
				&why,
				&shadow_mtx,
				true,
				nullptr,
				{ 0, 0 }
		);
	}
	else
	{
		for (auto& indices : aTypeExt->ShadowIndices)
			pThis->DrawVoxelShadow(main_vxl,
				indices.first,
				key,
				&pThis->Type->VoxelCaches.Shadow,
				bound,
				&why,
				&shadow_mtx,
				true,
				nullptr,
				{ 0, 0 }
		);
	}

	return FinishDrawing;
}

void TranslateAngleRotated(Matrix3D* mtx , TechnoClass* pThis  , TechnoTypeClass* pType) {
	float arf = pThis->AngleRotatedForwards;
	float ars = pThis->AngleRotatedSideways;
	// lazy, don't want to hook inside Shadow_Matrix
	if (std::fabs(ars) >= 0.005 || std::fabs(arf) >= 0.005)
	{
		// index key is already invalid
		const auto c_arf = Math::cos(arf);
		const auto c_ars = Math::cos(ars);
		mtx->TranslateX(float(Math::signum(arf) * pType->VoxelScaleX * (1 - c_arf)));
		mtx->TranslateY(float(Math::signum(-ars) * pType->VoxelScaleY * (1 - c_ars)));
		mtx->ScaleX((float)c_arf);
		mtx->ScaleY((float)c_ars);
	}
}

VoxelStruct* GetmainVxl(TechnoClass* pThis, TechnoTypeClass* pType , VoxelIndexKey& key){

	if (pType->NoSpawnAlt && pThis->SpawnManager && pThis->SpawnManager->CountDockedSpawns() == 0)
	{
		key.Invalidate();// I'd just assume most of the time we have spawn
		return &TechnoTypeExtContainer::Instance.Find(pType)->SpawnAltData;
	}

	return &pType->MainVoxel;
}

void DecideScaleAndIndex(Matrix3D* mtx, TechnoClass* pThis, TechnoTypeClass* pType, VoxelIndexKey& key, ILocomotion* iLoco , int height)
{
	const double baseScale_log = RulesExtData::Instance()->AirShadowBaseScale_log; // -ln(baseScale) precomputed

	if (RulesExtData::Instance()->HeightShadowScaling && height > 0)
	{
		auto uTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

		const double minScale = RulesExtData::Instance()->HeightShadowScaling_MinScale;
		if (const auto jjloco = locomotion_cast<JumpjetLocomotionClass*>(iLoco))
		{
			const float cHeight = (float)uTypeExt->ShadowSizeCharacteristicHeight.Get(jjloco->Height);

			if (cHeight > 0)
			{
				mtx->Scale((float)std::max(GeneralUtils::Pade2_2(baseScale_log * height / cHeight), minScale));

				if (jjloco->NextState != JumpjetLocomotionClass::State::Hovering)
					key.Invalidate();
			}
		}
		else
		{
			const float cHeight = (float)uTypeExt->ShadowSizeCharacteristicHeight.Get(RulesClass::Instance->CruiseHeight);

			if (cHeight > 0)
			{
				mtx->Scale((float)std::max(GeneralUtils::Pade2_2(baseScale_log * height / cHeight), minScale));
				key.Invalidate();
			}
		}
	}
	else if (!RulesExtData::Instance()->HeightShadowScaling && pType->ConsideredAircraft)
	{
		mtx->Scale((float)GeneralUtils::Pade2_2(baseScale_log));
	}
}

// Shadow_Point of RocketLoco was forgotten to be set to {0,0}. It was an oversight.
DEFINE_JUMP(VTABLE, 0x7F0B4C, 0x4CF940);

DEFINE_HOOK(0x73C47A, UnitClass_DrawAsVXL_Shadow, 0x5)
{
	//Debug::Log(__FUNCTION__" Exec\n");
	GET(UnitClass*, pThis, EBP);
	enum { SkipDrawing = 0x73C5C9 };

	auto const loco = pThis->Locomotor.GetInterfacePtr();

	if (pThis->Type->NoShadow || !loco->Is_To_Have_Shadow())
		return SkipDrawing;

	REF_STACK(Matrix3D, shadow_matrix, STACK_OFFSET(0x1C4, -0x130));
	GET_STACK(VoxelIndexKey, vxl_index_key, STACK_OFFSET(0x1C4, -0x1B0));
	LEA_STACK(RectangleStruct*, bounding, STACK_OFFSET(0x1C4, 0xC));
	LEA_STACK(Point2D*, floor, STACK_OFFSET(0x1C4, -0x1A4));
	GET_STACK(Surface* const, surface, STACK_OFFSET(0x1C4, -0x1A8));

	GET(UnitTypeClass*, pType, EBX);
	// This is not necessarily pThis->Type : UnloadingClass or WaterImage
	// This is the very reason I need to do this here, there's no less hacky way to get this Type from those inner calls

	const auto height = pThis->GetHeight();
	DecideScaleAndIndex(&shadow_matrix, pThis, pType, vxl_index_key, loco, height);

	VoxelStruct* main_vxl = GetmainVxl(pThis , pType , vxl_index_key);

	// TODO : adjust shadow point according to height
	// There was a bit deviation that I cannot decipher, might need help with that
	// But it turns out it has basically no visual difference

	auto shadow_point = loco->Shadow_Point();
	auto why = *floor + shadow_point;
	TranslateAngleRotated(&shadow_matrix, pThis, pType);

	auto mtx = Game::VoxelDefaultMatrix() * (shadow_matrix);
	{
		auto& arr = mtx.row;
		arr[0][2] = arr[1][2] = arr[2][2] = arr[2][1] = arr[2][0] = 0;
	}
	if (height > 0)
		shadow_point.Y += 1;

	const auto uTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (uTypeExt->ShadowIndices.empty())
	{
		if (pType->ShadowIndex >= 0 && pType->ShadowIndex < main_vxl->HVA->LayerCount)
			pThis->DrawVoxelShadow(
				   main_vxl,
				   pType->ShadowIndex,
				   vxl_index_key,
				   &pType->VoxelCaches.Shadow,
				   bounding,
				   &why,
				   &mtx,
				   true,
				   surface,
				   shadow_point
			);
	}
	else
	{
		for (const auto& indices : uTypeExt->ShadowIndices)
			pThis->DrawVoxelShadow(
				   main_vxl,
				   indices.first,
				   indices.first == pType->ShadowIndex ? vxl_index_key : std::bit_cast<VoxelIndexKey>(-1),
					&pType->VoxelCaches.Shadow,
				   bounding,
				   &why,
				   &mtx,
				    indices.first == pType->ShadowIndex,
				   surface,
				   shadow_point
			);
	}

	if (!uTypeExt->TurretShadow.Get(RulesExtData::Instance()->DrawTurretShadow) || main_vxl == &pType->TurretVoxel)
		return SkipDrawing;

	Matrix3D rot = Matrix3D::GetIdentity();
	uTypeExt->ApplyTurretOffset(&rot, Game::Pixel_Per_Lepton());
	rot.RotateZ(static_cast<float>(pThis->SecondaryFacing.Current().GetRadian<32>() - pThis->PrimaryFacing.Current().GetRadian<32>()));
	auto tur_mtx = mtx * rot; // unfortunately we won't have TurretVoxelScaleX/Y given the amount of work
	{
		auto& arr = tur_mtx.row;
		arr[0][2] = arr[1][2] = arr[2][2] = arr[2][1] = arr[2][0] = 0;
	}
	auto tur = TechnoTypeExtData::GetTurretsVoxel(pType , pThis->CurrentTurretNumber);

	// sorry but you're fucked
	if (tur && tur->VXL && tur->HVA)
		pThis->DrawVoxelShadow(
			tur,
			0,
			std::bit_cast<VoxelIndexKey>(-1), // no cache, no use for valid key
			nullptr, // no cache atm
			bounding,
			&why,
			&tur_mtx,
			false,
			surface,
			shadow_point
		);

	auto bar = TechnoTypeExtData::GetBarrelsVoxel(pType, pThis->CurrentTurretNumber);
	// and you are utterly fucked
	if (bar && bar->VXL && bar->HVA)
		pThis->DrawVoxelShadow(
			bar,
			0,
			std::bit_cast<VoxelIndexKey>(-1), // no cache, no use
			nullptr,//no cache atm
			bounding,
			&why,
			&tur_mtx,
			false,
			surface,
			shadow_point
		);
	// Add caches in Ext if necessary, remember not to serialize these shit
	// IndexClass<ShadowVoxelIndexKey, VoxelCacheStruct*> VoxelTurretShadowCache {};
	// IndexClass<ShadowVoxelIndexKey, VoxelCacheStruct*> VoxelBarrelShadowCache {};

	return SkipDrawing;
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
			pThis->DrawVoxelShadow(main_vxl, index.first, index_key, shadow_cache, bound, &a3, pMatrix, a9, pSurface, pos);
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