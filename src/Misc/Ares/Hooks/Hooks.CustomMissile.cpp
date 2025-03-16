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
#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/Techno/Body.h>

#include <Conversions.h>

#include <Locomotor/Cast.h>
#include <SpawnManagerClass.h>

#pragma region SpawnManagerHooks
ASMJIT_PATCH(0x6B6D60, SpawnManagerClass_CTOR_CustomMissile, 6)
{
	GET(SpawnManagerClass* const, pSpawnManager, ESI);
	return TechnoTypeExtContainer::Instance.Find(pSpawnManager->SpawnType)->IsCustomMissile ? 0x6B6D86 : 0x0;
}

#pragma region Update
ASMJIT_PATCH(0x6B78F8, SpawnManagerClass_Update_CustomMissile, 6)
{
	GET(TechnoTypeClass* const, pSpawnType, EAX);
	return TechnoTypeExtContainer::Instance.Find(pSpawnType)->IsCustomMissile ? 0x6B791F : 0x0;
}

ASMJIT_PATCH(0x6B7A6A, SpawnManagerClass_Update_CustomMissile2, 5)
{
	GET(SpawnManagerClass*, pSpawnManager, ESI);
	GET(int, idxSpawn, EDI);

	auto pSpawnType = pSpawnManager->SpawnType;

	const auto pExt = TechnoTypeExtContainer::Instance.Find(pSpawnType);

	if (pExt->IsCustomMissile) {
		auto node = &pSpawnManager->SpawnedNodes.Items[idxSpawn]->NodeSpawnTimer;
		node->StartTime = Unsorted::CurrentFrame();
		node->TimeLeft = pExt->CustomMissileData->PauseFrames + pExt->CustomMissileData->TiltFrames;
		return 0x6B7B03;
	}

	return 0;
}

//new
ASMJIT_PATCH(0x6B750B, SpawnManagerClass_Update_CustomMissilePreLauchAnim, 0x5)
{
	GET(AircraftClass*, pSpawned, EDI);

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pSpawned->Type);

	if (pSpawned->Type == RulesClass::Instance->CMisl.Type) {
		return 0x0;
	} else if (pTypeExt->IsCustomMissile) {
		if(AnimTypeClass* pType = pTypeExt->CustomMissilePreLauchAnim) {
			AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pType, pSpawned->Location, 2, 1, 0x600, -10, false),
				pSpawned->Owner,
				nullptr,
				pSpawned,
				true
			);
		}
	}

	return 0x6B757A;
}

// new
ASMJIT_PATCH(0x6B74BC, SpawnManagerClass_Update_MissileCoordOffset, 0x6)
{
	enum
	{
		OffsetBy28 = 0x6B74C4,
		GetPrimaryFacing = 0x6B74DB
	};

	//GET(SpawnManagerClass*, pThis, ESI);
	//GET(AircraftClass*, pSpawned, EDI);
	GET(AircraftTypeClass* const, pMissile, EAX);

	const auto pExt = TechnoTypeExtContainer::Instance.Find(pMissile);

	if (pExt->IsCustomMissile && pExt->CustomMissileOffset.isset()) {
		R->Stack(0x2C, R->ECX<int>() - pExt->CustomMissileOffset->X);
		R->Stack(0x30, R->EDX<int>() - pExt->CustomMissileOffset->Y);
	}
	else if(pMissile == RulesClass::Instance->CMisl.Type) {
		return OffsetBy28;
	}

	return GetPrimaryFacing;
}

#pragma endregion

ASMJIT_PATCH(0x6B7D50, SpawnManagerClass_CountDockedSpawns, 0x6)
{
	GET(SpawnManagerClass*, pThis, ECX);

	int nCur = 0;
	if(pThis) { // some function call this without checking , so here it is the check
		for (auto const& pNode : pThis->SpawnedNodes)
		{
			const auto nStatus = pNode->Status;
			const auto nEligible =
				nStatus == SpawnNodeStatus::Idle
				|| nStatus == SpawnNodeStatus::Reloading
				|| nStatus == SpawnNodeStatus::Dead
				// spawn timer should be updated somewhere ?
				&& pNode->NodeSpawnTimer.IsNotActive();

			if (nEligible)
			{
				++nCur;
			}

		}
	}

	R->EAX(nCur);
	return 0x6B7D73;
}
#pragma endregion
