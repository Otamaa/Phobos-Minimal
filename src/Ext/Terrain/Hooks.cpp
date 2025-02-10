#include "Body.h"
#include <SpecificStructures.h>
#include <ScenarioClass.h>

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/TerrainType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/Cell/Body.h>

#include <Utilities/Macro.h>

DEFINE_HOOK(0x71C672, TerrainClass_CathFire_AttachedAnim, 0x8) {
	GET(TerrainClass*, pThis, ESI);
	GET(AnimClass*, pAnim, EDI);

	pAnim->SetOwnerObject(pThis);
	TerrainExtContainer::Instance.Find(pThis)->AttachedAnim.reset(pAnim);

	return 0x71C67A;
}

DEFINE_HOOK(0x71D09D, TerrainClass_UnLImbo_Light, 0x6)
{
	GET(TerrainClass*, pThis, ECX);
	GET(CoordStruct*, pCoord, EBP);

	TerrainExtData::Unlimbo(pThis, pCoord);
	return 0;
}

DEFINE_HOOK(0x71CA15, TerrainClass_Limbo_Light, 0x6)
{
	GET(bool, nLimboed, EAX);
	GET(TerrainClass*, pThis, EDI);

	if (nLimboed)
	{
		TerrainExtContainer::Instance.Find(pThis)->LighSource.reset(nullptr);
		TerrainExtContainer::Instance.Find(pThis)->AttachedAnim.reset(nullptr);
		TerrainExtContainer::Instance.Find(pThis)->AttachedFireAnim.reset(nullptr);
	}

	return 0;
}

DEFINE_HOOK(0x71C2BC, TerrainClass_Draw_CustomPal, 0x6)
{
	GET(TerrainClass*, pThis, ESI);
	GET(ConvertClass*, pConvert, EDX);
	GET(TerrainTypeClass*, pThisType, EAX);


	const auto pTerrainExt = TerrainTypeExtContainer::Instance.Find(pThisType);

	if (const auto pConvertData = pTerrainExt->CustomPalette) {
		auto const pCell = pThis->GetCell();
		int wallOwnerIndex = pCell->WallOwnerIndex;
		int colorSchemeIndex = HouseClass::CurrentPlayer->ColorSchemeIndex;

		if (wallOwnerIndex >= 0)
			colorSchemeIndex = HouseClass::Array->GetItem(wallOwnerIndex)->ColorSchemeIndex;


		pConvert = pConvertData->ColorschemeDataVector->Items[colorSchemeIndex]->LightConvert;
		R->EBP(pCell->Intensity_Normal);
	}

	R->EDX(pConvert);
	return 0x0;
}

DEFINE_HOOK(0x5F4FEF, ObjectClass_Put_RegisterLogic_Terrain, 0x6)
{
	GET(ObjectClass*, pThis, ESI);
	GET(ObjectTypeClass*, pType, EBX);

	enum { FurtherCheck = 0x5F501B, NoUpdate = 0x5F5045 };

	if(pThis->WhatAmI() == AbstractType::VeinholeMonster) {
		return FurtherCheck;
	}

	if (!pType->IsLogic) {
		return NoUpdate;
	}

	if (pType->WhatAmI() == TerrainTypeClass::AbsID) {
		auto const pTerrainType = static_cast<TerrainTypeClass* const>(pType);
		if (!pTerrainType->SpawnsTiberium
			&& !pTerrainType->IsFlammable
			&& !pTerrainType->IsAnimated
			&& !pTerrainType->IsVeinhole
			) {
			return NoUpdate;
		}
	}

	return FurtherCheck;
}
//#endif


DEFINE_HOOK(0x71C6EE, TerrainClass_FireOut_Crumbling, 0x6)
{
	enum { StartCrumbling = 0x71C6F8, Skip = 0x71C72B };

	GET(TerrainClass*, pThis, ESI);

	auto const pTypeExt = TerrainTypeExtContainer::Instance.Find(pThis->Type);

	if (!pThis->TimeToDie && pTypeExt->HasCrumblingFrames)
	{
		// Needs to be added to the logic layer for the anim to work.
		LogicClass::Instance->AddObject(pThis, false);
		VocClass::PlayIndexAtPos(pTypeExt->CrumblingSound, pThis->GetCoords());

		return StartCrumbling;
	}

	return Skip;
}
