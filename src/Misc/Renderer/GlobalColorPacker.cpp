#include "GlobalColorPacker.h"

#include <Surface.h>
#include <Drawing.h>

void GlobalColorPacker::SetColorPacker()
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

#include <Utilities/Patch.h>
#include <Utilities/Macro.h>
#include <FileSystem.h>

#include <Utilities/Debug.h>

#ifdef _Debug
ASMJIT_PATCH(0x52BF1A, GameInit_AnimPal_Log, 0x5) {
	ConvertClass* animPal = FileSystem::ANIM_PAL();

	if (animPal->BufferMid)
	{
		uint16_t* lut = reinterpret_cast<uint16_t*>(animPal->BufferMid);

		// Check palette index 14 (usually yellow in RA2 palettes)
		uint16_t yellowPixel = lut[14];
		Debug::Log("[Palette] Index 14 (yellow) = 0x%04X\n", yellowPixel);

		// Check index 1 (usually white/light)
		uint16_t whitePixel = lut[1];
		Debug::Log("[Palette] Index 1 (white) = 0x%04X\n", whitePixel);
	}

	return 0x0;
}
#endif