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

DEFINE_HOOK(0x6E232E, ActionClass_PlayAnimAt, 0x5)
{
	GET(TActionClass*, pAction, ESI);
	GET_STACK(HouseClass*, pHouse, 0x1C);
	LEA_STACK(CoordStruct*, pCoords, 0xC);

	AnimTypeClass* AnimType = AnimTypeClass::Array->GetItemOrDefault(pAction->Value);

	if (!AnimType)
		return 0x6E2378;

	AnimClass* Anim = GameCreate<AnimClass>(AnimType, pCoords ,0 , 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200 , 0 , false);

	if (AnimType->MakeInfantry > -1) {
		AnimTypeExt::SetMakeInfOwner(Anim, pHouse , nullptr);
	}

	AnimExt::SetAnimOwnerHouseKind(Anim, pHouse, nullptr, false);

	R->EAX<AnimClass*>(Anim);

	return 0x6E2368;
}