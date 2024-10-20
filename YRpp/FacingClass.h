#pragma once

#include <algorithm>
#include <GeneralStructures.h>

class FacingClass
{
public:
	constexpr explicit FacingClass() noexcept :
		DesiredFacing{ } ,
		StartFacing { },// The starting direction from which to calcuate the rotation.
		RotationTimer { },
		ROT{ }
	{ }

	constexpr explicit FacingClass(int rate) noexcept :
		DesiredFacing { },
		StartFacing { },// The starting direction from which to calcuate the rotation.
		RotationTimer { },
		ROT { (DirType)MinImpl(rate,127) } {
	}

	constexpr explicit FacingClass(const DirStruct& facing) noexcept :
		DesiredFacing { facing },
		StartFacing { },// The starting direction from which to calcuate the rotation.
		RotationTimer { },
		ROT { } {
	}

	constexpr explicit FacingClass(DirType dir) noexcept :
		DesiredFacing { dir },
		StartFacing { },// The starting direction from which to calcuate the rotation.
		RotationTimer { },
		ROT { }
	{ }

	constexpr FacingClass(const FacingClass& another) noexcept
		: DesiredFacing { another.DesiredFacing }
		, StartFacing { another.StartFacing }
		, RotationTimer { another.RotationTimer }
		, ROT { another.ROT }
	{ }

	constexpr FacingClass& operator=(const FacingClass& another) noexcept
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

	constexpr short Difference_Raw() const {
		return short(DesiredFacing.Raw) - short(StartFacing.Raw);
	}

	constexpr short turn_rate() const {
		return short(this->ROT.Raw);
	}

	constexpr bool Set_Desired(const DirStruct& facing)
	{
		if (DesiredFacing == facing)
			return false;

		StartFacing = Current();
		DesiredFacing = facing;

		if (turn_rate() > 0)
			RotationTimer.Start(NumSteps());

		return true;
	}

	constexpr bool Set_Current(const DirStruct& facing)
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

	constexpr DirStruct Desired() const
	{
		return DesiredFacing;
	}

	constexpr DirStruct Current(int offset = 0) const
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

	static constexpr DirStruct FORCEINLINE Current(FacingClass* facing, int offset) {
		return facing->Current(offset);
	}

	constexpr DirStruct Next()
	{ return Current(1); }

	constexpr bool Is_Rotating() const
	{
		return turn_rate() > 0 && RotationTimer.GetTimeLeft();
	}

	constexpr bool Is_Rotating_L() const
	{
		if (!Is_Rotating())
			return false;

		return Difference_Raw() < 0;
	}

	constexpr bool Is_Rotating_G() const
	{
		if (!Is_Rotating())
			return false;

		return Difference_Raw() > 0;
	}

	constexpr DirStruct Difference() const {
		return DirStruct{ short(DesiredFacing.Raw) - short(StartFacing.Raw) };
	}

	constexpr void Set_ROT(int rate)
	{
		ROT.SetDir((DirType)MinImpl(rate,127));
	}

private:
	constexpr int NumSteps() const
	{
		return Math::abs(Difference_Raw()) / ROT.Raw;
	}

public:

	DirStruct DesiredFacing;
	DirStruct StartFacing; // The starting direction from which to calcuate the rotation.
	CDTimerClass RotationTimer;
	DirStruct ROT;
};