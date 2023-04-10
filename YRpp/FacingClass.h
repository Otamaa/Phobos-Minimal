#pragma once

#include <algorithm>
#include <GeneralStructures.h>

class FacingClass
{
public:
	explicit FacingClass() noexcept { }
	explicit FacingClass(int rate) noexcept
	{
		Set_ROT(rate);
	}

	explicit FacingClass(const DirStruct& facing) noexcept
	{
		DesiredFacing = facing;
	}

	explicit FacingClass(DirType dir) noexcept
	{
		DesiredFacing.SetDir(dir);
	}

	FacingClass(const FacingClass& another) noexcept
		: DesiredFacing { another.DesiredFacing }
		, StartFacing { another.StartFacing }
		, RotationTimer { another.RotationTimer }
		, ROT { another.ROT }
	{ }

	FacingClass& operator=(const FacingClass& another) noexcept
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

	short Difference_Raw() const {
		return short(DesiredFacing.Raw) - short(StartFacing.Raw);
	}

	short turn_rate() const {
		return short(this->ROT.Raw);
	}

	bool Set_Desired(const DirStruct& facing)
	{
		if (DesiredFacing == facing)
			return false;

		StartFacing = Current();
		DesiredFacing = facing;

		if (turn_rate() > 0)
			RotationTimer.Start(NumSteps());

		return true;
	}
	bool Set_Current(const DirStruct& facing)
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

	DirStruct Desired() const
	{
		return DesiredFacing;
	}

	DirStruct Current(int offset = 0) const
	{
		auto ret = this->DesiredFacing;
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

	DirStruct Next()
	{ return Current(1); }

	bool Is_Rotating() const
	{
		return turn_rate() > 0 && RotationTimer.GetTimeLeft();
	}

	bool Is_Rotating_L() const
	{
		if (!Is_Rotating())
			return false;

		return Difference_Raw() < 0;
	}

	bool Is_Rotating_G() const
	{
		if (!Is_Rotating())
			return false;

		return Difference_Raw() > 0;
	}

	DirStruct Difference() const {
		return DirStruct{ short(DesiredFacing.Raw) - short(StartFacing.Raw) };
	}

	void Set_ROT(int rate)
	{
		ROT.SetDir((DirType)std::min(rate,127));
	}

private:
	int NumSteps() const
	{
		return std::abs(Difference_Raw()) / ROT.Raw;
	}

public:

	DirStruct DesiredFacing;
	DirStruct StartFacing; // The starting direction from which to calcuate the rotation.
	CDTimerClass RotationTimer;
	DirStruct ROT;
};