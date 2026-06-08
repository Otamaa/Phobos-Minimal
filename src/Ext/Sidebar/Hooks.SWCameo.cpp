#include <Ext/SWType/Body.h>
#include <Utilities/Macro.h>

#pragma region SW TabIndex

int __fastcall WhichTab(AbstractType rtti, int idx, int unused)
{
	switch (rtti)
	{
	case AbstractType::Infantry:
	case AbstractType::InfantryType:
		return 2;
	case AbstractType::Unit:
	case AbstractType::UnitType:
	case AbstractType::Aircraft:
	case AbstractType::AircraftType:
		return 3;
	case AbstractType::Building:
	case AbstractType::BuildingType:
		return ObjectTypeClass::IsBuildCat5(rtti, idx) == BuildCat::Combat ? 1 : 0;
	case AbstractType::Special:
	case AbstractType::Super:
	case AbstractType::SuperWeaponType:
	{
		int tab = 1;

		if ((size_t)idx < (size_t)SuperWeaponTypeClass::Array->Count)
			tab = SWTypeExtContainer::Instance.Find(SuperWeaponTypeClass::Array->Items[idx])->TabIndex.Get();

		return tab;
	}
	default:
		return -1;
	}
}

ASMJIT_PATCH(0x6A5F6E, SidebarClass_6A5F20_TabIndex, 0x8)
{
	enum { ApplyTabIndex = 0x6A5FD3 };

	GET(AbstractType const, absType, ESI);
	GET(int const, typeIdx, EAX);

	R->EAX(WhichTab(absType, typeIdx, 0));
	return ApplyTabIndex;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x6ABC60, WhichTab)
 // Skip tabIndex check
#pragma endregion