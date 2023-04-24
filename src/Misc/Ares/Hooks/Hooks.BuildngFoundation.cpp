#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <RectangleStruct.h>
#include <CellStruct.h>
#include <Unsorted.h>

void GetDisplayRect(RectangleStruct& a1, CellStruct* a2)
{
	int v2 = -512;
	int v3 = -512;
	int v4 = 512;
	int v5 = 512;

	if (a2->X == 0x7FFF && a2->Y == 0x7FFF) {
		a1.X = 0;
		a1.Y = 0;
		a1.Width = 0;
		a1.Height = 0;
		return;
	}

	int16_t* v6 = &a2->Y;

	while (true)
	{
		int16_t v8 = *(v6 - 1);
		int v12 = v3;
		int v11 = v5;
		int v9 = v4;
		int v10 = v2;
		if (v8 == 0x7FFF)
		{
			v9 = v4;
			if (*v6 == 0x7FFF)
				break;
		}
		v3 = *v6;
		v6 += 2;
		v2 = v8;
		v5 = v3;
		v4 = v8;
		if (v8 >= v9)
			v4 = v9;
		if (v8 <= v10)
			v2 = v10;
		if (v3 >= v11)
			v5 = v11;
		if (v3 <= v12)
			v3 = v12;
	}

	a1.X = v4;
	a1.Y = v5;
	a1.Width = v2;
	a1.Height = v3;
}

DEFINE_OVERRIDE_HOOK_AGAIN(0x6D5573 , TacticalClass_DrawPlacement_CustomFoundation, 0x6)
DEFINE_OVERRIDE_HOOK(0x6D50FB , TacticalClass_DrawPlacement_CustomFoundation, 0x5)
{
	RectangleStruct nDispRect {};
	const bool bOnFB = R->Origin() == 0x6D50FB;
	GetDisplayRect(nDispRect, (!bOnFB ?
		Unsorted::CursorSizeSecond() : Unsorted::CursorSize()));

	int16_t nX = 0;
	if (nDispRect.Width - nDispRect.X >= 0)
		nX = LOWORD(nDispRect.Width) - LOWORD(nDispRect.X);

	int16_t nY = 0;
	if (nDispRect.Height - nDispRect.Y >= 0)
		nY = LOWORD(nDispRect.Height) - LOWORD(nDispRect.Y);

	const CellStruct v9res { (nX + 1)  , (nY + 1) };
	const CellStruct v10res { (int16_t)nDispRect.X , (int16_t)nDispRect.Y };

	R->Stack(0x14, v10res.Pack());
	R->Stack(0x18, v9res.Pack());
	R->EAX(v9res.Pack());
	R->ESI(nDispRect.Y);

	return (!bOnFB) ? 0x6D558F : 0x6D5116 ;
}
