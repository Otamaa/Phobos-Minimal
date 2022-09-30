#pragma once

#include <FileFormats/_Loader.h>
#include <vector>
#include <Utilities/TemplateDef.h>

struct ImageDatas
{
	VoxLib* Vxl;
	MotLib* Hva;
};

// TODO : explore more !
struct TTypeEXt
{
	static std::vector<std::pair<ImageDatas, ImageDatas>> AdditionalTurrents;

	static void Read(CCINIClass* pINI, int nIdx = 0)
	{
		INI_EX nINI(pINI);

		CCFileClass nVox;
		std::pair<ImageDatas, ImageDatas> nDummy {  };
		nVox.Load_Alloc_Data("Test.vxl", 0);
		if (auto pVoxLib = GameCreate<VoxLib>(&nVox))
			nDummy.first.Vxl = pVoxLib;


		CCFileClass nHva;
		nHva.Load_Alloc_Data("Test.hva", 0);
		if (auto pMotLib = GameCreate<MotLib>(&nHva))
			nDummy.first.Hva = pMotLib;

		AdditionalTurrents.push_back(nDummy);

	}
};