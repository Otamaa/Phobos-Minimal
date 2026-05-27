#include "Body.h"

#include <Ext/House/Body.h>
#include <Ext/Building/Body.h>

#include <Utilities/Cast.h>

BuildingClass* __fastcall FakeObjectTypeClass::WhoCanBuildMe(ObjectTypeClass* pThis, discard_t, bool intheory, bool bool2, bool legal, HouseClass* house)
{
	if(auto pTechno = type_cast<TechnoTypeClass*>(pThis)) {
		const auto nBuffer = HouseExtData::HasFactory(
			house,
			pTechno,
			intheory,
			bool2,
			legal,
		false);

		return nBuffer.first >= NewFactoryState::Available_Alternative ?
			nBuffer.second : nullptr;
	}

	return nullptr;
}