#pragma once

#include <GeneralDefinitions.h>
#include <Helpers/CompileTime.h>
#include <CoordStruct.h>

class TechnoClass;

class RadBeam
{
public:
	static COMPILETIMEEVAL constant_ptr<DynamicVectorClass<RadBeam*>, 0xB04A60u> const Array{};

	// Constructor removed - do not use it, use Allocate instead

	~RadBeam() = default;

	static RadBeam* __fastcall Allocate(RadBeamType mode)
		{ JMP_STD(0x659110); }

	static RadBeam* ManualAllocate(RadBeamType mode)
	{
		auto pBeam = GameCreate<RadBeam>(mode);
		RadBeam::Array()->AddItem(pBeam);
		pBeam->unknown_18 = (mode != RadBeamType::RadBeam) ? 20.0 : 10.0;
		return pBeam;
	}

	void SetColor(const ColorStruct &color)
		{ this->Color = color; }

	void SetCoordsSource(const CoordStruct &loc)
		{ this->SourceLocation = loc; }

	void SetCoordsTarget(const CoordStruct &loc)
		{ this->TargetLocation = loc; }

	void SetHeight(int nHeight) const
		{ JMP_THIS(0x659490); }

	void Func(int nA , int nB) const
		{ JMP_THIS(0x659510); }

	RadBeam(RadBeamType mode)
		{ JMP_THIS(0x6593F0); }
	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	DWORD unknown_0;
	TechnoClass* Owner;
	DWORD unknown_8;

	/**
	 * if there's difference in the Y coord of SourceLocation and TargetLocation,
	 * they're both converted to screen coords (2D)
	 * and the difference of those Y coords is taken as this field
	 */
	DWORD unknown_C;


	RadBeamType Type;
	DWORD unknown_14;
	double unknown_18;
	ColorStruct Color;
	BYTE gap23;
	CoordStruct SourceLocation; //FLH
	CoordStruct TargetLocation;
	DWORD Period;
	double Amplitude;
	DWORD unknown_48;
	DWORD unknown_4C;
	DWORD unknown_50;
	DWORD unknown_54;
	BYTE unknown_59;
	BYTE unknown_5A[3];
	CoordStruct coord5c;

	double unknown_68;
	CoordStruct AnotherLocation;
	DWORD unknown_7C;
	double unknown_80;
	int unknown_88;
	int unknown_8C;
	CoordStruct AndAnotherLocation;
	CoordStruct coord9C;
	CoordStruct coordA8;
	BYTE unknown_B4[4];
	double unknown_B8;
	ColorStruct RgbC0;
};

static_assert(sizeof(RadBeam) == 0xC8, "Invalid Sizes!");
