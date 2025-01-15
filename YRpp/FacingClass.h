#pragma once

#include <algorithm>
#include <GeneralStructures.h>

class FacingClass
{
public:
	COMPILETIMEEVAL explicit FacingClass() noexcept :
		DesiredFacing{ } ,
		StartFacing { },// The starting direction from which to calcuate the rotation.
		RotationTimer { },
		ROT{ }
	{ }

	COMPILETIMEEVAL explicit FacingClass(int rate) noexcept :
		DesiredFacing { },
		StartFacing { },// The starting direction from which to calcuate the rotation.
		RotationTimer { },
		ROT { (DirType)MinImpl(rate,127) } {
	}

	COMPILETIMEEVAL explicit FacingClass(const DirStruct& facing) noexcept :
		DesiredFacing { facing },
		StartFacing { },// The starting direction from which to calcuate the rotation.
		RotationTimer { },
		ROT { } {
	}

	COMPILETIMEEVAL explicit FacingClass(DirType dir) noexcept :
		DesiredFacing { dir },
		StartFacing { },// The starting direction from which to calcuate the rotation.
		RotationTimer { },
		ROT { }
	{ }

	COMPILETIMEEVAL FacingClass(const FacingClass& another) noexcept
		: DesiredFacing { another.DesiredFacing }
		, StartFacing { another.StartFacing }
		, RotationTimer { another.RotationTimer }
		, ROT { another.ROT }
	{ }

	COMPILETIMEEVAL FacingClass& operator=(const FacingClass& another) noexcept
	{
		if (this != &another)
		{
			DesiredFacing = another.DesiredFacing;
			StartFacing = another.StartFacing;
			RotationTimer = another.RotationTimer;
			ROT = another.ROT;
		}

		return *this;
	}

	COMPILETIMEEVAL short Difference_Raw() const {
		return short(DesiredFacing.Raw) - short(StartFacing.Raw);
	}

	COMPILETIMEEVAL short turn_rate() const {
		return short(this->ROT.Raw);
	}

	COMPILETIMEEVAL bool Set_Desired(const DirStruct& facing)
	{
		if (DesiredFacing == facing)
			return false;

		StartFacing = Current();
		DesiredFacing = facing;

		if (turn_rate() > 0)
			RotationTimer.Start(NumSteps());

		return true;
	}

	COMPILETIMEEVAL bool Set_Current(const DirStruct& facing)
	{
		bool ret = Current() != facing;
		if (ret)
		{
			DesiredFacing = facing;
			StartFacing = facing;
		}
		RotationTimer.Start(0);
		return ret;
	}

	COMPILETIMEEVAL DirStruct Desired() const
	{
		return DesiredFacing;
	}

	COMPILETIMEEVAL DirStruct Current(int offset = 0) const
	{
		DirStruct ret = this->DesiredFacing;
		if (Is_Rotating()) {
			short diff = Difference_Raw();
			short num_steps = short(NumSteps());

			if (num_steps > 0) {
				int steps_left = RotationTimer.GetTimeLeft() - offset;
				ret.Raw = unsigned short(((short)ret.Raw - steps_left * diff / num_steps));
			}
		}

		return ret;
	}

	static COMPILETIMEEVAL DirStruct FORCEDINLINE Current(FacingClass* facing, int offset) {
		return facing->Current(offset);
	}

	COMPILETIMEEVAL DirStruct Next()
	{ return Current(1); }

	COMPILETIMEEVAL bool Is_Rotating() const
	{
		return turn_rate() > 0 && RotationTimer.GetTimeLeft();
	}

	COMPILETIMEEVAL bool Is_Rotating_L() const
	{
		if (!Is_Rotating())
			return false;

		return Difference_Raw() < 0;
	}

	COMPILETIMEEVAL bool Is_Rotating_G() const
	{
		if (!Is_Rotating())
			return false;

		return Difference_Raw() > 0;
	}

	COMPILETIMEEVAL DirStruct Difference() const {
		return DirStruct{ short(DesiredFacing.Raw) - short(StartFacing.Raw) };
	}

	COMPILETIMEEVAL void Set_ROT(int rate)
	{
		ROT.SetDir((DirType)MinImpl(rate,127));
	}

private:
	COMPILETIMEEVAL int NumSteps() const
	{
		return Math::abs(Difference_Raw()) / ROT.Raw;
	}

public:

	DirStruct DesiredFacing;
	DirStruct StartFacing; // The starting direction from which to calcuate the rotation.
	CDTimerClass RotationTimer;
	DirStruct ROT;
};