#include "Phobos.h"
#include <StaticInits.cpp>

void Phobos::CmdLineParse(char **ppArgs, int nNumArgs)
{

    // > 1 because the exe path itself counts as an argument, too!
    for (int i = 1; i < nNumArgs; i++)
    {
        const char *pArg = ppArgs[i];
    }
}

DEFINE_HOOK(52F639, _YR_CmdLineParse, 5)
{
    GET(char **, ppArgs, ESI);
    GET(int, nNumArgs, EDI);

    Phobos::CmdLineParse(ppArgs, nNumArgs);
    return 0;
}

//DllMain
//bool __stdcall DllMain(HANDLE hInstance, DWORD dwReason, LPVOID v)
//{
//	switch (dwReason) {
//	case DLL_PROCESS_ATTACH:
//		break;
//	case DLL_PROCESS_DETACH:
//		break;
//	}
//
//	return true;
//}
