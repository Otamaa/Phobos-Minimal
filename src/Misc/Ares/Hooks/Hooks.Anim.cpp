#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/Terrain/Body.h>

#include <TerrainTypeClass.h>
#include <New/Type/ArmorTypeClass.h>

#include <Notifications.h>
#include <InfantryClass.h>

#include "Header.h"

ASMJIT_PATCH(0x4232CE, AnimClass_Draw_SetPalette, 6)
{
	GET(AnimClass*, pThis, ESI);
	//GET(AnimTypeClass*, AnimType, EAX);

	const auto pData = AnimTypeExtContainer::Instance.TryFind(pThis->Type);

	if (pData ) {
		if(const auto pConvert = pData->Palette.GetConvert()) {
			R->ECX<ConvertClass*>(pConvert);
			return 0x4232D4;
		}
	}

	return 0;
}

/*
ASMJIT_PATCH_AGAIN(0x42511B, AnimClass_Expired_ScorchFlamer, 0x7)
ASMJIT_PATCH_AGAIN(0x4250C9, AnimClass_Expired_ScorchFlamer, 0x7)
ASMJIT_PATCH(0x42513F, AnimClass_Expired_ScorchFlamer, 0x7)
{
	GET(AnimClass*, pThis, ESI);
	auto pType = pThis->Type;

	CoordStruct crd = pThis->GetCoords();

	if (pType->Flamer)
	{
		// always create at least one small fire
		if (auto const pAnim1 = TechnoExt_ExtData::SpawnAnim(crd, RulesClass::Instance->SmallFire, 64))
			AnimExtData::SetAnimOwnerHouseKind(pAnim1, pAnim1->Owner, nullptr);

		// 50% to create another small fire
		if (ScenarioClass::Instance->Random.RandomFromMax(99) < 50)
		{
			if (auto const pAnim2 = TechnoExt_ExtData::SpawnAnim(crd, RulesClass::Instance->SmallFire, 160))
				AnimExtData::SetAnimOwnerHouseKind(pAnim2, pAnim2->Owner, nullptr);
		}

		// 50% chance to create a large fire
		if (ScenarioClass::Instance->Random.RandomFromMax(99) < 50)
		{
			if (auto const pAnim3 = TechnoExt_ExtData::SpawnAnim(crd, RulesClass::Instance->LargeFire, 112))
				AnimExtData::SetAnimOwnerHouseKind(pAnim3, pAnim3->Owner, nullptr);
		}

	}
	else if (pType->Scorch)
	{
		// creates a SmallFire anim that is attached to the same object
		// this anim is attached to.
		if (pThis->GetHeight() < 10)
		{
			switch (pThis->GetCell()->LandType)
			{
			case LandType::Water:
			case LandType::Beach:
			case LandType::Ice:
			case LandType::Rock:
				break;
			default:
				if (auto pAnim = TechnoExt_ExtData::SpawnAnim(crd, RulesClass::Instance->SmallFire, 0))
				{
					if (pThis->OwnerObject)
					{
						pAnim->SetOwnerObject(pThis->OwnerObject);
					}
				}
			}
		}
	}

	return 0;
}*/