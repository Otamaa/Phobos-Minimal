#include "Body.h"

#include <Ext/Rules/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>

#include <Utilities/Macro.h>

// Replace single call
DEFINE_FUNCTION_JUMP(CALL,0x4ABCEB, FakeTacticalClass::Tactical_MakeFilteredSelection);
DEFINE_FUNCTION_JUMP(LJMP, 0x732C30, FakeTacticalClass::TypeSelectFilter)

// Replace vanilla function. For in case another module tries to call the vanilla function at offset
DEFINE_FUNCTION_JUMP(LJMP, 0x6D9FF0, FakeTacticalClass::Tactical_MakeFilteredSelection);

ASMJIT_PATCH(0x73298D, TypeSelectExecute_UseIFVMode, 0x5) {

	for (const auto pObject : ObjectClass::CurrentObjects()) {
		if (const auto pTechno = flag_cast_to<TechnoClass*, true>(pObject)){
			auto pType = GET_TECHNOTYPE(pTechno);
			const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

			const std::string gunnerID = pTypeExt->GetGunnerID(pTechno->CurrentWeaponNumber);

			if (std::ranges::none_of(TacticalExtData::IFVGroups, [gunnerID](const std::string& id) {
				return !_stricmp(id.c_str(), gunnerID.c_str()); }))			
				TacticalExtData::IFVGroups.emplace_back(gunnerID);
		}
	}

	return 0;
}

ASMJIT_PATCH(0x732C06, TypeSelectExecute_Clear, 0x6)
{
	TacticalExtData::IFVGroups.clear();
	return 0;
}