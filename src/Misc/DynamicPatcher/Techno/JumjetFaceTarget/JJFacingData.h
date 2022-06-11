#pragma once
#ifdef COMPILE_PORTED_DP_FEATURES
#include <Utilities/TemplateDef.h>

class JJFacingData
{
public:

	Valueable<bool> Enable;
	Valueable<int> Facing;
	Valueable<int> Forward;

	JJFacingData()
		:Enable { false }
		, Facing { 8 }
		, Forward { -2 }
	{ }

	void SetFacing(int facing, int y)
	{
		Facing = facing;
		Forward = -2 * y;
	}

	~JJFacingData() = default;

	void Read(INI_EX& parser, const char* pSection)
	{
		Enable.Read(parser, pSection, "JumpjetFacingToTarget");
		if (Enable)
		{
			Facing.Read(parser, pSection, "JumpjetFacing");

			int nDummyFacing = Facing;
			if (nDummyFacing >= 8)
			{
				int x = nDummyFacing % 8;
				int y = nDummyFacing / 8;
				if (x == 0)
				{
					 SetFacing(nDummyFacing, y);
				}
				else if (x > 4)
				{
					SetFacing(8 * (y + 1), y + 1);
				}
				else
				{
					SetFacing(8 * y, y);
				}
			}

		}
	}

	template <typename T>
	void Serialize(T& Stm)
	{
		Debug::Log("Loading Element From JJFacingData ! \n");
		Stm
			.Process(Enable)
			.Process(Facing)
			.Process(Forward)
			;
	}

};
#endif