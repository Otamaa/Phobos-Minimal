#pragma once

#include <GeneralDefinitions.h>
#include <Helpers/CompileTime.h>
#include <CoordStruct.h>

class TechnoClass;

class RadBeam
{
public:
	static constexpr constant_ptr<DynamicVectorClass<RadBeam*>, 0xB04A60u> const Array{};

	// Constructor removed - do not use it, use Allocate instead

	~RadBeam() = default;

	static RadBeam* __fastcall Allocate(RadBeamType mode)
		{ JMP_STD(0x659110); }

	static RadBeam* ManualAllocate(RadBeamType mode)
	{
		if (auto pBeam = GameCreate<RadBeam>(mode))
		{
			RadBeam::Array()->AddItem(pBeam);
			pBeam->unknown_18 = (mode != RadBeamType::RadBeam) ? 20.0 : 10.0;
			return pBeam;
		}

		return nullptr;
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
	byte unknown_8;

	/**
	 * if there's difference in the Y coord of SourceLocation and TargetLocation,
	 * they're both converted to screen coords (2D)
	 * and the difference of those Y coords is taken as this field
	 */
	DWORD unknown_C;


	RadBeamType Type;
	DWORD unknown_14;
	double unknown_18;
	DECLARE_PROPERTY(ColorStruct, Color);
	DECLARE_PROPERTY(CoordStruct, SourceLocation); //FLH
	DECLARE_PROPERTY(CoordStruct, TargetLocation);
	DWORD Period;
	double Amplitude;
	double unknown_48;
	DWORD unknown_50;
	DWORD unknown_54;
	byte unknown_58;
	DECLARE_PROPERTY(CoordStruct, coord5c);

	double unknown_68;
	DECLARE_PROPERTY(CoordStruct, AnotherLocation);
	DWORD unknown_7C;
	double unknown_80;
	DWORD unknown_88;
	DWORD unknown_8C;
	DECLARE_PROPERTY(CoordStruct, AndAnotherLocation);
	DECLARE_PROPERTY(CoordStruct, coord9C);
	DECLARE_PROPERTY(CoordStruct, coordA8);
	DWORD unknown_B4;
	double unknown_B8;
	DECLARE_PROPERTY(ColorStruct, RgbC0);
};
