#include <Helpers/Macro.h>
#include <PCX.h>
#include <FileFormats/SHP.h>
#include <Ext/Rules/Body.h>
#include <LoadProgressManager.h>

DEFINE_HOOK(0x6B9D9C, RGB_PCX_Loader, 0x7)
{
	GET(BSurface*, pSurf, EDI);
	if (pSurf->BytesPerPixel == 2) {
		return 0x6B9EE7;
	}
	return 0;
}

DEFINE_HOOK(0x5535D0, PCX_LoadScreen, 0x6)
{
	LEA_STACK(char*, name, 0x84);

	char pFilename[0x20];
	strcpy_s(pFilename, name);
	_strlwr_s(pFilename);

	if (strstr(pFilename, ".pcx")) {
		PCX::Instance->LoadFile(pFilename);

		auto pcx = PCX::Instance->GetSurface(pFilename);
		if (pcx) {
			GET_BASE(DSurface*, pSurf, 0x60);
			RectangleStruct pSurfBounds = { 0, 0, pSurf->Width, pSurf->Height };
			RectangleStruct pcxBounds = { 0, 0, pcx->Width, pcx->Height };

			RectangleStruct destClip = { 0, 0, pcx->Width, pcx->Height };
			destClip.X = (pSurf->Width - pcx->Width) / 2;
			destClip.Y = (pSurf->Height - pcx->Height) / 2;

			pSurf->CopyFrom(&pSurfBounds, &destClip, pcx, &pcxBounds, &pcxBounds, true, true);
		}
		return 0x553603;
	}
	return 0;
}

DEFINE_HOOK(0x552F81, PCX_LoadingScreen_Campaign, 0x5)
{
	GET(LoadProgressManager*, pThis, EBP);

	DSurface* pSurface = static_cast<DSurface*>(pThis->ProgressSurface);
	char filename[0x40];
	strcpy_s(filename, ScenarioClass::Instance->LS800BkgdName);
	_strlwr_s(filename);

	if (strstr(filename, ".pcx"))
	{
		PCX::Instance->LoadFile(filename);

		if (auto pPCX = PCX::Instance->GetSurface(filename))
		{
			RectangleStruct pSurfBounds = { 0, 0, pSurface->Width, pSurface->Height };
			RectangleStruct pcxBounds = { 0, 0, pPCX->Width, pPCX->Height };
			RectangleStruct destClip = { 0, 0, pPCX->Width, pPCX->Height };

			destClip.X = (pSurface->Width - pPCX->Width) / 2;
			destClip.Y = (pSurface->Height - pPCX->Height) / 2;

			pSurface->CopyFrom(&pSurfBounds, &destClip, pPCX, &pcxBounds, &pcxBounds, true, true);
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
			&& strstr(pFilename, ".pcx"))
		{
			PCX::Instance->LoadFile(pFilename);
			if (auto CameoPCX = PCX::Instance->GetSurface(pFilename))
			{
				GET(int, destX, ESI);
				GET(int, destY, EBP);

				RectangleStruct bounds = { destX, destY, 60, 48 };
				PCX::Instance->BlitToSurface(&bounds, DSurface::Sidebar, CameoPCX);

				return 0x6A9A43; //skip drawing shp cameo
			}

		}
	}

	return 0;
}
