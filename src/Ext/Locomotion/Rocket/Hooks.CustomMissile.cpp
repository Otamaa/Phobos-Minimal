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
#include <Ext/Aircraft/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/AircraftType/Body.h>

#include <Conversions.h>

#include <Locomotor/Cast.h>
#include <SpawnManagerClass.h>

#pragma region SpawnManagerHooks
ASMJIT_PATCH(0x6B6D60, SpawnManagerClass_CTOR_CustomMissile, 6)
{
	GET(SpawnManagerClass* const, pSpawnManager, ESI);
	return AircraftTypeExtContainer::Instance.Find(pSpawnManager->SpawnType)->IsCustomMissile ? 0x6B6D86 : 0x0;
}

#pragma region Update
ASMJIT_PATCH(0x6B78F8, SpawnManagerClass_Update_CustomMissile, 6)
{
	GET(TechnoTypeClass* const, pSpawnType, EAX);

	if(pSpawnType->WhatAmI() == AbstractType::AircraftType
		&& AircraftTypeExtContainer::Instance.Find((AircraftTypeClass*)pSpawnType)->IsCustomMissile)
		return 0x6B791F ;

	return 0x0;
}

ASMJIT_PATCH(0x6B7A6A, SpawnManagerClass_Update_CustomMissile2, 5)
{
	GET(SpawnManagerClass*, pSpawnManager, ESI);
	GET(int, idxSpawn, EDI);

	auto pSpawnType = pSpawnManager->SpawnType;

	if (pSpawnType->WhatAmI() == AbstractType::AircraftType){
		const auto pExt = AircraftTypeExtContainer::Instance.Find(pSpawnType);

		if (pExt->IsCustomMissile) {
			auto node = &pSpawnManager->SpawnedNodes.Items[idxSpawn]->NodeSpawnTimer;
			node->StartTime = Unsorted::CurrentFrame();
			node->TimeLeft = pExt->CustomMissileData->PauseFrames + pExt->CustomMissileData->TiltFrames;
			return 0x6B7B03;
		}
	}

	return 0;
}

// new
//#pragma optimize("", off )
//ASMJIT_PATCH(0x41ADC0, AircraftClass_InWhichLayer_Crash, 0x9)
//{
//	GET(AircraftClass*, pThis, ECX);
//	auto pLoco = pThis->Locomotor.GetInterfacePtr();
//	R->EAX(pLoco->In_Which_Layer());
//	return 0x41ADE4;
//}

ASMJIT_PATCH(0x6B7498, SpawnManagerClass_AI_Statte0_Handlestuffs, 0x8)
{
	GET(SpawnManagerClass*, pThis, ESI);
	GET(AircraftClass*, pSpawned, EDI);
	GET(CoordStruct*, pCoord, EAX);

	auto pMissile = pThis->SpawnType;
	auto animCoord = *pCoord;
	const bool IsCMisl = pMissile == RulesClass::Instance->CMisl.Type;
	const auto pExt = AircraftTypeExtContainer::Instance.Find(pMissile);
	AnimTypeClass* _pAnim = nullptr;

	if (pExt->IsCustomMissile){
		if(pExt->CustomMissileOffset.isset()){
			animCoord.X -= pExt->CustomMissileOffset->X;
			animCoord.Y -= pExt->CustomMissileOffset->Y;
		}

		_pAnim = pExt->CustomMissilePreLauchAnim;

	} else if(IsCMisl) {
		animCoord.X -= 28;
		animCoord.Y -= 28;

		_pAnim = AnimTypeClass::Find(GameStrings::V3TAKEOFF());
	}

	animCoord.Z += 10;

	auto priFacing = pThis->Owner->PrimaryFacing.Current();
	if(pSpawned->Unlimbo(animCoord, (DirType)priFacing.GetFacing<8>())){
		if (_pAnim) {
			AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(_pAnim, pSpawned->Location, 2, 1, 0x600, -10, false),
				pSpawned->Owner,
				nullptr,
				pSpawned->IsAlive ? pSpawned : nullptr,
				true, false
			);
		}
	}

	R->EBX(R->Stack<int>(0x14));
	return 0x6B757A;
}
//#pragma optimize("", on )

#pragma endregion

#pragma endregion
