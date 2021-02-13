#include <Helpers/Macro.h>
#include <PCX.h>

DEFINE_HOOK(6B9D9C, RGB_PCX_Loader, 7)
{
	GET(BSurface*, pSurf, EDI);
	if (pSurf->BytesPerPixel == 2) {
		return 0x6B9EE7;
	}
	return 0;
}

DEFINE_HOOK(5535D0, PCX_LoadScreen, 6)
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

			pSurf->Blit(&pSurfBounds, &destClip, pcx, &pcxBounds, &pcxBounds, true, true);
		}
		return 0x553603;
	}
	return 0;
}
