#pragma once
#include <YRMath.h>

void static_init_math_pow_sqrt_0076C0A0()
{
	long double v0; // st7

	v0 = pow(256.0, 2.0);
	dbl_B72A30 = FastMath::Sqrt(v0 + v0);
}

signed __int64 static_init_math_cell_height_leptons_0076C170()
{
	signed __int64 result; // rax

	result = (signed __int64)(FastMath::Tan(DEG90_AS_RAD_76C150 - DEG60_AS_RAD_76C130) * dbl_B72A30 * 0.5);
	CellHeightLeptons_76C170 = result;
	return result;
}

