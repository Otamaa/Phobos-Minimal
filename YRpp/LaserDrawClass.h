/*
	Lasers
*/

#pragma once

#include <GeneralDefinitions.h>
#include <ProgressTimer.h>
#include <Helpers/CompileTime.h>
#include <ArrayClasses.h>
#include <CoordStruct.h>
#include <ColorStruct.h>

class LaserDrawClass
{
public:
	static COMPILETIMEEVAL constant_ptr<DynamicVectorClass<LaserDrawClass*>, 0xABC878u> const Array{};
	static COMPILETIMEEVAL reference2D<Point2D, 0xABC7F8u, 8, 2> const DrawDatas{};

	static void __fastcall DrawAll() {
		JMP_STD(0x550240);
	}

	//Constructor, Destructor
	LaserDrawClass(const CoordStruct& source, const CoordStruct& target, const ColorStruct& innerColor,
		const ColorStruct& outerColor, const ColorStruct& outerSpread, int duration)
			: LaserDrawClass(source, target, 0, 1, innerColor, outerColor, outerSpread, duration)
	{ }

	LaserDrawClass
	(CoordStruct source, CoordStruct target,
		int zAdjust = 0,
		BYTE unknown = 1u,
		ColorStruct innerColor = ColorStruct::Empty,
		ColorStruct outerColor = ColorStruct::Empty,
		ColorStruct outerSpread = ColorStruct::Empty,
		int duration = 0,
		bool blinks = false,
		bool fades = true,
		float startIntensity = 1.0f,
		float endIntensity = 0.0f
	)
		{ JMP_THIS(0x54FE60); }

	~LaserDrawClass()
		{ JMP_THIS(0x54FFB0); }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================
public:
	DECLARE_PROPERTY(ProgressTimer, Progress);
	int Thickness; // only respected if IsHouseColor
	bool IsHouseColor;
	bool IsSupported; // this changes the values for InnerColor (false: halve, true: double), HouseColor only
	PROTECTED_PROPERTY(BYTE, align_22[2]);
	CoordStruct Source;
	CoordStruct Target;
	int ZAdjust;
	char field_40;
	ColorStruct InnerColor;
	ColorStruct OuterColor;
	ColorStruct OuterSpread;
	PROTECTED_PROPERTY(BYTE, align_4A[2]);
	int Duration;
	bool Blinks;
	bool BlinkState;
	bool Fades;
	PROTECTED_PROPERTY(BYTE, align_53);
	float StartIntensity;
	float EndIntensity;
};

static_assert(sizeof(LaserDrawClass) == 0x5C);