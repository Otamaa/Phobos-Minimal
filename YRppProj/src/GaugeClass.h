#pragma once

#include <ControlClass.h>

class NOVTABLE GaugeClass : public ControlClass
{
public:

	//Destructor
	virtual ~GaugeClass() RX;

	//GadgetClass
	virtual bool Draw(bool forced) override;//JMP_THIS(0x4E2690);
	virtual bool Action(GadgetFlag flags, DWORD* pKey, KeyModifier modifier) override;//JMP_THIS(0x4E2830);

	//ControlClass

	//GaugeClass
	virtual bool SetMaximum(int value);//JMP_THIS(0x4E2580);
	virtual bool SetValue(int value);//JMP_THIS(0x4E25A0);
	virtual int GetValue();//JMP_THIS(0x4E30A0);
	virtual void SetThumb(bool value);//JMP_THIS(0x4E30B0); // Set HasThumb
	virtual int GetThumbPixel();// JMP_THIS(0x4E30C0); // return 4 if not overloaded
	virtual void DrawThumb();// JMP_THIS(0x4E29A0);
	virtual int PixelToValue(int pixel);// JMP_THIS(0x4E25D0);
	virtual int ValueToPixel(int value);//JMP_THIS(0x4E2650);

	//Non virtual

	//Statics

	//Constructors
	GaugeClass(unsigned int nID, int nX, int nY, int nWidth, int nHeight);
	GaugeClass(unsigned int nID, RectangleStruct rect);

	//Properties
public:

	bool IsColorized;
	bool HasThumb;
	bool IsHorizontal;
	int MaxValue;
	int CurrentValue;
	int ClickDiff;
};