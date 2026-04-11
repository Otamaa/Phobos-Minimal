#include "PlacingBuildingStruct.h"

#include "SavegameDef.h"

template <typename T>
bool Serialize(PlacingBuildingStruct* pThis , T& Stm)
{
	return Stm
		.Process(pThis->Type)
		.Process(pThis->DrawType)
		.Process(pThis->Times)
		.Process(pThis->Timer)
		.Process(pThis->TopLeft)
		.Success()
		//&& Stm.RegisterChange(this)
		; // announce this type
}

bool PlacingBuildingStruct::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{ return Serialize(this, Stm); }

bool PlacingBuildingStruct::Save(PhobosStreamWriter& Stm) const
{ return Serialize(const_cast<PlacingBuildingStruct*>(this), Stm); }