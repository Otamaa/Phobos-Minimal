#pragma once

#include <ArrayClasses.h>
#include <GeneralStructures.h>
#include <Helpers/CompileTime.h>
#include <CoordStruct.h>

class NOVTABLE SpotlightClass
{
public:
	//Static
	static constexpr constant_ptr<DynamicVectorClass<SpotlightClass*>, 0xAC1678u> const Array{};

	void Draw()
		; // { JMP_THIS(0x5FF850); }

	void Update()
		; // { JMP_THIS(0x5FF320); }

	~SpotlightClass();
	SpotlightClass(CoordStruct coords, int size);

public:

	CoordStruct Coords;
	int MovementRadius;
	int Size;
	SpotlightFlags DisableFlags;
};
