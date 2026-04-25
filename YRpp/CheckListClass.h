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
	CheckListClass(unsigned int nID, int nX, int nY, int nWidth, int nHeight, TextPrintType eFlag, SHPStruct* UpSHP, SHPStruct* DownSHP) noexcept
		: CheckListClass(noinit_t()) { JMP_THIS(0x488280); }

	CheckListClass(unsigned int nID, RectangleStruct InRect, TextPrintType eFlag, SHPStruct* UpSHP, SHPStruct* DownSHP) noexcept
		: CheckListClass(nID, InRect.X, InRect.Y, InRect.Width, InRect.Height, eFalg, UpSHP, DownSHP) { }
protected:
	explicit __forceinline CheckListClass(noinit_t)  noexcept
		: ListClass(noinit_t())
	{
	}

	//Properties
public:

	bool IsReadOnly;
};