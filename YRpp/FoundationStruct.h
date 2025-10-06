#pragma once
#include <CellStruct.h>

struct FoundationStruct
{
	CellStruct Datas[30u];
};
static_assert(sizeof(FoundationStruct) == 0x78);
