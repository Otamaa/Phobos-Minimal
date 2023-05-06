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
			RectangleStruct pSurfBounds = { 0, 0, pSurf->Width, pSurf->Height };
			RectangleStruct pcxBounds = { 0, 0, pcx->Width, pcx->Height };
			RectangleStruct destClip = { (pSurf->Width - pcx->Width) / 2, (pSurf->Height - pcx->Height) / 2, pcx->Width, pcx->Height };

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

//DEFINE_HOOK(0x5535D0, PCX_LoadScreen, 0x6)
//{
//	LEA_STACK(char*, name, 0x84);
//
//	char pFilename[0x20];
//	strcpy_s(pFilename, name);
//	_strlwr_s(pFilename);
//
//	if (CRT::strstr(pFilename, ".pcx")
//		//|| CRT::strstr(pFilename, ".png")
//		) {
//
//		BSurface* pCXSurf = nullptr;
//
//		if(PCX::Instance->LoadFile(pFilename))
//			pCXSurf = PCX::Instance->GetSurface(pFilename);
//
//		if (pCXSurf) {
//			GET_BASE(DSurface*, pSurf, 0x60);
//			RectangleStruct pSurfBounds = { 0, 0, pSurf->Width, pSurf->Height };
//			RectangleStruct pcxBounds = { 0, 0, pCXSurf->Width, pCXSurf->Height };
//
//			RectangleStruct destClip = { 0, 0, pCXSurf->Width, pCXSurf->Height };
//			destClip.X = (pSurf->Width - pCXSurf->Width) / 2;
//			destClip.Y = (pSurf->Height - pCXSurf->Height) / 2;
//
//			pSurf->Copy_From(pSurfBounds, destClip, pCXSurf, pcxBounds, pcxBounds, true, true);
//		}
//		return 0x553603;
//	}
//	return 0;
//}

/*
DEFINE_HOOK(0x552F81, PCX_LoadingScreen_Campaign, 0x5)
{
	GET(LoadProgressManager*, pThis, EBP);

	DSurface* pSurface = reinterpret_cast<DSurface*>(pThis->ProgressSurface);
	char pFilename[0x20];
	strcpy_s(pFilename, ScenarioClass::Instance->LS800BkgdName);
	_strlwr_s(pFilename);

	if (strstr(pFilename, ".pcx") == 0
		|| strstr(pFilename, ".png") == 0 )
	{
		BSurface* pCXSurf = nullptr;

		if (PCX::Instance->LoadFile(pFilename))
			pCXSurf = PCX::Instance->GetSurface(pFilename);

		if (pCXSurf)
		{
			RectangleStruct destClip = {
				(pSurface->Width - pCXSurf->Width) / 2,
				(pSurface->Height - pCXSurf->Height) / 2,
				pCXSurf->Width, pCXSurf->Height
			};

			PCX::Instance->BlitToSurface(&destClip, pSurface, pCXSurf);
		}

		R->EBX(R->EDI());
		return 0x552FC6;
	}

	return 0;
}*/

DEFINE_HOOK(0x6A99F3, StripClass_Draw_DrawMissing, 0x6)
{
	GET_STACK(SHPStruct const*, pCameo, STACK_OFFS(0x48C, 0x444));

	if (pCameo)
	{
		auto pCameoRef = pCameo->AsReference();
		char pFilename[0x20];
		strcpy_s(pFilename, RulesExt::Global()->MissingCameo.data());
		_strlwr_s(pFilename);

		if (!_stricmp(pCameoRef->Filename, GameStrings::XXICON_SHP())
			&& (strstr(pFilename, ".pcx")))
		{
			BSurface* pCXSurf = nullptr;

			if (PCX::Instance->LoadFile(pFilename))
				pCXSurf = PCX::Instance->GetSurface(pFilename);

			if (pCXSurf)
			{
				GET(int, destX, ESI);
				GET(int, destY, EBP);

				RectangleStruct bounds { destX, destY, 60, 48 };
				PCX::Instance->BlitToSurface(&bounds, DSurface::Sidebar, pCXSurf);

				return 0x6A9A43; //skip drawing shp cameo
			}
		}
	}

	return 0;
}