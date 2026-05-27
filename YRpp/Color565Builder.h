#pragma once
#include <Surface.h>

class Color565Builder
{
public:
	HicolorLayout Layout;

	COMPILETIMEEVAL unsigned Build(unsigned r, unsigned g, unsigned b) const
	{
		switch (Layout)
		{
		case HicolorLayout::RGB:
			return DSurface::Build_Hicolor_Pixel_RGB(r, g, b);

		case HicolorLayout::RBG:
			return DSurface::Build_Hicolor_Pixel_RBG(r, g, b);

		case HicolorLayout::GRB:
			return DSurface::Build_Hicolor_Pixel_GRB(r, g, b);

		case HicolorLayout::GBR:
			return DSurface::Build_Hicolor_Pixel_GBR(r, g, b);

		case HicolorLayout::BRG:
			return DSurface::Build_Hicolor_Pixel_BRG(r, g, b);
		default:
			DSurface::Temp = nullptr; //:p
		}

		return 0;
	}
};