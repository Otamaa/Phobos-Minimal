#pragma once

#include <GaugeClass.h>
#include <RectangleStruct.h>

class NOVTABLE TriColorGaugeClass : public GaugeClass
{
public:

	//Destructor
	virtual ~TriColorGaugeClass() RX;

	//GadgetClass

	//ControlClass

	//GaugeClass

	//TriColorGaugeClass
	virtual bool SetRedLimit(int value) R0;
	virtual bool SetYellowLimit(int value) R0;

	//Non virtual

	//Statics

	//Constructors
	TriColorGaugeClass(unsigned int nID, int nX, int nY, int nWidth, int nHeight);
	TriColorGaugeClass(unsigned int nID, RectangleStruct rect);
public:

	int RedLimit;
	int YellowLimit;
};