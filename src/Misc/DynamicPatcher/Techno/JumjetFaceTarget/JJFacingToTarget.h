#pragma once
#ifdef COMPILE_PORTED_DP_FEATURES
#include <GeneralStructures.h>

class JJFacingToTarget
{
public:
	bool NeedToTurn;
	DirStruct ToDir;
	int Facing;

	JJFacingToTarget()
		: NeedToTurn { false }, ToDir { } , Facing{ 8 }
	{ }

	~JJFacingToTarget() = default;

	void TurnTo(DirStruct toDir, int facing) {
		NeedToTurn = true;
		ToDir = toDir;
		Facing = facing;
	}

	void Turning() {
		NeedToTurn = false;
	}

	void Cancel() {
		NeedToTurn = false;
	}

	template <typename T>
	void Serialize(T& Stm)
	{
		 Stm
			.Process(NeedToTurn)
			.Process(ToDir)
			.Process(Facing)
			;
	}
};
#endif