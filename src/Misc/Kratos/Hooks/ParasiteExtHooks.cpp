#include <exception>
#include <Windows.h>

#include <AnimClass.h>
#include <AnimTypeClass.h>
#include <ParasiteClass.h>

#include <Misc/Kratos/Kratos.h>
#include <Helpers/Macro.h>

#include <Misc/Kratos/Ext/Helper/Status.h>

#ifdef _ENABLE_HOOKS

// Take over create Anim by ParasiteEat
ASMJIT_PATCH(0x62A13F, ParasiteClass_Update_Anim_Remap, 0x5)
{
	GET(ParasiteClass*, pParasite, ESI);
	GET(AnimTypeClass*, pAnimType, EBP);
	GET_STACK(CoordStruct, location, 0x34);

	AnimClass* pAnim = GameCreate<AnimClass>(pAnimType, location);
	SetAnimOwner(pAnim, flag_cast_to<TechnoClass*>(pParasite->Victim));
	pAnim->Owner = pParasite->Owner->Owner;

	return 0;
}

// Eat by Ginormous Squid
/*
ASMJIT_PATCH(0x6297F0, ParasiteClass_GrappleUpdate_Anim_Remap, 0x6)
{
	GET(ParasiteClass*, pParasite, ESI);

	return 0;
}*/

#endif