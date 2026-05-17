#include "ColorPacker.h"

#include <Surface.h>
#include <Drawing.h>

void ColorPacker::SetColorPacker()
{
	// RGB565 color shifts
	Drawing::RedShiftLeft = 11;
	Drawing::RedShiftRight = 3;
	Drawing::GreenShiftLeft = 5;
	Drawing::GreenShiftRight = 2;
	Drawing::BlueShiftLeft = 0;
	Drawing::BlueShiftRight = 3;

	DSurface::RGBMode = RGBMode::RGB565; // ColorMode RGB565
}
