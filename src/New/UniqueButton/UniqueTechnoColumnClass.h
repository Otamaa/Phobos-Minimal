#pragma once
#include "UniqueTechnoButtonClass.h"

class UniqueTechnoColumnClass
{
public:
	static UniqueTechnoColumnClass Instance;

	void InitClear();
	void InitIO();

	void SwitchVisible();

	void Update();

	UniqueTechnoButtonClass* Buttons[8] { };
	int Hovering { -1 };
	bool Visible { true };
};
