#include <Utilities/Patch.h>
#include <Utilities/Macro.h>

#include <Ext/Aircraft/Body.h>
#include <Ext/Unit/Body.h>
#include <Ext/Infantry/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/House/Body.h>

int FakeBuildingClass::__GetCrewCount()
{
	if(this->_GetExtData()->PreventCrew) {
		return 0;
	}

	int count = 0;

	if (!this->NoCrew && this->Type->Crewed) {
		auto pHouse = this->Owner;

		// get the divisor
		int divisor = HouseExtData::GetSurvivorDivisor(pHouse);

		if (divisor > 0) {
			// if captured, less survivors
			if (this->HasBeenCaptured) {
				divisor *= 2;
			}

			// value divided by "cost per survivor"
			// clamp between 1 and 5
			count = std::clamp(this->Type->GetRefund(pHouse, 0) / divisor, 1, 5);
		}
	}

	return count;
}

 int FakeAircraftClass::__GetCrewCount(){
	if(this->_GetExtData()->PreventCrew) {
		return 0;
	}

	return this->Type->Crewed != 0;
 }

 int FakeUnitClass::__GetCrewCount(){
	if(this->_GetExtData()->PreventCrew) {
		return 0;
	}

	return this->Type->Crewed != 0;
 }

int FakeInfantryClass::__GetCrewCount(){
	if(this->_GetExtData()->PreventCrew) {
		return 0;
	}

	return this->Type->Crewed != 0;
 }

DEFINE_FUNCTION_JUMP(VTABLE, 0x7E418C, FakeBuildingClass::__GetCrewCount)
DEFINE_FUNCTION_JUMP(LJMP, 0x451330, FakeBuildingClass::__GetCrewCount)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E2574, FakeAircraftClass::__GetCrewCount)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7EB328, FakeInfantryClass::__GetCrewCount)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5F40, FakeUnitClass::__GetCrewCount)
