#pragma once

#include <Surface.h>

class SurfaceExt : public Surface
{
public:
	void BlurRect(const RectangleStruct& rect, float blurSize);

};

static_assert(sizeof(SurfaceExt) == sizeof(Surface), "Size Missmatch!");
