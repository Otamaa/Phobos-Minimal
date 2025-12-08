#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <RectangleStruct.h>
#include <CellStruct.h>
#include <Unsorted.h>

#include "Header.h"

ASMJIT_PATCH(0x6D50FB , TacticalClass_DrawPlacement_CustomFoundation, 0x5)
{
	RectangleStruct bounds {};
	const bool bOnFB = R->Origin() == 0x6D50FB;

	// Get bounding rectangle of foundation cells
	CustomFoundation::GetDisplayRect(&bounds, (!bOnFB ?
		Unsorted::CursorSizeSecond() : Unsorted::CursorSize()));

	// Calculate actual dimensions
    // bounds.Width is MaxX, bounds.X is MinX
    // bounds.Height is MaxY, bounds.Y is MinY
    CellStruct size{
		.X = (short)std::max(0, bounds.Width - bounds.X) + 1,   // Width = MaxX - MinX + 1
		.Y = (short)std::max(0, bounds.Height - bounds.Y) + 1  // Height = MaxY - MinY + 1
	};

    CellStruct origin_cell {
		.X = (short)bounds.X ,// MinX
    	.Y = (short)bounds.Y  // MinY
	};

	R->Stack(0x14, origin_cell.Pack());
	R->Stack(0x18, size.Pack());
	R->EAX(size.Pack());
	R->ESI(bounds.Y);

	return (!bOnFB) ? 0x6D558F : 0x6D5116 ;
}ASMJIT_PATCH_AGAIN(0x6D5573, TacticalClass_DrawPlacement_CustomFoundation, 0x6)

