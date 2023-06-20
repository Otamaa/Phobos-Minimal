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
#include <timeapi.h>

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

template<typename T>
concept TimerType = std::convertible_to<T, int>&& requires (T t)
{
	{ t() }->std::same_as<long>;
};

struct FrameTimer
{
	long operator()()const { return *reinterpret_cast<long*>(0xA8ED84); }
	operator long() const { return *reinterpret_cast<long*>(0xA8ED84); }
};

struct SystemTimer
{
	long operator()() const { return timeGetTime() >> 4; }
	operator long() const { return timeGetTime() >> 4; }
};

//used for timed events, time measured in frames!
template<TimerType Clock>
class TimerClass
{
public:
	int StartTime{ -1 };
	Clock CurrentTime {}; // timer
	int TimeLeft{ 0 };

	constexpr TimerClass() = default;
	TimerClass(int duration) { this->Start(duration); }
	~TimerClass() = default;

	TimerClass(const TimerClass& other) {
		this->StartTime = other.StartTime;
		this->TimeLeft = other.TimeLeft;
	}

	TimerClass& operator=(const TimerClass& other) = default;

	void Start(int duration) {
		this->StartTime = this->CurrentTime;
		this->TimeLeft = duration;
	}

	void Stop() {
		this->StartTime = -1;
		this->TimeLeft = 0;
	}

	void Pause() {
		if(this->IsTicking()) {
			this->TimeLeft = this->GetTimeLeft();
			this->StartTime = -1;
		}
	}

	void Resume() {
		if(!this->IsTicking()) {
			this->StartTime = this->CurrentTime;
		}
	}

	int GetTimeLeft() const {
		if(!this->IsTicking()) {
			return this->TimeLeft;
		}

		auto passed = this->CurrentTime - this->StartTime;
		auto left = this->TimeLeft - passed;

		return (left <= 0) ? 0 : left;
	}

	// returns whether a ticking timer has finished counting down.
	bool Completed() const {
		return this->IsTicking() && !this->HasTimeLeft();
	}

	// returns whether a delay is active or a timer is still counting down.
	// this is the 'opposite' of Completed() (meaning: incomplete / still busy)
	// and logically the same as !Expired() (meaning: blocked / delay in progress)
	bool InProgress() const {
		return this->IsTicking() && this->HasTimeLeft();
	}

	bool IsNotActive() const {
		return this->IsTicking() && !this->TimeLeft;
	}

	// returns whether a delay is inactive. same as !InProgress().
	bool Expired() const {
		return !this->IsTicking() || !this->HasTimeLeft();
	}

	// Sometimes I want to know if the timer has ever started
	bool HasStarted() const
	{
		return this->IsTicking() || this->HasTimeLeft();
	}

	void Add(int nTime) {
		this->Pause();
		this->TimeLeft += nTime;
		this->Resume();
	}

	bool IsTicking() const {
		return this->StartTime != -1;
	}

	bool HasTimeLeft() const {
		return this->GetTimeLeft() > 0;
	}
};

using CDTimerClass = TimerClass<FrameTimer>;
using SystemTimerClass = TimerClass<SystemTimer>;
static_assert(offsetof(CDTimerClass, TimeLeft) == 0x8);
static_assert(offsetof(SystemTimerClass, TimeLeft) == 0x8);
static_assert(sizeof(SystemTimerClass) == 0xC, "Invalid Size !");
static_assert(sizeof(CDTimerClass) == 0xC, "Invalid Size !");

class RepeatableTimerStruct : public CDTimerClass
{
public:
	int Duration{ 0 };

	constexpr RepeatableTimerStruct() = default;
	RepeatableTimerStruct(int duration) { this->Start(duration); }

	void Start(int duration) {
		this->Duration = duration;
		this->Restart();
	}

	void Restart() {
		this->CDTimerClass::Start(this->Duration);
	}
};

typedef RepeatableTimerStruct RateTimer;

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