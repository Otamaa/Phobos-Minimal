#include <Helpers/Macro.h>
#include <PCX.h>
#include <FileFormats/SHP.h>
#include <Ext/Rules/Body.h>

DEFINE_HOOK(0x6B9D9C, RGB_PCX_Loader, 0x7)
{
	GET(BSurface* const, pSurf, EDI);
	return (pSurf->BytesPerPixel == 2) ? 0x6B9EE7: 0x0;
}

DEFINE_HOOK(0x5535D0, LoadProgressMgr_Draw_PCXLoadingScreen, 0x6)
{
	LEA_STACK(char*, name, 0x84);

	char pFilename[0x20];
	strcpy_s(pFilename, name);
	_strlwr_s(pFilename);

	BSurface* pcx = nullptr;
	char nBuffer[0x40];

	IMPL_SNPRNINTF(nBuffer, sizeof(nBuffer) , GameStrings::LSSOBS_SHP(),
		Game::ScreenWidth() != 640 ? GameStrings::_800() : GameStrings::_640());
	if (!_stricmp(pFilename, nBuffer))
	{
		IMPL_SNPRNINTF(nBuffer, sizeof(nBuffer) , "ls%sobs.pcx",
			Game::ScreenWidth() != 640 ? GameStrings::_800() : GameStrings::_640());

		if(PCX::Instance->LoadFile(nBuffer))
			pcx = PCX::Instance->GetSurface(nBuffer);
	}

	if (strstr(pFilename, ".pcx") || pcx)
	{
		if (!pcx)
		{
			PCX::Instance->LoadFile(pFilename);
			pcx = PCX::Instance->GetSurface(pFilename);
		}

		if (pcx)
		{
			GET_BASE(DSurface*, pSurf, 0x60);
			RectangleStruct pSurfBounds { 0, 0, pSurf->Width, pSurf->Height };
			RectangleStruct pcxBounds { 0, 0, pcx->Width, pcx->Height };
			RectangleStruct destClip { (pSurf->Width - pcx->Width) / 2, (pSurf->Height - pcx->Height) / 2, pcx->Width, pcx->Height };

			pSurf->Copy_From(pSurfBounds, destClip, pcx, pcxBounds, pcxBounds, true, true);
		}

		return 0x553603;
	}

	return 0;
}

DEFINE_HOOK(0x552FCB, LoadProgressMgr_Draw_PCXLoadingScreen_Campaign, 0x6)
{
	char filename[0x40];
	strcpy_s(filename, ScenarioClass::Instance->LS800BkgdName);
	_strlwr_s(filename);

	if (strstr(filename, ".pcx"))
	{
		BSurface* pPCX = nullptr;

		if (PCX::Instance->LoadFile(filename))
			pPCX = PCX::Instance->GetSurface(filename);

		if (pPCX) {
			GET_BASE(DSurface*, pSurface, 0x60);

			RectangleStruct pSurfBounds { 0, 0, pSurface->Width, pSurface->Height };
			RectangleStruct pcxBounds { 0, 0, pPCX->Width, pPCX->Height };
			RectangleStruct destClip { (pSurface->Width - pPCX->Width) / 2, (pSurface->Height - pPCX->Height) / 2, pPCX->Width, pPCX->Height };

			pSurface->Copy_From(pSurfBounds, destClip, pPCX, pcxBounds, pcxBounds, true, true);
		}

		return 0x552FFF;
	}

	return 0;
}