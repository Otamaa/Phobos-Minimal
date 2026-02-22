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

#include <Locomotor/JumpjetLocomotionClass.h>
#include <Locomotor/Cast.h>

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

	enum { CleanFlag = 0x73B78A, SkipFlag = 0x73B790 };

	const auto pDrawTypeExt = TechnoTypeExtContainer::Instance.Find(technoType);
	const auto& nOffs = TechnoTypeExtContainer::Instance.Find(technoType)->TurretOffset;

	return (nOffs->IsEmpty()
		&& pDrawTypeExt->ExtraTurretCount <= 0
		&& pDrawTypeExt->ExtraBarrelCount <= 0)
		? CleanFlag : SkipFlag;
}

#ifdef _old

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
#else

#ifdef Rewrite
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

		return TechnoTypeExtData::GetTurretsVoxel(pDrawType , currentTurretNumber);
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
		pDrawTypeExt->ApplyTurretOffset(&mtxTurret, Game::Pixel_Per_Lepton());

		FacingClass* pPrimaryFacing = &pThis->PrimaryFacing;
		// Align with the jj Draw_Matrix calc changing.
		if (auto pJJLoco = locomotion_cast<JumpjetLocomotionClass*>(pThis->Locomotor)) {
			if (!pThis->IsAttackedByLocomotor)
				pPrimaryFacing = &pJJLoco->Facing;
		}

		double primaryRad = pPrimaryFacing->Current().GetRadian<32>();

		mtxTurret.RotateZ(static_cast<float>(pThis->SecondaryFacing.Current().GetRadian<32>() - primaryRad));

		if (pThis->TurretRecoil.State != RecoilData::RecoilState::Inactive)
			mtxTurret.TranslateX(-pThis->TurretRecoil.TravelSoFar);

		return mtxTurret;
	};

	Matrix3D mtxTurret = shouldRedraw ? getTurretMatrix() : mtx;
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
				Matrix3D mtxBarrel = mtxTurret;
				mtxBarrel.Translate(-mtx.Row[0].W, -mtx.Row[1].W, -mtx.Row[2].W);
				mtxBarrel.RotateY(static_cast<float>(-pThis->BarrelFacing.Current().GetRadian<32>()));

				if (pThis->BarrelRecoil.State != RecoilData::RecoilState::Inactive)
					mtxBarrel.TranslateX(-pThis->BarrelRecoil.TravelSoFar);

				mtxBarrel.Translate(mtx.Row[0].W, mtx.Row[1].W, mtx.Row[2].W);
				return mtxBarrel;
			};
			Matrix3D mtxBarrel = shouldRedraw ? getBarrelMatrix() : mtx;

			auto getBarrelVoxel = [pDrawType, notChargeTurret, currentTurretNumber]() -> VoxelStruct*
			{
				if (notChargeTurret)
					return &pDrawType->BarrelVoxel;

				return TechnoTypeExtData::GetBarrelsVoxel(pDrawType , currentTurretNumber);
			};
			const auto pBarrelVoxel = TechnoTypeExtData::GetBarrelsVoxelFixedUp(pDrawType, currentTurretNumber);

			// draw barrel
			pThis->Draw_A_VXL(pBarrelVoxel, hvaFrameIdx, brlKey, (IndexClass<int, int>*)brlCache, rect, center, &mtxBarrel, brightness, (DWORD)blit, 0);
		};

		const auto turretDir = pThis->SecondaryFacing.Current().GetFacing<4>();

		// The orientation of the turret can affect the layer order of the barrel and turret
		if (turretDir != 0 && turretDir != 3)
		{
			// draw turret
			pThis->Draw_A_VXL(pTurretVoxel, hvaFrameIdx, turKey, (IndexClass<int, int>*)turCache, rect, center, &mtxTurret, brightness, (DWORD)blit, 0);

			drawBarrel();
		}
		else
		{
			drawBarrel();

			// draw turret
			pThis->Draw_A_VXL(pTurretVoxel, hvaFrameIdx, turKey, (IndexClass<int, int>*)turCache, rect, center, &mtxTurret, brightness, (DWORD)blit, 0);
		}
	}
	else
	{
		pThis->Draw_A_VXL(pTurretVoxel, hvaFrameIdx, turKey, (IndexClass<int, int>*)turCache, rect, center, &mtxTurret, brightness, (DWORD)blit, 0);
	}

	return SkipGameCode;
}
#endif

ASMJIT_PATCH(0x73C7AC, UnitClass_DrawAsSHP_DrawTurret_TintFix, 0x6)
{
	enum { SkipDrawCode = 0x73CE00 };

	GET(UnitClass*, pThis, EBP);

	const auto pThisType = pThis->Type;
	const VoxelStruct barrelVoxel = pThisType->BarrelVoxel;

	if (barrelVoxel.VXL && barrelVoxel.HVA)
		return 0;

	GET(UnitTypeClass*, pType, ECX);
	GET(SHPStruct*, pShape, EDI);
	GET(const int, bodyFrameIdx, EBX);
	REF_STACK(Point2D, location, STACK_OFFSET(0x128, 0x4));
	REF_STACK(RectangleStruct, bounds, STACK_OFFSET(0x128, 0xC));
	GET_STACK(const int, extraLight, STACK_OFFSET(0x128, 0x1C));

	const bool tooBigToFitUnderBridge = pType->TooBigToFitUnderBridge
		&& pThis->sub_703B10() && !pThis->sub_703E70();
	const int zAdjust = tooBigToFitUnderBridge ? -16 : 0;
	const ZGradient zGradient = tooBigToFitUnderBridge ? ZGradient::Ground : pThis->GetZGradient();

	pThis->Draw_A_SHP(pShape, bodyFrameIdx, &location, &bounds, 0, 256, zAdjust, zGradient, 0, extraLight, 0, 0, 0, 0, 0, 0);

	const auto secondaryDir = pThis->SecondaryFacing.Current();
	const int frameIdx = secondaryDir.GetFacing<32>(4) + pType->WalkFrames * pType->Facings;

	const auto primaryDir = pThis->PrimaryFacing.Current();
	const double bodyRad = primaryDir.GetRadian<32>();
	Matrix3D mtx = Matrix3D::GetIdentity();
	mtx.RotateZ(static_cast<float>(bodyRad));
	TechnoTypeExtContainer::Instance.Find(pType)->ApplyTurretOffset(&mtx,Game::Pixel_Per_Lepton());
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

#ifdef _fromPR1983
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

	const auto pExt = TechnoExtContainer::Instance.Find(pThis);
	const auto pDrawTypeExt = TechnoTypeExtContainer::Instance.Find(pDrawType);
	const bool notChargeTurret = pThis->Type->TurretCount <= 0 || pThis->Type->IsGattling;

	auto getTurretVoxel = [pDrawType, notChargeTurret, currentTurretNumber]() -> VoxelStruct*
		{
			if (notChargeTurret)
				return &pDrawType->TurretVoxel;

			return TechnoTypeExtData::GetTurretsVoxel(pDrawType, currentTurretNumber);
		};
	const auto pTurretVoxel = getTurretVoxel();

	auto getBarrelVoxel = [pDrawType, notChargeTurret, currentTurretNumber]() -> VoxelStruct*
		{
			if (notChargeTurret)
				return &pDrawType->BarrelVoxel;

			return TechnoTypeExtData::GetBarrelsVoxel(pDrawType, currentTurretNumber);
		};
	const auto pBarrelVoxel = haveBar ? getBarrelVoxel() : nullptr;

	constexpr BlitterFlags blit = BlitterFlags::Alpha | BlitterFlags::Flat;
	// When in recoiling or have no cache, need to recalculate drawing matrix
	const bool shouldRedraw = !haveTurretCache || haveBar && !haveBarrelCache;

	// The orientation of the turret can affect the layer order of the barrel and turret
	const auto turretDir = pThis->SecondaryFacing.Current().GetFacing<4>();
	const bool barrelOverTechno = pDrawTypeExt->BarrelOverTurret.Get(turretDir != 0 && turretDir != 3);

	auto drawTurret = [=, &mtx](int turIdx)
		{
			const auto pTurData = pDrawType->TurretRecoil ? ((turIdx < 0) ? &pThis->TurretRecoil : &pExt->ExtraTurretRecoil[turIdx]) : nullptr;
			const bool turretInRecoil = pTurData && pTurData->State != RecoilData::RecoilState::Inactive;

			// When in recoiling or is not main turret, need to bypass cache and draw without saving
			const bool turShouldRedraw = turretInRecoil || turIdx >= 0;
			const auto turKey = turShouldRedraw ? -1 : flags;
			const auto turCache = turShouldRedraw ? nullptr : &pDrawType->VoxelCaches.TurretWeapon;

			auto shouldCalculateMatrix = [=]()
				{
					if (!haveBar)
						return false;

					if (pThis->BarrelRecoil.State != RecoilData::RecoilState::Inactive)
						return true;

					return pDrawTypeExt->ExtraBarrelCount.Get() > 0;
				};
			auto getTurretMatrix = [=, &mtx]() -> Matrix3D
				{
					auto mtx_turret = mtx;
					pDrawTypeExt->ApplyTurretOffset(&mtx_turret, Game::Pixel_Per_Lepton(), turIdx);
					mtx_turret.RotateZ(static_cast<float>(pThis->SecondaryFacing.Current().GetRadian<32>() - pThis->PrimaryFacing.Current().GetRadian<32>()));

					if (turretInRecoil)
						mtx_turret.TranslateX(-pTurData->TravelSoFar);

					return mtx_turret;
				};
			auto mtx_turret = (shouldRedraw || turShouldRedraw || shouldCalculateMatrix()) ? getTurretMatrix() : mtx;

			auto drawBarrel = [=, &mtx_turret, &mtx](int brlIdx)
				{
					const auto idx = brlIdx + ((turIdx + 1) * (pDrawTypeExt->ExtraBarrelCount.Get() + 1));
					const auto pBrlData = pDrawType->TurretRecoil ? ((idx < 0) ? &pThis->BarrelRecoil : &pExt->ExtraBarrelRecoil[idx]) : nullptr;
					const bool barrelInRecoil = pBrlData && pBrlData->State != RecoilData::RecoilState::Inactive;

					// When in recoiling or is not main barrel, need to bypass cache and draw without saving
					const bool brlShouldRedraw = turretInRecoil || barrelInRecoil || idx >= 0;
					const auto brlKey = brlShouldRedraw ? -1 : flags;
					const auto brlCache = brlShouldRedraw ? nullptr : &pDrawType->VoxelCaches.TurretBarrel;

					auto getBarrelMatrix = [=, &mtx_turret, &mtx]() -> Matrix3D
						{
							auto mtx_barrel = mtx_turret;
							mtx_barrel.Translate(-mtx.Row[0].W, -mtx.Row[1].W, -mtx.Row[2].W);
							mtx_barrel.RotateY(static_cast<float>(-pThis->BarrelFacing.Current().GetRadian<32>()));
							const auto offset = ((brlIdx < 0) ? pDrawTypeExt->BarrelOffset.Get() : pDrawTypeExt->ExtraBarrelOffsets[brlIdx]);
							mtx_barrel.TranslateY(static_cast<float>(Game::Pixel_Per_Lepton()* offset));

							if (barrelInRecoil)
								mtx_barrel.TranslateX(-pBrlData->TravelSoFar);

							mtx_barrel.Translate(mtx.Row[0].W, mtx.Row[1].W, mtx.Row[2].W);
							return mtx_barrel;
						};
					auto mtx_barrel = (shouldRedraw || brlShouldRedraw) ? getBarrelMatrix() : mtx;

					// draw barrel
					pThis->Draw_A_VXL(pBarrelVoxel, hvaFrameIdx, brlKey, brlCache, rect, center, &mtx_barrel, brightness, blit, 0);
				};

			auto drawBarrels = [&drawBarrel, pDrawTypeExt, turretDir]()
				{
					const auto exBrlCount = pDrawTypeExt->ExtraBarrelCount.Get();

					if (exBrlCount > 0)
					{
						std::vector<int> barrels;
						barrels.emplace_back(-1);

						for (int i = 0; i < exBrlCount; ++i)
							barrels.emplace_back(i);

						const auto barrelsSize = barrels.size();
						const bool faceRight = turretDir == 0 || turretDir == 1;
						std::sort(&barrels[0], &barrels[barrelsSize], [pDrawTypeExt, faceRight](const auto& idxA, const auto& idxB)
							{
										const auto offsetA = idxA < 0 ? pDrawTypeExt->BarrelOffset.Get() : pDrawTypeExt->ExtraBarrelOffsets[idxA];
										const auto offsetB = idxB < 0 ? pDrawTypeExt->BarrelOffset.Get() : pDrawTypeExt->ExtraBarrelOffsets[idxB];

										return faceRight ? (offsetA > offsetB) : (offsetA <= offsetB);
							});

						for (const auto& i : barrels)
							drawBarrel(i);
					}
					else
					{
						drawBarrel(-1);
					}
				};

			if (barrelOverTechno)
			{
				// draw turret
				pThis->Draw_A_VXL(pTurretVoxel, hvaFrameIdx, turKey, turCache, rect, center, &mtx_turret, brightness, blit, 0);

				if (haveBar)
					drawBarrels();
			}
			else
			{
				if (haveBar)
					drawBarrels();

				// draw turret
				pThis->Draw_A_VXL(pTurretVoxel, hvaFrameIdx, turKey, turCache, rect, center, &mtx_turret, brightness, blit, 0);
			}
		};

	auto drawTurrets = [&drawTurret, pThis, pDrawTypeExt]()
		{
			const auto exTurCount = pDrawTypeExt->ExtraTurretCount.Get();

			if (exTurCount > 0)
			{
				std::vector<int> turrets;
				turrets.emplace_back(-1);

				for (int i = 0; i < exTurCount; ++i)
					turrets.emplace_back(i);

				const auto turretsSize = turrets.size();
				std::sort(&turrets[0], &turrets[turretsSize], [pThis, pDrawTypeExt](const auto& idxA, const auto& idxB)
					{
							const auto pOffsetA = idxA < 0 ? reinterpret_cast<CoordStruct*>(pDrawTypeExt->TurretOffset.operator->()) : &pDrawTypeExt->ExtraTurretOffsets[idxA];
							const auto pOffsetB = idxB < 0 ? reinterpret_cast<CoordStruct*>(pDrawTypeExt->TurretOffset.operator->()) : &pDrawTypeExt->ExtraTurretOffsets[idxB];

							if (pOffsetA->Z < pOffsetB->Z)
								return true;

							if (pOffsetA->Z > pOffsetB->Z)
								return false;

							const auto pointA = TacticalClass::Instance->CoordsToClient(TechnoExtData::GetFLHAbsoluteCoords(pThis, *pOffsetA));
							const auto pointB = TacticalClass::Instance->CoordsToClient(TechnoExtData::GetFLHAbsoluteCoords(pThis, *pOffsetB));

							if (pointA.Y < pointB.Y)
								return true;

							if (pointA.Y > pointB.Y)
								return false;

							return pointA.X <= pointB.X;
					});

				for (const auto& i : turrets)
					drawTurret(i);
			}
			else
			{
				drawTurret(-1);
			}
		};
	drawTurrets();

	return SkipGameCode;
}
#else

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
			pDrawTypeExt->ApplyTurretOffset(&mtxTurret, Game::Pixel_Per_Lepton());

			FacingClass* pPrimaryFacing = &pThis->PrimaryFacing;
			// Align with the jj Draw_Matrix calc changing.
			if (auto pJJLoco = locomotion_cast<JumpjetLocomotionClass*>(pThis->Locomotor))
			{
				if (!pThis->IsAttackedByLocomotor)
					pPrimaryFacing = &pJJLoco->Facing;
			}

			double primaryRad = pPrimaryFacing->Current().GetRadian<32>();

			mtxTurret.RotateZ(static_cast<float>(pThis->SecondaryFacing.Current().GetRadian<32>() - primaryRad));

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
#endif

#endif

Matrix3D NOINLINE getTurretMatrix(const Matrix3D& mtx , UnitClass* pThis , TechnoTypeExtData* pDrawTypeExt) {

	Matrix3D mtx_turret = mtx;

	pDrawTypeExt->ApplyTurretOffset(&mtx_turret, Game::Pixel_Per_Lepton());
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
		if (GET_TECHNOTYPEEXT(pOwner)->Promote_IncludeSpawns) {
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

	const auto pType = GET_TECHNOTYPE(pThis);

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
	return GET_TECHNOTYPEEXT(pThis)->NoManualMove.Get() ? 0x700C62 : 0;
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
					if (!pOwnerHouse->IsNeutral() && !pOwnerHouse->IsAlliedWith(HouseClass::CurrentPlayer))
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

ASMJIT_PATCH(0x6FDFA8, TechnoClass_FireAt_SprayOffsets, 0x5)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EBX);
	LEA_STACK(CoordStruct*, pCoord, 0xB0 - 0x28);

	auto pType = GET_TECHNOTYPE(pThis);
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
			VoiceAttack = isElite && pType->VoiceSecondaryEliteWeaponAttack != -1? pType->VoiceSecondaryEliteWeaponAttack : pType->VoiceSecondaryWeaponAttack;
		else
			VoiceAttack = isElite && pType->VoicePrimaryEliteWeaponAttack!= -1 ? pType->VoicePrimaryEliteWeaponAttack : pType->VoicePrimaryWeaponAttack;
	}

	return VoiceAttack;
}

ASMJIT_PATCH(0x7090A0, TechnoClass_VoiceAttack, 0x7)
{
	GET(TechnoClass*, pThis, ECX);
	GET_STACK(AbstractClass*, pTarget, 0x4);

	const auto pType = GET_TECHNOTYPE(pThis);
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

ThreatType __forceinline GetThreatType(TechnoClass* pThis, TechnoTypeExtData* pTypeExt, ThreatType result)
{
	ThreatType flags = pThis->Veterancy.IsElite() ? pTypeExt->ThreatTypes.Y : pTypeExt->ThreatTypes.X;
	return result | flags;
}

ASMJIT_PATCH(0x7431C9, FootClass_SelectAutoTarget_MultiWeapon, 0x7)				// UnitClass_SelectAutoTarget
{
	GET(FootClass*, pThis, ESI);
	GET(ThreatType, result, EDI);
	enum { InfantryReturn = 0x51E31B, UnitReturn = 0x74324F, UnitGunner = 0x7431E4 };

	const bool isUnit = R->Origin() == 0x7431C9;
	const auto pType = GET_TECHNOTYPE(pThis);
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (isUnit
		&& !pType->IsGattling && pType->TurretCount > 0
		&& (pType->Gunner || !pTypeExt->MultiWeapon)) {
		return UnitGunner;
	}

	R->EDI(GetThreatType(pThis, pTypeExt, result));
	return isUnit ? UnitReturn : InfantryReturn;
}ASMJIT_PATCH_AGAIN(0x51E2CF, FootClass_SelectAutoTarget_MultiWeapon, 0x6)	// InfantryClass_SelectAutoTarget

ASMJIT_PATCH(0x445F04, BuildingClass_SelectAutoTarget_MultiWeapon, 0xA)
{
	GET(BuildingClass*, pThis, ESI);
	GET_STACK(ThreatType, result, STACK_OFFSET(0x8, 0x4));
	enum { ReturnThreatType = 0x445F58, Continue = 0x445F0E };

	if (pThis->UpgradeLevel > 0 || pThis->CanOccupyFire()) {
		R->EAX(pThis->GetWeapon(0));
		return Continue;
	}

	R->EDI(GetThreatType(pThis, TechnoTypeExtContainer::Instance.Find(pThis->Type), result));
	return ReturnThreatType;
}

ASMJIT_PATCH(0x6F398E, TechnoClass_CombatDamage_MultiWeapon, 0x7)
{
	enum { ReturnDamage = 0x6F3ABB, GunnerDamage = 0x6F39AD, Continue = 0x6F39F4 };

	GET(TechnoClass*, pThis, ESI);

	const AbstractType rtti = pThis->WhatAmI();

	if (rtti == AbstractType::Building)
	{
		const auto pBuilding = static_cast<BuildingClass*>(pThis);

		if (pBuilding->UpgradeLevel > 0 || pBuilding->CanOccupyFire())
			return Continue;
	}

	const auto pType = GET_TECHNOTYPE(pThis);
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (rtti == AbstractType::Unit
		&& !pType->IsGattling && pType->TurretCount > 0
		&& (pType->Gunner || !pTypeExt->MultiWeapon))
	{
		return GunnerDamage;
	}

	R->EAX(pThis->Veterancy.IsElite() ? pTypeExt->CombatDamages.Y : pTypeExt->CombatDamages.X);
	return ReturnDamage;
}

static int GetMultiWeaponRange(TechnoClass* pThis)
{
	int range = -1;
	const auto pType = pThis->GetTechnoType();
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pTypeExt->MultiWeapon) {
		int selectCount = MinImpl(pType->WeaponCount, pTypeExt->MultiWeapon_SelectCount);
		range = 0;

		for (int index = selectCount - 1; index >= 0; --index) {
			int weaponRange = pThis->GetWeaponRange(index);

			if (weaponRange > range)
				range = weaponRange;
		}
	}

	return range;
}

static int GetGuardRange(TechnoClass* pThis, int control)
{
	if (control == -1)
		return -1;

	auto const pType = pThis->GetTechnoType();
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	int range = pType->GuardRange;

	if (pThis->CurrentMission == Mission::Area_Guard && pTypeExt->AreaGuardRange.isset())
		range = pTypeExt->AreaGuardRange.Get();

	if (!control) // Control = 0, used for ThreatType=Range target acquisition.
	{
		if (range && !pThis->IsEngineer())
			return range;

		return 0;
	}

	// Set range from weapon range if GuardRange is not set.
	if (!range)
	{
		// Handle special weapon configurations.
		if (!pType->IsGattling && (pType->HasMultipleTurrets() || pTypeExt->MultiWeapon)) {
			if (pType->HasMultipleTurrets())
				range = pThis->GetWeaponRange(pThis->CurrentWeaponNumber);
			else
				range = GetMultiWeaponRange(pThis);
		} else {
			int weaponRange0 = pThis->GetWeaponRange(0);
			int weaponRange1 = pThis->GetWeaponRange(1);

			if (weaponRange0 < weaponRange1)
				range = weaponRange1;
			else
				range = weaponRange0;
		}
	}


	//TechnoClass_GetGuardRange_AreaGuardRange
	const bool isPlayer = pThis->Owner->IsControlledByHuman();
	const auto pRulesExt = RulesExtData::Instance();

	double multiplier, addend, max = {};

	if (isPlayer) {
		multiplier = pTypeExt->PlayerGuardModeGuardRangeMultiplier.Get(pRulesExt->PlayerGuardModeGuardRangeMultiplier);
		addend = pTypeExt->PlayerGuardModeGuardRangeAddend.Get(pRulesExt->PlayerGuardModeGuardRangeAddend);
		max = pTypeExt->MaxGuardRange.Get(pRulesExt->PlayerGuardModeGuardRangeMax);
	} else {
		multiplier = pTypeExt->AIGuardModeGuardRangeMultiplier.Get(pRulesExt->AIGuardModeGuardRangeMultiplier);
		addend = pTypeExt->AIGuardModeGuardRangeAddend.Get(pRulesExt->AIGuardModeGuardRangeAddend);
		max = pTypeExt->MaxGuardRange.Get(pRulesExt->AIGuardModeGuardRangeMax);
	}

	const int min = ((control == 2) ? 1792 : 0);
	const int areaGuardRange = (static_cast<int>(range * multiplier + static_cast<int>(addend)));
	//


	return std::clamp(areaGuardRange, min, (int)max);
}

ASMJIT_PATCH(0x707E60, TechnoClass_GetGuardRange, 0x7)
{
	enum { SkipGameCode = 0x707F4B };

	GET(TechnoClass*, pThis, ECX);
	GET_STACK(int, control, 0x4);

	R->EAX(GetGuardRange(pThis, control));
	return 0x707F4E;
}