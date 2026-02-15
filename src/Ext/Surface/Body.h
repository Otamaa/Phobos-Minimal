#pragma once

#include <Surface.h>

class NOVTABLE DSurfaceExt : public DSurface
{
public:
	void BlurRect(const RectangleStruct& rect, float blurSize);

};

static_assert(sizeof(DSurfaceExt) == sizeof(DSurface), "Size Missmatch!");
