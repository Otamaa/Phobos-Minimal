#include <Helpers/Macro.h>
#include <PCX.h>
#include <FileFormats/SHP.h>
#include <Ext/Rules/Body.h>

DEFINE_HOOK(0x6B9D9C, RGB_PCX_Loader, 0x7)
{
	GET(BSurface*, pSurf, EDI);
	return(pSurf->BytesPerPixel == 2) ? 0x6B9EE7: 0x0;
}

DEFINE_HOOK(0x5535D0, PCX_LoadScreen, 0x6)
{
	LEA_STACK(char*, name, 0x84);

	char pFilename[0x20];
	strcpy_s(pFilename, name);
	_strlwr_s(pFilename);

	if (PhobosCRT::stristr(pFilename, ".pcx")
		|| PhobosCRT::stristr(pFilename, ".png")) {

		BSurface* pCXSurf = nullptr;

		if(PCX::Instance->LoadFile(pFilename))
			pCXSurf = PCX::Instance->GetSurface(pFilename);

		if (pCXSurf) {
			GET_BASE(DSurface*, pSurf, 0x60);
			RectangleStruct pSurfBounds = { 0, 0, pSurf->Width, pSurf->Height };
			RectangleStruct pcxBounds = { 0, 0, pCXSurf->Width, pCXSurf->Height };

			RectangleStruct destClip = { 0, 0, pCXSurf->Width, pCXSurf->Height };
			destClip.X = (pSurf->Width - pCXSurf->Width) / 2;
			destClip.Y = (pSurf->Height - pCXSurf->Height) / 2;

			pSurf->Copy_From(pSurfBounds, destClip, pCXSurf, pcxBounds, pcxBounds, true, true);
		}
		return 0x553603;
	}
	return 0;
}

DEFINE_HOOK(0x552F81, PCX_LoadingScreen_Campaign, 0x5)
{
	GET(LoadProgressManager*, pThis, EBP);

	DSurface* pSurface = reinterpret_cast<DSurface*>(pThis->ProgressSurface);
	const std::string fileName = ScenarioClass::Instance->LS800BkgdName;

	if (fileName.length() < 4U || fileName.empty()) {
		Debug::Log("[LS800BkgdName] illegal filename [%s]\n", fileName.c_str());
		return 0x0;
	}

	if (PhobosCRT::stristr(fileName.c_str(), ".pcx") == 0
		|| PhobosCRT::stristr(fileName.c_str(), ".png") == 0 )
	{
		PCX::Instance->LoadFile(fileName.c_str());
		if (BSurface* pcx = PCX::Instance->GetSurface(fileName.c_str()))
		{
			RectangleStruct destClip = {
				(pSurface->Width - pcx->Width) / 2,
				(pSurface->Height - pcx->Height) / 2,
				pcx->Width, pcx->Height
			};

			PCX::Instance->BlitToSurface(&destClip, pSurface, pcx);
		}

		R->EBX(R->EDI());
		return 0x552FC6;
	}

	return 0;
}

DEFINE_HOOK(0x6A99F3, StripClass_Draw_DrawMissing, 0x6)
{
	GET_STACK(SHPStruct*, pCameo, STACK_OFFS(0x48C, 0x444));

	if (pCameo)
	{
		auto pCameoRef = pCameo->AsReference();
		char pFilename[0x20];
		strcpy_s(pFilename, RulesExt::Global()->MissingCameo.data());
		_strlwr_s(pFilename);

		if (!_stricmp(pCameoRef->Filename, "xxicon.shp")
			&& (PhobosCRT::stristr(pFilename, ".pcx")
				|| PhobosCRT::stristr(pFilename, ".png")))
		{

			BSurface* pCXSurf = nullptr;

			if (PCX::Instance->LoadFile(pFilename))
				pCXSurf = PCX::Instance->GetSurface(pFilename);

			if (pCXSurf)
			{
				GET(int, destX, ESI);
				GET(int, destY, EBP);

				RectangleStruct bounds = { destX, destY, 60, 48 };
				PCX::Instance->BlitToSurface(&bounds, DSurface::Sidebar, pCXSurf);

				return 0x6A9A43; //skip drawing shp cameo
			}
		}
	}

	return 0;
}