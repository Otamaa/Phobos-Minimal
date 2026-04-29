#include "Body.h"

#include <Ext/Aircraft/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>

#include <Utilities/Macro.h>
#include <Utilities/Patch.h>

int FakeTechnoClass::_GetSight(TechnoClass* pThis) {
	return  pThis->GetTechnoType()->Sight;
}

void __fastcall FakeTechnoClass::__See(
	TechnoClass* pThis, discard_t,
	bool         incremental,
	int          a6,
	bool         useCustomHouse,
	HouseClass* customHouse,
	int          sightRangeOverride)
{
	if (!pThis->IsOnMap || !pThis->Owner || pThis->Owner->Type->MultiplayPassive)
		return;

	const int sight = _GetSight(pThis);
	if (sight == 0 && sightRangeOverride == 0) 
		return;

	CoordStruct& coord = pThis->Location;

	int newIncrease = 10 * (coord.Z / RulesClass::Instance->LeptonsPerSightIncrease);
	if (newIncrease > pThis->SightIncrease) 
		incremental = false;

	pThis->SightIncrease = static_cast<signed char>(newIncrease);

	int sightRange = static_cast<int>(
		sight * (pThis->SightIncrease * 0.01 + 1.0));

	if (pThis->HasAbility(AbilityType::Sight) && RulesClass::Instance->VeteranSight != 0.0) {
		sightRange = static_cast<int>(sightRange * RulesClass::Instance->VeteranSight);
	}

	if (sightRangeOverride != 0)
		sightRange = sightRangeOverride;

	const bool useCustom = useCustomHouse && customHouse != nullptr;

	if (!pThis->SightInitialized)
	{
		pThis->SightInitialized = true;
		pThis->LastSightCoords = coord;
		pThis->LastSightRange = sightRange;

		if (sightRange == 0)
			return; 

		HouseClass* pRevealFor = useCustom ? customHouse
			: pThis->Owner;

		MapClass::Instance->RevealArea2(&coord, sightRange, pRevealFor,
					   incremental, a6,
					   /*a7*/ false, /*a8*/ true, /*a9*/ false);
		return;
	}

	if (useCustom && sightRange != 0)
	{
		MapClass::Instance->RevealArea2(&coord, sightRange, customHouse,
					   incremental, a6,
					   /*a7*/ false, /*a8*/ true, /*a9*/ false);
	}
}

DEFINE_FUNCTION_JUMP(LJMP, 0x70AF50, FakeTechnoClass::__See);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E272C, FakeTechnoClass::__See); //Aircraft
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E4344, FakeTechnoClass::__See); //Building
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E911C, FakeTechnoClass::__See); //Foot
DEFINE_FUNCTION_JUMP(VTABLE, 0x7EB4E0, FakeTechnoClass::__See); //Infantry
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F4DE8, FakeTechnoClass::__See); //Techno
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F60F8, FakeTechnoClass::__See); //Unit

void FakeAircraftClass::__Look(bool incremental, int arg_4)
{
	if (!this->Owner)
		return;

	int sightRange = FakeTechnoClass::_GetSight(this);
	const int height = this->GetHeight();

	if (height == 0)
	{
		sightRange = 1;
		CoordStruct& coord = this->Location;
		HouseClass* pOwner = this->Owner;

		MapClass::Instance->RevealArea2(&coord, sightRange, pOwner,
					   incremental, arg_4, /*a7*/false, /*a8*/true, /*a9*/false);

		MapClass::Instance->RevealArea2(&coord, sightRange, pOwner,
					   incremental, arg_4, /*a7*/false, /*a8*/true, /*a9*/true);
	}
	else if (sightRange == 0 && (ScenarioClass::Instance->SpecialFlags.RawFlags & 0x1000) != 0)
	{

		const int fogRange = RulesClass::Instance->AircraftFogReveal;    
		const int halfFlight = RulesClass::Instance->FlightLevel / 2;    
		CoordStruct& coord = this->Location;

		const bool low1 = (this->GetHeight() < halfFlight);
		MapClass::Instance->RevealArea2(&coord, fogRange, this->Owner,
					   /*incremental*/false, /*a6*/0,
					   /*a7*/true, /*a8*/low1, /*a9*/false);

		const bool low2 = (this->GetHeight() < halfFlight);
		MapClass::Instance->RevealArea2(&coord, fogRange, this->Owner,
					   /*incremental*/false, /*a6*/0,
					   /*a7*/true, /*a8*/low2, /*a9*/true);
	}
}

void __fastcall FakeTechnoClass::__Look(TechnoClass* pThis, discard_t, bool incremental, int arg_4)
{
	if (!pThis->IsOnMap || !pThis->Owner)
		return;

	if (pThis->Owner->Type->MultiplayPassive
		&& SessionClass::Instance->GameMode != GameMode::Campaign)
	{
		return;
	}

	CoordStruct& coord = pThis->Location;

	int newIncrease = 10 * (coord.Z / RulesClass::Instance->LeptonsPerSightIncrease);
	if (newIncrease > pThis->SightIncrease)
		incremental = false;
	pThis->SightIncrease = static_cast<signed char>(newIncrease);

	int sightRange = static_cast<int>(
		FakeTechnoClass::_GetSight(pThis) * (pThis->SightIncrease * 0.01 + 1.0));

	if (pThis->HasAbility(AbilityType::Sight) && RulesClass::Instance->VeteranSight != 0.0) {
		sightRange = static_cast<int>(sightRange * RulesClass::Instance->VeteranSight);
	}

	if (sightRange == 0)
		return;

	MapClass::Instance->RevealArea1(&coord, sightRange, pThis->Owner,
					incremental, arg_4,
					/*a7*/false, /*a8*/true, /*a9*/true);
}

DEFINE_FUNCTION_JUMP(LJMP, 0x41ADF0, FakeAircraftClass::__Look)//Aircraft
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E23C4, FakeAircraftClass::__Look)//Aircraft
DEFINE_FUNCTION_JUMP(LJMP, 0x70ADC0, FakeTechnoClass::__Look);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E3FDC, FakeTechnoClass::__Look); //Building
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E8DB4, FakeTechnoClass::__Look); //Foot
DEFINE_FUNCTION_JUMP(VTABLE, 0x7EB178, FakeTechnoClass::__Look); //Infantry
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F4A80, FakeTechnoClass::__Look); //Techno
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5D90, FakeTechnoClass::__Look); //Unit

ASMJIT_PATCH(0x446816, BuildingClass_Place_RevealToAll_UpdateSight, 0x5)
{
	enum { SkipGameCode = 0x44682F };

	GET(FakeBuildingClass*, pThis, EBP);
	const int radius = pThis->_GetTypeExtData()->RevealToAll_Radius.Get(FakeTechnoClass::_GetSight(pThis));
	pThis->UpdateSight(false, false, true, HouseClass::CurrentPlayer, radius);
	return SkipGameCode;
}

ASMJIT_PATCH_AGAIN(0x440819, BuildingClass_Unlimbo_DynamicSight, 0x6);
ASMJIT_PATCH(0x440842, BuildingClass_Unlimbo_DynamicSight, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	R->EDX(FakeTechnoClass::_GetSight(pThis));
	return R->Origin() + 0x6;
}

ASMJIT_PATCH(0x51E0E5, InfantryClass_Unlimbo_DynamicSight, 0x6)
{
	GET(TechnoClass*, pThis, EDI);
	R->EAX(FakeTechnoClass::_GetSight(pThis));
	return R->Origin() + 0x6;
}