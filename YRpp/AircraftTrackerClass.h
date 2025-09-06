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
	static COMPILETIMEEVAL reference<AircraftTrackerClass, 0x887888u> Instance { };

	FootClass* Get() { JMP_THIS(0x4137A0); }

	void Add(FootClass* entry) { JMP_THIS(0x4134A0); }
	void Update(FootClass* entry, CellStruct oldPos, CellStruct newPos) { JMP_THIS(0x4138C0); }
	void Remove(FootClass* entry) { JMP_THIS(0x4135D0); }

	bool Clear() { JMP_THIS(0x413800); }

	bool IsJumpjet(FootClass* entry) { JMP_THIS(0x4135A0); }
	int GetVectorIndex(CellStruct pos) { JMP_THIS(0x412AC0); }
	int AircraftTrackerClass_413590() { JMP_THIS(0x413590); }
	int AircraftTrackerClass_412A40(int a1, int a2, int a3) { JMP_THIS(0x412A40); }
	int AircraftTrackerClass_4129C0(int a2) { JMP_THIS(0x4129C0); }
	void Detach(FootClass* entry) { JMP_THIS(0x413490); }
	void AircraftTrackerClass_logics_412B40(CellClass* cell, int range) { JMP_THIS(0x412B40); }

	// Fills CurrentVector with items from TrackerVectors matching given range around cell.
	void FillCurrentVector(CellClass* pCell, int range) { JMP_THIS(0x412B40) }

	HRESULT Load(IStream* pStm) { JMP_THIS(0x4136C0); }
	HRESULT Save(IStream* pStm) { JMP_THIS(0x413850); }
	// TODO write other entries

private:
	AircraftTrackerClass() { }

public:
	DynamicVectorClass<TechnoClass*> TrackerVectors[20][20];
	DynamicVectorClass<TechnoClass*> CurrentVector;
};

struct NextAirTarget
{
	AircraftTrackerClass* Instance;
	FootClass* Current;

	NextAirTarget(AircraftTrackerClass* instance, CellClass* centre, int range)
	{
		this->Instance = instance;
		instance->AircraftTrackerClass_logics_412B40(centre, range);
		this->Current = this->Instance->Get();
	}

	NextAirTarget(CellClass* centre, int range)
	{
		NextAirTarget(&AircraftTrackerClass::Instance(), centre, range);
	}

	~NextAirTarget() = default;

	explicit operator bool() const {
		return Current != nullptr;
	}

	FootClass* operator ++()
	{
		Current = this->Instance->Get();
		return Current;
	}
};
static_assert(sizeof(AircraftTrackerClass) == 0x2598);