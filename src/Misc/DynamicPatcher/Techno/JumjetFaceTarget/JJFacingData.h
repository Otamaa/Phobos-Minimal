#pragma once
#include <Utilities/TemplateDef.h>

class JJFacingData
{
public:

	Valueable<bool> Enable { false };
	Valueable<int> Facing { 8 };
	Valueable<int> Forward { -2 };

	void SetFacing(int facing, int y)
	{
		Facing = facing;
		Forward = -2 * y;
	}

	void Read(INI_EX& parser, const char* pSection);

	template <typename T>
	void Serialize(T& Stm)
	{
		//Debug::Log("Loading Element From JJFacingData ! \n");
		Stm
			.Process(Enable)
			.Process(Facing)
			.Process(Forward)
			;

		//Stm.RegisterChange(this);
	}

};
