#pragma once

#include <AbstractClass.h>

// this refers to the "planning mode" waypoints you place with your mouse, not mapping waypoints
class WaypointClass
{
public:
	//need to define a == operator so it can be used in array classes
	bool operator == (const WaypointClass& tWaypoint) const
	{
		return (Coords.SimilarTo(tWaypoint.Coords) && unknown == tWaypoint.unknown);
	}

	//Properties
	CellStruct Coords;
	DWORD       unknown;
};

class DECLSPEC_UUID("F73125BA-1054-11D2-8172-006008055BB5")
	NOVTABLE WaypointPathClass : public AbstractClass
{
public:
	static const AbstractType AbsID = AbstractType::Waypoint;

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override R0;

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override R0;
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override R0;

	//Destructor
	virtual ~WaypointPathClass() RX;

	//AbstractClass
	virtual AbstractType WhatAmI() const override RT(AbstractType);
	virtual int Size() const override R0;

	WaypointClass* GetWaypoint(int idx) const
	{ JMP_THIS(0x763980); }
	WaypointClass* GetWaypointAfter(int idx) const
	{ JMP_THIS(0x763BA0); }
	void Clear() const
	{ JMP_THIS(0x763BE0); }
	void Find(int nIndex) const
	{ JMP_THIS(0x763B30); }
	WaypointClass* SetWaypoint(int nIdx, WaypointClass* pSetTo) const
	{ JMP_THIS(0x763B30); }
	bool SetWaypoint(CoordStruct& nWhere) const
	{ JMP_THIS(0x7639A0); }
	bool WaypointExistsAt(CoordStruct& nWhere) const
	{ JMP_THIS(0x763A50); }
	static int __fastcall StringToWaypointIdx(char* pChar)
	{ JMP_STD(0x763690); }
	static char* __fastcall WaypointIdxToString(int nIdx)
	{ JMP_STD(0x763610); }

	// Constructor
	WaypointPathClass(int idx)
		: WaypointPathClass(noinit_t())
	{ JMP_THIS(0x763810); }

protected:
	explicit __forceinline WaypointPathClass(noinit_t)
		: AbstractClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	int  CurrentWaypointIndex; //seems that way
	DynamicVectorClass<WaypointClass> Waypoints; // actual path waypoints, no *
};

static_assert(sizeof(WaypointPathClass) == 0x40, "Invalid size.");
