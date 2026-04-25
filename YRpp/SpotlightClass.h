#pragma once

#include <ArrayClasses.h>
#include <GeneralStructures.h>
#include <Helpers/CompileTime.h>
#include <CoordStruct.h>

class NOVTABLE SpotlightClass
{
public:
	//Static
	static COMPILETIMEEVAL constant_ptr<DynamicVectorClass<SpotlightClass*>, 0xAC1678u> const Array{};

	//Destructor
	~SpotlightClass()
		{ THISCALL(0x5FF2D0); }

	void Draw()
		{ JMP_THIS(0x5FF850); }

	void Update()
		{ JMP_THIS(0x5FF320); }

	static void __fastcall Draw_All()
		{ JMP_FAST(0x5FFFA0); }

	//Constructor
	SpotlightClass(CoordStruct coords, int size)
	: SpotlightClass(coords.X , coords.Y , coords.Z , size)
		{ }

	SpotlightClass(int X , int Y , int Z , int size)
	{ JMP_THIS(0x5FF250); }
	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	CoordStruct Coords;
	int MovementRadius;
	int Size;
	SpotlightFlags DisableFlags;
};
