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
	static fmt::basic_memory_buffer<char, 20> buffer {};
	for (int i = 0; i < (int)SidebarExtData::TabProducingProgress.size(); i++) {
		if(!SidebarExtData::TabProducingProgressIsLoaded[i]) {
			buffer.clear();
			fmt::format_to(std::back_inserter(buffer), "tab{:02}pp.SHP", i);
			buffer.push_back('\0');
			SidebarExtData::TabProducingProgress[i] = GameCreate<SHPReference>(
				buffer.data());
			SidebarExtData::TabProducingProgressIsLoaded[i] = true;
		}

		buffer.clear();
	}

	return 0;
}

ASMJIT_PATCH(0x6A5EA1, SidebarClass_UnloadShapes_AdditionalFiles, 0x5)
{
	for (int i = 0; i < (int)SidebarExtData::TabProducingProgress.size(); i++)
	{
		//the shape is already invalid if the name not event there ,..
		if(SidebarExtData::TabProducingProgressIsLoaded[i]){
			SidebarExtData::TabProducingProgress[i] = nullptr;
			SidebarExtData::TabProducingProgressIsLoaded[i] = false;
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