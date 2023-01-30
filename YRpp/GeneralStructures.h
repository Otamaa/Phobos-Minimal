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

//used for cell coordinates/vectors
//using FloatVelocity = Vector3D<float>;
//typedef AbstractClass* TARGET;

class Fixed;
struct SWRange {
	SWRange(float widthOrRange = -1.0f, int height = -1) : WidthOrRange(widthOrRange), Height(height) {}
	SWRange(int widthOrRange, int height = -1) : WidthOrRange(static_cast<float>(widthOrRange)), Height(height) {}

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

	float WidthOrRange;
	int Height;
};

//used for timed events, time measured in frames!
class TimerStruct
{
public:
	int StartTime{ -1 };
	int : 32; // timer
	int TimeLeft{ 0 };

	constexpr TimerStruct() = default;
	TimerStruct(int duration) { this->Start(duration); }

	void Start(int duration) {
		this->StartTime = *reinterpret_cast<int*>(0xA8ED84);
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
			this->StartTime = *reinterpret_cast<int*>(0xA8ED84);
		}
	}

	int GetTimeLeft() const {
		if(!this->IsTicking()) {
			return this->TimeLeft;
		}

		auto passed = *reinterpret_cast<int*>(0xA8ED84) - this->StartTime;
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

	// returns whether a delay is inactive. same as !InProgress().
	bool Expired() const {
		return !this->IsTicking() || !this->HasTimeLeft();
	}

	void Add(int nTime) {
		this->Pause();
		this->TimeLeft = nTime;
		this->Resume();
	}

	bool IsTicking() const {
		return this->StartTime != -1;
	}

	bool HasTimeLeft() const {
		return this->GetTimeLeft() > 0;
	}
};

typedef TimerStruct CDTimerClass;
class RepeatableTimerStruct : public TimerStruct
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
		this->TimerStruct::Start(this->Duration);
	}
};

typedef RepeatableTimerStruct RateTimer;

// like a compass with 2 ^ 16 units equalling 360?
//struct DirStruct
//{
//	using value_type = short;
//	using unsigned_type = unsigned short;
//
//	DirStruct() : DirStruct(0) { }
//	explicit DirStruct(int value) : Value(static_cast<value_type>(value)) { }
//
//	explicit DirStruct(double rad) : DirStruct() {
//		this->radians(rad);
//	}
//
//	explicit DirStruct(double nVelZ, double nVelDistanceXY) : DirStruct() {
//		this->radians(Math::atan2(nVelZ, nVelDistanceXY));
//	}
//
//	DirStruct(size_t bits, value_type value)
//		: DirStruct(static_cast<value_type>(TranslateFixedPoint(bits, 16, static_cast<unsigned_type>(value), 0)))
//	{ }
//
//	bool operator == (const DirStruct& rhs) const {
//		return this->Value == rhs.Value;
//	}
//
//	bool operator != (const DirStruct& rhs) const {
//		return this->Value != rhs.Value;
//	}
//
//	DirStruct& operator /= (const value_type nFace) {
//		JMP_THIS(0x5B2970);
//	}
//
//	DirStruct& operator += (const DirStruct& rhs) {
//		reinterpret_cast<unsigned_type&>(this->Value) += static_cast<unsigned_type>(rhs.value());
//		return *this;
//	}
//
//	DirStruct operator + (const DirStruct& rhs) const {
//		return DirStruct(*this) += rhs;
//	}
//
//	DirStruct& operator -= (const DirStruct& rhs) {
//		reinterpret_cast<unsigned_type&>(this->Value) -= static_cast<unsigned_type>(rhs.value());
//		return *this;
//	}
//
//	DirStruct operator - (const DirStruct& rhs) const {
//		return DirStruct(*this) -= rhs;
//	}
//
//	DirStruct operator - () const {
//		return DirStruct(-this->Value);
//	}
//
//	DirStruct operator + () const {
//		return *this;
//	}
//
//	template <size_t Bits>
//	value_type value() const {
//		static_assert(Bits > 0 && Bits <= 16, "Bits has to be greater than 0 and lower or equal to 16.");
//
//		return static_cast<value_type>(TranslateFixedPoint(16, Bits, static_cast<unsigned_type>(this->Value), 0));
//	}
//
//	template <>
//	value_type value<16>() const {
//		return this->Value;
//	}
//
//	template <size_t Bits>
//	void value(value_type value) {
//		static_assert(Bits > 0 && Bits <= 16, "Bits has to be greater than 0 and lower or equal to 16.");
//
//		this->Value = static_cast<value_type>(TranslateFixedPoint(Bits, 16, static_cast<unsigned_type>(value), 0));
//	}
//
//	value_type value8() const {
//		return this->value<3>();
//	}
//
//	void value8(value_type value) {
//		this->value<3>(value);
//	}
//
//	value_type value32() const {
//		return this->value<5>();
//	}
//
//	void value32(value_type value) {
//		this->value<5>(value);
//	}
//
//	value_type value256() const {
//		return this->value<8>();
//	}
//
//	void value256(value_type value) {
//		this->value<8>(value);
//	}
//
//	value_type value() const {
//		return this->value<16>();
//	}
//
//	void value(value_type value) {
//		this->value<16>(value);
//	}
//
//	// The vanilla YR formula
//	double get_radian() const {
//		return static_cast<double>(value32() - 8) * -(Math::Pi / 16);
//	}
//
//	template <size_t Bits = 16>
//	double radians() const {
//		static_assert(Bits > 0 && Bits <= 16, "Bits has to be greater than 0 and lower or equal to 16.");
//
//		static const int Max = ((1 << Bits) - 1);
//
//		int value = Max / 4 - this->value<Bits>();
//		return -value * -(Math::TwoPi / Max);
//	}
//
//	template <size_t Bits = 16>
//	void radians(double rad) {
//		static_assert(Bits > 0 && Bits <= 16, "Bits has to be greater than 0 and lower or equal to 16.");
//
//		static const int Max = ((1 << Bits) - 1);
//
//		int value = static_cast<int>(rad * (Max / Math::TwoPi));
//		this->value<Bits>(static_cast<value_type>(Max / 4 - value));
//	}
//
//	value_type GetValue(size_t Bits = 16)
//	{
//		if (Bits > 0 && Bits <= 16)
//			return (value_type)(TranslateFixedPoint(16, Bits, (this->Value), 0));
//
//		return 0;
//	}
//
//	// pThis.Value >= (pDir2.Value - pDir3.Value)
//	bool CompareToTwoDir(DirStruct& pBaseDir, DirStruct& pDirFrom)
//		{ JMP_THIS(0x5B2990); }
//
//	bool Func_5B29C0(DirStruct& pDir2, DirStruct& pDir3)
//		{ JMP_THIS(0x5B29C0);}
//
//private:
//	value_type Value;
//	unsigned_type unused_2;
//};

// managing a facing that can change over time
//struct FacingClass
//{
//	FacingClass() : Timer(0) { }
//
//	FacingClass(short rot) : FacingClass() {
//		this->turn_rate(rot);
//	}
//
//	short turn_rate() const {
//		return this->ROT.value();
//	}
//
//	void turn_rate(short value) {
//		if(value > 127) {
//			value = 127;
//		}
//
//		this->ROT.value<8>(value);
//	}
//
//	bool IsRotating() const
//	{ JMP_THIS(0x4C9480); }
//
//	int turn_timeLeft()
//	{ return this->Timer.GetTimeLeft(); }
//
//	int turn_timerStartTime()
//	{ return this->Timer.StartTime; }
//
//	void stop_turn()
//	{ this->Timer.Stop(); }
//
//	bool in_motion() const {
//		return this->turn_rate() > 0 && this->Timer.GetTimeLeft();
//	}
//
//	DirStruct target() const {
//		return this->Value;
//	}
//
//	DirStruct current(bool flip = false, int offset = 0) const {
//		auto ret = this->Value;
//
//		if(this->in_motion()) {
//			auto diff = this->difference();
//			auto num_steps = static_cast<short>(this->num_steps());
//
//			if(num_steps > 0) {
//				auto steps_left = this->Timer.GetTimeLeft() - offset;
//				ret.value(static_cast<short>(ret.value() - steps_left * diff / num_steps));
//			}
//		}
//
//		return ret;
//	}
//
//	DirStruct next(bool flip = false)
//	{ return current(flip, 1); }
//
//	bool set(const DirStruct& value) {
//		bool ret = (this->current() != value);
//
//		if(ret) {
//			this->Value = value;
//			this->Initial = value;
//		}
//
//		this->Timer.Start(0);
//
//		return ret;
//	}
//
//	bool turn(const DirStruct& value, bool flip = false) {
//		if(this->Value == value) {
//			return false;
//		}
//
//		this->Initial = this->current(flip);
//		this->Value = value;
//
//		if(this->turn_rate() > 0) {
//			this->Timer.Start(this->num_steps(flip));
//		}
//
//		return true;
//	}
//
//private:
//	short difference() const {
//		return static_cast<short>(this->Value.value() - this->Initial.value());
//	}
//
//	int difference(bool flip)
//	{
//		int v = this->Value.value();
//		if (v < 0)
//			v = 65536 - -v;
//		int i = this->Initial.value();
//		if (i < 0)
//			i = 65536 - -i;
//		int a = v - i;
//		int b = this->Value.value() - this->Initial.value();
//		int diff = std::abs(a) < std::abs(b) ? a : b;
//		if (!flip)
//		{
//			return diff;
//		}
//		int flipDiff = 65536 - std::abs(diff);
//		return diff < 0 ? flipDiff : -flipDiff;
//	}
//
//	int num_steps(bool flip = false) const {
//		return abs(this->difference()) / this->turn_rate();
//	}
//
//	DirStruct Value; // target facing
//	DirStruct Initial; // rotation started here
//	TimerStruct Timer; // counts rotation steps
//	DirStruct ROT; // Rate of Turn. INI Value * 256
//};

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

template<typename T , size_t count>
class ArrayWrapper
{
	T Data[count];
public:
	constexpr auto begin() const { return  std::begin(Data); }
	constexpr auto end() const { return  std::end(Data); }
	constexpr auto begin() { return std::begin(Data); }
	constexpr auto end() { return std::end(Data); }

	constexpr int size() const { return count; }

	T at(int Index) const { return Data[Index]; }
	T& operator[](int nIdx) { return Data[nIdx]; }
	T operator[](int nIdx) const { return Data[nIdx]; }
};