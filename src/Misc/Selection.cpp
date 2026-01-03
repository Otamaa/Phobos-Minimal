
#include <Ext/Tactical/Body.h>
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

	if (!RulesExtData::Instance()->TypeSelectUseIFVMode)
		return 0;

	TacticalExtData::IFVGroups.clear();

	for (const auto pObject : ObjectClass::CurrentObjects()) {
		if (const auto pTechno = flag_cast_to<TechnoClass*, true>(pObject)){
			auto pType = GET_TECHNOTYPE(pTechno);
			const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

			if (!pType->Gunner || (size_t)pTechno->CurrentWeaponNumber >= pTypeExt->WeaponGroupAs.size())
				continue;

			auto gunnerID = &pTypeExt->WeaponGroupAs[pTechno->CurrentWeaponNumber];

			if (gunnerID->empty() || !GeneralUtils::IsValidString(gunnerID->c_str())){
				sprintf_s(gunnerID->data(), 0x20, "%d", pTechno->CurrentWeaponNumber + 1);
			}

			if (std::ranges::none_of(TacticalExtData::IFVGroups, [gunnerID](const char* pID) { return IS_SAME_STR_(pID, gunnerID->c_str()); }))
				TacticalExtData::IFVGroups.emplace_back(gunnerID->c_str());
		}
	}

	return 0;
}
