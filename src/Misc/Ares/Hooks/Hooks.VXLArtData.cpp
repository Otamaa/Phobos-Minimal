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
#include <SpawnManagerClass.h>

ASMJIT_PATCH(0x5F8277, ObjectTypeClass_Load3DArt_NoSpawnAlt1, 7)
{
	REF_STACK(bool, bLoadFailed, 0x13);
	GET(ObjectTypeClass*, pThis, ESI);

	const auto pType = type_cast<UnitTypeClass*>(pThis);

	if (!pType)
		return 0x5F8640;

	if (pType->NoSpawnAlt)
	{
		auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

		std::string _buffer = pThis->ImageFile;
		_buffer += "WO";
		ImageStatusses nPairStatus = ImageStatusses::ReadVoxel(_buffer.c_str());
	
		if (!nPairStatus.Loaded) {
			Debug::LogInfo("{} Techno NoSpawnAlt Image[{}] cannot be loaded ,returning load failed ! ", pThis->ID, _buffer.c_str());
			bLoadFailed = true;
			return 0x5F8287;
		}

		nPairStatus.swap(pTypeExt->SpawnAltData);
	}

	return 0x5F8287;
}

//ObjectTypeClass_Load3DArt_NoSpawnAlt2
DEFINE_JUMP(LJMP, 0x5F848C, 0x5F8844);

ASMJIT_PATCH(0x5F887B, ObjectTypeClass_Load3DArt_Barrels, 6)
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
		//	Debug::LogInfo("Reading Barrel [%d] for [%s] Which is More than array size ! ", i, pThis->ImageFile);
		//	return 0x5F8844;
		//}

		auto &nArr = i < TechnoTypeClass::MaxWeapons ?
			pThis->ChargerBarrels[i] :
			pTypeExt->BarrelImageData[i - TechnoTypeClass::MaxWeapons];

		ImageStatusses nPairStatus = ImageStatusses::ReadVoxel(_buffer.c_str());

		if (!nPairStatus.Loaded) {
			Debug::LogInfo("{} Techno Barrel [{}] at[{}] cannot be loaded , breaking the loop ! ", pThis->ID, _buffer.c_str(), i);
			break;
		}

		nPairStatus.swap(nArr);
	}

	return 0x5F8A6A;
}

ASMJIT_PATCH(0x5F865F, ObjectTypeClass_Load3DArt_Turrets, 6)
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
		//	Debug::LogInfo("Reading Turrent [%d] for [%s] Which is More than array size ! ", i, pThis->ImageFile);
		//	return 0x5F8844;
		//}

		auto& nArr = i < TechnoTypeClass::MaxWeapons ?
			pThis->ChargerTurrets[i] :
			pTypeExt->TurretImageData[i - TechnoTypeClass::MaxWeapons];
			;

		ImageStatusses nPairStatus = ImageStatusses::ReadVoxel(_buffer.c_str());

		if (!nPairStatus.Loaded) {
			Debug::LogInfo("{} Techno Turret [{}] at[{}] cannot be loaded , breaking the loop ! ", pThis->ID, _buffer.c_str(), i);
			break;
		}

		nPairStatus.swap(nArr);
	}

	return 0x5F868C;
}

ASMJIT_PATCH(0x73B90E, UnitClass_DrawVXL_Barrels1, 7)
{
	GET(UnitTypeClass*, pUnit, EBX);
	GET(int, nIdx, EAX);

	R->Stack(0x2C, R->ESI());
	const auto pData = TechnoTypeExtData::GetBarrelsVoxel(pUnit, nIdx);
	return (!pData->VXL || !pData->HVA) ? 0x73B94A : 0x73B928;
}

ASMJIT_PATCH(0x73BCCD, UnitClass_DrawVXL_Barrels2, 7)
{
	GET(UnitTypeClass*, pUnit, EBX);
	GET(int, nIdx, ECX);
	R->EDX(TechnoTypeExtData::GetBarrelsVoxel(pUnit, nIdx));
	return 0x73BCD4;
}

ASMJIT_PATCH(0x73BD6A, UnitClass_DrawVXL_Barrels3, 7)
{
	GET(UnitTypeClass*, pUnit, EBX);
	GET(int, nIdx, ESI);
	R->ECX(TechnoTypeExtData::GetBarrelsVoxel(pUnit, nIdx));
	return 0x73BD71;
}

ASMJIT_PATCH(0x73BD15, UnitClass_DrawVXL_Turrets, 7)
{
	GET(UnitTypeClass*, pUnit, EBX);
	GET(int, nIdx, ESI);
	R->ECX(TechnoTypeExtData::GetTurretsVoxel(pUnit, nIdx));
	return 0x73BD1C;
}

ASMJIT_PATCH(0x5F8084, ObjectTypeClass_UnloadTurretArt, 6)
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

	for(auto& bar : pThis->ChargerBarrels){
		bar.~VoxelStruct();
	}

	pTypeExt->BarrelImageData.clear();
	pTypeExt->TurretImageData.clear();
	pTypeExt->SpawnAltData.~VoxelStruct();

	return 0;
}

ASMJIT_PATCH(0x73B6E3, UnitClass_DrawVXL_NoSpawnAlt, 6)
{
	GET(UnitTypeClass*, pType, EBX);
	R->EDX(&TechnoTypeExtContainer::Instance.Find(pType)->SpawnAltData);
	return 0x73B6E9;
}

int ChooseFrame(FootClass* pThis, int shadow_index_now, VoxelStruct* pVXL)
{
	auto pType = pThis->GetTechnoType();

	// Turret or Barrel
	if (pVXL != &pType->MainVoxel)
	{
		// verify just in case:
		auto who_are_you = reinterpret_cast<uintptr_t*>(reinterpret_cast<DWORD>(pVXL) - (offsetof(TechnoTypeClass, MainVoxel)));
		if (who_are_you[0] == UnitTypeClass::vtable) {
			pType = reinterpret_cast<TechnoTypeClass*>(who_are_you);//you are someone else
		} else if ((&TechnoTypeExtContainer::Instance.Find(pType)->SpawnAltData) != pVXL) {
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

static Matrix3D* __fastcall BounceClass_ShadowMatrix(BounceClass* self, void*, Matrix3D* ret) {

	Matrix3D::FromQuaternion(ret, &self->CurrentAngle);
	*ret = Matrix3D { 1, 0, 0 , 0,	0, 0.25f, -0.4330127f , 0, 0, -0.4330127f, 0.75f , 0 } * (*ret)*  Matrix3D { 1,0,0,0,0,1,0,0,0,0,0,0 };
	return ret;
}
DEFINE_FUNCTION_JUMP(CALL, 0x749CAC, BounceClass_ShadowMatrix);

//the deeper part
ASMJIT_PATCH(0x7072A1, suka707280_ChooseTheGoddamnMatrix, 0x7)
{
	//Debug::LogInfo(__FUNCTION__" Exec");
	GET(FootClass*, pThis, EBX);//Maybe Techno later
	GET(VoxelStruct*, pVXL, EBP);
	GET_STACK(Matrix3D*, pMat, STACK_OFFSET(0xE8, 0xC));
	GET_STACK(int, shadow_index_now, STACK_OFFSET(0xE8, 0x18));// it's used later, otherwise I could have chosen the frame index earlier
	REF_STACK(Matrix3D, matRet, STACK_OFFSET(0xE8, -0x60));  //matRet is not initiated  ???

	int frameChoosen = ChooseFrame(pThis, shadow_index_now, pVXL);//Don't want to use goto

	matRet = (*pMat) * pVXL->HVA->Matrixes[shadow_index_now + pVXL->HVA->LayerCount * frameChoosen];
	matRet *= Matrix3D { 1,0,0,0,0,1,0,0,0,0,0,0 };

	double l2 = 0;
	auto& arr = matRet.Row;
	for (int i = 0; i < 3; i++)	for (int j = 0; j < 3; j++)	l2 += arr[i][j] * arr[i][j];
	if (l2 < 0.03) R->Stack(STACK_OFFSET(0xE8, 0x20), true);

	// Recover vanilla instructions
	if (pThis->GetTechnoType()->UseBuffer)
		*reinterpret_cast<DWORD*>(0xB43180) = 1;

	REF_STACK(Matrix3D, b, STACK_OFFSET(0xE8, -0x90));
	b.MakeIdentity();// we don't do scaling here anymore

	return 0x707331;
}

AircraftTypeClass* GetAircraftTypeExtra(AircraftClass* pAircraft)
{
	auto const pData = TechnoTypeExtContainer::Instance.Find(pAircraft->Type);

	if (pData->Image_Yellow && pAircraft->IsYellowHP())
	{
		return (AircraftTypeClass*)pData->Image_Yellow.Get();
	}
	else if(pData->Image_Red && pAircraft->IsRedHP())
	{
		return (AircraftTypeClass*)pData->Image_Red.Get();
	}

	return pAircraft->Type;
}

ASMJIT_PATCH(0x414987, AircraftClass_Draw_Extra, 0x6)
{
	GET(AircraftClass*, pThis, EBP);
	R->ESI<AircraftTypeClass*>(GetAircraftTypeExtra(pThis));
	return 0x41498D;
}

ASMJIT_PATCH(0x414665, AircraftClass_Draw_ExtraSHP, 0x6)
{
	GET(AircraftClass*, pThis, EBP);
	R->EAX<AircraftTypeClass*>(GetAircraftTypeExtra(pThis));
	return 0x41466B;
}

//aircraft only
ASMJIT_PATCH(0x4147F9, AircraftClass_Draw_Shadow, 0x6)
{
	//Debug::LogInfo(__FUNCTION__" Exec");
	GET(AircraftClass*, pThis, EBP);
	GET(const int, height, EBX);
	REF_STACK(VoxelIndexKey, key, STACK_OFFSET(0xCC, -0xBC));
	REF_STACK(Point2D, flor, STACK_OFFSET(0xCC, -0xAC));
	GET_STACK(RectangleStruct*, bound, STACK_OFFSET(0xCC, 0x10));
	enum { FinishDrawing = 0x4148A5 };

	auto loco = pThis->Locomotor.GetInterfacePtr();
	if (pThis->Type->NoShadow || pThis->CloakState != CloakState::Uncloaked ||  !loco->Is_To_Have_Shadow() || pThis->IsSinking)
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
		if (key.Is_Valid_Key() && (Math::abs(arf) > 0.005 || Math::abs(ars) > 0.005))
			key.Invalidate();

		shadow_mtx.RotateY((float)(ars));
		shadow_mtx.RotateX((float)(arf));

	} else if (height > 0) {
		if(const auto rocketloco = locomotion_cast<RocketLocomotionClass*>(pThis->Locomotor)){
			shadow_mtx.RotateY((float)std::cos(rocketloco->CurrentPitch));
			key.Invalidate();
		}
	}

	shadow_mtx = Game::VoxelDefaultMatrix() * shadow_mtx;
	//Point2D why = flor + loco->Shadow_Point();
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
				&flor,
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
				&flor,
				&shadow_mtx,
				true,
				nullptr,
				{ 0, 0 }
		);
	}

	return FinishDrawing;
}

struct JumpjetTiltReference
{
	static COMPILETIMEEVAL OPTIONALINLINE int BaseSpeed = 32 ;
	static COMPILETIMEEVAL OPTIONALINLINE double BaseTilt { Math::HalfPi / 4 };
	static COMPILETIMEEVAL OPTIONALINLINE int BaseTurnRaw { 32768 };
	static COMPILETIMEEVAL OPTIONALINLINE float MaxTilt { static_cast<float>(Math::HalfPi) };
	static COMPILETIMEEVAL OPTIONALINLINE float ForwardBaseTilt { (float)(BaseTilt / (float)BaseSpeed) };
	static COMPILETIMEEVAL OPTIONALINLINE float SidewaysBaseTilt { (float)(BaseTilt / float(BaseTurnRaw * BaseSpeed)) };
};

static void TranslateAngleRotated(Matrix3D* mtx , FootClass* pThis  , TechnoTypeClass* pType, VoxelIndexKey& key) {
	float arf = pThis->AngleRotatedForwards;
	float ars = pThis->AngleRotatedSideways;
	const auto jjloco = locomotion_cast<JumpjetLocomotionClass*>(pThis->Locomotor.GetInterfacePtr());
	const auto uTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	// lazy, don't want to hook inside Shadow_Matrix
	if (Math::abs(ars) >= 0.005 || Math::abs(arf) >= 0.005)
	{
		// index key is already invalid
		key.Invalidate();
		const auto c_arf = std::cos(arf);
		const auto c_ars = std::cos(ars);
		mtx->TranslateX(float(Math::signum(arf) * pType->VoxelScaleX * (1 - c_arf)));
		mtx->TranslateY(float(Math::signum(-ars) * pType->VoxelScaleY * (1 - c_ars)));
		mtx->RotateY(arf);
		mtx->RotateX(ars);
	} else if (jjloco && uTypeExt->JumpjetTilt && jjloco->NextState != JumpjetLocomotionClass::State::Grounded
		&& jjloco->__currentSpeed > 0.0 && pThis->IsAlive && pThis->Health > 0 && !pThis->IsAttackedByLocomotor)
	{
		const auto forwardSpeedFactor = jjloco->__currentSpeed * uTypeExt->JumpjetTilt_ForwardSpeedFactor;
		const auto forwardAccelFactor = jjloco->Acceleration * uTypeExt->JumpjetTilt_ForwardAccelFactor;

		arf += MinImpl(JumpjetTiltReference::MaxTilt, static_cast<float>((forwardAccelFactor + forwardSpeedFactor)
			* JumpjetTiltReference::ForwardBaseTilt));

		const auto& locoFace = jjloco->Facing;

		if (locoFace.Is_Rotating())
		{
			const auto sidewaysSpeedFactor = jjloco->__currentSpeed * uTypeExt->JumpjetTilt_SidewaysSpeedFactor;
			const auto sidewaysRotationFactor = static_cast<short>(locoFace.Difference().Raw)
				* uTypeExt->JumpjetTilt_SidewaysRotationFactor;

			ars += std::clamp(static_cast<float>(sidewaysSpeedFactor * sidewaysRotationFactor
				* JumpjetTiltReference::SidewaysBaseTilt), -JumpjetTiltReference::MaxTilt, JumpjetTiltReference::MaxTilt);
		}

		if (Math::abs(ars) >= 0.005 || Math::abs(arf) >= 0.005)
		{
			key.Invalidate();
			mtx->RotateX(ars);
			mtx->RotateY(arf);
		}
	}
}

static VoxelStruct* GetmainVxl(TechnoClass* pThis, TechnoTypeClass* pType , VoxelIndexKey& key){

	if (pType->NoSpawnAlt && pThis->SpawnManager && pThis->SpawnManager->CountDockedSpawns() == 0)
	{
		key.Invalidate();// I'd just assume most of the time we have spawn
		return &TechnoTypeExtContainer::Instance.Find(pType)->SpawnAltData;
	}

	return &pType->MainVoxel;
}

static void DecideScaleAndIndex(Matrix3D* mtx, TechnoClass* pThis, TechnoTypeClass* pType, VoxelIndexKey& key, ILocomotion* iLoco , int height)
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
DEFINE_JUMP(LJMP, 0x706BDD, 0x706C01); // I checked it a priori

ASMJIT_PATCH(0x73C47A, UnitClass_DrawAsVXL_Shadow, 0x5)
{
	//Debug::LogInfo(__FUNCTION__" Exec");
	GET(UnitClass*, pThis, EBP);

	auto const loco = pThis->Locomotor.GetInterfacePtr();

	if (pThis->Type->NoShadow
		|| pThis->CloakState != CloakState::Uncloaked
		|| !loco->Is_To_Have_Shadow())
		return 0x73C5C9;

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
	TranslateAngleRotated(&shadow_matrix, pThis, pType , vxl_index_key);

	auto mtx = Game::VoxelDefaultMatrix() * (shadow_matrix);

	if (height > 0)
		shadow_point.Y += 1;

	const auto uTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (!pType->UseTurretShadow ) {
		if(uTypeExt->ShadowIndices.empty()) {
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
	}

	if (main_vxl == &pType->TurretVoxel
		|| (!pType->UseTurretShadow
			&& !uTypeExt->TurretShadow.Get(RulesExtData::Instance()->DrawTurretShadow)))
		return 0x73C5C9;

	uTypeExt->ApplyTurretOffset(&mtx, Game::Pixel_Per_Lepton());
	mtx.RotateZ(static_cast<float>(pThis->SecondaryFacing.Current().GetRadian<32>() - pThis->PrimaryFacing.Current().GetRadian<32>()));
	const bool inRecoil = pType->TurretRecoil && pThis->TurretRecoil.State != RecoilData::RecoilState::Inactive;

	if (inRecoil)
		mtx.TranslateX(-pThis->TurretRecoil.TravelSoFar);

	auto tur = TechnoTypeExtData::GetTurretsVoxelFixedUp(pType , pThis->CurrentTurretNumber);

	// sorry but you're fucked
	if (tur && tur->VXL && tur->HVA) {

		auto bar = TechnoTypeExtData::GetBarrelsVoxelFixedUp(pType, pThis->CurrentTurretNumber);
		auto haveBar = bar && bar->VXL && bar->HVA && !bar->VXL->LoadFailed;

		if (vxl_index_key.Is_Valid_Key())
			vxl_index_key.MinorVoxel.TurretFacing = pThis->SecondaryFacing.Current().GetFacing<32>();

		auto* cache = &pType->VoxelCaches.Shadow;

		if (!pType->UseTurretShadow)
		{
			if (haveBar)
				cache = nullptr;
			else
				cache = tur != &pType->TurretVoxel ?
				nullptr // man what can I say, you are fucked, for now
				: reinterpret_cast<decltype(cache)>(&pType->VoxelCaches.TurretBarrel) // excuse me
				;
		}

		pThis->DrawVoxelShadow(
			tur,
			0,
			(inRecoil ? std::bit_cast<VoxelIndexKey>(-1) : vxl_index_key),
			(inRecoil ? nullptr : cache),
			bounding,
			&why,
			&mtx,
			(!inRecoil && cache != nullptr),
			surface,
			shadow_point
		);

		// and you are utterly fucked
		if (haveBar){

			if (pType->TurretRecoil && pThis->BarrelRecoil.State != RecoilData::RecoilState::Inactive)
				mtx.TranslateX(-pThis->BarrelRecoil.TravelSoFar);

			mtx.ScaleX(static_cast<float>(std::cos(-pThis->BarrelFacing.Current().GetRadian<32>())));

			pThis->DrawVoxelShadow(
				bar,
				0,
				std::bit_cast<VoxelIndexKey>(-1), // no cache, no use
				nullptr,//no cache atm
				bounding,
				&why,
				&mtx,
				false,
				surface,
				shadow_point
			);
		}
	}

	return 0x73C5C9;
}

ASMJIT_PATCH(0x73B4A0, UnitClass_DrawVXL_WaterType, 9)
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

	R->EBX<ObjectTypeClass*>(Image);
	return 0x73B4DA;
}

ASMJIT_PATCH(0x715320, TechnoTypeClass_LoadFromINI_EarlyReader, 6)
{
	GET(CCINIClass*, pINI, EDI);
	GET(TechnoTypeClass*, pType, EBP);

	INI_EX exINI(pINI);
	auto pExt = TechnoTypeExtContainer::Instance.Find(pType);

	pExt->WaterImage.Read(exINI, pType->ID, "WaterImage");
	pExt->WaterImage_Yellow.Read(exINI, pType->ID, "WaterImage.ConditionYellow");
	pExt->WaterImage_Red.Read(exINI, pType->ID, "WaterImage.ConditionRed");

	pExt->Image_Yellow.Read(exINI, pType->ID, "Image.ConditionYellow");
	pExt->Image_Red.Read(exINI, pType->ID, "Image.ConditionRed");

	return 0;
}

ASMJIT_PATCH(0x73C485, UnitClass_DrawVXL_NoSpawnAlt_SkipShadow, 8)
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