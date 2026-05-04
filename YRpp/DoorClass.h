#pragma once

#include <Timers.h>
#include <ASMMacros.h>

enum class DoorState : char
{
	Close = 0x0,
	Open = 0x1,
};

struct NOVTABLE DoorClass
{
	double RateDouble {};
	RepeatableTimer Timer {};
	DoorState ToTransition { DoorState::Close };
	DoorState State { DoorState::Close };

private:
	int _pad {};

public:

	bool IsOpening() const {
		JMP_THIS(0x4A5110);
	}

	bool IsClosing() const {
		JMP_THIS(0x4A5130);
	}

	bool HasFinished() const {
		JMP_THIS(0x4A5150);
	}

	bool IsOpen() const {
		JMP_THIS(0x4A51B0);
	}

	bool IsClosed() const {
		JMP_THIS(0x4A51D0);
	}

	void Open(double rate) const {
		JMP_THIS(0x4A51F0);
	}

	void Close(double rate) const {
		JMP_THIS(0x4A5240);
	}

	void FlipState() const {
		JMP_THIS(0x4A5290);
	}

	void ForceOpen() const {
		JMP_THIS(0x4A52D0);
	}

	void ForceClose() const {
		JMP_THIS(0x4A52E0);
	}

	double GetCompletePercent() const {
		JMP_THIS(0x4A52F0);
	}

	void Update() const {
		JMP_THIS(0x4A5360);
	}
};
static_assert(sizeof(DoorClass) == 0x20, "Invalid Size!");
static_assert(offsetof(DoorClass, RateDouble) == 0x0, "ClassMember Shifted !");
static_assert(offsetof(DoorClass, Timer) == 0x8, "ClassMember Shifted !");
static_assert(offsetof(DoorClass, ToTransition) == 0x18, "ClassMember Shifted !");
static_assert(offsetof(DoorClass, State) == 0x19, "ClassMember Shifted !");
