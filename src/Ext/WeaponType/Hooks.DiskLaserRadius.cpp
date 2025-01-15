#include "Body.h"

#include <Helpers/Macro.h>
#include <DiskLaserClass.h>

// Angles = [ Pi/180*int((i*360/16+270)%360) for i in range(0,16)]
static COMPILETIMEEVAL double CosLUT[DiskLaserClass::DrawCoords.c_size()]
{
	0, 0.37460659341591196, 0.7071067811865474, 0.9205048534524403,
	1, 0.9271838545667874, 0.7071067811865476, 0.3907311284892737,
	0, -0.37460659341591207, -0.7071067811865475, -0.9205048534524404,
	-1, -0.9271838545667874, -0.7071067811865477, -0.3907311284892738
};

static COMPILETIMEEVAL double SinLUT[DiskLaserClass::DrawCoords.c_size()]
{
	-1, -0.9271838545667874, -0.7071067811865477, -0.3907311284892739,
	0, 0.374606593415912, 0.7071067811865476, 0.9205048534524404,
	1, 0.9271838545667874, 0.7071067811865476, 0.39073112848927377,
	0, -0.374606593415912, -0.7071067811865475, -0.9205048534524403
};

DEFINE_HOOK(0x4A757B, DiskLaserClass_AI_Circle, 0x6)
{
	GET(FakeWeaponTypeClass*, pWeapon, EDX);

	if (WeaponTypeExtData::nOldCircumference != pWeapon->_GetExtData()->DiskLaser_Circumference) {

		const int new_Circumference = pWeapon->_GetExtData()->DiskLaser_Circumference;
		WeaponTypeExtData::nOldCircumference = new_Circumference;

		for (size_t i = 0u; i < DiskLaserClass::DrawCoords.c_size(); i++)
		{
			DiskLaserClass::DrawCoords[i].X = (int)(new_Circumference * CosLUT[i]);
			DiskLaserClass::DrawCoords[i].Y = (int)(new_Circumference * SinLUT[i]);
		}
	}

	return 0;
}
