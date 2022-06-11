#pragma once

struct RGBAColor
{
	RGBAColor(float r, float g, float b) :
		R(r), G(g), B(b), A(1.0f)
	{
	}

	RGBAColor(float r, float g, float b, float a) :
		R(r), G(g), B(b), A(a)
	{
	}

	RGBAColor operator-()
	{
		return RGBAColor(-R, -G, -B, -A);
	}

	RGBAColor operator+(RGBAColor& b)
	{
		return RGBAColor(
			 R + b.R,
			 G + b.G,
			 B + b.B,
			 A + b.A);
	}

	RGBAColor operator-(RGBAColor& b)
	{
		return RGBAColor(
			 R - b.R,
			 G - b.G,
			 B - b.B,
			 A - b.A);
	}

	RGBAColor operator*(double r)
	{
		return RGBAColor(
			 (float)(R * r),
			 (float)(G * r),
			 (float)(B * r),
			 (float)(A * r));
	}
	RGBAColor operator/(double r)
	{
		return RGBAColor(
			 (float)(R / r),
			 (float)(G / r),
			 (float)(B / r),
			 (float)(A / r));
	}

	double operator*(RGBAColor& b)
	{
		return R * b.R
			+ G * b.G
			+ B * b.B
			+ A * b.A;
	}

	bool operator==(RGBAColor& b)
	{
		return R == b.R && G == b.G && B == b.B && A == b.A;
	}

	bool operator!=(RGBAColor& b)
	{
		return !((*this) == b);
	}

	float R, G, B, A;
};
