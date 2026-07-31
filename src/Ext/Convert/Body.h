#pragma once

#include <ConvertClass.h>

class ConvertExtData
{
public:

	SHPReference* AttachedToObject;
	Blitter* Blitters[20];
	RLEBlitter* RLEBlitters[18];
};

class NOVTABLE ConvertClassExt : ConvertClass
{
public:
	void DeallocBlitters();
	void AllocBlitters();

private:
	void AllocBlitters8();
	void DeallocBlitters8();
	void AllocBlitters16();
	void DeallocBlitters16();
};