#pragma once
#include <GeneralStructures.h>

class JJFacingToTarget
{
public:
	bool NeedToTurn { false };
	DirStruct ToDir { };
	int Facing { 8 };

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
