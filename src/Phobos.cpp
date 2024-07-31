#include "Phobos.h"

#include <Drawing.h>

#include <Utilities/Debug.h>
#include <Utilities/Patch.h>
#include <Utilities/Macro.h>

#include "Utilities/AresHelper.h"

#ifndef IS_RELEASE_VER
bool HideWarning = false;
#endif

HANDLE Phobos::hInstance = 0;

char Phobos::readBuffer[Phobos::readLength];
wchar_t Phobos::wideBuffer[Phobos::readLength];
const char Phobos::readDelims[4] = ",";

const char* Phobos::AppIconPath = nullptr;
unsigned int Phobos::Phobos_latest_Version[4] = {};


bool Phobos::DisplayDamageNumbers = false;
bool Phobos::IsLoadingSaveGame = false;

#ifdef STR_GIT_COMMIT
const wchar_t* Phobos::VersionDescription = L"Phobos nightly build (" STR_GIT_COMMIT L" @ " STR_GIT_BRANCH L"). DO NOT SHIP IN MODS!";
#elif !defined(IS_RELEASE_VER)
const wchar_t* Phobos::VersionDescription = L"Phobos development build #" _STR(BUILD_NUMBER) L". Please test the build before shipping.";
#else
//const wchar_t* Phobos::VersionDescription = L"Phobos release build v" FILE_VERSION_STR L".";
#endif

struct scoped_handle
{
    scoped_handle(HINTERNET handle) : handle(handle) {}
    ~scoped_handle() { InternetCloseHandle(handle); }

    operator HINTERNET() const { return handle; }

private:
    HINTERNET handle;
};

void Phobos::CmdLineParse(char** ppArgs, int nNumArgs)
{
	bool foundInheritance = false;
	bool foundInclude = false;

	// > 1 because the exe path itself counts as an argument, too!
	for (int i = 1; i < nNumArgs; i++)
	{
		const char* pArg = ppArgs[i];

		if (_stricmp(pArg, "-Icon") == 0)
		{
			Phobos::AppIconPath = ppArgs[++i];
		}
#ifndef IS_RELEASE_VER
		if (_stricmp(pArg, "-b=" _STR(BUILD_NUMBER)) == 0)
		{
			HideWarning = true;
		}
#endif
		if (_stricmp(pArg, "-Inheritance") == 0)
		{
			foundInheritance = true;
		}
		if (_stricmp(pArg, "-Include") == 0)
		{
			foundInclude = true;
		}
	}

	if (foundInclude)
	{
		Patch::Apply_RAW(0x474200, // Apply CCINIClass_ReadCCFile1_DisableAres
			{ 0x8B, 0xF1, 0x8D, 0x54, 0x24, 0x0C }
		);

		Patch::Apply_RAW(0x474314, // Apply CCINIClass_ReadCCFile2_DisableAres
			{ 0x81, 0xC4, 0xA8, 0x00, 0x00, 0x00 }
		);
	}
	else
	{
		Patch::Apply_RAW(0x474230, // Revert CCINIClass_Load_Inheritance
			{ 0x8B, 0xE8, 0x88, 0x5E, 0x40 }
		);
	}

	if (foundInheritance)
	{
		Patch::Apply_RAW(0x528A10, // Apply INIClass_GetString_DisableAres
			{ 0x83, 0xEC, 0x0C, 0x33, 0xC0 }
		);

		Patch::Apply_RAW(0x526CC0, // Apply INIClass_GetKeyName_DisableAres
			{ 0x8B, 0x54, 0x24, 0x04, 0x83, 0xEC, 0x0C }
		);
	}
	else
	{
		Patch::Apply_RAW(0x528BAC, // Revert INIClass_GetString_Inheritance_NoEntry
			{ 0x8B, 0x7C, 0x24, 0x2C, 0x33, 0xC0, 0x8B, 0x4C, 0x24, 0x28 }
		);
	}

	Debug::Log("Initialized version: " PRODUCT_VERSION "\n");
}


void Phobos::PhobosCheckUpdate()
{
    const scoped_handle handle = InternetOpen(TEXT("Phobos"), INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
    if (handle == nullptr)
        return;

    constexpr auto api_url = TEXT("https://api.github.com/repos/Phobos-developers/Phobos/tags"); // This is the API URL

    const scoped_handle request = InternetOpenUrl(handle, api_url, nullptr, 0, INTERNET_FLAG_RELOAD | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_NO_CACHE_WRITE, 0);
    if (request == nullptr)
        return;
    DWORD timeout = 2000;
    InternetSetOption(request, INTERNET_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout));
    InternetSetOption(request, INTERNET_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout));

    char response_data[128];
    if (DWORD len = 0; InternetReadFile(request, response_data, sizeof(response_data) - 1, &len) && len > 0)
    {
        response_data[len] = '\0';

        const char *version_major_offset = std::strchr(response_data, 'v');
        if (version_major_offset == nullptr) return; else version_major_offset++;
        const char *version_minor_offset = std::strchr(version_major_offset, '.');
        if (version_minor_offset == nullptr) return; else version_minor_offset++;
        const char *version_revision_offset = std::strchr(version_minor_offset, '.');
        if (version_revision_offset == nullptr) return; else version_revision_offset++;
        const char *version_patch_offset = std::strchr(version_revision_offset, '.');
        if (version_patch_offset == nullptr) return; else version_patch_offset++;

        Phobos_latest_Version[0] = static_cast<unsigned int>(std::strtoul(version_major_offset, nullptr, 10));
        Phobos_latest_Version[1] = static_cast<unsigned int>(std::strtoul(version_minor_offset, nullptr, 10));
        Phobos_latest_Version[2] = static_cast<unsigned int>(std::strtoul(version_revision_offset, nullptr, 10));
        Phobos_latest_Version[3] = static_cast<unsigned int>(std::strtoul(version_patch_offset, nullptr, 10));
    }
        Phobos::PhobosUpdateLog();
}

void Phobos::PhobosUpdateLog()
{
if ((Phobos_latest_Version[0] > VERSION_MAJOR) ||
    (Phobos_latest_Version[0] == VERSION_MAJOR && Phobos_latest_Version[1] > VERSION_MINOR) ||
    (Phobos_latest_Version[0] == VERSION_MAJOR && Phobos_latest_Version[1] == VERSION_MINOR && Phobos_latest_Version[2] > VERSION_REVISION) ||
    (Phobos_latest_Version[0] == VERSION_MAJOR && Phobos_latest_Version[1] == VERSION_MINOR && Phobos_latest_Version[2] == VERSION_REVISION && Phobos_latest_Version[3] > VERSION_PATCH))
{
   Debug::Log("A new version is available for you to update. You can access the 'https://github.com/Phobos-developers/Phobos/releases' for the update.\n");
}
else
{
   Debug::Log("Your version is already the latest version, and you can also use other build versions by visiting 'https://github.com/Phobos-developers/Phobos/releases'.\n");
}   
} 

void Phobos::ExeRun()
{
	Patch::ApplyStatic();

#ifdef DEBUG

	if (Phobos::DetachFromDebugger())
	{
		MessageBoxW(NULL,
		L"You can now attach a debugger.\n\n"

		L"Press OK to continue YR execution.",
		L"Debugger Notice", MB_OK);
	}
	else
	{
		MessageBoxW(NULL,
		L"You can now attach a debugger.\n\n"

		L"To attach a debugger find the YR process in Process Hacker "
		L"/ Visual Studio processes window and detach debuggers from it, "
		L"then you can attach your own debugger. After this you should "
		L"terminate Syringe.exe because it won't automatically exit when YR is closed.\n\n"

		L"Press OK to continue YR execution.",
		L"Debugger Notice", MB_OK);
	}

	if (!Console::Create())
	{
		MessageBoxW(NULL,
		L"Failed to allocate the debug console!",
		L"Debug Console Notice", MB_OK);
	}

#endif
}

void Phobos::ExeTerminate()
{
	Console::Release();
}

// =============================
// hooks

bool __stdcall DllMain(HANDLE hInstance, DWORD dwReason, LPVOID v)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		Phobos::hInstance = hInstance;
	}
	return true;
}

DEFINE_HOOK(0x7CD810, ExeRun, 0x9)
{
	Phobos::ExeRun();
	AresHelper::Init();
	if (Phobos::Config::CheckUpdate_Enable) 
            {
            Phobos::PhobosCheckUpdate();
            }
	return 0;
}
// Avoid confusing the profiler unless really necessary
#ifdef DEBUG
DEFINE_NAKED_HOOK(0x7CD8EA, _ExeTerminate)
{
	// Call WinMain
	SET_REG32(EAX, 0x6BB9A0);
	CALL(EAX);
	PUSH_REG(EAX);

	__asm {call Phobos::ExeTerminate};

	// Jump back
	POP_REG(EAX);
	SET_REG32(EBX, 0x7CD8EF);
	__asm {jmp ebx};
}
#endif
DEFINE_HOOK(0x52F639, _YR_CmdLineParse, 0x5)
{
	GET(char**, ppArgs, ESI);
	GET(int, nNumArgs, EDI);

	Phobos::CmdLineParse(ppArgs, nNumArgs);
	Debug::LogDeferredFinalize();
	return 0;
}

DEFINE_HOOK(0x67E44D, LoadGame_SetFlag, 0x5)
{
	Phobos::IsLoadingSaveGame = true;
	return 0;
}

DEFINE_HOOK(0x67E68A, LoadGame_UnsetFlag, 0x5)
{
	Phobos::IsLoadingSaveGame = false;
	return 0;
}

#ifndef IS_RELEASE_VER
DEFINE_HOOK(0x4F4583, GScreenClass_DrawText, 0x6)
{
#ifndef STR_GIT_COMMIT
	if (!HideWarning)
#endif // !STR_GIT_COMMIT
	{
		auto wanted = Drawing::GetTextDimensions(Phobos::VersionDescription, { 0,0 }, 0, 2, 0);

		RectangleStruct rect = {
			DSurface::Composite->GetWidth() - wanted.Width - 10,
			0,
			wanted.Width + 10,
			wanted.Height + 10
		};

		Point2D location { rect.X + 5,5 };

		DSurface::Composite->FillRect(&rect, COLOR_BLACK);
		DSurface::Composite->DrawText(Phobos::VersionDescription, &location, COLOR_RED);
	}
	return 0;
}
#endif
