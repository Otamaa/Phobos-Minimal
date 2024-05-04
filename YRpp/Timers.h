#pragma once

#include <windows.h>
#include <timeapi.h>

template<typename T>
concept TimerType = std::convertible_to<T, int>&& requires (T t)
{
	{ t() }->std::same_as<long>;
};

struct FrameTimer
{
	constexpr FORCEINLINE long operator()()const { return *reinterpret_cast<long*>(0xA8ED84); }
	constexpr FORCEINLINE operator long() const { return *reinterpret_cast<long*>(0xA8ED84); }
};

struct SystemTimer
{
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


//used for timed events, time measured in frames!
template<TimerType Clock>
class TimerClass
{
public:
	int StartTime;
	Clock CurrentTime; // timer
	int TimeLeft;

	constexpr TimerClass() : StartTime { -1 }, TimeLeft { 0 } { }


	explicit TimerClass(int duration) : StartTime { -1 }, TimeLeft { duration } {
		this->StartTime = this->CurrentTime;
	}

	constexpr TimerClass(noinit_t()){ }
	constexpr ~TimerClass() = default;

	TimerClass(const TimerClass& other) : StartTime { other.StartTime }, TimeLeft { other.TimeLeft } { }

	TimerClass& operator=(const TimerClass& other) {
		this->StartTime = other.StartTime;
		this->TimeLeft = other.TimeLeft;
		return *this;
	}

	TimerClass& operator = (TimerClass&&) =  default;

	constexpr FORCEINLINE void Start(int duration)
	{
		this->StartTime = this->CurrentTime;
		this->TimeLeft = duration;
	}

	constexpr FORCEINLINE void Stop()
	{
		this->StartTime = -1;
		this->TimeLeft = 0;
	}

	constexpr FORCEINLINE void Pause()
	{
		if (this->IsTicking())
		{
			this->TimeLeft = this->GetTimeLeft();
			this->StartTime = -1;
		}
	}

	constexpr FORCEINLINE void Resume()
	{
		if (!this->IsTicking())
		{
			this->StartTime = this->CurrentTime;
		}
	}

	constexpr FORCEINLINE int GetTimeLeft() const
	{
		if (!this->IsTicking())
		{
			return this->TimeLeft;
		}

		const int passed = this->CurrentTime - this->StartTime;
		const int left = this->TimeLeft - passed;
		const int remaining = (left <= 0) ? 0 : left;
		//this->TimeLeft = remaining;
		return remaining;
	}

	// returns whether a ticking timer has finished counting down.
	constexpr FORCEINLINE bool Completed() const
	{
		return this->IsTicking() && !this->HasTimeLeft();
	}

	// returns whether a delay is active or a timer is still counting down.
	// this is the 'opposite' of Completed() (meaning: incomplete / still busy)
	// and logically the same as !Expired() (meaning: blocked / delay in progress)
	constexpr FORCEINLINE bool InProgress() const
	{
		return this->IsTicking() && this->HasTimeLeft();
	}

	constexpr FORCEINLINE bool IsNotActive() const
	{
		return this->IsTicking() && !this->TimeLeft;
	}

	// returns whether a delay is inactive. same as !InProgress().
	constexpr FORCEINLINE bool Expired() const
	{
		return !this->IsTicking() || !this->HasTimeLeft();
	}

	// Sometimes I want to know if the timer has ever started
	constexpr FORCEINLINE bool HasStarted() const
	{
		return this->IsTicking() || this->HasTimeLeft();
	}

	constexpr FORCEINLINE void Add(int nTime)
	{
		this->Pause();
		this->TimeLeft += nTime;
		this->Resume();
	}

	constexpr FORCEINLINE bool IsTicking() const
	{
		return this->StartTime != -1;
	}

	constexpr FORCEINLINE bool HasTimeLeft() const
	{
		return this->GetTimeLeft() > 0;
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

class RepeatableTimerStruct : public CDTimerClass
{
public:
	int Duration { 0 };

	constexpr RepeatableTimerStruct() = default;
	RepeatableTimerStruct(const RepeatableTimerStruct&) = default;
	RepeatableTimerStruct& operator = (const RepeatableTimerStruct&) = default;
	RepeatableTimerStruct& operator = (RepeatableTimerStruct&&) = default;
	RepeatableTimerStruct(int duration) { this->Start(duration); }

	constexpr FORCEINLINE void Start(int duration)
	{
		this->Duration = duration;
		this->Restart();
	}

	constexpr FORCEINLINE void Restart()
	{
		this->CDTimerClass::Start(this->Duration);
	}
};

typedef RepeatableTimerStruct RateTimer;
