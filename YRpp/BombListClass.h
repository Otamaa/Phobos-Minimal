#pragma once

#include <AbstractClass.h>
#include <ArrayClasses.h>
#include <BombClass.h>
#include <Helpers/CompileTime.h>

//forward declarations
class ObjectClass;
class TechnoClass;
class CStreamClass;
//this class contains a vector of BombClass, a vector of bomb-revealing TechnoClass, and some other properties
class BombListClass
{
public:
	static constexpr reference<BombListClass, 0x87F5D8u> const Instance{};

	// draws all the visible bombs, expires the outdated ones
 	void Update()
		{ JMP_THIS(0x438BF0); }

	// the main one, ivan planting a bomb (creates a BombClass inside)
	void Plant(TechnoClass *SourceObject, ObjectClass *TargetObject)
		{ JMP_THIS(0x438E70); }

	// duh
	void AddDetector(TechnoClass *Detector)
		{ JMP_THIS(0x439080); }

	void AddBomb(BombClass* pBomb)
		{ JMP_THIS(0x439030); }

	// duh
	void RemoveDetector(TechnoClass *Detector)
		{ JMP_THIS(0x4390D0); }

	void PointerGotInvalid(AbstractClass* pInvalid)
		{ JMP_THIS(0x439150); }

	void Clear()
		{ JMP_THIS(0x439110); }

	void Detach(TechnoClass* Owner)
		{ JMP_THIS(0x439150); }

	//CStreamClass *stream
	HRESULT Save(CStreamClass* stream)
		{ JMP_THIS(0x4391C0); }

	HRESULT Load(CStreamClass* stream)
	    { JMP_THIS(0x439260); }

protected:
	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	DynamicVectorClass<BombClass *> Bombs;				//all the BombClass instances on the map
	DynamicVectorClass<TechnoClass *> Detectors;		//all the BombSight'ed objects currently on the map
	int UpdateDelay; // defaults to 100, some iterators set it to 1
};

static_assert(sizeof(BombListClass) == 0x34, "Invalid size.");