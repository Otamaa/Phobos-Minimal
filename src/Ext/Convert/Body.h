#pragma once

#include <ConvertClass.h>

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