#pragma once

#include <ControlClass.h>

class NOVTABLE ToggleClass : public ControlClass
{
public:

	//Destructor
	virtual ~ToggleClass() RX;

	//GadgetClass

	//ControlClass

	//ToggleClass

	//Non virtual
	void TurnOn();//{ JMP_THIS(0x723EA0); }
	void TurnOff();// { JMP_THIS(0x723EB0); }

	//Statics

	//Constructors
	ToggleClass(unsigned int nID, int nX, int nY, int nWidth, int nHeight);
	ToggleClass(unsigned int nID, RectangleStruct rect);

public:

	bool IsPressed;
	bool IsOn;
	DWORD ToggleType;
};