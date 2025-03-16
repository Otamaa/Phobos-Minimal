#include "Body.h"

#include <Ext/House/Body.h>
#include <FactoryClass.h>
#include <FileSystem.h>
#include <Ext/Side/Body.h>
#include <Phobos.h>

#include <Ext/TechnoType/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/Scenario/Body.h>

ASMJIT_PATCH(0x6A593E, SidebarClass_InitForHouse_AdditionalFiles, 0x5)
{
	for (int i = 0; i < (int)SidebarExtData::TabProducingProgress.size(); i++) {
		if(!SidebarExtData::TabProducingProgress[i]) {
			SidebarExtData::TabProducingProgress[i] = GameCreate<SHPReference>(std::format("tab{:02}pp.SHP", i).c_str());
		}
	}

	return 0;
}

ASMJIT_PATCH(0x6A5EA1, SidebarClass_UnloadShapes_AdditionalFiles, 0x5)
{
	for (int i = 0; i < (int)SidebarExtData::TabProducingProgress.size(); i++)
	{
		//the shape is already invalid if the name not event there ,..
		if(SidebarExtData::TabProducingProgress[i]){
			if(!Phobos::Otamaa::ExeTerminated && SidebarExtData::TabProducingProgress[i]->Filename) {
				GameDelete<false, false>(SidebarExtData::TabProducingProgress[i]);
			}
			//
			SidebarExtData::TabProducingProgress[i] = nullptr;
		}
	}

	return 0;
}

ASMJIT_PATCH(0x6A6EB1, SidebarClass_DrawIt_ProducingProgress, 0x6)
{
	SidebarExtData::DrawProducingProgress();
	return 0;
}

ASMJIT_PATCH(0x72FCB5, InitSideRectangles_CenterBackground, 0x5)
{
	if (Phobos::UI::CenterPauseMenuBackground)
	{
		GET(RectangleStruct*, pRect, EAX);
		GET_STACK(int, width, STACK_OFFSET(0x18, -0x4));
		GET_STACK(int, height, STACK_OFFSET(0x18, -0x8));

		pRect->X = (width - 168 - pRect->Width) / 2;
		pRect->Y = (height - 32 - pRect->Height) / 2;

		R->EAX(pRect);
	}

	return 0;
}