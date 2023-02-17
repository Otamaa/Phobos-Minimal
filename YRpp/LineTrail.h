#pragma once

#include <GeneralDefinitions.h>
#include <ObjectClass.h>

#include <Helpers/CompileTime.h>
#include <CoordStruct.h>

struct LineTrailNode
{
	CoordStruct Position;
	int Value;
};

class ObjectClass;
class LineTrail
{
public:
	static constexpr constant_ptr<DynamicVectorClass<LineTrail*>, 0xABCB78u> const Array{};

	//Constructor, Destructor
	LineTrail()
		{ JMP_THIS(0x556A20); }

	~LineTrail()
		{ JMP_THIS(0x556B30); }

	LineTrail(ObjectClass* pOwner, ColorStruct const& nColor, int nDec) : LineTrail{}
	{
		Owner = pOwner;
		Color = nColor;
		Decrement = nDec;
	}

	void Detach()
		{ JMP_THIS(0x556AD0); }

	void AI()
		{ JMP_THIS(0x556B70); }

	void Draw()
		{ JMP_THIS(0x556C00); }

	void DrawAll()
		{ JMP_THIS(0x556D40); }

	bool IsAlive()
		{ JMP_THIS(0x556D20); }

	void SetDecrement(int val)
		{ JMP_THIS(0x556B50); }

	static void DeleteAll()
		{ JMP_STD(0x556DF0); }


	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	DECLARE_PROPERTY(ColorStruct, Color);
	ObjectClass* Owner;
	int Decrement;
	int ActiveSlot;
	ArrayWrapper<LineTrailNode, 32u> Trails;
};