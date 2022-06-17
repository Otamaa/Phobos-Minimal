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
#include <Ext/BulletType/Body.h>
#include <Ext/Techno/Body.h>

DEFINE_HOOK(0x6F64A9, TechnoClass_DrawHealthBar_Hide, 0x5)
{
	GET(TechnoClass*, pThis, ECX);
	auto pTypeData = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	auto pUnit = specific_cast<UnitClass*>(pThis);
	bool bDisableThis = pUnit && pUnit->DeathFrameCounter > 0;

	if ((pTypeData && pTypeData->HealthBar_Hide.Get()) || pThis->TemporalTargetingMe || pThis->IsSinking || bDisableThis)
		return 0x6F6AB6;

	return 0;
}

DEFINE_HOOK(0x6F3C56, TechnoClass_Transform_6F3AD0_TurretMultiOffset, 0x0)
{
	LEA_STACK(Matrix3D*, mtx, STACK_OFFS(0xD8, 0x90));
	GET(TechnoTypeClass*, technoType, EDX);

	TechnoTypeExt::ApplyTurretOffset(technoType, mtx);

	return 0x6F3C6D;
}

DEFINE_HOOK(0x6F3E6E, FootClass_firecoord_6F3D60_TurretMultiOffset, 0x0)
{
	LEA_STACK(Matrix3D*, mtx, STACK_OFFS(0xCC, 0x90));
	GET(TechnoTypeClass*, technoType, EBP);

	TechnoTypeExt::ApplyTurretOffset(technoType, mtx);

	return 0x6F3E85;
}

DEFINE_HOOK(0x73B780, UnitClass_DrawVXL_TurretMultiOffset, 0x0)
{
	GET(TechnoTypeClass*, technoType, EAX);

	auto const pTypeData = TechnoTypeExt::ExtMap.Find(technoType);

	if (pTypeData && *pTypeData->TurretOffset.GetEx() == CoordStruct { 0, 0, 0 })
		return 0x73B78A;

	return 0x73B790;
}

DEFINE_HOOK(0x73BA4C, UnitClass_DrawVXL_TurretMultiOffset1, 0x0)
{
	LEA_STACK(Matrix3D*, mtx, STACK_OFFS(0x1D0, 0x13C));
	GET(TechnoTypeClass*, technoType, EBX);

	double& factor = *reinterpret_cast<double*>(0xB1D008);

	TechnoTypeExt::ApplyTurretOffset(technoType, mtx, factor);

	return 0x73BA68;
}

DEFINE_HOOK(0x73C890, UnitClass_Draw_1_TurretMultiOffset, 0x0)
{
	LEA_STACK(Matrix3D*, mtx, 0x80);
	GET(TechnoTypeClass*, technoType, EAX);

	TechnoTypeExt::ApplyTurretOffset(technoType, mtx, 1 / 8);

	return 0x73C8B7;
}

DEFINE_HOOK(0x43E0C4, BuildingClass_Draw_43DA80_TurretMultiOffset, 0x0)
{
	LEA_STACK(Matrix3D*, mtx, 0x60);
	GET(TechnoTypeClass*, technoType, EDX);

	TechnoTypeExt::ApplyTurretOffset(technoType, mtx, 1 / 8);

	return 0x43E0E8;
}

DEFINE_HOOK(0x6B7282, SpawnManagerClass_AI_PromoteSpawns, 0x5)
{
	GET(SpawnManagerClass*, pThis, ESI);

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Owner->GetTechnoType());
	if (pTypeExt && pTypeExt->Promote_IncludeSpawns)
	{
		for (auto i : pThis->SpawnedNodes)
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

	auto pType = pThis->GetTechnoType();

	ConvertClass* pDrawer = FileSystem::ANIM_PAL;
	SHPStruct* pSHP = FileSystem::OREGATH_SHP;
	int idxFrame = -1;

	if (auto const pData = TechnoTypeExt::ExtMap.Find(pType))
	{
		auto idxTiberium = pThis->GetCell()->GetContainedTiberiumIndex();
		auto idxArray = pData->OreGathering_Tiberiums.size() > 0 ? pData->OreGathering_Tiberiums.IndexOf(idxTiberium) : 0;

		if (idxTiberium != -1 && idxArray != -1)
		{
			auto const pAnimType = pData->OreGathering_Anims.size() > 0 ? pData->OreGathering_Anims[idxArray] : nullptr;
			auto const nFramesPerFacing = pData->OreGathering_FramesPerDir.size() > 0 ? pData->OreGathering_FramesPerDir[idxArray] : 15;

			if (pAnimType)
			{
				pSHP = pAnimType->GetImage();
				if (auto const pAnimExt = AnimTypeExt::ExtMap.Find(pAnimType))
				{
					if (auto const pPalette = pAnimExt->Palette.GetConvert())
						pDrawer = pPalette;
				}
			}

			idxFrame = nFramesPerFacing * nFacing + (Unsorted::CurrentFrame + pThis->WalkedFramesSoFar) % nFramesPerFacing;
		}
	}

	if(idxFrame == -1)
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

	if (auto pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
		return pExt->NoManualMove.Get() ? 0x700C62 : 0;

	return 0;
}

DEFINE_HOOK(0x54B8E9, JumpjetLocomotionClass_In_Which_Layer_Deviation, 0x6)
{
	GET(TechnoClass*, pThis, EAX);

	if (pThis->IsInAir())
	{
		if (auto const pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
		{
			if (!pExt->JumpjetAllowLayerDeviation.Get(RulesExt::Global()->JumpjetAllowLayerDeviation.Get()))
			{
				R->EDX(INT32_MAX); // Override JumpjetHeight / CruiseHeight check so it always results in 3 / Layer::Air.
				return 0x54B96B;
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x73CF46, UnitClass_Draw_It_KeepUnitVisible, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pTypeExt && pTypeExt->DeployingAnim_KeepUnitVisible.Get() && (pThis->Deploying || pThis->Undeploying))
		return 0x73CF62;

	return 0;
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
			auto const pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
			if (pExt && pExt->DeployingAnim_AllowAnyDirection.Get())
				return PlayAnim;

			return 0;
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

	if (auto const pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
	{
		if (auto const pAnim = GameCreate<AnimClass>(pThis->Type->DeployingAnim,
			pThis->Location, 0, 1, 0x600, 0,
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

	bool isDeploying = R->Origin() == 0x739C86;
	bool isDoneWithDeployUndeploy = isDeploying ? pThis->Deployed : !pThis->Deployed;

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

	if (auto pFoot = generic_cast<FootClass*>(pObject)) {
		if (!pObject->IsDisguised()) {
			if (auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pFoot->GetTechnoType())) {
				if(!HouseClass::IsPlayerObserver()){
					if (auto pOwnerHouse = pFoot->GetOwningHouse()) {
						if (!pOwnerHouse->IsNeutral() && !pOwnerHouse->IsAlliedWith(HouseClass::Player)) {
							if (auto pEnemyUIName = pTechnoTypeExt->EnemyUIName.Get().Text) {
								if (wcslen(pEnemyUIName)) {
									R->EAX(pEnemyUIName);
									return SetUIName;
								}
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

//#ifdef CLONE_HEALTH_HOOK
DEFINE_HOOK(0x443C81, BuildingClass_ExitObject_InitialClonedHealth, 0x7)
{
	GET(BuildingClass*, pBuilding, ESI);
	GET(FootClass*, pFoot, EDI);

	if (pBuilding && pBuilding->Type->Cloning && pFoot) {
		if (auto pTypeExt = TechnoTypeExt::ExtMap.Find(pBuilding->GetTechnoType())) {
			if (auto pTypeUnit = pFoot->GetTechnoType()) {
				if(pTypeExt->InitialStrength_Cloning.Get().X || pTypeExt->InitialStrength_Cloning.Get().Y) {
					Vector2D<double> range = pTypeExt->InitialStrength_Cloning.Get();
					double percentage = range.X >= range.Y ? range.X : static_cast<double>(ScenarioClass::Instance->Random.RandomRanged(static_cast<int>(range.X * 100), static_cast<int>(range.Y * 100)) / 100.0);
					int strength = static_cast<int>(pTypeUnit->Strength * percentage);
					strength = Math::LessOrEqualTo(strength, 1);

					pFoot->Health = strength;
					pFoot->EstimatedHealth = strength;
				}
			}
		}
	}

	return 0;
}
//#endif