#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <RectangleStruct.h>
#include <CellStruct.h>

static constexpr constant_ptr<CellStruct, 0x880964u> CursorSize {};
static constexpr constant_ptr<CellStruct, 0x880974u> CursorSizeSecond {};

struct WRect
{
	short X, Y, Width, Height;
};

// It almost done but i change something the it completely broken

#pragma optimize("", off )
void NOINLINE GetOccupyDimension(RectangleStruct& res ,CellStruct* a2) {
	if (!a2 || *a2 == CellStruct::Empty) {
		res = { 0,0,0,0 };
		return;
	}

	int nYMin_res = -512;
	int nXmin_res = -512;

	int nYMax_res = 512;
	int nXmax_res = 512;

	for (++a2; !(*a2 == CellStruct::Empty); )
	{
		nYMin_res = a2->Y;
		nYMax_res = a2->Y;
		if (a2->Y >= 512)
			nYMax_res = 512;
		if (a2->Y <= -512)
			nYMin_res = -512;

		nXmin_res = a2->X;
		nXmax_res = a2->X;
		if (a2->X >= 512)
			nXmax_res = 512;
		if (a2->X <= -512)
			nXmin_res = -512;
	}

	res = { nYMax_res  ,  nXmax_res  , nYMin_res ,  nXmin_res };
	/*
	res.X = nYMax_res;
	res.Y = nXmax_res;
	res.Width = nYMin_res;
	res.Height = nXmin_res;
	*/
}

DEFINE_OVERRIDE_HOOK_AGAIN(0x6D5573 , sub_6D5030_CustomFoundation, 0x6)
DEFINE_OVERRIDE_HOOK(0x6D50FB ,sub_6D5030_CustomFoundation, 0x5)
{
	RectangleStruct nDispRect {};
	auto pCursorA = CursorSize();
	auto pCursorB = CursorSizeSecond();
	const bool bOnFB = R->Origin() == 0x6D50FB;
	GetOccupyDimension(nDispRect, (!bOnFB ?
		pCursorB : pCursorA));

	short nX = 0;
	if (nDispRect.Width - nDispRect.X >= 0)
		nX = (short)nDispRect.Width - (short)nDispRect.X;

	short nY = 0;
	if (nDispRect.Height - nDispRect.Y >= 0)
		nY = (short)nDispRect.Height - (short)nDispRect.Y;

	const CellStruct v9res { (nX + 1)  , (nY + 1) };
	const CellStruct v10res { (short)nDispRect.X , (short)nDispRect.Y };

	//const auto nRet = DisplayClass::Instance->FoundationBoundsSize((!bOnFB ? pCursorB : &pCursorA));
	R->Stack(0x14, v10res);
	R->Stack(0x18, v9res);
	R->EAX((DWORD)v9res);
	R->ESI((short)nDispRect.Y);

	return (!bOnFB) ? 0x6D558F : 0x6D5116 ;
}

//DEFINE_HOOK_AGAIN(0x6D5573 , sub_6D5030_CustomFoundation, 0x6)
//DEFINE_HOOK(0x6D50FB ,sub_6D5030_CustomFoundation, 0x5)
//{	
//	auto pCursorA = Unsorted::CursorSize();
//	auto pCursorB = Unsorted::CursorSizeSecond();
//	const bool bOnFB = R->Origin() == 0x6D50FB;
//
//	Debug::Log("Cursor Data [%x] \n", !bOnFB ? pCursorB : pCursorA);
//	return 0x0;
//}
#pragma optimize("", on )