#include "Body.h"
#include <SpecificStructures.h>
#include <ScenarioClass.h>

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/TerrainType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/Cell/Body.h>

#include <Utilities/Macro.h>

//#pragma optimize("", off )
ASMJIT_PATCH(0x71C5D2, TerrainClass_CatchFire_AttachFireAnim, 0x6)
{
	GET(FakeTerrainClass*, pThis, EDI);

	if(pThis->Type->SpawnsTiberium || !pThis->Type->IsFlammable)
		return 0x71C69D;

	if (auto fire = pThis->_GetTypeExtData()->TreeFires.GetElements(RulesClass::Instance->TreeFire)) {
		AnimTypeClass* pAnimType = nullptr;

		auto Loc = pThis->Location + CoordStruct { 0 , 0, 80 };

		if (fire.size() == 1) {
			pAnimType = fire[0];
		} else {
			pAnimType = fire[ScenarioClass::Instance->Random.RandomRanged(0, fire.size() - 1)];
		}

		if (pAnimType) {
			auto pAnim = GameCreate<AnimClass>(pAnimType, Loc, 0, 255, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0);
			pAnim->SetOwnerObject(pThis);
			pThis->_GetExtData()->AttachedFireAnim.reset(pAnim);
			pAnim->ZAdjust -= 20;
		}
	}

	pThis->IsBurning = true;
	return 0x71C69D;
}
//#pragma optimize("", on )

ASMJIT_PATCH(0x71D09D, TerrainClass_UnLImbo_Light, 0x6)
{
	GET(TerrainClass*, pThis, ECX);
	GET(CoordStruct*, pCoord, EBP);

	TerrainExtData::Unlimbo(pThis, pCoord);
	return 0;
}

ASMJIT_PATCH(0x71CA15, TerrainClass_Limbo_Light, 0x6)
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

ASMJIT_PATCH(0x71C2BC, TerrainClass_Draw_CustomPal, 0x6)
{
	GET(TerrainClass*, pThis, ESI);
	GET(ConvertClass*, pConvert, EDX);
	GET(TerrainTypeClass*, pThisType, EAX);


	const auto pTerrainExt = TerrainTypeExtContainer::Instance.Find(pThisType);

	if (const auto pConvertData = pTerrainExt->CustomPalette.GetConvert()) {
		auto const pCell = pThis->GetCell();
		int wallOwnerIndex = pCell->WallOwnerIndex;
		int colorSchemeIndex = HouseClass::CurrentPlayer->ColorSchemeIndex;

		if (wallOwnerIndex >= 0)
			colorSchemeIndex = HouseClass::Array->GetItem(wallOwnerIndex)->ColorSchemeIndex;


		pConvert = pTerrainExt->CustomPalette.ColorschemeDataVector->Items[colorSchemeIndex]->LightConvert;
		R->EBP(pCell->Intensity_Normal);
	}

	R->EDX(pConvert);
	return 0x0;
}

ASMJIT_PATCH(0x5F4FEF, ObjectClass_Put_RegisterLogic_Terrain, 0x6)
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

	if (pThis->WhatAmI() == TerrainClass::AbsID) {
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


ASMJIT_PATCH(0x71C6EE, TerrainClass_FireOut_Crumbling, 0x6)
{
	enum { StartCrumbling = 0x71C6F8, Skip = 0x71C72B };

	GET(TerrainClass*, pThis, ESI);

	auto const pTypeExt = TerrainTypeExtContainer::Instance.Find(pThis->Type);

	if (!pThis->TimeToDie && pTypeExt->HasCrumblingFrames)
	{
		// Needs to be added to the logic layer for the anim to work.
		LogicClass::Instance->AddObject(pThis, false);
		VocClass::SafeImmedietelyPlayAt(pTypeExt->CrumblingSound, pThis->GetCoords());

		return StartCrumbling;
	}

	return Skip;
}
