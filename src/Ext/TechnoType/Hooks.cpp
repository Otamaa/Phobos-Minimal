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

DEFINE_HOOK(0x6F64A9, TechnoClass_DrawHealthBar_Hide, 0x5)
{
	enum
	{
		Draw = 0x0,
		DoNotDraw = 0x6F6AB6
	};

	GET(TechnoClass*, pThis, ECX);

	if (const auto pUnit = specific_cast<UnitClass*>(pThis))
		if (pUnit->DeathFrameCounter > 0)
			return DoNotDraw;

	//if (auto pBuilding = specific_cast<BuildingClass*>(pThis))
	//{
	//	auto pBldExt = BuildingExt::ExtMap.Find(pBuilding);
	//	if (pBldExt->LimboID != -1)
	//		return DoNotDraw;
	//}

	if ((TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())->HealthBar_Hide.Get()) || pThis->TemporalTargetingMe || pThis->IsSinking)
		return DoNotDraw;



	return Draw;
}

DEFINE_HOOK(0x6F3C56, TechnoClass_Transform_6F3AD0_TurretMultiOffset, 0x5) //0
{
	GET(TechnoTypeClass*, pType, EDX);
	LEA_STACK(Matrix3D*, mtx, STACK_OFFS(0xD8, 0x90));

	const auto& nOffs = TechnoTypeExt::ExtMap.Find(pType)->TurretOffset;

	float x = static_cast<float>(nOffs->X * TechnoTypeExt::TurretMultiOffsetDefaultMult);
	float y = static_cast<float>(nOffs->Y * TechnoTypeExt::TurretMultiOffsetDefaultMult);
	float z = static_cast<float>(nOffs->Z * TechnoTypeExt::TurretMultiOffsetDefaultMult);

	mtx->Translate(x, y, z);

	return 0x6F3C6D;
}

DEFINE_HOOK(0x6F3E6E, FootClass_firecoord_6F3D60_TurretMultiOffset, 0x6) //0
{

	GET(TechnoTypeClass*, pType, EBP);
	LEA_STACK(Matrix3D*, mtx, STACK_OFFS(0xCC, 0x90));

	const auto& nOffs = TechnoTypeExt::ExtMap.Find(pType)->TurretOffset;

	float x = static_cast<float>(nOffs->X * TechnoTypeExt::TurretMultiOffsetDefaultMult);
	float y = static_cast<float>(nOffs->Y * TechnoTypeExt::TurretMultiOffsetDefaultMult);
	float z = static_cast<float>(nOffs->Z * TechnoTypeExt::TurretMultiOffsetDefaultMult);

	mtx->Translate(x, y, z);

	return 0x6F3E85;
}

DEFINE_HOOK(0x73B780, UnitClass_DrawVXL_TurretMultiOffset, 0x6) //0
{
	GET(TechnoTypeClass*, technoType, EAX);

	const auto& nOffs = TechnoTypeExt::ExtMap.Find(technoType)->TurretOffset;

	return nOffs.Get() == Vector3D<int>::Empty ?
		0x73B78A : 0x73B790;
}

DEFINE_HOOK(0x73BA4C, UnitClass_DrawVXL_TurretMultiOffset1, 0x6) //0
{
	GET(TechnoTypeClass*, pType, EBX);
	LEA_STACK(Matrix3D*, mtx, STACK_OFFS(0x1D0, 0x13C));

	const auto& nOffs = TechnoTypeExt::ExtMap.Find(pType)->TurretOffset;
	const double& factor = *reinterpret_cast<double*>(0xB1D008);

	float x = static_cast<float>(nOffs->X * factor);
	float y = static_cast<float>(nOffs->Y * factor);
	float z = static_cast<float>(nOffs->Z * factor);

	mtx->Translate(x, y, z);

	return 0x73BA68;
}

DEFINE_HOOK(0x73C890, UnitClass_Draw_1_TurretMultiOffset, 0x8) //0
{
	GET(TechnoTypeClass*, pType, EAX);
	LEA_STACK(Matrix3D*, mtx, 0x80);

	const auto& nOffs = TechnoTypeExt::ExtMap.Find(pType)->TurretOffset;

	float x = static_cast<float>(nOffs->X * TechnoTypeExt::TurretMultiOffsetOneByEightMult);
	float y = static_cast<float>(nOffs->Y * TechnoTypeExt::TurretMultiOffsetOneByEightMult);
	float z = static_cast<float>(nOffs->Z * TechnoTypeExt::TurretMultiOffsetOneByEightMult);

	mtx->Translate(x, y, z);

	return 0x73C8B7;
}

DEFINE_HOOK(0x43E0C4, BuildingClass_Draw_43DA80_TurretMultiOffset, 0x5) //0
{
	GET(TechnoTypeClass*, pType, EDX);
	LEA_STACK(Matrix3D*, mtx, 0x60);

	const auto& nOffs = TechnoTypeExt::ExtMap.Find(pType)->TurretOffset;

	float x = static_cast<float>(nOffs->X * TechnoTypeExt::TurretMultiOffsetOneByEightMult);
	float y = static_cast<float>(nOffs->Y * TechnoTypeExt::TurretMultiOffsetOneByEightMult);
	float z = static_cast<float>(nOffs->Z * TechnoTypeExt::TurretMultiOffsetOneByEightMult);

	mtx->Translate(x, y, z);

	return 0x43E0E8;
}

DEFINE_HOOK(0x6B7282, SpawnManagerClass_AI_PromoteSpawns, 0x5)
{
	GET(SpawnManagerClass*, pThis, ESI);

	if (TechnoTypeExt::ExtMap.Find(pThis->Owner->GetTechnoType())->Promote_IncludeSpawns)
	{
		for (const auto i : pThis->SpawnedNodes)
		{
			if (i->Unit && i->Unit->Veterancy.Veterancy < pThis->Owner->Veterancy.Veterancy)
				i->Unit->Veterancy.Add(pThis->Owner->Veterancy.Veterancy - i->Unit->Veterancy.Veterancy);
		}
	}

	return 0;
}

DEFINE_HOOK(0x73D223, UnitClass_DrawIt_OreGath, 0x6)
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
	auto idxTiberium = pThis->GetCell()->GetContainedTiberiumIndex();

	if (idxTiberium != -1)
	{
		const auto pData = TechnoTypeExt::ExtMap.Find(pType);
		const auto idxArray = pData->OreGathering_Tiberiums.IndexOf(idxTiberium);

		if (idxArray != -1)
		{
			const auto nFramesPerFacing = pData->OreGathering_FramesPerDir.GetItemAtOrDefault(idxArray , 15);

			if (auto pAnimType = pData->OreGathering_Anims.GetItemAtOrMax(idxArray))
			{
				pSHP = pAnimType->GetImage();
				if (const auto pPalette = AnimTypeExt::ExtMap.Find(pAnimType)->Palette)
					pDrawer = pPalette->GetConvert<PaletteManager::Mode::Temperate>();
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

DEFINE_HOOK(0x700C58, TechnoClass_CanPlayerMove_NoManualMove, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	return TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())->NoManualMove.Get() ? 0x700C62 : 0;
}

DEFINE_HOOK(0x73CF46, UnitClass_Draw_It_KeepUnitVisible, 0x6)
{
	GET(UnitClass*, pThis, ESI);
	return (TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())->DeployingAnim_KeepUnitVisible.Get() &&
		(pThis->Deploying || pThis->Undeploying)) ?
		0x73CF62 : 0;
}

// Ares hooks in at 739B8A, this goes before it and skips it if needed.
DEFINE_HOOK(0x739B7C, UnitClass_Deploy_DeployDir, 0x6)
{
	enum { SkipAnim = 0x739C70, PlayAnim = 0x739B9E };

	GET(UnitClass*, pThis, ESI);

	if (!pThis->InAir)
	{
		if (pThis->Type->DeployingAnim)
		{
			return (TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())->DeployingAnim_AllowAnyDirection.Get()) ? PlayAnim : 0;
		}

		pThis->Deployed = true;
	}

	return SkipAnim;
}

DEFINE_HOOK_AGAIN(0x739D8B, UnitClass_DeployUndeploy_DeployAnim, 0x5)
DEFINE_HOOK(0x739BA8, UnitClass_DeployUndeploy_DeployAnim, 0x5)
{
	enum { Deploy = 0x739C20, DeployUseUnitDrawer = 0x739C0A, Undeploy = 0x739E04, UndeployUseUnitDrawer = 0x739DEE };

	GET(UnitClass*, pThis, ESI);

	bool isDeploying = R->Origin() == 0x739BA8;
	auto const pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	{
		if (auto const pAnim = GameCreate<AnimClass>(pThis->Type->DeployingAnim,
			pThis->Location, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0,
			!isDeploying ? pExt->DeployingAnim_ReverseForUndeploy.Get() : false))
		{
			pThis->DeployAnim = pAnim;
			pAnim->SetOwnerObject(pThis);

			if (pExt->DeployingAnim_UseUnitDrawer)
				return isDeploying ? DeployUseUnitDrawer : UndeployUseUnitDrawer;
		}
		else
		{
			pThis->DeployAnim = nullptr;
		}
	}

	return isDeploying ? Deploy : Undeploy;
}

DEFINE_HOOK_AGAIN(0x739E81, UnitClass_DeployUndeploy_DeploySound, 0x6)
DEFINE_HOOK(0x739C86, UnitClass_DeployUndeploy_DeploySound, 0x6)
{
	enum { DeployReturn = 0x739CBF, UndeployReturn = 0x739EB8 };

	GET(UnitClass*, pThis, ESI);

	const bool isDeploying = R->Origin() == 0x739C86;
	const bool isDoneWithDeployUndeploy = isDeploying ? pThis->Deployed : !pThis->Deployed;

	if (isDoneWithDeployUndeploy)
		return 0; // Only play sound when done with deploying or undeploying.

	return isDeploying ? DeployReturn : UndeployReturn;
}

// Issue #503
// Author : Otamaa
DEFINE_HOOK(0x4AE670, DisplayClass_GetToolTip_EnemyUIName, 0x8)
{
	enum { SetUIName = 0x4AE678 };

	GET(ObjectClass*, pObject, ECX);

	if (!HouseExt::IsObserverPlayer())
	{
		if (auto pFoot = generic_cast<FootClass*>(pObject))
		{
			if (!pObject->IsDisguised())
			{
				if (const auto pOwnerHouse = pFoot->GetOwningHouse())
				{
					if (!pOwnerHouse->IsNeutral() && !pOwnerHouse->IsAlliedWith(HouseClass::CurrentPlayer))
					{
						const auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pFoot->GetTechnoType());
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

#include <Misc/AresData.h>

DEFINE_HOOK(0x4DB157, FootClass_DrawVoxelShadow_TurretShadow, 0x8)
{
	GET(FootClass*, pThis, ESI);
	GET_STACK(Point2D, pos, STACK_OFFSET(0x18, 0x28));
	GET_STACK(Surface*, pSurface, STACK_OFFSET(0x18, 0x24));
	GET_STACK(bool, a9, STACK_OFFSET(0x18, 0x20)); // unknown usage
	GET_STACK(Matrix3D*, pMatrix, STACK_OFFSET(0x18, 0x1C));
	GET_STACK(Point2D*, a4, STACK_OFFSET(0x18, 0x14)); // unknown usage
	GET_STACK(Point2D, a3, STACK_OFFSET(0x18, -0x10)); // unknown usage
	GET_STACK(int*, a5, STACK_OFFSET(0x18, 0x10)); // unknown usage
	GET_STACK(int, angle, STACK_OFFSET(0x18, 0xC));
	GET_STACK(int, idx, STACK_OFFSET(0x18, 0x8));
	GET_STACK(VoxelStruct*, pVXL, STACK_OFFSET(0x18, 0x4));

	
	auto pType = pThis->GetTechnoType();
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	const auto tur = pType->Gunner || pType->IsChargeTurret
		? AresData::GetTurretsVoxel(pType , pThis->CurrentTurretNumber)
		: &pType->TurretVoxel;

	if (pTypeExt->TurretShadow.Get(RulesExt::Global()->DrawTurretShadow) && tur->VXL && tur->HVA)
	{
		Matrix3D mtx; 
		pThis->Locomotor->Shadow_Matrix(&mtx, nullptr);
		mtx.RotateZ((float)(pThis->SecondaryFacing.Current().GetRadian<32>() - pThis->PrimaryFacing.Current().GetRadian<32>()));
		const auto pTurOffset = pTypeExt->TurretOffset.GetEx();
		float x = (float)(pTurOffset->X / 8);
		float y = (float)(pTurOffset->Y / 8);
		float z = -tur->VXL->TailerData->MinBounds.Z;
		mtx.Translate(x, y, z);
		Matrix3D::MatrixMultiply(&mtx, &Game::VoxelDefaultMatrix(), &mtx);

		pThis->DrawVoxelShadow(tur, 0, angle, 0, a4, &a3, &mtx, a9, pSurface, pos);

		const auto bar = pType->ChargerBarrels  ? 
			AresData::GetBarrelsVoxel(pType, pThis->CurrentTurretNumber)
			: &pType->BarrelVoxel;

		if (bar->VXL && bar->HVA)
			pThis->DrawVoxelShadow(bar, 0, angle, 0, a4, &a3, &mtx, a9, pSurface, pos);
	}

	if (pTypeExt->ShadowIndices.empty())
	{
		pThis->DrawVoxelShadow(pVXL, idx, angle, a5, a4, &a3, pMatrix, a9, pSurface, pos);
	}
	else
	{
		for (auto index : pTypeExt->ShadowIndices) {
			pMatrix->TranslateZ(-pVXL->HVA->Matrixes[index].GetZVal());
			Matrix3D::MatrixMultiply(pMatrix, &Game::VoxelDefaultMatrix(), pMatrix);
			pThis->DrawVoxelShadow(pVXL, index, angle, a5, a4, &a3, pMatrix, a9, pSurface, pos);
		}
	}

	return 0x4DB195;
}