#pragma once

#include <GadgetClass.h>
#include <vector>

class CommandClass;
class PhobosCommandButtonClass : GadgetClass
{
	int ID { 0 };
	bool Hovering { false };
	CommandClass* Action {};
};

class TabClassToggleClass : public GadgetClass
{
	int ID { 0 };
	bool Hovering { false };
};

class PhobosTabClass : public GadgetClass
{
	static PhobosTabClass Instance;

	bool IsOpen { false };
	TabClassToggleClass* ToggleButtonOpen;
	TabClassToggleClass* ToggleButtonClose;
	std::vector<PhobosCommandButtonClass*> Buttons {};
};