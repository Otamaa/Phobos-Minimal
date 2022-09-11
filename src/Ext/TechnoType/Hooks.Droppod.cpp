#include "Body.h"

#include <DropPodLocomotionClass.h>

DEFINE_HOOK(0x4B5BCD, DroppodLoco_Process_Speed, 0x6)
{
	//GET(DropPodLocomotionClass*, pThis, EDI);
	GET(RulesClass*, pRules, EDX);

	//auto nLinked = pThis->Owner ? pThis->Owner->get_ID() : "None";
	//Debug::Log(__FUNCTION__"DroppodLoco [%x] Owner [%s] \n",pThis, nLinked);
	R->EAX(pRules->DropPodSpeed);

	return 0x4B5BD3;
}

// Angle ,4B5BEC , EDX ,4B5BF2
DEFINE_HOOK(0x4B5BEC, DroppodLoco_Process_Angle1, 0x6)
{
	//(DropPodLocomotionClass*, pThis, EDI);
	GET(RulesClass*, pRules, EDX);

	//auto nLinked = pThis->Owner ? pThis->Owner->get_ID() : "None";
	//Debug::Log(__FUNCTION__"DroppodLoco [%x] Owner [%s] \n", pThis, nLinked);
	R->EDX(pRules->DropPodAngle);

	return 0x4B5BF2;
}

// Angle , 4B5C14,ECX ,4B5C1A
DEFINE_HOOK(0x4B5C14, DroppodLoco_Process_Angle2, 0x6)
{
	//GET(DropPodLocomotionClass*, pThis, EDI);
	GET(RulesClass*, pRules, EDX);

	//auto nLinked = pThis->Owner ? pThis->Owner->get_ID() : "None";
	//Debug::Log(__FUNCTION__"DroppodLoco [%x] Owner [%s] \n", pThis, nLinked);
	R->ECX(pRules->DropPodAngle);

	return 0x4B5C1A;
}

// Angle , 4B5C50, EAX ,4B5C56
DEFINE_HOOK(0x4B5C50, DroppodLoco_Process_Angle3, 0x6)
{
	//GET(DropPodLocomotionClass*, pThis, EDI);
	GET(RulesClass*, pRules, EAX);

	//auto nLinked = pThis->Owner ? pThis->Owner->get_ID() : "None";
	//Debug::Log(__FUNCTION__"DroppodLoco [%x] Owner [%s] \n", pThis,nLinked);
	R->EAX(pRules->DropPodAngle);

	return 0x4B5C56;
}

//Fix crash if anim not there 4B602D , 0x6
//DEFINE_HOOK(0x4B6025, )
// Direction is unused , which default to 128
// so , Droppod anim based on Direction is gone on YR , because of this
// some anim were not checking type before construct can cause possible crash if the type is nullptr/invalid

/*
DEFINE_HOOK(0x4B5FCC, DroppodLoco_Process_AnimCheck , 0x6)
{
	GET(DropPodLocomotionClass*, pThis, EDI);
	GET(WeaponTypeClass*, pWeapon, ESI);
	REF_STACK(CoordStruct, nCoord, 0x30);

	if (auto pAnimType = MapClass::SelectDamageAnimation(pWeapon->Damage * 2, pWeapon->Warhead, LandType::Clear, nCoord))
	{
		if (auto pAnim = GameCreate<AnimClass>(pAnimType,nCoord,0,1,0x2600,-15,false))
		{
			pAnim->SetHouse(pThis->Owner->Owner);
		}
	}

	return 0x4B602D;
}*/