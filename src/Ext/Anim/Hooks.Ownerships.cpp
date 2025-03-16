#include "Body.h"
#include <Ext/AnimType/Body.h>
#include <Utilities/Macro.h>

ASMJIT_PATCH(0x423991, AnimClass_BounceAI_BounceAnim, 0x5)
{
	GET(FakeAnimTypeClass*, pBounceAnim, ECX);
	GET(AnimClass*, pThis, EBP);

	//const auto pTypeExt = pBounceAnim->_GetExtData();
	TechnoClass* pObject = AnimExtData::GetTechnoInvoker(pThis);
	HouseClass* pHouse = pThis->Owner ? pThis->Owner : ((pObject) ? pObject->GetOwningHouse() : nullptr);

	auto nCoord = pThis->GetCoords();
	AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pBounceAnim, nCoord, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0),
		pHouse,
		nullptr,
		pObject,
		false
	);

	return 0x4239D3;
}

ASMJIT_PATCH(0x423F31, AnimClass_Spawns_Override, 0x6)
{
	GET(FakeAnimClass*, pThis, ESI);
	GET_STACK(int, X, 0x88 - 0x4C);
	GET_STACK(int, Y, 0x88 - 0x48);
	GET_STACK(int, Z, 0x88 - 0x44);

	if(!pThis->Type->Spawns  || pThis->Type->SpawnCount <= 0)
		return 0x423FC6;

	CoordStruct nCoord { X , Y , Z };

	const auto nMax = pThis->Type->SpawnCount == 1 ?
		 1 : ScenarioClass::Instance->Random.RandomFromMax((pThis->Type->SpawnCount * 2));

	const auto pAnimTypeExt = pThis->_GetTypeExtData();
	TechnoClass* pTech = AnimExtData::GetTechnoInvoker(pThis);
	HouseClass* pOwner = pThis->Owner ? pThis->Owner : pTech ? pTech->GetOwningHouse() : nullptr;
	auto nDelay = pAnimTypeExt->Spawns_Delay.Get();

	for (int i = nMax; i > 0; --i) {
		AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pThis->Type->Spawns, nCoord, nDelay, 1, AnimFlag(0x600), 0, false),
		pOwner,
		nullptr,
		pTech,
		false
		);
	}

	R->Stack(0x88 - 0x4C , nCoord.X);
	R->Stack(0x88 - 0x48 , nCoord.Y);
	R->Stack(0x88 - 0x44 , nCoord.Z);
	return 0x423FC6;
}
