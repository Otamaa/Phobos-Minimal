#include "Body.h"

#include <AnimClass.h>
#include <UnitClass.h>
#include <AircraftClass.h>
#include <InfantryClass.h>
#include <BuildingClass.h>
#include <ScenarioClass.h>
#include <HouseClass.h>
#include <SpawnManagerClass.h>
#include <BulletClass.h>


#include <Ext/AnimType/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/House/Body.h>
#include <Ext/Unit/Body.h>
#include <Ext/UnitType/Body.h>

#include <Utilities/Macro.h>
#include <Utilities/Patch.h>

#include <TacticalClass.h>

#include <Locomotor/JumpjetLocomotionClass.h>
#include <Locomotor/Cast.h>

ASMJIT_PATCH(0x711F60, TechnoTypeClass_RefundAmount_Disable, 0x8)
{
	GET(TechnoTypeClass*, pThis, ECX);

	if (TechnoTypeExtContainer::Instance.Find(pThis)->Soylent_Zero)
	{
		R->EAX(0);
		return 0x712036;
	}

	return 0x0;
}

ASMJIT_PATCH(0x71532B, TechnoTypeClass_LoadFromINI_BarrelAnimData_Fix, 0x8)
{
	GET(TechnoTypeClass*, pThis, EBP);
	GET_STACK(CCINIClass*, pINI, STACK_OFFSET(0x37C, 0x4));

	const auto pSection = pThis->ID;
	auto& barrelData = pThis->BarrelAnimData;

	barrelData.Travel = pINI->ReadInteger(pSection, "BarrelTravel", barrelData.Travel);
	barrelData.CompressFrames =MaxImpl(pINI->ReadInteger(pSection, "BarrelCompressFrames", barrelData.CompressFrames), 1);
	barrelData.HoldFrames = MaxImpl(pINI->ReadInteger(pSection, "BarrelHoldFrames", barrelData.HoldFrames), 1);
	barrelData.RecoverFrames = MaxImpl(pINI->ReadInteger(pSection, "BarrelRecoverFrames", barrelData.RecoverFrames), 1);

	R->ESI(pINI);

	return 0x7153DA;
}

ASMJIT_PATCH(0x711F39, TechnoTypeClass_CostOf_FactoryPlant, 0x8)
{
	GET(TechnoTypeClass*, pThis, ESI);
	GET(HouseClass*, pHouse, EDI);
	REF_STACK(float, mult, STACK_OFFSET(0x10, -0x8));

	auto const pHouseExt = HouseExtContainer::Instance.Find(pHouse);

	if (!pHouseExt->RestrictedFactoryPlants.empty())
		mult *= pHouseExt->GetRestrictedFactoryPlantMult(pThis);

	return 0;
}

ASMJIT_PATCH(0x711FDF, TechnoTypeClass_RefundAmount_FactoryPlant, 0x8)
{
	GET(TechnoTypeClass*, pThis, ESI);
	GET(HouseClass*, pHouse, EDI);
	REF_STACK(float, mult, STACK_OFFSET(0x10, -0x4));

	auto const pHouseExt = HouseExtContainer::Instance.Find(pHouse);

	if (!pHouseExt->RestrictedFactoryPlants.empty())
		mult *= pHouseExt->GetRestrictedFactoryPlantMult(pThis);

	return 0;
}

ASMJIT_PATCH(0x707319, TechnoClass_CalcVoxelShadow_ShadowScale, 0x6)
{
	GET(TechnoTypeClass*, pType, EAX);

	auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	if (pTypeExt->ShadowScale > 0)
	{
		REF_STACK(Matrix3D, mtx, STACK_OFFSET(0xE8, -0x90));
		mtx.Scale(pTypeExt->ShadowScale);
		return 0x707331;
	}

	return 0;
}

ASMJIT_PATCH(0x6F3E6E, FootClass_firecoord_6F3D60_TurretMultiOffset, 0x6) //0
{

	GET(TechnoTypeClass*, pType, EBP);
	LEA_STACK(Matrix3D*, mtx, STACK_OFFS(0xCC, 0x90));

	TechnoTypeExtContainer::Instance.Find(pType)->ApplyTurretOffset(mtx);

	return 0x6F3E85;
}

ASMJIT_PATCH(0x73B780, UnitClass_DrawVXL_TurretMultiOffset, 0x6) //0
{
	GET(TechnoTypeClass*, technoType, EAX);

	enum { CleanFlag = 0x73B78A, SkipFlag = 0x73B790 };

	const auto pDrawTypeExt = TechnoTypeExtContainer::Instance.Find(technoType);
	const auto& nOffs = TechnoTypeExtContainer::Instance.Find(technoType)->TurretOffset;

	return (nOffs->IsEmpty()
		&& pDrawTypeExt->ExtraTurretCount <= 0
		&& pDrawTypeExt->ExtraBarrelCount <= 0)
		? CleanFlag : SkipFlag;
}

ASMJIT_PATCH(0x73CCF4, UnitClass_DrawSHP_FacingsB_TurretShape, 0xA)
{
	enum { SkipGameCode = 0x73CD06 };

	GET(UnitClass*, pThis, EBP);
	GET(UnitTypeClass*, pType, ECX);

	const auto pTurretShape = UnitTypeExtContainer::Instance.Find(pType)->TurretShape;
	const int StartFrame = pTurretShape ? 0 : (pType->WalkFrames * pType->Facings);
	const int frameIdx = pThis->SecondaryFacing.Current().GetFacing<32>(4) + StartFrame;

	if (pTurretShape)
		R->EDI(pTurretShape);

	R->ECX(pThis);
	R->EAX(frameIdx);
	return 0x73CD06;
}

ASMJIT_PATCH(0x73C7AC, UnitClass_DrawAsSHP_DrawTurret_TintFix, 0x6)
{
	enum { SkipDrawCode = 0x73CE00 };

	GET(UnitClass*, pThis, EBP);

	const auto pThisType = pThis->Type;
	if (pThisType->BarrelVoxel.VXL && pThisType->BarrelVoxel.HVA)
		return 0;

	GET(UnitTypeClass*, pType, ECX);
	GET(SHPStruct*, pShape, EDI);
	GET(const int, bodyFrameIdx, EBX);
	REF_STACK(Point2D, location, STACK_OFFSET(0x128, 0x4));
	REF_STACK(RectangleStruct, bounds, STACK_OFFSET(0x128, 0xC));
	GET_STACK(const int, extraLight, STACK_OFFSET(0x128, 0x1C));

	const bool tooBigToFitUnderBridge = pType->TooBigToFitUnderBridge
		&& pThis->IsNearBridge() && !pThis->sub_703E70();
	const int zAdjust = tooBigToFitUnderBridge ? -16 : 0;
	const ZGradient zGradient = tooBigToFitUnderBridge ? ZGradient::Ground : pThis->GetZGradient();

	const auto pTurretShape = UnitTypeExtContainer::Instance.Find(pType)->TurretShape;
	const int StartFrame = pTurretShape ? 0 : (pType->WalkFrames * pType->Facings);

	if (pTurretShape)
		pShape = pTurretShape;

	pThis->Draw_A_SHP(pShape, bodyFrameIdx, &location, &bounds, 0, 256, zAdjust, zGradient, 0, extraLight, 0, 0, 0, 0, 0, 0);

	const auto secondaryDir = pThis->SecondaryFacing.Current();
	const int frameIdx = secondaryDir.GetFacing<32>(4) + StartFrame;

	const auto primaryDir = pThis->PrimaryFacing.Current();
	const double bodyRad = primaryDir.GetRadian<32>();
	Matrix3D mtx = Matrix3D::GetIdentity();
	mtx.RotateZ(static_cast<float>(bodyRad));
	TechnoTypeExtContainer::Instance.Find(pType)->ApplyTurretOffset(&mtx);
	const double turretRad = pType->Turret ? secondaryDir.GetRadian<32>() : bodyRad;
	mtx.RotateZ(static_cast<float>(turretRad - bodyRad));

	const auto res = mtx.GetTranslation();
	const auto offset = CoordStruct { static_cast<int>(res.X), static_cast<int>(-res.Y), static_cast<int>(res.Z) };
	Point2D drawPoint = location + TacticalClass::Instance->CoordsToScreen(offset);

	const bool originalDrawShadow = std::exchange(Game::bDrawShadow(), false);
	pThis->Draw_A_SHP(pShape, frameIdx, &drawPoint, &bounds, 0, 256, static_cast<DWORD>(-32), zGradient, 0, extraLight, 0, 0, 0, 0, 0, 0);
	Game::bDrawShadow = originalDrawShadow;
	return SkipDrawCode;
}

double UnitExtData::GetPrimaryRadian(UnitClass* pThis)
{
	// Align with the jj Draw_Matrix calc changing.
	if (auto const pJJLoco = locomotion_cast<JumpjetLocomotionClass*>(pThis->Locomotor)) {
		if (!pThis->IsAttackedByLocomotor)
			return pJJLoco->Facing.Current().GetRadian<32>();
	}

	return pThis->PrimaryFacing.Current().GetRadian<32>();
}

ASMJIT_PATCH(0x73BA12, UnitClass_DrawAsVXL_RewriteTurretDrawing, 0x6)
{
	enum { SkipGameCode = 0x73BEA4 };

	GET(UnitClass* const, pThis, EBP);
	GET(TechnoTypeClass* const, pDrawType, EBX);
	GET_STACK(const bool, haveTurretCache, STACK_OFFSET(0x1C4, -0x1B3));
	GET_STACK(const bool, haveBar, STACK_OFFSET(0x1C4, -0x1B2));
	GET(const bool, haveBarrelCache, EAX);
	REF_STACK(Matrix3D, drawMatrix, STACK_OFFSET(0x1C4, -0x130));
	GET_STACK(const int, flags, STACK_OFFSET(0x1C4, -0x198));
	GET_STACK(const int, brightness, STACK_OFFSET(0x1C4, 0x1C));
	GET_STACK(const int, hvaFrameIdx, STACK_OFFSET(0x1C4, -0x18C));
	GET_STACK(const int, currentTurretNumber, STACK_OFFSET(0x1C4, -0x1A8));
	LEA_STACK(Point2D* const, center, STACK_OFFSET(0x1C4, -0x194));
	LEA_STACK(RectangleStruct* const, rect, STACK_OFFSET(0x1C4, -0x164));

	// base matrix
	const auto mtx = Game::VoxelDefaultMatrix() * drawMatrix;

	const auto pDrawTypeExt = TechnoTypeExtContainer::Instance.Find(pDrawType);
	const bool notChargeTurret = pThis->Type->TurretCount <= 0 || pThis->Type->IsGattling;

	auto getTurretVoxel = [pDrawType, notChargeTurret, currentTurretNumber]() -> VoxelStruct*
		{
			if (notChargeTurret)
				return &pDrawType->TurretVoxel;

			return TechnoTypeExtData::GetTurretsVoxel(pDrawType, currentTurretNumber);
		};
	const auto pTurretVoxel = getTurretVoxel();

	// When in recoiling or have no cache, need to recalculate drawing matrix
	const bool inRecoil = pDrawType->TurretRecoil && (pThis->TurretRecoil.State != RecoilData::RecoilState::Inactive || pThis->BarrelRecoil.State != RecoilData::RecoilState::Inactive);
	const bool shouldRedraw = !haveTurretCache || haveBar && !haveBarrelCache || inRecoil;

	// When in recoiling, need to bypass cache and draw without saving
	const auto turKey = inRecoil ? -1 : flags;
	const auto turCache = inRecoil ? nullptr : &pDrawType->VoxelCaches.TurretWeapon;

	auto getTurretMatrix = [=, &mtx]() -> Matrix3D
		{
			Matrix3D mtxTurret = mtx;
			pDrawTypeExt->ApplyTurretOffset(&mtxTurret, Math::Pixel_Per_Lepton);

			mtxTurret.RotateZ(static_cast<float>(pThis->SecondaryFacing.Current().GetRadian<32>() - UnitExtData::GetPrimaryRadian(pThis)));

			if (pThis->TurretRecoil.State != RecoilData::RecoilState::Inactive)
				mtxTurret.TranslateX(-pThis->TurretRecoil.TravelSoFar);

			return mtxTurret;
		};

	auto mtxTurret = shouldRedraw ? getTurretMatrix() : mtx;
	constexpr BlitterFlags blit = BlitterFlags::Alpha | BlitterFlags::Flat;

	// Only when there is a barrel will its calculation and drawing be considered
	if (haveBar)
	{
		auto drawBarrel = [=, &mtxTurret, &mtx]()
			{
				// When in recoiling, need to bypass cache and draw without saving
				const auto brlKey = inRecoil ? -1 : flags;
				const auto brlCache = inRecoil ? nullptr : &pDrawType->VoxelCaches.TurretBarrel;

				auto getBarrelMatrix = [=, &mtxTurret, &mtx]() -> Matrix3D
					{
						auto mtxBarrel = mtxTurret;
						mtxBarrel.Translate(-mtx.Row[0].W, -mtx.Row[1].W, -mtx.Row[2].W);
						mtxBarrel.RotateY(static_cast<float>(-pThis->BarrelFacing.Current().GetRadian<32>()));

						if (pThis->BarrelRecoil.State != RecoilData::RecoilState::Inactive)
							mtxBarrel.TranslateX(-pThis->BarrelRecoil.TravelSoFar);

						mtxBarrel.Translate(mtx.Row[0].W, mtx.Row[1].W, mtx.Row[2].W);
						return mtxBarrel;
					};
				auto mtxBarrel = shouldRedraw ? getBarrelMatrix() : mtx;

				auto getBarrelVoxel = [pDrawType, notChargeTurret, currentTurretNumber]() -> VoxelStruct*
					{
						if (notChargeTurret)
							return &pDrawType->BarrelVoxel;

						return TechnoTypeExtData::GetBarrelsVoxel(pDrawType, currentTurretNumber);
					};
				const auto pBarrelVoxel = getBarrelVoxel();

				// draw barrel
				pThis->Draw_A_VXL(pBarrelVoxel, hvaFrameIdx, brlKey, brlCache, rect, center, &mtxBarrel, brightness, blit, 0);
			};

		const auto turretDir = pThis->SecondaryFacing.Current().GetFacing<4>();

		// The orientation of the turret can affect the layer order of the barrel and turret
		if (turretDir != 0 && turretDir != 3)
		{
			// draw turret
			pThis->Draw_A_VXL(pTurretVoxel, hvaFrameIdx, turKey, turCache, rect, center, &mtxTurret, brightness, blit, 0);

			drawBarrel();
		}
		else
		{
			drawBarrel();

			// draw turret
			pThis->Draw_A_VXL(pTurretVoxel, hvaFrameIdx, turKey, turCache, rect, center, &mtxTurret, brightness, blit, 0);
		}
	}
	else
	{
		pThis->Draw_A_VXL(pTurretVoxel, hvaFrameIdx, turKey, turCache, rect, center, &mtxTurret, brightness, blit, 0);
	}

	return SkipGameCode;
}

Matrix3D NOINLINE getTurretMatrix(const Matrix3D& mtx , UnitClass* pThis , TechnoTypeExtData* pDrawTypeExt) {

	Matrix3D mtx_turret = mtx;

	pDrawTypeExt->ApplyTurretOffset(&mtx_turret, Math::Pixel_Per_Lepton);
	mtx_turret.RotateZ(static_cast<float>(pThis->SecondaryFacing.Current().GetRadian<32>() - pThis->PrimaryFacing.Current().GetRadian<32>()));

	if (pThis->TurretRecoil.State != RecoilData::RecoilState::Inactive)
		mtx_turret.TranslateX(-pThis->TurretRecoil.TravelSoFar);

	return mtx_turret;
}

//TODO update
//ASMJIT_PATCH(0x73BA12, UnitClass_DrawAsVXL_RewriteTurretDrawing, 0x6)
//{
//	GET(UnitClass* const, pThis, EBP);
//	GET(UnitTypeClass* const, pDrawType, EBX);
//	GET_STACK(const bool, haveTurretCache, STACK_OFFSET(0x1C4, -0x1B3));
//	GET_STACK(const bool, haveBar, STACK_OFFSET(0x1C4, -0x1B2));
//	GET(const bool, haveBarrelCache, EAX);
//	REF_STACK(Matrix3D, draw_matrix, STACK_OFFSET(0x1C4, -0x130));
//	GET_STACK(const int, flags, STACK_OFFSET(0x1C4, -0x198));
//	GET_STACK(const int, brightness, STACK_OFFSET(0x1C4, 0x1C));
//	GET_STACK(const int, hvaFrameIdx, STACK_OFFSET(0x1C4, -0x18C));
//	GET_STACK(const int, currentTurretNumber, STACK_OFFSET(0x1C4, -0x1A8));
//	LEA_STACK(Point2D* const, center, STACK_OFFSET(0x1C4, -0x194));
//	LEA_STACK(RectangleStruct* const, rect, STACK_OFFSET(0x1C4, -0x164));
//
//	// base matrix
//	const Matrix3D mtx = Game::VoxelDefaultMatrix() * draw_matrix;
//
//	const auto pDrawTypeExt = TechnoTypeExtContainer::Instance.Find(pDrawType);
//
//	VoxelStruct* pTurretVoxel = TechnoTypeExtData::GetTurretsVoxelFixedUp(pDrawType, currentTurretNumber);
//
//	// When in recoiling or have no cache, need to recalculate drawing matrix
//	const bool inRecoil = pDrawType->TurretRecoil && (pThis->TurretRecoil.State != RecoilData::RecoilState::Inactive || pThis->BarrelRecoil.State != RecoilData::RecoilState::Inactive);
//	const bool shouldRedraw = !haveTurretCache || haveBar && !haveBarrelCache || inRecoil;
//
//	// When in recoiling, need to bypass cache and draw without saving
//	const auto turKey = inRecoil ? -1 : flags;
//	const auto turCache = inRecoil ? nullptr : reinterpret_cast<IndexClass<int, int>*>(&pDrawType->VoxelCaches.TurretWeapon);
//
//	Matrix3D mtx_turret = shouldRedraw ? getTurretMatrix(mtx, pThis , pDrawTypeExt) : mtx;
//
//	// 10240u -> (BlitterFlags::Alpha | BlitterFlags::Flat);
//
//	// Only when there is a barrel will its calculation and drawing be considered
//	if (haveBar)
//	{
//		auto drawBarrel = [=, &mtx_turret, &mtx]()
//			{
//				// When in recoiling, need to bypass cache and draw without saving
//				const auto brlKey = inRecoil ? -1 : flags;
//				const auto brlCache = inRecoil ? nullptr : reinterpret_cast<IndexClass<int, int>*>(&pDrawType->VoxelCaches.TurretBarrel);
//
//				auto getBarrelMatrix = [=, &mtx_turret, &mtx]() -> Matrix3D
//					{
//						auto mtx_barrel = mtx_turret;
//						mtx_barrel.Translate(-mtx.Row[0].W, -mtx.Row[1].W, -mtx.Row[2].W);
//						mtx_barrel.RotateY(static_cast<float>(-pThis->BarrelFacing.Current().GetRadian<32>()));
//
//						if (pThis->BarrelRecoil.State != RecoilData::RecoilState::Inactive)
//							mtx_barrel.TranslateX(-pThis->BarrelRecoil.TravelSoFar);
//
//						mtx_barrel.Translate(mtx.Row[0].W, mtx.Row[1].W, mtx.Row[2].W);
//						return mtx_barrel;
//					};
//				auto mtx_barrel = shouldRedraw ? getBarrelMatrix() : mtx;
//				const auto pBarrelVoxel = TechnoTypeExtData::GetBarrelsVoxelFixedUp(pDrawType, currentTurretNumber);
//
//				// draw barrel
//				pThis->Draw_A_VXL(pBarrelVoxel, hvaFrameIdx, brlKey, brlCache, rect, center, &mtx_barrel, brightness, 10240u, 0);
//			};
//
//		const auto turretDir = pThis->SecondaryFacing.Current().GetFacing<4>();
//
//		// The orientation of the turret can affect the layer order of the barrel and turret
//		if (turretDir != 0 && turretDir != 3)
//		{
//			// draw turret
//			pThis->Draw_A_VXL(pTurretVoxel, hvaFrameIdx, turKey, turCache, rect, center, &mtx_turret, brightness, 10240u, 0);
//			drawBarrel();
//		}
//		else
//		{
//			drawBarrel();
//			// draw turret
//			pThis->Draw_A_VXL(pTurretVoxel, hvaFrameIdx, turKey, turCache, rect, center, &mtx_turret, brightness, 10240u, 0);
//		}
//	}
//	else
//	{
//		pThis->Draw_A_VXL(pTurretVoxel, hvaFrameIdx, turKey, turCache, rect, center, &mtx_turret, brightness, 10240u, 0);
//	}
//
//	return 0x73BEA4;
//}

ASMJIT_PATCH(0x73C890, UnitClass_Draw_1_TurretMultiOffset, 0x8) //0
{
	GET(TechnoTypeClass*, pType, EAX);
	LEA_STACK(Matrix3D*, mtx, 0x80);

	TechnoTypeExtContainer::Instance.Find(pType)->ApplyTurretOffset(mtx, TechnoTypeExtData::TurretMultiOffsetOneByEightMult);

	return 0x73C8B7;
}

ASMJIT_PATCH(0x43E0C4, BuildingClass_Draw_43DA80_TurretMultiOffset, 0x5) //0
{
	GET(TechnoTypeClass*, pType, EDX);
	LEA_STACK(Matrix3D*, mtx, 0x60);

	TechnoTypeExtContainer::Instance.Find(pType)->ApplyTurretOffset(mtx, TechnoTypeExtData::TurretMultiOffsetOneByEightMult);

	return 0x43E0E8;
}

ASMJIT_PATCH(0x73CCE1, UnitClass_DrawSHP_TurretOffest, 0x6)
{
	GET(UnitClass*, pThis, EBP);
	REF_STACK(Point2D, pos, STACK_OFFSET(0x15C, -0xE8));

	Matrix3D mtx = Matrix3D::GetIdentity();
	mtx.RotateZ(static_cast<float>(pThis->PrimaryFacing.Current().GetRadian<32>()));

	TechnoTypeExtContainer::Instance.Find(pThis->Type)->ApplyTurretOffset(&mtx);

	double turretRad = pThis->TurretFacing().GetRadian<32>();
	double bodyRad = pThis->PrimaryFacing.Current().GetRadian<32>();
	float angle = (float)(turretRad - bodyRad);
	mtx.RotateZ(angle);
	auto res = mtx.GetTranslation();
	CoordStruct location { static_cast<int>(res.X), static_cast<int>(-res.Y), static_cast<int>(res.Z) };
	Point2D temp = TacticalClass::Instance()->CoordsToScreen(location);
	pos += temp;

	return 0;
}

#include <Ext/Cell/Body.h>


// ASMJIT_PATCH(0x73CF46, UnitClass_Draw_It_KeepUnitVisible, 0x6)
// {
// 	GET(UnitClass*, pThis, ESI);

// 	if((pThis->Deploying || pThis->Undeploying) &&
// 		TechnoTypeExtContainer::Instance.Find(pThis->Type)->DeployingAnim_KeepUnitVisible){
// 			return 0x73CF62;
// 		}

// 	return 0;
// }

// Ares hooks in at 739B8A, this goes before it and skips it if needed.
// ASMJIT_PATCH(0x739B7C, UnitClass_Deploy_DeployDir, 0x6)
// {
// 	enum { SkipAnim = 0x739C70, PlayAnim = 0x739B9E };
//
// 	GET(UnitClass*, pThis, ESI);
//
// 	if (!pThis->InAir)
// 	{
// 		if (pThis->Type->DeployingAnim)
// 		{
// 			return (TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType())->DeployingAnim_AllowAnyDirection.Get()) ? PlayAnim : 0;
// 		}
//
// 		pThis->Deployed = true;
// 	}
//
// 	return SkipAnim;
// }

AnimTypeClass* GetDeployAnim(UnitClass* pThis)
{
	//auto const pExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);

	//if (pExt->DeployAnims.empty())
	//	return nullptr;

	//if(((pExt->DeployAnims.size() & 28u) != 0u))
	//	return pExt->DeployAnims[0];

	//const auto nIdx = (((pExt->DeployAnims.size() >> 2) * (((pThis->PrimaryFacing.Current().Raw >> 7) + 1) >> 1)) >> 8);
	//return pExt->DeployAnims[nIdx];
	return pThis->Type->DeployingAnim;
}

// bool NOINLINE SetAnim(AnimTypeClass* pAnimType , UnitClass* pUnit , bool isDeploying)
// {
// 	if(pUnit->DeployAnim) {
// 		return true;
// 	}
//
// 	auto const pExt = TechnoTypeExtContainer::Instance.Find(pUnit->Type);
//
// 	if (pAnimType) {
// 		auto const pAnim = GameCreate<AnimClass>(pAnimType,
// 			pUnit->Location, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0,
// 				!isDeploying ? pExt->DeployingAnim_ReverseForUndeploy.Get() : false);
//
// 			pUnit->DeployAnim = pAnim;
// 			pAnim->SetOwnerObject(pUnit);
//
// 			if (pExt->DeployingAnim_UseUnitDrawer) {
// 				pAnim->LightConvert = pUnit->GetRemapColour();
// 			}
//
// 		return true;
// 	}
//
// 	return false;
// }

//ASMJIT_PATCH(0x739B7C, UnitClass_SimpleDeploy_Facing, 0x6)
//{
//	GET(UnitClass*, pThis, ESI);
//	auto const pType = pThis->Type;
//	enum { PlayDeploySound = 0x739C70  , SetAnimTimer = 0x739C20 , SetDeployingState = 0x739C62 };
//	//auto const pExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);
//
//	if (!pThis->InAir)
//	{
//		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
//
//		if (!pTypeExt->DeployingAnim_AllowAnyDirection)
//		{
//			// not sure what is the bitfrom or bitto so it generate this result
//			// yes iam dum , iam sorry - otamaa
//			const auto nRulesDeployDir = ((((RulesClass::Instance->DeployDir) >> 4) + 1) >> 1) & 7;
//			const FacingType nRaw = pTypeExt->DeployDir.isset() ? pTypeExt->DeployDir.Get() : (FacingType)nRulesDeployDir;
//			const auto nCurrent = (((((pThis->PrimaryFacing.Current().Raw) >> 12) + 1) >> 1) & 7);
//
//			if (nCurrent != (int)nRaw)
//			{
//				if (const auto pLoco = pThis->Locomotor.GetInterfacePtr())
//				{
//					if (!pLoco->Is_Moving_Now())
//					{
//						pLoco->Do_Turn(DirStruct { nRaw });
//					}
//
//					return PlayDeploySound; //adjust the facing first
//				}
//			}
//		}
//
//		if (const auto pAnimType = GetDeployAnim(pThis))
//		{
//			if(!pThis->DeployAnim) {
//				auto const pAnim = GameCreate<AnimClass>(pAnimType,
//				pThis->Location, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false);
//
//				pThis->DeployAnim = pAnim;
//				pAnim->SetOwnerObject(pThis);
//
//				if (pTypeExt->DeployingAnim_UseUnitDrawer) {
//					pAnim->LightConvert = pThis->GetRemapColour();
//				}
//			}
//
//			pThis->Animation.Stage = pAnimType->Start;
//			pThis->Animation.Timer.Start(pAnimType->Rate);
//		}
//
//		pThis->Deployed = true;
//	}
//
//	return PlayDeploySound;
//}

//ASMJIT_PATCH(0x739D73 , UnitClass_UnDeploy_DeployAnim , 0x6)
//{
//	GET(UnitClass*, pThis, ESI);
//
//	const auto pAnimType = GetDeployAnim(pThis);
//
//	if(!pAnimType)
//		return 0x739E4F;
//
//	if(pThis->DeployAnim)
//		return 0x739E04;
//
//	auto const pExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);
//
//	auto const pAnim = GameCreate<AnimClass>(pAnimType,
//	pThis->Location, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0,
//	pExt->DeployingAnim_ReverseForUndeploy);
//
//	pThis->DeployAnim = pAnim;
//	pAnim->SetOwnerObject(pThis);
//
//	if (pExt->DeployingAnim_UseUnitDrawer) {
//		pAnim->LightConvert = pThis->GetRemapColour();
//	}
//
//	return 0x739E04;
//}

//ASMJIT_PATCH(0x714706, TechnoTypeClass_read_DeployAnim, 0x9)
//{
//	GET(TechnoTypeClass*, pThis, EBP);
//	pThis->UnloadingClass = R->EAX<UnitTypeClass*>();
//	R->EAX((UnitTypeClass*)nullptr);
//	return 0x71473F;
//}

// ASMJIT_PATCH_AGAIN(0x739D8B, UnitClass_DeployUndeploy_DeployAnim, 0x5)
// ASMJIT_PATCH(0x739BA8, UnitClass_DeployUndeploy_DeployAnim, 0x5)
// {
// 	enum { Deploy = 0x739C20, DeployUseUnitDrawer = 0x739C0A, Undeploy = 0x739E04, UndeployUseUnitDrawer = 0x739DEE };
//
// 	GET(UnitClass*, pThis, ESI);
//
// 	bool isDeploying = R->Origin() == 0x739BA8;
// 	auto const pExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);
//
//
// 	if (auto const pAnim = GameCreate<AnimClass>(pThis->Type->DeployingAnim,
// 			pThis->Location, 0, 1, 0x600, 0,
// 			!isDeploying ? pExt->DeployingAnim_ReverseForUndeploy : false))
// 	{
// 			pThis->DeployAnim = pAnim;
// 			pAnim->SetOwnerObject(pThis);
//
// 			if (pExt->DeployingAnim_UseUnitDrawer)
// 				return isDeploying ? DeployUseUnitDrawer : UndeployUseUnitDrawer;
// 	} else {
// 			pThis->DeployAnim = nullptr;
// 	}
//
// 	return isDeploying ? Deploy : Undeploy;
// }
//
//ASMJIT_PATCH(0x739C86, UnitClass_DeployUndeploy_DeploySound, 0x6)
//{
//	enum { DeployReturn = 0x739CBF, UndeployReturn = 0x739EB8 };
//
//	GET(UnitClass*, pThis, ESI);
//
//	const bool isDeploying = R->Origin() == 0x739C86;
//	const bool isDoneWithDeployUndeploy = isDeploying ? pThis->Deployed : !pThis->Deployed;
//
//	if (isDoneWithDeployUndeploy)
//		return 0; // Only play sound when done with deploying or undeploying.
//
//	return isDeploying ? DeployReturn : UndeployReturn;
//}ASMJIT_PATCH_AGAIN(0x739E81, UnitClass_DeployUndeploy_DeploySound, 0x6)

#include <Locomotor/HoverLocomotionClass.h>

// namespace SimpleDeployerTemp {
// 	bool HoverDeployedToLand = false;
// 	AnimTypeClass* DeployingAnim = nullptr;
// }

// ASMJIT_PATCH(0x739CBF, UnitClass_Deploy_DeployToLandHover, 0x5)
// {
// 	GET(UnitClass*, pThis, ESI);

// 	if (pThis->Deployed && pThis->Type->DeployToLand && pThis->Type->Locomotor == HoverLocomotionClass::ClassGUID())
// 		SimpleDeployerTemp::HoverDeployedToLand = true;

// 	return 0;
// }

// ASMJIT_PATCH(0x73E5B1, UnitClass_Unload_DeployToLandHover, 0x8)
// {
// 	if (SimpleDeployerTemp::HoverDeployedToLand)
// 	{
// 		GET(UnitClass*, pThis, ESI);

// 		// Ares' DeployToLand 'fix' for Hover IsSimpleDeployer vehicles does not set/reset certain values
// 		// and has a chance to get stuck in Unload mission as a result, following should remedy that.
// 		pThis->SetHeight(0);
// 		pThis->InAir = false;
// 		pThis->ForceMission(Mission::Guard);
// 	}

// 	SimpleDeployerTemp::HoverDeployedToLand = false;
// 	return 0;
// }ASMJIT_PATCH_AGAIN(0x73DED8, UnitClass_Unload_DeployToLandHover, 0x7)

// // Trick Ares into thinking it can deploy in any direction if anim does not constrain it by temporarily removing the anim.
// ASMJIT_PATCH(0x514325, HoverLocomotionClass_Process_DeployingAnim1, 0x8)
// {
// 	GET(ILocomotion*, iLoco, ESI);
// 	GET(bool, isMoving, EAX);
//
// 	auto const pLinkedTo = static_cast<LocomotionClass*>(iLoco)->LinkedTo;
// 	auto const pType = pLinkedTo->GetTechnoType();
//
// 	if (pType->DeployToLand && pType->DeployingAnim)
// 	{
// 		auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
//
// 		if (pTypeExt->DeployingAnim_AllowAnyDirection)
// 		{
// 			SimpleDeployerTemp::DeployingAnim = pType->DeployingAnim;
// 			pType->DeployingAnim = nullptr;
// 		}
// 	}
//
// 	return isMoving ? 0x51432D : 0x514A21;
// }

// // Restore the DeployingAnim to normal after.
// ASMJIT_PATCH(0x514AD0, HoverLocomotionClass_Process_DeployingAnim2, 0x5)
// {
// 	GET(ILocomotion*, iLoco, ESI);
//
// 	if (SimpleDeployerTemp::DeployingAnim)
// 	{
// 		auto const pLinkedTo = static_cast<LocomotionClass*>(iLoco)->LinkedTo;
// 		auto const pType = pLinkedTo->GetTechnoType();
// 		pType->DeployingAnim = SimpleDeployerTemp::DeployingAnim;
// 		SimpleDeployerTemp::DeployingAnim = nullptr;
// 	}
//
// 	return 0;
// }

// Issue #503
// Author : Otamaa
ASMJIT_PATCH(0x4AE670, DisplayClass_GetToolTip_EnemyUIName, 0x8)
{
	enum { SetUIName = 0x4AE678 };

	GET(ObjectClass*, pObject, ECX);

	if (!HouseExtData::IsObserverPlayer())
	{
		if (auto pFoot = flag_cast_to<FootClass*, false>(pObject))
		{
			if (!pObject->IsDisguised())
			{
				if (const auto pOwnerHouse = pFoot->GetOwningHouse())
				{
					if (!pOwnerHouse->IsNeutral() && !pOwnerHouse->IsAlliedWith(HouseClass::CurrentPlayer()))
					{
						const auto pTechnoTypeExt = GET_TECHNOTYPEEXT(pFoot);
						{
							if (!pTechnoTypeExt->EnemyUIName.Get().empty())
							{
								R->EAX(pTechnoTypeExt->EnemyUIName.Get().Text);
								return SetUIName;
							}
						}
					}
				}
			}
		}
	}

	R->EAX(pObject->GetUIName());
	return SetUIName;
}

int __fastcall TechnoTypeExtContainer::__Repair_Cost(TechnoTypeClass* pThis)
{
	if (pThis->Strength <= 0)
		return 1;

	int cost = pThis->GetCost();

	if (!cost)
		return 1;

	int nStep = RulesClass::Instance->RepairStep;

	if (nStep <= 0)
		nStep = 1;

	int calc = int((double(cost) / (double(pThis->Strength) / double(nStep))
		* RulesClass::Instance->RepairPercent));

	if (calc <= 0)
		calc = 1;

	return nStep;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x7120D0, TechnoTypeExtContainer::__Repair_Cost);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E2918, TechnoTypeExtContainer::__Repair_Cost);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F4F88, TechnoTypeExtContainer::__Repair_Cost);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F62C8, TechnoTypeExtContainer::__Repair_Cost);