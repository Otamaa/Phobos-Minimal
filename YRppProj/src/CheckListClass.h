#pragma once

#include <ListClass.h>

class NOVTABLE CheckListClass : public ListClass
{
public:

	//Destructor
	virtual ~CheckListClass() RX;

	//GadgetClass

	//ControlClass

	//ListClass

	//CheckListClass

	//Non virtual

	//Statics

	//Constructors
	CheckListClass(unsigned int nID,
		int nX, int nY, int nWidth, int nHeight,
		TextPrintType eFlag,
		SHPStruct* UpSHP,
		SHPStruct* DownSHP);

	//Properties
public:

	bool IsReadOnly;
};