#pragma once

#include <Helpers/CompileTime.h>

#include <GeneralStructures.h>
#include <ArrayClasses.h>

class TechnoClass;
class CellClass;
// Tracks aerial units via 20x20 vectors spread across the maps for efficient search
class AircraftTrackerClass
{
public:
	static constexpr reference<AircraftTrackerClass, 0x887888u> Instance { };

	TechnoClass* Get() { JMP_THIS(0x4137A0); }

	void Add(TechnoClass* entry) { JMP_THIS(0x4134A0); }
	void Update(TechnoClass* entry, CellStruct oldPos, CellStruct newPos) { JMP_THIS(0x4138C0); }
	void Remove(TechnoClass* entry) { JMP_THIS(0x4135D0); }

	bool Clear() { JMP_THIS(0x413800); }

	bool IsJumpjet(TechnoClass* entry) { JMP_THIS(0x4135A0); }
	int GetVectorIndex(CellStruct pos) { JMP_THIS(0x412AC0); }
	int AircraftTrackerClass_413590() { JMP_THIS(0x413590); }
	int AircraftTrackerClass_412A40(int a1, int a2, int a3) { JMP_THIS(0x412A40); }
	int AircraftTrackerClass_4129C0(int a2) { JMP_THIS(0x4129C0); }
	void Detach(TechnoClass* entry) { JMP_THIS(0x413490); }
	void AircraftTrackerClass_logics_412B40(CellClass* cell, int range) { JMP_THIS(0x412B40); }

	//HRESULT LoadFromStream(IStream* pStm) { JMP_THIS(0x4136C0); }
	//HRESULT SaveFromStream(IStream* pStm) { JMP_THIS(0x413850); }
	// TODO write other entries

private:
	AircraftTrackerClass() { }

public:
	DynamicVectorClass<TechnoClass*> TrackerVectors[20][20];
	DynamicVectorClass<TechnoClass*> CurrentVector;
};

static_assert(sizeof(AircraftTrackerClass) == 0x2598);