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

DEFINE_HOOK(0x707319, TechnoClass_CalcVoxelShadow_ShadowScale, 0x6)
{
	GET(TechnoTypeClass*, pType, EAX);

	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	if (pTypeExt->ShadowScale > 0)
	{
		REF_STACK(Matrix3D, mtx, STACK_OFFSET(0xE8, -0x90));
		mtx.Scale(pTypeExt->ShadowScale);
		return 0x707331;
	}

	return 0;
}

DEFINE_HOOK(0x6F64A0, TechnoClass_DrawHealthBar_Hide, 0x5)
{
	enum
	{
		Draw = 0x0,
		DoNotDraw = 0x6F6ABD
	};

	GET(TechnoClass*, pThis, ECX);

	const auto what = pThis->WhatAmI();

	if(what == UnitClass::AbsID) {
		const auto pUnit = (UnitClass*)pThis;

		if (pUnit->DeathFrameCounter > 0)
			return DoNotDraw;
	}

	if(what == BuildingClass::AbsID) {
		const auto pBld = (BuildingClass*)pThis;

		if(BuildingTypeExt::ExtMap.Find(pBld->Type)->Firestorm_Wall)
			return DoNotDraw;
	}

	if ((TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())->HealthBar_Hide.Get())
		|| pThis->TemporalTargetingMe
		|| pThis->IsSinking
	)
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

DEFINE_HOOK(0x73CCE1, UnitClass_DrawSHP_TurretOffest, 0x6)
{
	GET(UnitClass*, pThis, EBP);
	REF_STACK(Point2D, pos, STACK_OFFSET(0x15C, -0xE8));

	Matrix3D mtx;
	mtx.MakeIdentity();
	mtx.RotateZ(static_cast<float>(pThis->PrimaryFacing.Current().GetRadian<32>()));
	const auto& nOffs = TechnoTypeExt::ExtMap.Find(pThis->Type)->TurretOffset;

	float x = static_cast<float>(nOffs->X * TechnoTypeExt::TurretMultiOffsetDefaultMult);
	float y = static_cast<float>(nOffs->Y * TechnoTypeExt::TurretMultiOffsetDefaultMult);
	float z = static_cast<float>(nOffs->Z * TechnoTypeExt::TurretMultiOffsetDefaultMult);

	mtx.Translate(x, y, z);

	double turretRad = pThis->TurretFacing().GetRadian<32>();
	double bodyRad = pThis->PrimaryFacing.Current().GetRadian<32>();
	float angle = (float)(turretRad - bodyRad);
	mtx.RotateZ(angle);
	auto res = Matrix3D::MatrixMultiply(mtx, Vector3D<float>::Empty);
	auto location = CoordStruct { static_cast<int>(res.X), static_cast<int>(-res.Y), static_cast<int>(res.Z) };
	Point2D temp;
	pos += *TacticalClass::Instance()->CoordsToScreen(&temp, &location);

	return 0;
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
			const auto nFramesPerFacing = pData->OreGathering_FramesPerDir.GetItemAtOrDefault(idxArray, 15);

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

AnimTypeClass* GetDeployAnim(UnitClass* pThis)
{
	//auto const pExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

	//if (pExt->DeployAnims.empty())
	//	return nullptr;

	//if(((pExt->DeployAnims.size() & 28u) != 0u))
	//	return pExt->DeployAnims[0];

	//const auto nIdx = (((pExt->DeployAnims.size() >> 2) * (((pThis->PrimaryFacing.Current().Raw >> 7) + 1) >> 1)) >> 8);
	//return pExt->DeployAnims[nIdx];
	return pThis->Type->DeployingAnim;
}

bool NOINLINE SetAnim(AnimTypeClass* pAnimType , UnitClass* pUnit , bool isDeploying)
{
	if(pUnit->DeployAnim) {
		return true;
	}

	auto const pExt = TechnoTypeExt::ExtMap.Find(pUnit->Type);

	if (auto const pAnim = GameCreate<AnimClass>(pAnimType,
			pUnit->Location, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0,
				!isDeploying ? pExt->DeployingAnim_ReverseForUndeploy.Get() : false))
	{
			pUnit->DeployAnim = pAnim;
			pAnim->SetOwnerObject(pUnit);

			if (pExt->DeployingAnim_UseUnitDrawer) {
				pAnim->LightConvert = pUnit->GetRemapColour();
			}

		return true;
	}

	return false;
}

DEFINE_HOOK(0x739B90 , UnitClass_Deploy_DeployAnim , 0x6)
{
	GET(UnitClass*, pThis, ESI);

	const auto pAnimType = GetDeployAnim(pThis);

	if(!pAnimType)
		return 0x739C6A;

	return SetAnim(pAnimType , pThis , true) ?
		0x739C20 : 0x739C62;
}

DEFINE_HOOK(0x739D73 , UnitClass_UnDeploy_DeployAnim , 0x6)
{
	GET(UnitClass*, pThis, ESI);

	const auto pAnimType = GetDeployAnim(pThis);

	if(!pAnimType)
		return 0x739E4F;

	return SetAnim(pAnimType , pThis , false) ?
		0x739E04 : 0x739E46;
}

//DEFINE_HOOK(0x714706, TechnoTypeClass_read_DeployAnim, 0x9)
//{
//	GET(TechnoTypeClass*, pThis, EBP);
//	pThis->UnloadingClass = R->EAX<UnitTypeClass*>();
//	R->EAX((UnitTypeClass*)nullptr);
//	return 0x71473F;
//}

// DEFINE_HOOK_AGAIN(0x739D8B, UnitClass_DeployUndeploy_DeployAnim, 0x5)
// DEFINE_HOOK(0x739BA8, UnitClass_DeployUndeploy_DeployAnim, 0x5)
// {
// 	enum { Deploy = 0x739C20, DeployUseUnitDrawer = 0x739C0A, Undeploy = 0x739E04, UndeployUseUnitDrawer = 0x739DEE };
//
// 	GET(UnitClass*, pThis, ESI);
//
// 	bool isDeploying = R->Origin() == 0x739BA8;
// 	auto const pExt = TechnoTypeExt::ExtMap.Find(pThis->Type);
//
// 	{
// 		if(auto pAnimType = GetDeployAnim(pThis)) {
// 			if (auto const pAnim = GameCreate<AnimClass>(pAnimType,
// 				pThis->Location, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0,
// 				!isDeploying ? pExt->DeployingAnim_ReverseForUndeploy.Get() : false))
// 			{
// 				pThis->DeployAnim = pAnim;
// 				pAnim->SetOwnerObject(pThis);
//
// 				if (pExt->DeployingAnim_UseUnitDrawer)
// 					return isDeploying ? DeployUseUnitDrawer : UndeployUseUnitDrawer;
// 			}
// 			else
// 			{
// 				pThis->DeployAnim = nullptr;
// 			}
// 		}
// 	}
//
// 	return isDeploying ? Deploy : Undeploy;
// }

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
					if (!pOwnerHouse->IsNeutral() && !pOwnerHouse->IsAlliedWith_(HouseClass::CurrentPlayer))
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
