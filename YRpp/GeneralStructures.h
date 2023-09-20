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
struct SWRange {
	SWRange() : WidthOrRange(-1.0f), Height(-1) { }
	SWRange(float widthOrRange, int height) : WidthOrRange(widthOrRange), Height(height) {}
	SWRange(int widthOrRange, int height) : WidthOrRange((float)widthOrRange), Height(height) {}
	SWRange(int widthOrRange) : WidthOrRange((float)widthOrRange), Height(-1) {}

	~SWRange() = default;

	SWRange(const SWRange& other) = default;
	SWRange& operator=(const SWRange& other) = default;

	float range() const {
		return this->WidthOrRange;
	}

	int width() const {
		return static_cast<int>(this->WidthOrRange);
	}

	int height() const {
		return this->Height;
	}

	bool empty() const {
		return this->WidthOrRange < 0.0
			&& this->Height < 0;
	}

public:

	float WidthOrRange;
	int Height;
};


// the velocities along the axes, or something like that
class VelocityClass final : public Vector3D<double>
{
public:
	//scalar multiplication
	VelocityClass operator*(double r) const
	{
		return {
			static_cast<double>(X * r),
			static_cast<double>(Y * r),
			static_cast<double>(Z * r) };
	}

	VelocityClass operator-(const VelocityClass& a) const {
		return { X - a.X, Y - a.Y, Z - a.Z };
	}

	DirStruct* GetDirectionFromXY(DirStruct* pRetDir)
	{ JMP_THIS(0x41C2E0); }

	void SetIfZeroXY()
	{ JMP_THIS(0x41C460); }

	double DistanceXY()
	{ return MagnitudeXY(); }

	double Distance()
	{ return Magnitude(); }

	void Func_5B2A30(Fixed* pFixed)
	{ JMP_THIS(0x5B2A30);}
};

struct BasePlanningCell {
	int Weight;
	CellStruct Position;
};

// this crap is used in several Base planning routines
struct BasePlanningCellContainer {
	BasePlanningCell * Items;
	int Count;
	int Capacity;
	bool Sorted;
	DWORD Unknown_10;

	bool AddCapacity(int AdditionalCapacity)
		{ JMP_THIS(0x510860); }

	// for qsort
	static int __cdecl Comparator(const void *, const void *)
		{ JMP_STD(0x5108F0); }
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

// latest c++ langue already provide this
// this one is just a wrapper that i purposfully made so i can add more functionality later if needed
template<typename T , size_t count>
class ArrayWrapper
{
	T Data[count];
public:
	constexpr auto begin() const { return  std::begin(Data); }
	constexpr auto end() const { return  std::end(Data); }
	constexpr auto begin() { return std::begin(Data); }
	constexpr auto end() { return std::end(Data); }

	constexpr size_t size() const { return count; }

	T at(int Index) const { return Data[Index]; }
	T& operator[](int nIdx) { return Data[nIdx]; }
	T operator[](int nIdx) const { return Data[nIdx]; }
};