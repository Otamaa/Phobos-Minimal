#pragma once

#include <Base/Always.h>

#include <windows.h>
#include <timeapi.h>
#include <algorithm>

template<typename T>
concept TimerType = std::convertible_to<T, int>&& requires (T t)
{
	{ t() }->std::same_as<long>;
};

struct FrameTimer
{
	COMPILETIMEEVAL FORCEDINLINE long operator()()const { return *reinterpret_cast<long*>(0xA8ED84); }
	COMPILETIMEEVAL FORCEDINLINE operator long() const { return *reinterpret_cast<long*>(0xA8ED84); }
};

struct SystemTimer
{
	static DWORD GetTime() { return timeGetTime() >> 4;}
	long operator()() const { return timeGetTime() >> 4; }
	operator long() const { return timeGetTime() >> 4; }
};

class MSTimer
{
public:
	MSTimer() { timeBeginPeriod(1); }
	~MSTimer() { timeEndPeriod(1); }

	long operator () () const { return timeGetTime(); }
	operator long() const { return timeGetTime(); }
};

template<TimerType Clock>
class BasicTimer {
	int Time;
	Clock CTime; // timer
};

//used for timed events, time measured in frames!
template<TimerType Clock>
class TimerClass
{
public:
	int StartTime;
	Clock CurrentTime; // timer
	int TimeLeft;

	COMPILETIMEEVAL TimerClass() : StartTime { -1 }, TimeLeft { 0 } { }

	COMPILETIMEEVAL explicit TimerClass(int duration) : StartTime { -1 }, TimeLeft { duration } {
		this->StartTime = this->CurrentTime;
	}

	COMPILETIMEEVAL TimerClass(noinit_t()){ }
	COMPILETIMEEVAL ~TimerClass() = default;

	COMPILETIMEEVAL TimerClass(const TimerClass& other) : StartTime { other.StartTime }, TimeLeft { other.TimeLeft } { }

	COMPILETIMEEVAL TimerClass& operator=(const TimerClass& other) {
		this->StartTime = other.StartTime;
		this->TimeLeft = other.TimeLeft;
		return *this;
	}

	COMPILETIMEEVAL TimerClass& operator = (TimerClass&&) =  default;

	COMPILETIMEEVAL TimerClass& operator=(long set) {
		StartTime = CurrentTime();
		TimeLeft = set;
		return *this;
	}

	COMPILETIMEEVAL FORCEDINLINE void Start(int duration) {
		this->StartTime = this->CurrentTime;
		this->TimeLeft = duration;
	}

	COMPILETIMEEVAL FORCEDINLINE void Stop() {
		this->StartTime = -1;
		this->TimeLeft = 0;
	}

	COMPILETIMEEVAL FORCEDINLINE void Pause() {
		if (this->IsTicking()) {
			this->TimeLeft = this->GetTimeLeft();
			this->StartTime = -1;
		}
	}

	COMPILETIMEEVAL FORCEDINLINE void Resume() {
		if (!this->IsTicking()) {
			this->StartTime = this->CurrentTime;
		}
	}

	COMPILETIMEEVAL FORCEDINLINE int GetTimeLeft() const {
		if (!this->IsTicking()) {
			return this->TimeLeft;
		}

		const int passed = this->CurrentTime - this->StartTime;
		const int left = this->TimeLeft - passed;
		const int remaining = (left <= 0) ? 0 : left;
		//this->TimeLeft = remaining;
		return remaining;
	}

	// returns whether a ticking timer has finished counting down.
	COMPILETIMEEVAL FORCEDINLINE bool Completed() const {
		return this->IsTicking() && !this->HasTimeLeft();
	}

	// returns whether a delay is active or a timer is still counting down.
	// this is the 'opposite' of Completed() (meaning: incomplete / still busy)
	// and logically the same as !Expired() (meaning: blocked / delay in progress)
	COMPILETIMEEVAL FORCEDINLINE bool InProgress() const {
		return this->IsTicking() && this->HasTimeLeft();
	}

	COMPILETIMEEVAL FORCEDINLINE bool IsNotActive() const {
		return this->IsTicking() && !this->TimeLeft;
	}

	// returns whether a delay is inactive. same as !InProgress().
	COMPILETIMEEVAL FORCEDINLINE bool Expired() const {
		return !this->IsTicking() || !this->HasTimeLeft();
	}

	// Sometimes I want to know if the timer has ever started
	COMPILETIMEEVAL FORCEDINLINE bool HasStarted() const {
		return this->IsTicking() || this->HasTimeLeft();
	}

	COMPILETIMEEVAL FORCEDINLINE void Add(int nTime) {
		this->Pause();
		this->TimeLeft += nTime;
		this->Resume();
	}

	COMPILETIMEEVAL FORCEDINLINE bool IsTicking() const {
		return this->StartTime != -1;
	}

	COMPILETIMEEVAL FORCEDINLINE bool HasTimeLeft() const {
		return this->GetTimeLeft() > 0;
	}

	COMPILETIMEEVAL FORCEDINLINE double GetRatePassed() const {
		const int rate = this->Rate;
		return rate ? static_cast<double>(rate - this->GetTimeLeft()) / static_cast<double>(rate) : 1.0;
	}
};

using SystemTimerClass = TimerClass<SystemTimer>;
using MSTimerClass = TimerClass<MSTimer>;

static_assert(offsetof(SystemTimerClass, TimeLeft) == 0x8);
static_assert(offsetof(MSTimerClass, TimeLeft) == 0x8);

static_assert(sizeof(SystemTimerClass) == 0xC, "Invalid Size !");
static_assert(sizeof(MSTimerClass) == 0xC, "Invalid Size !");

using CDTimerClass = TimerClass<FrameTimer>;

static_assert(offsetof(CDTimerClass, TimeLeft) == 0x8);
static_assert(sizeof(CDTimerClass) == 0xC, "Invalid Size !");

class RepeatableTimer : public CDTimerClass
{
public:
	int Rate { 0 };

	COMPILETIMEEVAL RepeatableTimer() = default;

	COMPILETIMEEVAL RepeatableTimer(const RepeatableTimer& that) {
		CDTimerClass::operator=(that);
		Rate = that.Rate;
	}

	COMPILETIMEEVAL RepeatableTimer& operator = (const RepeatableTimer& that) {
		CDTimerClass::operator=(that);
		Rate = that.Rate;
		return *this;
	}

	COMPILETIMEEVAL RepeatableTimer& operator=(long set) {
		CDTimerClass::operator=(set);
		Rate = set;
		return *this;
	}

	COMPILETIMEEVAL RepeatableTimer& operator = (RepeatableTimer&&) = default;
	COMPILETIMEEVAL RepeatableTimer(int duration) { this->Start(duration); }

	COMPILETIMEEVAL FORCEDINLINE void Start(int duration)
	{
		this->Rate = duration;
		this->Restart();
	}

	COMPILETIMEEVAL FORCEDINLINE void Restart()
	{
		this->CDTimerClass::Start(this->Rate);
	}

	COMPILETIMEEVAL OPTIONALINLINE bool Expired() const { return Percent_Expired() == 1.0f; }

	COMPILETIMEEVAL OPTIONALINLINE unsigned long GetValue() const
	{
		unsigned long remain = TimeLeft;

		if (((unsigned long)StartTime) != 0xFFFFFFFFU) {

			unsigned long value = this->CurrentTime - ((unsigned long)StartTime);

			if (value < remain) {
				return remain - value;
			} else {
				return 0;
			}
		}

		return remain;
	}

	COMPILETIMEEVAL OPTIONALINLINE float Percent_Expired() const
	{
		unsigned long rate = Rate;
		if (!rate) {
			return 1.0;
		}

		return (float)(rate - GetValue()) / (float)rate;
	}
};

typedef RepeatableTimer RateTimer;
