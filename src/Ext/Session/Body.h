#pragma once

#include <SessionClass.h>

class NOVTABLE FakeSessionClass : public SessionClass
{
public:

	int  _Game_GetLinkedColor(PlayerColorSlot idx);
	void _Read_Scenario_Descriptions();
};