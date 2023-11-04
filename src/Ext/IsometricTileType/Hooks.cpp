#include "Body.h"

#include <Helpers/Macro.h>

#include <CellClass.h>
#include <ConvertClass.h>

DEFINE_HOOK(0x544E70, IsometricTileTypeClass_Init_Drawer, 0x8)
{
	GET(CellClass*, pCell, ESI); // Luckily, pCell is just ESI, so we don't need other hooks to set it

	GET(int, red, ECX);
	GET(int, green, EDX);
	GET_STACK(int, blue, 0x4);

	int isoTileTypeIndex = pCell->IsoTileTypeIndex;

	if (auto pIso = IsometricTileTypeClass::Array->GetItemOrDefault(isoTileTypeIndex))
	{
		const auto pExt = IsometricTileTypeExtContainer::Instance.Find(pIso);

		if(pExt->Palette){
			R->EAX(pExt->GetLightConvert(red, green, blue));
			return 0x544FDE;
		}
	}

	return 0;
}