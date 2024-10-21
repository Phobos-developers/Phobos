#include <Helpers/Macro.h>
#include <PCX.h>
#include <FileFormats/SHP.h>
#include <Ext/Rules/Body.h>
#include <LoadProgressManager.h>

DEFINE_HOOK(0x6B9D9C, RGB_PCX_Loader, 0x7)
{
	GET(BSurface*, pSurf, EDI);

	return pSurf->BytesPerPixel == 2 ? 0x6B9EE7 : 0;
}

DEFINE_HOOK(0x5535D0, LoadProgressMgr_Draw_PCXLoadingScreen, 0x6)
{
	LEA_STACK(char*, name, 0x84);

	char pFilename[0x20];
	strcpy_s(pFilename, name);
	_strlwr_s(pFilename);

	int ScreenWidth = *(int*)0x8A00A4;
	BSurface* pcx = nullptr;

	sprintf_s(Phobos::readBuffer, GameStrings::LSSOBS_SHP() /* "ls%sobs.shp" */,
		ScreenWidth != 640 ? GameStrings::_800() /* "800" */ : GameStrings::_640() /* "640" */);
	if (!_stricmp(pFilename, Phobos::readBuffer))
	{
		sprintf_s(Phobos::readBuffer, "ls%sobs.pcx",
			ScreenWidth != 640 ? GameStrings::_800() : GameStrings::_640());
		PCX::Instance->LoadFile(Phobos::readBuffer);
		pcx = PCX::Instance->GetSurface(Phobos::readBuffer);
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

			pSurf->CopyFrom(&pSurfBounds, &destClip, pcx, &pcxBounds, &pcxBounds, true, true);
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
		PCX::Instance->LoadFile(filename);

		if (auto const pPCX = PCX::Instance->GetSurface(filename))
		{
			GET_BASE(DSurface*, pSurface, 0x60);

			RectangleStruct pSurfBounds = { 0, 0, pSurface->Width, pSurface->Height };
			RectangleStruct pcxBounds = { 0, 0, pPCX->Width, pPCX->Height };
			RectangleStruct destClip = { (pSurface->Width - pPCX->Width) / 2, (pSurface->Height - pPCX->Height) / 2, pPCX->Width, pPCX->Height };

			pSurface->CopyFrom(&pSurfBounds, &destClip, pPCX, &pcxBounds, &pcxBounds, true, true);
		}

		return 0x552FFF;
	}

	return 0;
}

DEFINE_HOOK(0x553076,LoadProgressMgr_Draw_ExtraText_Campaign,0x5)
{
	GET(LoadProgressManager*, self, EBP);
	if(Phobos::Config::NoSaveLoad)
	{
		auto surface = static_cast<DSurface*>(self->ProgressSurface); // just for borrowing wrappers
		auto* rectC = reinterpret_cast<RectangleStruct*>(&self->field_C);
		Point2D pos
		{
			rectC->X + rectC->Width - 100,
			rectC->Y + 10
		};
		LEA_STACK(RectangleStruct*, pBnd, STACK_OFFSET(0x1268,-0x1204));
		if(auto logo = FileSystem::LoadSHPFile("hardcorelogo.shp"))
		{
			surface->DrawSHP(FileSystem::PALETTE_PAL,logo,0,&pos,pBnd,BlitterFlags::bf_400,0,0,ZGradient::Ground,1000,0,nullptr,0,0,0);
		}
		else
		{
			auto msg = GeneralUtils::LoadStringUnlessMissing("TXT_HARDCORE_MODE",L"HARDCORE");
			surface->DrawTextA(msg,&pos,COLOR_RED);
		}
	}
	return 0;
}

DEFINE_HOOK(0x6A99F3, StripClass_Draw_DrawMissing, 0x6)
{
	GET_STACK(SHPStruct*, pCameo, STACK_OFFSET(0x48C, -0x444));

	if (pCameo)
	{
		auto pCameoRef = pCameo->AsReference();
		char pFilename[0x20];
		strcpy_s(pFilename, RulesExt::Global()->MissingCameo.data());
		_strlwr_s(pFilename);

		if (!_stricmp(pCameoRef->Filename, GameStrings::XXICON_SHP)
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
