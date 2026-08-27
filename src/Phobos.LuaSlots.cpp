#include "Phobos.LuaSlots.h"

#include <Utilities/SavegameDef.h>
#include <CellStruct.h>

bool PhobosLuaSlot::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	bool result = Stm.Process(this->Type);

	if (result) {
		switch (this->Type)
		{
		case PhobosLuaSlotType::Float :
			result &= Stm.Process(this->AsFloat);
		case PhobosLuaSlotType::Bool :
			result &= Stm.Process(this->AsBool);
		case PhobosLuaSlotType::Cell:
			result &= Stm.Process((*(CellStruct*)(&this->AsCell)));
		case PhobosLuaSlotType::ObjectId:
			result &= Stm.Process(this->AsObjectId);
		default: //Init and others
			result &= Stm.Process(this->AsInt);
			break;
		}
	}

	return result;
}

bool PhobosLuaSlot::Save(PhobosStreamWriter& Stm) const
{
	bool result = Stm.Process(this->Type);

	if (result)
	{
		switch (this->Type)
		{
		case PhobosLuaSlotType::Float:
			result &= Stm.Process(this->AsFloat);
		case PhobosLuaSlotType::Bool:
			result &= Stm.Process(this->AsBool);
		case PhobosLuaSlotType::Cell:
			result &= Stm.Process((*(CellStruct*)(&this->AsCell)));
		case PhobosLuaSlotType::ObjectId:
			result &= Stm.Process(this->AsObjectId);
		default: //Init and others
			result &= Stm.Process(this->AsInt);
			break;
		}
	}

	return result;
}

bool PhobosLuaSlots::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Stm
		.Process(this->Slots);
}

bool PhobosLuaSlots::Save(PhobosStreamWriter& Stm) const
{
	return Stm
		.Process(this->Slots);
}