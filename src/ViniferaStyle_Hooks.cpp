#include "ViniferaStyle_Hooks.h"
#include <Ext/Techno/Body.h>
#include <Utilities/Patch.h>

#ifdef aaa
DECLARE_PATCH(Ares_TechnoClass_Update)
{
	GET_REGISTER_STATIC_TYPE(TechnoClass*, pThis, esi);
	GET_REGISTER_STATIC_TYPE(DWORD, pTechnoExt, edi);
	static bool cur = *reinterpret_cast<bool*>(pTechnoExt + 0x9C);
	static auto pExt = TechnoExt::ExtMap[pThis]; //need better ext , map look-up is slow
	if (pExt)
		pExt->IsDriverKilled = cur;

	static uintptr_t Ares_TechnoClass_Update_ret = ((Phobos::AresBaseAddress + (uintptr_t)0x4A6A7));
	_asm { pop edi}; //stolen byte
	JMP_REG(eax, Ares_TechnoClass_Update_ret);
}


void Vinifera_Style::RegisterHooks() {
	//Patch_Jump(GetAddr("TechnoClass_Update", 0x386), &Ares_TechnoClass_Update);
}
#endif