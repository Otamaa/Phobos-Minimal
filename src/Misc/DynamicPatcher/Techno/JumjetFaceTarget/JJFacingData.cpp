#include "JJFacingData.h"
#ifdef COMPILE_PORTED_DP_FEATURES
void JJFacingData::Read(INI_EX& parser, const char* pSection)
{
	//Enable.Read(parser, pSection, "JumpjetFacingToTarget");
	//if (Enable)
	//{
	//	Facing.Read(parser, pSection, "JumpjetFacing");

	//	int nDummyFacing = Facing;
	//	if (nDummyFacing >= 8)
	//	{
	//		int x = nDummyFacing % 8;
	//		int y = nDummyFacing / 8;
	//		if (x == 0)
	//		{
	//			SetFacing(nDummyFacing, y);
	//		}
	//		else if (x > 4)
	//		{
	//			SetFacing(8 * (y + 1), y + 1);
	//		}
	//		else
	//		{
	//			SetFacing(8 * y, y);
	//		}
	//	}

	//}
}
#endif