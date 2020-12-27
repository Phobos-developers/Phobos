#include <StaticInits.cpp>

#include "Phobos.h"

#ifndef IS_RELEASE_VER
	bool HideWarning = false;
#endif

char Phobos::readBuffer[Phobos::readLength];
const char Phobos::readDelims[4] = ",";

const char* Phobos::AppIconPath = nullptr;

void Phobos::CmdLineParse(char** ppArgs, int nNumArgs)
{
	// > 1 because the exe path itself counts as an argument, too!
	for (int i = 1; i < nNumArgs; i++)
	{
		const char* pArg = ppArgs[i];

		if (_stricmp(pArg, "-Icon") == 0)
		{
			Phobos::AppIconPath = ppArgs[++i];
		}
		#ifndef IS_RELEASE_VER
		if (_stricmp(pArg, "-b=" VERSION_STR) == 0)
		{
			HideWarning = true;
		}
		#endif
	}
}

CCINIClass* Phobos::OpenConfig(const char* file) {
	CCINIClass* pINI = GameCreate<CCINIClass>();

	if (pINI) {
		CCFileClass* cfg = GameCreate<CCFileClass>(file);

		if (cfg) {
			if (cfg->Exists()) {
				pINI->ReadCCFile(cfg);
			}
			GameDelete(cfg);
		}
	}

	return pINI;
}

void Phobos::CloseConfig(CCINIClass*& pINI) {
	if (pINI) {
		GameDelete(pINI);
		pINI = nullptr;
	}
}

// =============================
// hooks

/*
//DllMain
bool __stdcall DllMain(HANDLE hInstance, DWORD dwReason, LPVOID v)
{
	switch (dwReason) {
	case DLL_PROCESS_ATTACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	}

	return true;
}
*/

DEFINE_HOOK(52F639, _YR_CmdLineParse, 5)
{
	GET(char**, ppArgs, ESI);
	GET(int, nNumArgs, EDI);

	Phobos::CmdLineParse(ppArgs, nNumArgs);
	return 0;
}

#ifndef IS_RELEASE_VER
DEFINE_HOOK(4F4583, GScreenClass_DrawText, 6)
{
	if (!HideWarning) {
		auto string = L"This version of Phobos (Build #" VERSION_STR L") is unstable";
		auto wanted = Drawing::GetTextDimensions(string);

		RectangleStruct rect = {
			DSurface::Composite->GetWidth() - wanted.Width - 10,
			0,
			wanted.Width + 10,
			wanted.Height + 10
		};

		DSurface::Composite->FillRect(&rect, COLOR_BLACK);
		DSurface::Composite->DrawTextA(string, rect.X + 5, 5, COLOR_RED);
	}
	return 0;
}
#endif
