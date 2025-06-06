/*
	ProgressTimer for various game classes.

	This class uses a timer set to a specified interval to increase the current
	value by a specified amount.
*/

#pragma once

#include <GeneralStructures.h>

//called Stage class on Game
class ProgressTimer
{
public:
	COMPILETIMEEVAL ProgressTimer() = default;
	ProgressTimer(const ProgressTimer&) = default;
	ProgressTimer& operator = (const ProgressTimer&) = default;

	ProgressTimer(int duration) { this->Start(duration); }

	void Start(int duration) {
		this->Timer.Start(duration);
	}

	void Start(int duration, int step) {
		this->Step = step;
		this->Start(duration);
	}

	// returns whether the value changed.
	bool Update() {
		if(this->Timer.GetTimeLeft() || !this->Timer.Rate) {
			// timer is still running or hasn't been set yet.
			this->HasChanged = false;
		} else {
			// timer expired. move one step forward.
			this->Stage += this->Step;
			this->HasChanged = true;
			this->Timer.Restart();
		}

		return this->HasChanged;
	}

public:
	int Stage{ 0 }; // the current value , stage
	bool HasChanged{ false }; // if the timer expired this frame and the value changed
	RepeatableTimer Timer{};
	int Step{ 1 }; // added to value every time the timer expires
};

typedef ProgressTimer StageClass;