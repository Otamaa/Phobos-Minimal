#include "Body.h"

#include <Ext/House/Body.h>
#include <FactoryClass.h>
#include <FileSystem.h>
#include <Ext/Side/Body.h>
#include <Phobos.h>

DEFINE_HOOK(0x6A593E, SidebarClass_InitForHouse_AdditionalFiles, 0x5)
{
	char filename[0x20];

	for (int i = 0; i < (int)SidebarExt::TabProducingProgress.size(); i++) {

		auto& shp = SidebarExt::TabProducingProgress[i];

		if(!shp) {
			IMPL_SNPRNINTF(filename,sizeof(filename), "tab%02dpp%s", i , GameStrings::dot_SHP());

			if (auto pFile = FileSystem::LoadSHPRef(filename)) {
				shp = pFile;
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x6A5EA1, SidebarClass_UnloadShapes_AdditionalFiles, 0x5)
{
	if (!Phobos::Otamaa::ExeTerminated)
		return 0x0;

	for (int i = 0; i < (int)SidebarExt::TabProducingProgress.size(); i++)
	{
		auto& shp = SidebarExt::TabProducingProgress[i];

		if(shp){
			GameDelete<false, false>(shp);
		}

		shp = nullptr;
	}

	return 0;
}

DEFINE_HOOK(0x6A6EB1, SidebarClass_DrawIt_ProducingProgress, 0x6)
{
	SidebarExt::DrawProducingProgress();
	return 0;
}

DEFINE_HOOK(0x72FCB5, InitSideRectangles_CenterBackground, 0x5)
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