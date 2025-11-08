#pragma once

#include <ASMMacros.h>
#include <YRPPCore.h>
#include <CRT.h>
#include <YRMath.h>
#include <YRMathVector.h>
#include <GeneralDefinitions.h> // need eDirection for FacingClass
#include <BasicStructures.h>
#include <CellStruct.h>
#include <Point2D.h>
#include <Point2DByte.h>
#include <Point3D.h>
#include <DirStruct.h>
#include <string.h>
#include <Timers.h>

//used for cell coordinates/vectors
//using FloatVelocity = Vector3D<float>;
//typedef AbstractClass* TARGET;

class Fixed;

struct BasePlanningCell {
	int Weight;
	CellStruct Position;
};

// this crap is used in several Base planning routines

	// for qsort
OPTIONALINLINE NAKED int __cdecl BasePlanningCellContainer_Comparator(const void*, const void*)
	{ JMP(0x5108F0); }

struct BasePlanningCellContainer {
	BasePlanningCell * Items;
	int Count;
	int Capacity;
	bool Sorted;
	DWORD Unknown_10;

	bool AddCapacity(int AdditionalCapacity)
		{ JMP_THIS(0x510860); }
};

// combines number and a string
template<typename T>
struct NamedValue {
	const char* Name;
	T Value;

	bool operator== (T value) const {
		return this->Value == value;
	}

	bool operator == (const char* name) const {
		return !CRT::strcmpi(this->Name, name);
	}

	bool operator== (const NamedValue<T>& other) const {
		return this->Value == other.Value && *this == other.Name;
	}

	bool operator!= (const NamedValue<T>& other) const {
		return !(*this == other);
	}
};
