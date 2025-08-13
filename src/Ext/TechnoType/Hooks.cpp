#include <AnimClass.h>
#include <UnitClass.h>
#include <AnimClass.h>
#include <InfantryClass.h>
#include <BuildingClass.h>
#include <ScenarioClass.h>
#include <HouseClass.h>
#include <SpawnManagerClass.h>
#include <BulletClass.h>

#include "Body.h"
#include <Ext/AnimType/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/House/Body.h>

#include <TacticalClass.h>

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

// ASMJIT_PATCH(0x711F39, TechnoTypeClass_CostOf_FactoryPlant, 0x8)
// {
// 	GET(TechnoTypeClass*, pThis, ESI);
// 	GET(HouseClass*, pHouse, EDI);
// 	REF_STACK(float, mult, STACK_OFFSET(0x10, -0x8));
//
// 	auto const pHouseExt = HouseExtContainer::Instance.Find(pHouse);
//
// 	if (!pHouseExt->RestrictedFactoryPlants.empty())
// 		mult *= pHouseExt->GetRestrictedFactoryPlantMult(pThis);

// 	return 0;
// }

// ASMJIT_PATCH(0x711FDF, TechnoTypeClass_RefundAmount_FactoryPlant, 0x8)
// {
// 	GET(TechnoTypeClass*, pThis, ESI);
// 	GET(HouseClass*, pHouse, EDI);
// 	REF_STACK(float, mult, STACK_OFFSET(0x10, -0x4));
//
// 	auto const pHouseExt = HouseExtContainer::Instance.Find(pHouse);
//
// 	if (!pHouseExt->RestrictedFactoryPlants.empty())
// 		mult *= pHouseExt->GetRestrictedFactoryPlantMult(pThis);
//
// 	return 0;
// }

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

	const auto& nOffs = TechnoTypeExtContainer::Instance.Find(pType)->TurretOffset;

	float x = static_cast<float>(nOffs->X * TechnoTypeExtData::TurretMultiOffsetDefaultMult);
	float y = static_cast<float>(nOffs->Y * TechnoTypeExtData::TurretMultiOffsetDefaultMult);
	float z = static_cast<float>(nOffs->Z * TechnoTypeExtData::TurretMultiOffsetDefaultMult);

	mtx->Translate(x, y, z);

	return 0x6F3E85;
}

ASMJIT_PATCH(0x73B780, UnitClass_DrawVXL_TurretMultiOffset, 0x6) //0
{
	GET(TechnoTypeClass*, technoType, EAX);

	const auto& nOffs = TechnoTypeExtContainer::Instance.Find(technoType)->TurretOffset;

	return nOffs.Get() == Vector3D<int>::Empty ?
		0x73B78A : 0x73B790;
}

#ifndef _old

 ASMJIT_PATCH(0x73BA4C, UnitClass_DrawVXL_TurretMultiOffset1, 0x6) //0
 {
 	GET(TechnoTypeClass*, pType, EBX);
 	LEA_STACK(Matrix3D*, mtx, STACK_OFFS(0x1D0, 0x13C));

 	const auto& nOffs = TechnoTypeExtContainer::Instance.Find(pType)->TurretOffset;

 	float x = static_cast<float>(nOffs->X * Game::Pixel_Per_Lepton());
 	float y = static_cast<float>(nOffs->Y * Game::Pixel_Per_Lepton());
 	float z = static_cast<float>(nOffs->Z * Game::Pixel_Per_Lepton());

 	mtx->Translate(x, y, z);

 	return 0x73BA68;
 }

#ifdef FUCKED
ASMJIT_PATCH(0x73BA12, UnitClass_DrawAsVXL_RewriteCalculateTurretMatrix, 0x6)
{
	enum { SkipGameCode = 0x73BEA4 };

	GET(UnitClass* const, pThis, EBP);
	GET(UnitTypeClass* const, pDrawType, EBX);
	GET_STACK(const bool, haveTurretCache, STACK_OFFSET(0x1C4, -0x1B3));
	GET_STACK(const bool, haveBarrelVXL, STACK_OFFSET(0x1C4, -0x1B2));
	GET(const bool, haveBarrelCache, EAX);
	LEA_STACK(Matrix3D* const, pMtx_buffer1, STACK_OFFSET(0x1C4, -0x130));
	LEA_STACK(Matrix3D* const, pMtx_turret, STACK_OFFSET(0x1C4, -0xF0));
	LEA_STACK(Matrix3D* const, pMtx_barrel, STACK_OFFSET(0x1C4, -0x90));

	if (haveTurretCache && (!haveBarrelVXL || haveBarrelCache) && (!pDrawType->TurretRecoil // When in recoiling, need to recalculate drawing matrix
		|| pThis->TurretRecoil.State == RecoilData::RecoilState::Inactive && pThis->BarrelRecoil.State == RecoilData::RecoilState::Inactive))
	{
		memcpy(pMtx_turret, pMtx_buffer1, sizeof(Matrix3D));
		memcpy(pMtx_barrel, pMtx_buffer1, sizeof(Matrix3D));
	}
	else
	{
		LEA_STACK(Matrix3D* const, pMtx_buffer2, STACK_OFFSET(0x1C4, -0xC0));
		LEA_STACK(Matrix3D* const, pMtx_buffer3, STACK_OFFSET(0x1C4, -0x60));
		LEA_STACK(Matrix3D* const, pMtx_buffer4, STACK_OFFSET(0x1C4, -0x30));

		// Turret
		const auto& nOffs = TechnoTypeExtContainer::Instance.Find(pDrawType)->TurretOffset;

		float x = static_cast<float>(nOffs->X * Game::Pixel_Per_Lepton());
		float y = static_cast<float>(nOffs->Y * Game::Pixel_Per_Lepton());
		float z = static_cast<float>(nOffs->Z * Game::Pixel_Per_Lepton());

		pMtx_buffer1->Translate(x, y, z);
		pMtx_buffer1->RotateZ(static_cast<float>(pThis->SecondaryFacing.Current().GetRadian<32>() - pThis->PrimaryFacing.Current().GetRadian<32>()));

		if (pDrawType->TurretRecoil && pThis->TurretRecoil.State != RecoilData::RecoilState::Inactive)
			pMtx_buffer1->TranslateX(-pThis->TurretRecoil.TravelSoFar);

		memcpy(pMtx_buffer2, pMtx_buffer1, sizeof(Matrix3D));
		memcpy(pMtx_buffer4, &Game::VoxelDefaultMatrix(), sizeof(Matrix3D));
		memcpy(pMtx_turret, Matrix3D::MatrixMultiply(pMtx_buffer3, pMtx_buffer4, pMtx_buffer1), sizeof(Matrix3D));

		// Barrel
		pMtx_buffer2->Translate(-pMtx_buffer1->Row[0].W, -pMtx_buffer1->Row[1].W, -pMtx_buffer1->Row[2].W);
		pMtx_buffer2->RotateY(static_cast<float>(-pThis->BarrelFacing.Current().GetRadian<32>()));

		if (pDrawType->TurretRecoil && pThis->BarrelRecoil.State != RecoilData::RecoilState::Inactive)
			pMtx_buffer2->TranslateX(-pThis->BarrelRecoil.TravelSoFar);

		pMtx_buffer2->Translate(pMtx_buffer1->Row[0].W, pMtx_buffer1->Row[1].W, pMtx_buffer1->Row[2].W);
		memcpy(pMtx_buffer3, &Game::VoxelDefaultMatrix(), sizeof(Matrix3D));
		memcpy(pMtx_barrel, Matrix3D::MatrixMultiply(pMtx_buffer4, pMtx_buffer3, pMtx_buffer2), sizeof(Matrix3D));
	}

	GET_STACK(const int, flags, STACK_OFFSET(0x1C4, -0x198));
	GET_STACK(const int, brightness, STACK_OFFSET(0x1C4, 0x1C));
	GET_STACK(const int, hvaFrameIdx, STACK_OFFSET(0x1C4, -0x18C));
	LEA_STACK(Point2D* const, center, STACK_OFFSET(0x1C4, -0x194));
	LEA_STACK(RectangleStruct* const, rect, STACK_OFFSET(0x1C4, -0x164));

	if (pThis->Type->TurretCount <= 0 || pThis->Type->IsGattling)
	{
		bool notDrawBarrelYet = true;
		const bool canDrawBarrel = pDrawType->BarrelVoxel.VXL && pDrawType->BarrelVoxel.HVA;
		// When in recoiling, bypass cache and draw without saving
		const bool inRecoil = pDrawType->TurretRecoil
			&& (pThis->BarrelRecoil.State != RecoilData::RecoilState::Inactive
				|| pThis->TurretRecoil.State != RecoilData::RecoilState::Inactive);

		// Barrel behind
		if (canDrawBarrel) // Adjusted the inspection sequence
		{
			const auto dir = pThis->SecondaryFacing.Current().GetFacing<4>();

			if (dir == 0 || dir == 3)
			{
				const auto brlKey = inRecoil ? -1 : flags;
				const auto brlCache = inRecoil ? nullptr : reinterpret_cast<IndexClass<int, int>*>(&pDrawType->VoxelCaches.TurretBarrel);

				pThis->Draw_A_VXL(&pDrawType->BarrelVoxel, hvaFrameIdx, brlKey, brlCache, rect, center, pMtx_barrel, brightness,
					static_cast<DWORD>(static_cast<BlitterFlags>(BlitterFlags::Alpha | BlitterFlags::Flat)), 0);

				notDrawBarrelYet = false;
			}
		}

		// Turret
		const auto turKey = inRecoil ? -1 : flags;
		const auto turCache = inRecoil ? nullptr : reinterpret_cast<IndexClass<int, int>*>(&pDrawType->VoxelCaches.TurretWeapon);

		pThis->Draw_A_VXL(&pDrawType->TurretVoxel, hvaFrameIdx, turKey, turCache, rect, center, pMtx_turret, brightness,
			static_cast<DWORD>(static_cast<BlitterFlags>(BlitterFlags::Alpha | BlitterFlags::Flat)), 0);

		// Barrel above
		if (canDrawBarrel && notDrawBarrelYet) // Adjusted the inspection sequence
		{
			const auto brlKey = inRecoil ? -1 : flags;
			const auto brlCache = inRecoil ? nullptr : reinterpret_cast<IndexClass<int, int>*>(&pDrawType->VoxelCaches.TurretBarrel);

			pThis->Draw_A_VXL(&pDrawType->BarrelVoxel, hvaFrameIdx, brlKey, brlCache, rect, center, pMtx_barrel, brightness,
				static_cast<DWORD>(static_cast<BlitterFlags>(BlitterFlags::Alpha | BlitterFlags::Flat)), 0);
		}
	}
	else
	{
		GET_STACK(const int, currentTurretNumber, STACK_OFFSET(0x1C4, -0x1A8));

		bool notDrawBarrelYet = true;
		// When in recoiling, bypass cache and draw without saving
		const bool inRecoil = pDrawType->TurretRecoil
			&& (pThis->BarrelRecoil.State != RecoilData::RecoilState::Inactive
				|| pThis->TurretRecoil.State != RecoilData::RecoilState::Inactive);

		// Barrel behind
		if (haveBarrelVXL) // Adjusted the inspection sequence
		{
			const auto dir = pThis->SecondaryFacing.Current().GetFacing<4>();

			if (dir == 0 || dir == 3)
			{
				const auto brlKey = inRecoil ? -1 : flags;
				const auto brlCache = inRecoil ? nullptr : reinterpret_cast<IndexClass<int, int>*>(&pDrawType->VoxelCaches.TurretBarrel);

				const auto pBarrelVoxel = TechnoTypeExtData::GetBarrelsVoxelFixedUp(pDrawType, currentTurretNumber);

				pThis->Draw_A_VXL(pBarrelVoxel, hvaFrameIdx, brlKey, brlCache, rect, center, pMtx_barrel, brightness,
					static_cast<DWORD>(static_cast<BlitterFlags>(BlitterFlags::Alpha | BlitterFlags::Flat)), 0);

				notDrawBarrelYet = false;
			}
		}

		// Turret
		const auto turKey = inRecoil ? -1 : flags;
		const auto turCache = inRecoil ? nullptr : reinterpret_cast<IndexClass<int, int>*>(&pDrawType->VoxelCaches.TurretWeapon);

		const auto pTurretVoxel = TechnoTypeExtData::GetTurretsVoxelFixedUp(pDrawType, currentTurretNumber);

		pThis->Draw_A_VXL(pTurretVoxel, hvaFrameIdx, turKey, turCache, rect, center, pMtx_turret, brightness,
			static_cast<DWORD>(static_cast<BlitterFlags>(BlitterFlags::Alpha | BlitterFlags::Flat)), 0);

		// Barrel above
		if (haveBarrelVXL && notDrawBarrelYet) // Adjusted the inspection sequence
		{
			const auto brlKey = inRecoil ? -1 : flags;
			const auto brlCache = inRecoil ? nullptr : reinterpret_cast<IndexClass<int, int>*>(&pDrawType->VoxelCaches.TurretBarrel);

			const auto pBarrelVoxel = TechnoTypeExtData::GetBarrelsVoxelFixedUp(pDrawType ,currentTurretNumber);

			pThis->Draw_A_VXL(pBarrelVoxel, hvaFrameIdx, brlKey, brlCache, rect, center, pMtx_barrel, brightness,
				static_cast<DWORD>(static_cast<BlitterFlags>(BlitterFlags::Alpha | BlitterFlags::Flat)), 0);
		}
	}

	return SkipGameCode;
}
#endif

#else

Matrix3D NOINLINE getTurretMatrix(const Matrix3D& mtx , UnitClass* pThis , TechnoTypeExtData* pDrawTypeExt) {

	Matrix3D mtx_turret = mtx;

	pDrawTypeExt->ApplyTurretOffset(&mtx_turret, Game::Pixel_Per_Lepton());
	mtx_turret.RotateZ(static_cast<float>(pThis->SecondaryFacing.Current().GetRadian<32>() - pThis->PrimaryFacing.Current().GetRadian<32>()));

	if (pThis->TurretRecoil.State != RecoilData::RecoilState::Inactive)
		mtx_turret.TranslateX(-pThis->TurretRecoil.TravelSoFar);

	return mtx_turret;
}

//TODO update
ASMJIT_PATCH(0x73BA12, UnitClass_DrawAsVXL_RewriteTurretDrawing, 0x6)
{
	GET(UnitClass* const, pThis, EBP);
	GET(UnitTypeClass* const, pDrawType, EBX);
	GET_STACK(const bool, haveTurretCache, STACK_OFFSET(0x1C4, -0x1B3));
	GET_STACK(const bool, haveBar, STACK_OFFSET(0x1C4, -0x1B2));
	GET(const bool, haveBarrelCache, EAX);
	REF_STACK(Matrix3D, draw_matrix, STACK_OFFSET(0x1C4, -0x130));
	GET_STACK(const int, flags, STACK_OFFSET(0x1C4, -0x198));
	GET_STACK(const int, brightness, STACK_OFFSET(0x1C4, 0x1C));
	GET_STACK(const int, hvaFrameIdx, STACK_OFFSET(0x1C4, -0x18C));
	GET_STACK(const int, currentTurretNumber, STACK_OFFSET(0x1C4, -0x1A8));
	LEA_STACK(Point2D* const, center, STACK_OFFSET(0x1C4, -0x194));
	LEA_STACK(RectangleStruct* const, rect, STACK_OFFSET(0x1C4, -0x164));

	// base matrix
	const Matrix3D mtx = Game::VoxelDefaultMatrix() * draw_matrix;

	const auto pDrawTypeExt = TechnoTypeExtContainer::Instance.Find(pDrawType);

	VoxelStruct* pTurretVoxel = TechnoTypeExtData::GetTurretsVoxelFixedUp(pDrawType, currentTurretNumber);

	// When in recoiling or have no cache, need to recalculate drawing matrix
	const bool inRecoil = pDrawType->TurretRecoil && (pThis->TurretRecoil.State != RecoilData::RecoilState::Inactive || pThis->BarrelRecoil.State != RecoilData::RecoilState::Inactive);
	const bool shouldRedraw = !haveTurretCache || haveBar && !haveBarrelCache || inRecoil;

	// When in recoiling, need to bypass cache and draw without saving
	const auto turKey = inRecoil ? -1 : flags;
	const auto turCache = inRecoil ? nullptr : reinterpret_cast<IndexClass<int, int>*>(&pDrawType->VoxelCaches.TurretWeapon);

	Matrix3D mtx_turret = shouldRedraw ? getTurretMatrix(mtx, pThis , pDrawTypeExt) : mtx;

	// 10240u -> (BlitterFlags::Alpha | BlitterFlags::Flat);

	// Only when there is a barrel will its calculation and drawing be considered
	if (haveBar)
	{
		auto drawBarrel = [=, &mtx_turret, &mtx]()
			{
				// When in recoiling, need to bypass cache and draw without saving
				const auto brlKey = inRecoil ? -1 : flags;
				const auto brlCache = inRecoil ? nullptr : reinterpret_cast<IndexClass<int, int>*>(&pDrawType->VoxelCaches.TurretBarrel);

				auto getBarrelMatrix = [=, &mtx_turret, &mtx]() -> Matrix3D
					{
						auto mtx_barrel = mtx_turret;
						mtx_barrel.Translate(-mtx.Row[0].W, -mtx.Row[1].W, -mtx.Row[2].W);
						mtx_barrel.RotateY(static_cast<float>(-pThis->BarrelFacing.Current().GetRadian<32>()));

						if (pThis->BarrelRecoil.State != RecoilData::RecoilState::Inactive)
							mtx_barrel.TranslateX(-pThis->BarrelRecoil.TravelSoFar);

						mtx_barrel.Translate(mtx.Row[0].W, mtx.Row[1].W, mtx.Row[2].W);
						return mtx_barrel;
					};
				auto mtx_barrel = shouldRedraw ? getBarrelMatrix() : mtx;
				const auto pBarrelVoxel = TechnoTypeExtData::GetBarrelsVoxelFixedUp(pDrawType, currentTurretNumber);

				// draw barrel
				pThis->Draw_A_VXL(pBarrelVoxel, hvaFrameIdx, brlKey, brlCache, rect, center, &mtx_barrel, brightness, 10240u, 0);
			};

		const auto turretDir = pThis->SecondaryFacing.Current().GetFacing<4>();

		// The orientation of the turret can affect the layer order of the barrel and turret
		if (turretDir != 0 && turretDir != 3)
		{
			// draw turret
			pThis->Draw_A_VXL(pTurretVoxel, hvaFrameIdx, turKey, turCache, rect, center, &mtx_turret, brightness, 10240u, 0);
			drawBarrel();
		}
		else
		{
			drawBarrel();
			// draw turret
			pThis->Draw_A_VXL(pTurretVoxel, hvaFrameIdx, turKey, turCache, rect, center, &mtx_turret, brightness, 10240u, 0);
		}
	}
	else
	{
		pThis->Draw_A_VXL(pTurretVoxel, hvaFrameIdx, turKey, turCache, rect, center, &mtx_turret, brightness, 10240u, 0);
	}

	return 0x73BEA4;
}

#endif

ASMJIT_PATCH(0x73C890, UnitClass_Draw_1_TurretMultiOffset, 0x8) //0
{
	GET(TechnoTypeClass*, pType, EAX);
	LEA_STACK(Matrix3D*, mtx, 0x80);

	const auto& nOffs = TechnoTypeExtContainer::Instance.Find(pType)->TurretOffset;

	float x = static_cast<float>(nOffs->X * TechnoTypeExtData::TurretMultiOffsetOneByEightMult);
	float y = static_cast<float>(nOffs->Y * TechnoTypeExtData::TurretMultiOffsetOneByEightMult);
	float z = static_cast<float>(nOffs->Z * TechnoTypeExtData::TurretMultiOffsetOneByEightMult);

	mtx->Translate(x, y, z);

	return 0x73C8B7;
}

ASMJIT_PATCH(0x43E0C4, BuildingClass_Draw_43DA80_TurretMultiOffset, 0x5) //0
{
	GET(TechnoTypeClass*, pType, EDX);
	LEA_STACK(Matrix3D*, mtx, 0x60);

	const auto& nOffs = TechnoTypeExtContainer::Instance.Find(pType)->TurretOffset;

	float x = static_cast<float>(nOffs->X * TechnoTypeExtData::TurretMultiOffsetOneByEightMult);
	float y = static_cast<float>(nOffs->Y * TechnoTypeExtData::TurretMultiOffsetOneByEightMult);
	float z = static_cast<float>(nOffs->Z * TechnoTypeExtData::TurretMultiOffsetOneByEightMult);

	mtx->Translate(x, y, z);

	return 0x43E0E8;
}

ASMJIT_PATCH(0x73CCE1, UnitClass_DrawSHP_TurretOffest, 0x6)
{
	GET(UnitClass*, pThis, EBP);
	REF_STACK(Point2D, pos, STACK_OFFSET(0x15C, -0xE8));

	Matrix3D mtx = Matrix3D::GetIdentity();
	mtx.RotateZ(static_cast<float>(pThis->PrimaryFacing.Current().GetRadian<32>()));
	const auto& nOffs = TechnoTypeExtContainer::Instance.Find(pThis->Type)->TurretOffset;

	float x = static_cast<float>(nOffs->X * TechnoTypeExtData::TurretMultiOffsetDefaultMult);
	float y = static_cast<float>(nOffs->Y * TechnoTypeExtData::TurretMultiOffsetDefaultMult);
	float z = static_cast<float>(nOffs->Z * TechnoTypeExtData::TurretMultiOffsetDefaultMult);

	mtx.Translate(x, y, z);

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

ASMJIT_PATCH(0x6B7230, SpawnManagerClass_AI_Dead, 0x5)
{
	GET(SpawnManagerClass*, pThis, ECX);

	if(!pThis->Owner || !pThis->Owner->IsAlive)
		return 0x6B7B6A;

	return 0x0;
}

ASMJIT_PATCH(0x6B7282, SpawnManagerClass_AI_PromoteSpawns, 0x5)
{
	GET(SpawnManagerClass*, pThis, ESI);

	if(auto pOwner = pThis->Owner) {
		if (TechnoTypeExtContainer::Instance.Find(pOwner->GetTechnoType())->Promote_IncludeSpawns) {
			for (const auto& i : pThis->SpawnedNodes) {
				if (i->Unit && i->Unit->Veterancy.Veterancy < pOwner->Veterancy.Veterancy)
					i->Unit->Veterancy.Add(pOwner->Veterancy.Veterancy - i->Unit->Veterancy.Veterancy);
			}
		}
	}

	return 0;
}

#include <Ext/Cell/Body.h>

ASMJIT_PATCH(0x73D223, UnitClass_DrawIt_OreGath, 0x6)
{
	GET(UnitClass*, pThis, ESI);
	GET(int, nFacing, EDI);
	GET_STACK(RectangleStruct*, pBounds, STACK_OFFS(0x50, -0x8));
	LEA_STACK(Point2D*, pLocation, STACK_OFFS(0x50, 0x18));
	GET_STACK(int, nBrightness, STACK_OFFS(0x50, -0x4));

	const auto pType = pThis->GetTechnoType();

	ConvertClass* pDrawer = FileSystem::ANIM_PAL;
	SHPStruct* pSHP = FileSystem::OREGATH_SHP;
	int idxFrame = -1;
	auto idxTiberium = ((FakeCellClass*)pThis->GetCell())->_GetTiberiumType();

	if (idxTiberium != -1)
	{
		const auto pData = TechnoTypeExtContainer::Instance.Find(pType);
		const auto idxArray = pData->OreGathering_Tiberiums.IndexOf(idxTiberium);

		if (idxArray != -1)
		{
			const auto nFramesPerFacing = pData->OreGathering_FramesPerDir.GetItemAtOrDefault(idxArray, 15);

			if (auto pAnimType = pData->OreGathering_Anims.GetItemAtOrMax(idxArray))
			{
				pSHP = pAnimType->GetImage();
				if (const auto pPalette = AnimTypeExtContainer::Instance.Find(pAnimType)->Palette.GetConvert())
					pDrawer = pPalette;
			}

			idxFrame = nFramesPerFacing * nFacing + (Unsorted::CurrentFrame + pThis->WalkedFramesSoFar) % nFramesPerFacing;
		}
	}

	if (idxFrame == -1)
		idxFrame = 15 * nFacing + (Unsorted::CurrentFrame + pThis->WalkedFramesSoFar) % 15;

	DSurface::Temp->DrawSHP(
		pDrawer, pSHP, idxFrame, pLocation, pBounds,
		BlitterFlags::Flat | BlitterFlags::Alpha | BlitterFlags::Centered,
		0, pThis->GetZAdjustment() - 2, ZGradient::Ground, nBrightness,
		0, nullptr, 0, 0, 0
	);

	R->EBP(nBrightness);
	R->EBX(pBounds);

	return 0x73D28C;
}

ASMJIT_PATCH(0x700C58, TechnoClass_CanPlayerMove_NoManualMove, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	return TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType())->NoManualMove.Get() ? 0x700C62 : 0;
}

ASMJIT_PATCH(0x4437B3, BuildingClass_CellClickedAction_NoManualMove, 0x6)
{
	GET(BuildingTypeClass*, pType, EDX);
	return TechnoTypeExtContainer::Instance.Find(pType)->NoManualMove ? 0x44384E : 0;
}

ASMJIT_PATCH(0x44F62B, BuildingClass_CanPlayerMove_NoManualMove, 0x6)
{
	GET(BuildingTypeClass*, pType, EDX);
	R->ECX(TechnoTypeExtContainer::Instance.Find(pType)->NoManualMove ? 0 : pType->UndeploysInto);
	return 0x44F631;
}

ASMJIT_PATCH(0x73CF46, UnitClass_Draw_It_KeepUnitVisible, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	if((pThis->Deploying || pThis->Undeploying) &&
		TechnoTypeExtContainer::Instance.Find(pThis->Type)->DeployingAnim_KeepUnitVisible){
			return 0x73CF62;
		}

	return 0;
}

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

ASMJIT_PATCH(0x739B7C, UnitClass_SimpleDeploy_Facing, 0x6)
{
	GET(UnitClass*, pThis, ESI);
	auto const pType = pThis->Type;
	enum { PlayDeploySound = 0x739C70  , SetAnimTimer = 0x739C20 , SetDeployingState = 0x739C62 };
	//auto const pExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);

	if (!pThis->InAir)
	{
		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

		if (!pTypeExt->DeployingAnim_AllowAnyDirection)
		{
			// not sure what is the bitfrom or bitto so it generate this result
			// yes iam dum , iam sorry - otamaa
			const auto nRulesDeployDir = ((((RulesClass::Instance->DeployDir) >> 4) + 1) >> 1) & 7;
			const FacingType nRaw = pTypeExt->DeployDir.isset() ? pTypeExt->DeployDir.Get() : (FacingType)nRulesDeployDir;
			const auto nCurrent = (((((pThis->PrimaryFacing.Current().Raw) >> 12) + 1) >> 1) & 7);

			if (nCurrent != (int)nRaw)
			{
				if (const auto pLoco = pThis->Locomotor.GetInterfacePtr())
				{
					if (!pLoco->Is_Moving_Now())
					{
						pLoco->Do_Turn(DirStruct { nRaw });
					}

					return PlayDeploySound; //adjust the facing first
				}
			}
		}

		if (const auto pAnimType = GetDeployAnim(pThis))
		{
			if(!pThis->DeployAnim) {
				auto const pAnim = GameCreate<AnimClass>(pAnimType,
				pThis->Location, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false);

				pThis->DeployAnim = pAnim;
				pAnim->SetOwnerObject(pThis);

				if (pTypeExt->DeployingAnim_UseUnitDrawer) {
					pAnim->LightConvert = pThis->GetRemapColour();
				}
			}

			pThis->Animation.Stage = pAnimType->Start;
			pThis->Animation.Timer.Start(pAnimType->Rate);
		}

		pThis->Deployed = true;
	}

	return PlayDeploySound;
}

ASMJIT_PATCH(0x739D73 , UnitClass_UnDeploy_DeployAnim , 0x6)
{
	GET(UnitClass*, pThis, ESI);

	const auto pAnimType = GetDeployAnim(pThis);

	if(!pAnimType)
		return 0x739E4F;

	if(pThis->DeployAnim)
		return 0x739E04;

	auto const pExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);

	auto const pAnim = GameCreate<AnimClass>(pAnimType,
	pThis->Location, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0,
	pExt->DeployingAnim_ReverseForUndeploy);

	pThis->DeployAnim = pAnim;
	pAnim->SetOwnerObject(pThis);

	if (pExt->DeployingAnim_UseUnitDrawer) {
		pAnim->LightConvert = pThis->GetRemapColour();
	}

	return 0x739E04;
}

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

ASMJIT_PATCH(0x739C86, UnitClass_DeployUndeploy_DeploySound, 0x6)
{
	enum { DeployReturn = 0x739CBF, UndeployReturn = 0x739EB8 };

	GET(UnitClass*, pThis, ESI);

	const bool isDeploying = R->Origin() == 0x739C86;
	const bool isDoneWithDeployUndeploy = isDeploying ? pThis->Deployed : !pThis->Deployed;

	if (isDoneWithDeployUndeploy)
		return 0; // Only play sound when done with deploying or undeploying.

	return isDeploying ? DeployReturn : UndeployReturn;
}ASMJIT_PATCH_AGAIN(0x739E81, UnitClass_DeployUndeploy_DeploySound, 0x6)

#include <Locomotor/HoverLocomotionClass.h>

namespace SimpleDeployerTemp {
	bool HoverDeployedToLand = false;
	AnimTypeClass* DeployingAnim = nullptr;
}

ASMJIT_PATCH(0x739CBF, UnitClass_Deploy_DeployToLandHover, 0x5)
{
	GET(UnitClass*, pThis, ESI);

	if (pThis->Deployed && pThis->Type->DeployToLand && pThis->Type->Locomotor == HoverLocomotionClass::ClassGUID())
		SimpleDeployerTemp::HoverDeployedToLand = true;

	return 0;
}

ASMJIT_PATCH(0x73E5B1, UnitClass_Unload_DeployToLandHover, 0x8)
{
	if (SimpleDeployerTemp::HoverDeployedToLand)
	{
		GET(UnitClass*, pThis, ESI);

		// Ares' DeployToLand 'fix' for Hover IsSimpleDeployer vehicles does not set/reset certain values
		// and has a chance to get stuck in Unload mission as a result, following should remedy that.
		pThis->SetHeight(0);
		pThis->InAir = false;
		pThis->ForceMission(Mission::Guard);
	}

	SimpleDeployerTemp::HoverDeployedToLand = false;
	return 0;
}ASMJIT_PATCH_AGAIN(0x73DED8, UnitClass_Unload_DeployToLandHover, 0x7)

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

// Do not display hover bobbing when landed during deploying.
ASMJIT_PATCH(0x513D2C, HoverLocomotionClass_ProcessBobbing_DeployToLand, 0x6)
{
	enum { SkipBobbing = 0x513F2A };

	GET(LocomotionClass*, pThis, ECX);

	if (auto const pUnit = cast_to<UnitClass*, false>(pThis->Owner)) {
		if (pUnit->Deploying && pUnit->Type->DeployToLand)
			return SkipBobbing;
	}

	return 0;
}

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
					if (!pOwnerHouse->IsNeutral() && !pOwnerHouse->IsAlliedWith(HouseClass::CurrentPlayer))
					{
						const auto pTechnoTypeExt = TechnoTypeExtContainer::Instance.Find(pFoot->GetTechnoType());
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

ASMJIT_PATCH(0x6FDFA8, TechnoClass_FireAt_SprayOffsets, 0x5)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EBX);
	LEA_STACK(CoordStruct*, pCoord, 0xB0 - 0x28);

	auto pType = pThis->GetTechnoType();
	auto pExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pType->SprayAttack) {
		if(pThis->CurrentBurstIndex) {
			pThis->SprayOffsetIndex = (pExt->SprayOffsets.size() / pWeapon->Burst + pThis->SprayOffsetIndex) % pExt->SprayOffsets.size();
		}
		else {
			pThis->SprayOffsetIndex = ScenarioClass::Instance->Random.RandomRanged(0, pExt->SprayOffsets.size() - 1);
		}

		auto& Coord = pExt->SprayOffsets[pThis->SprayOffsetIndex];
		pCoord->X = (pThis->Location.X + Coord->X);//X
		pCoord->Y = (pThis->Location.Y + Coord->Y);//Y
		R->EAX(pThis->Location.Z + Coord->Z); //Z
		return 0x6FE218;
	}

	return 0x6FE140;
}

ASMJIT_PATCH(0x744745, UnitClass_RegisterDestruction_Trigger, 0x5)
{
	GET(UnitClass*, pThis, ESI);
	GET(TechnoClass*, pAttacker, EDI);

	if (pThis && pThis->IsAlive && pAttacker)
	{
		if (auto pTag = pThis->AttachedTag)
		{
			pTag->RaiseEvent((TriggerEvent)AresTriggerEvents::DestroyedByHouse, pThis, CellStruct::Empty, false, pAttacker->GetOwningHouse());
		}
	}

	return 0x0;
}

ASMJIT_PATCH(0x738801, UnitClass_Destroy_DestroyAnim, 0x6) //was C
{
	GET(UnitClass* const, pThis, ESI);

	auto const Extension = TechnoExtContainer::Instance.Find(pThis);

	if (!Extension->ReceiveDamage)
	{
		AnimTypeExtData::ProcessDestroyAnims(pThis);
	}

	return 0x73887E;
}

int GetVoiceAttack(TechnoTypeClass* pType, int WeaponIndex, bool isElite, WeaponTypeClass* pWeaponType)
{
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	int VoiceAttack = -1;

	if (pWeaponType && pWeaponType->Damage < 0)
	{
		VoiceAttack = pTypeExt->VoiceIFVRepair.Get();

		if (VoiceAttack < 0)
			VoiceAttack = !strcmp(pType->ID, "FV") ? RulesClass::Instance->VoiceIFVRepair : -1;

		if (VoiceAttack >= 0)
			return VoiceAttack;
	}

	if (WeaponIndex >= 0 && int(pTypeExt->VoiceWeaponAttacks.size()) > WeaponIndex)
		VoiceAttack = isElite ? pTypeExt->VoiceEliteWeaponAttacks[WeaponIndex] : pTypeExt->VoiceWeaponAttacks[WeaponIndex];

	if (VoiceAttack < 0)
	{
		if (pTypeExt->IsSecondary(WeaponIndex))
			VoiceAttack = isElite ? pType->VoiceSecondaryEliteWeaponAttack : pType->VoiceSecondaryWeaponAttack;
		else
			VoiceAttack = isElite ? pType->VoicePrimaryEliteWeaponAttack : pType->VoicePrimaryWeaponAttack;
	}

	return VoiceAttack;
}

ASMJIT_PATCH(0x7090A0, TechnoClass_VoiceAttack, 0x7)
{
	GET(TechnoClass*, pThis, ECX);
	GET_STACK(AbstractClass*, pTarget, 0x4);

	const auto pType = pThis->GetTechnoType();
	const int WeaponIndex = pThis->SelectWeapon(pTarget);
	int VoiceAttack = GetVoiceAttack(pType, WeaponIndex, pThis->Veterancy.IsElite(), pThis->GetWeapon(WeaponIndex)->WeaponType);

	if (VoiceAttack >= 0) {
		pThis->QueueVoice(VoiceAttack);
		return 0x7091C7;
	}

	const auto& Lists = pType->VoiceAttack;

	if (Lists.Count > 0) {
		int idx = Random2Class::Global->RandomRanged(0, Lists.Count - 1);
		pThis->QueueVoice(Lists[idx]);
	}

	return 0x7091C7;
}