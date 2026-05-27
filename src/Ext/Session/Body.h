#pragma once

#include <SessionClass.h>

class NOVTABLE FakeSessionClass : public SessionClass
{
public:

	int  _Game_GetLinkedColor(PlayerColorSlot idx);
};