#include <Ext/Building/Body.h>

#include <SpecificStructures.h>
#include <ScenarioClass.h>
#include <WarheadTypeClass.h>

#include <Ext/BuildingType/Body.h>
#include <Ext/Anim/Body.h>
#include <Utilities/Macro.h>

#include <TacticalClass.h>

#ifndef ENABLE_NEWHOOKS
// Brief :
// The AnimClass* memory is owned by the vector , dont delete it
// just un-init it and replace it with nullptr is enough
namespace DamageFireAnims
{
	void FORCEDINLINE HandleRemoveAsExt(BuildingExtData* pExt) {
		for (auto& nFires : pExt->DamageFireAnims) {
			if (nFires && nFires->Type) {
 				//GameDelete<true,false>(nFires);
				nFires->TimeToDie = true;
				nFires->UnInit();
				nFires = nullptr;
			}
		}
	}

	void FORCEDINLINE HandleRemove(FakeBuildingClass* pThis) {
		HandleRemoveAsExt(pThis->_GetExtData());
	}

	void FORCEDINLINE HandleInvalidPtr(FakeBuildingClass* pThis, void* ptr) {
		for (auto& nFires : pThis->_GetExtData()->DamageFireAnims) {
			if (nFires == ptr) {
				nFires = nullptr;
			}
		}
	}

};

DEFINE_FUNCTION_JUMP(CALL, 0x43FC92, FakeBuildingClass::_OnFireAI);
DEFINE_FUNCTION_JUMP(LJMP, 0x43C0D0, FakeBuildingClass::_OnFireAI);

//ASMJIT_PATCH(0x46038A , BuildingTypeClass_ReadINI_SkipDamageFireAnims, 0x6) { return 0x46048E; }
DEFINE_JUMP(LJMP, 0x46038A, 0x46048E);
//DEFINE_JUMP(LJMP,0x43BA72, 0x43BA7F); //remove memset for buildingFireAnims

#define HANDLEREMOVE_HOOKS(addr ,reg ,name, size ,ret) \
ASMJIT_PATCH(addr , BuildingClass_##name##_DamageFireAnims , size ) { \
	GET(FakeBuildingClass*, pThis, reg);\
	DamageFireAnims::HandleRemove(pThis);\
	return ret;\
}

HANDLEREMOVE_HOOKS(0x43BDD5,ESI, DTOR, 0x6, 0x43BDF6)
HANDLEREMOVE_HOOKS(0x44AB87,EBP, Fire1, 0x6, 0x44ABAC)
HANDLEREMOVE_HOOKS(0x440076,ESI, Fire2, 0x6, 0x44009B)
HANDLEREMOVE_HOOKS(0x43FC99,ESI, Fire3, 0x6, 0x43FCBE)
HANDLEREMOVE_HOOKS(0x4458E4,ESI, Fire4, 0x6, 0x445905)

#undef HANDLEREMOVE_HOOKS

ASMJIT_PATCH(0x4415F9, BuildingClass_handleRemoveFire5, 0x5)
{
	GET(FakeBuildingClass*, pThis, ESI);
	DamageFireAnims::HandleRemove(pThis);
	R->EBX(0);
	return 0x44161C;
}

ASMJIT_PATCH(0x43C2A0, BuildingClass_RemoveFire_handle, 0x8) //was 5
{
	GET(FakeBuildingClass*, pThis, ECX);
	DamageFireAnims::HandleRemove(pThis);
	return 0x43C2C9;
}

//DEFINE_JUMP(LJMP, 0x44EA1C, 0x44EA2F);
ASMJIT_PATCH(0x44EA1C, BuildingClass_DetachOrInvalidPtr_handle, 0x6)
{
	GET(FakeBuildingClass*, pThis, ESI);
	GET(void*, ptr, EBP);
	DamageFireAnims::HandleInvalidPtr(pThis, ptr);
	return 0x44EA2F;
}

//remove it from load
DEFINE_JUMP(LJMP, 0x454154, 0x454170);
//ASMJIT_PATCH(0x454154 , BuildingClass_LoadGame_DamageFireAnims , 0x6) {
//	return 0x454170;
//}
#endif