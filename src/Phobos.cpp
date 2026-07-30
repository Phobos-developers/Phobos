#include "Phobos.h"

#include <commctrl.h>

#include <Misc/ExceptionHandler.h>

#include <Utilities/Debug.h>
#include <Utilities/Patch.h>
#include <Utilities/Macro.h>
#include "Utilities/AresHelper.h"
#include "Utilities/GeneralUtils.h"
#include "Utilities/Parser.h"

#ifndef IS_RELEASE_VER
bool HideWarning = false;
#endif

HANDLE Phobos::hInstance = 0;

char Phobos::readBuffer[Phobos::readLength];
wchar_t Phobos::wideBuffer[Phobos::readLength];

const char* Phobos::AppIconPath = nullptr;

bool Phobos::DisplayDamageNumbers = false;
bool Phobos::IsLoadingSaveGame = false;

bool Phobos::Optimizations::Applied = false;
bool Phobos::Optimizations::DisableBalloonHoverPathingFix = false;
bool Phobos::Optimizations::DisableRadDamageOnBuildings = true;
bool Phobos::Optimizations::DisableSyncLogging = false;

#ifdef STR_GIT_COMMIT
const wchar_t* Phobos::VersionDescription = L"Phobos nightly build (" STR_GIT_COMMIT L" @ " STR_GIT_BRANCH L"). DO NOT SHIP IN MODS!";
#elif !defined(IS_RELEASE_VER)
const wchar_t* Phobos::VersionDescription = L"Phobos development build #" _STR(BUILD_NUMBER) L". Please test the build before shipping.";
#else
//const wchar_t* Phobos::VersionDescription = L"Phobos release build v" FILE_VERSION_STR L".";
#endif


void Phobos::CmdLineParse(char** ppArgs, int nNumArgs)
{
	bool foundInheritance = false;
	bool foundInclude = false;
	// Enabled by default in all builds: an attached debugger receives
	// exceptions first, so the handler does not get in the way of debugging.
	bool dontSetExceptionHandler = false;
	Parser<bool> boolParser { };

	// > 1 because the exe path itself counts as an argument, too!
	for (int i = 1; i < nNumArgs; i++)
	{
		const char* pArg = ppArgs[i];
		std::string arg = pArg;

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
		if (arg.starts_with("-ExceptionHandler="))
		{
			auto delimIndex = arg.find("=");
			auto value = arg.substr(delimIndex + 1, arg.size() - delimIndex - 1);

			bool v = dontSetExceptionHandler;
			if (boolParser.TryParse(value.c_str(), &v))
				dontSetExceptionHandler = !v;
		}
		if (_stricmp(pArg, "-FullCrashDump") == 0)
		{
			ExceptionHandler::GenerateFullCrashDump = true;
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

	Game::DontSetExceptionHandler = dontSetExceptionHandler;

	// Phobos replaces the game's exception handler with its own (see
	// ExceptionHandler.cpp); it is reachable exactly when the game's main
	// loop handler is armed, so it shares the -ExceptionHandler toggle.
	if (!dontSetExceptionHandler)
		ExceptionHandler::Init();

	Debug::Log("Initialized version: " PRODUCT_VERSION "\n");
	Debug::Log("ExceptionHandler is %s\n", dontSetExceptionHandler ? "not present" : "present");
}

// gamemd.exe has no manifest, so its windows bind the ancient Common Controls
// v5 and render Win9x-style. Activating Phobos' embedded manifest (resource 2,
// carrying the comctl32 v6 dependency - see ExceptionHandler.rc) for the rest
// of the process' lifetime makes every window created on the main thread use
// modern visual styles: game dialogs, message boxes and the crash dialog.
static void ActivateCommonControls6()
{
	char modulePath[MAX_PATH] = { };
	GetModuleFileNameA(static_cast<HMODULE>(Phobos::hInstance), modulePath, sizeof(modulePath));

	ACTCTXA actCtx = { };
	actCtx.cbSize = sizeof(actCtx);
	actCtx.dwFlags = ACTCTX_FLAG_RESOURCE_NAME_VALID;
	actCtx.lpSource = modulePath;
	actCtx.lpResourceName = MAKEINTRESOURCEA(2); // ISOLATIONAWARE_MANIFEST_RESOURCE_ID

	HANDLE const hActCtx = CreateActCtxA(&actCtx);
	ULONG_PTR cookie = 0;
	if (hActCtx != INVALID_HANDLE_VALUE)
		ActivateActCtx(hActCtx, &cookie); // deliberately never deactivated

	// gamemd.exe has no manifest, so it loaded Common Controls v5 at startup
	// and the v6 theming subclasses were never installed. With the context
	// now active, loading comctl32 resolves to the v6 side-by-side assembly;
	// InitCommonControlsEx then registers its themed classes. Without this,
	// activating the manifest alone leaves controls rendering unthemed.
	if (HMODULE const comctl = LoadLibraryA("comctl32.dll"))
	{
		using InitCommonControlsEx_t = BOOL(WINAPI*)(const INITCOMMONCONTROLSEX*);
		if (auto const pInit = reinterpret_cast<InitCommonControlsEx_t>(GetProcAddress(comctl, "InitCommonControlsEx")))
		{
			INITCOMMONCONTROLSEX icc = { sizeof(icc), ICC_STANDARD_CLASSES | ICC_WIN95_CLASSES };
			pInit(&icc);
		}
	}
}

void Phobos::ExeRun()
{
	// SyringeEx sets these exported flags before installing any hooks; under an
	// older Syringe they remain false. Phobos relies on SyringeEx behavior
	// (e.g. relative-instruction relocation in trampolines), so refuse to run without it.
	if (!SyringeFeatures::ESPModification
		|| !SyringeFeatures::ZFPreservation
		|| !SyringeFeatures::ReladdrInstructionFixup)
	{
		MessageBoxW(NULL,
			L"This version of Phobos requires SyringeEx to run, but the game appears "
			L"to have been launched with an older version of Syringe.\n\n"

			L"Please replace Syringe.exe in your game folder with the latest SyringeEx release:\n"
			L"https://github.com/Phobos-developers/SyringeEx/releases",
			L"Phobos - unsupported Syringe version", MB_OK | MB_ICONERROR);

		ExitProcess(1u);
	}

	ActivateCommonControls6();

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
	Phobos::ApplyOptimizations();
	return 0;
}

DEFINE_HOOK(0x683E7F, ScenarioClass_Start_Optimizations, 0x7)
{
	Phobos::ApplyOptimizations();
	return 0;
}

DEFINE_HOOK(0x4F4583, GScreenClass_DrawText, 0x6)
{
	const int marginX = Phobos::Config::MessageDisplayInCenter ? 28 : 10;
	int coordY = 0;

#ifndef IS_RELEASE_VER
#ifndef STR_GIT_COMMIT
	if (!HideWarning)
#endif // !STR_GIT_COMMIT
	{
		auto wanted = Drawing::GetTextDimensions(Phobos::VersionDescription, { 0, 0 }, 0, 2, 0);

		RectangleStruct rect = {
			DSurface::Composite->GetWidth() - wanted.Width - marginX,
			0,
			wanted.Width + 10,
			wanted.Height + 10
		};

		Point2D location { rect.X + 5, 5 };
		DSurface::Composite->FillRect(&rect, COLOR_BLACK);
		DSurface::Composite->DrawText(Phobos::VersionDescription, &location, COLOR_RED);

		// add margin for next text
		coordY = rect.Height;
	}
#endif // !IS_RELEASE_VER

	if (!Phobos::Config::ShowGameTime || HouseClass::CurrentPlayer->IsObserver()) // already has a timer
		return 0;

	wchar_t buffer[0x20] {};
	const auto& timer = ScenarioClass::Instance->ElapsedTimer;
	int currentTime = timer.TimeLeft;

	if (timer.StartTime != -1)
		currentTime += SystemTimer::GetTime() - timer.StartTime;

	currentTime /= 60;
	const int hours = currentTime / 3600;
	const int minutes = (currentTime / 60) % 60;
	const int seconds = currentTime % 60;
	const auto text = GeneralUtils::LoadStringUnlessMissing("TXT_GAMETIME", L"Time:");

	if (hours > 0)
	{
		swprintf(buffer, std::size(buffer), L"%ls %d:%02d:%02d", text, hours, minutes, seconds);
	}
	else
	{
		swprintf(buffer, std::size(buffer), L"%ls %02d:%02d", text, minutes, seconds);
	}

	auto wantedB = Drawing::GetTextDimensions(buffer, { 0, 0 }, 0, 2, 0);

	RectangleStruct rectB = {
		DSurface::Composite->GetWidth() - wantedB.Width - marginX,
		coordY,
		wantedB.Width + 10,
		wantedB.Height + 10
	};

	Point2D locationB { rectB.X + 5, rectB.Y + 5 };
	ColorStruct color { 0x0, 0x0 ,0x0 };
	DSurface::Composite->FillRectTrans(&rectB, &color, Phobos::Config::ShowGameTime_BoardOpacity);
	DSurface::Composite->DrawText(buffer, &locationB, COLOR_WHITE);

	return 0;
}

// Mainly used to disable hooks for optimization.
// Called after loading saved game and at end of scenario start after all INI data etc has been initialized.
// Only executed once per game session.
void Phobos::ApplyOptimizations()
{
	if (Phobos::Optimizations::Applied)
		return;

	// Disable BuildingClass_AI_Radiation
	if (Phobos::Optimizations::DisableRadDamageOnBuildings)
		Patch::Apply_RAW(0x43FB23, { 0x53, 0x55, 0x56, 0x8B, 0xF1 });

	// Disable BalloonHover path finding fix
	if (Phobos::Optimizations::DisableBalloonHoverPathingFix)
	{
		Patch::Apply_RAW(0x64D592, { 0x0F, 0x8F, 0xB8, 0x00, 0x00, 0x00 });
		Patch::Apply_RAW(0x64D575, { 0x0F, 0x8F, 0xD5, 0x00, 0x00, 0x00 });
		Patch::Apply_RAW(0x64D5C5, { 0x8A, 0x44, 0x24, 0x13, 0x84, 0xC0 });
		Patch::Apply_RAW(0x51BFA2, { 0x85, 0x99, 0x40, 0x01, 0x00, 0x00 });
		Patch::Apply_RAW(0x73F0A7, { 0x8B, 0xD9, 0x8B, 0x8C, 0x24, 0x88, 0x00, 0x00, 0x00 });
		Patch::Apply_RAW(0x4D62C0, { 0x8A, 0x88, 0x95, 0x06, 0x00, 0x00 });
	}

	if (!SessionClass::IsMultiplayer())
	{
		// Disable TechnoClass_DeleteGap_CellCheck
		Patch::Apply_RAW(0x6FB5E5, { 0xB9, 0xE8, 0xF7, 0x87, 0x00 });

		// Disable TechnoClass_CreateGap_CellCheck
		Patch::Apply_RAW(0x6FB2FB, { 0xB9, 0xE8, 0xF7, 0x87, 0x00 });

		// Disable MapClass_ResetShroud_CellCheck
		Patch::Apply_RAW(0x577AFF, { 0x8B, 0x86, 0xF4, 0x00, 0x00, 0x00 });

		// Disable MapClass_ResetShroudForTMission_CellCheck
		Patch::Apply_RAW(0x577BF1, { 0x8B, 0x86, 0xF4, 0x00, 0x00, 0x00 });

		// Disable Random2Class_Random_SyncLog
		Patch::Apply_RAW(0x65C7D0, { 0xC3, 0x90, 0x90, 0x90, 0x90 });

		// Disable Random2Class_RandomRanged_SyncLog
		Patch::Apply_RAW(0x65C88A, { 0xC2, 0x08, 0x00, 0x90, 0x90 });

		// Disable FacingClass_Set_SyncLog
		Patch::Apply_RAW(0x4C9300, { 0x83, 0xEC, 0x10, 0x53, 0x56 });

		// Disable InfantryClass_AssignTarget_SyncLog
		Patch::Apply_RAW(0x51B1F0, { 0x53, 0x56, 0x8B, 0xF1, 0x57 });

		// Disable BuildingClass_AssignTarget_SyncLog
		Patch::Apply_RAW(0x443B90, { 0x56, 0x8B, 0xF1, 0x57, 0x83, 0xBE, 0xAC, 0x0, 0x0, 0x0, 0x13 });

		// Disable TechnoClass_AssignTarget_SyncLog
		Patch::Apply_RAW(0x6FCDB0, { 0x83, 0xEC, 0x0C, 0x53, 0x56 });

		// Disable AircraftClass_AssignDestination_SyncLog
		Patch::Apply_RAW(0x41AA80, { 0x53, 0x56, 0x57, 0x8B, 0x7C, 0x24, 0x10 });

		// Disable BuildingClass_AssignDestination_SyncLog
		Patch::Apply_RAW(0x455D50, { 0x56, 0x8B, 0xF1, 0x83, 0xBE, 0xAC, 0x0, 0x0, 0x0, 0x13 });

		// Disable InfantryClass_AssignDestination_SyncLog
		Patch::Apply_RAW(0x51AA40, { 0x83, 0xEC, 0x2C, 0x53, 0x55 });

		// Disable UnitClass_AssignDestination_SyncLog
		Patch::Apply_RAW(0x741970, { 0x81, 0xEC, 0x80, 0x0, 0x0, 0x0 });

		// Disable AircraftClass_OverrideMission_SyncLog
		Patch::Apply_RAW(0x41BB30, { 0x8B, 0x81, 0xAC, 0x0, 0x0, 0x0 });

		// Disable FootClass_OverrideMission_SyncLog
		Patch::Apply_RAW(0x4D8F40, { 0x8B, 0x54, 0x24, 0x4, 0x56 });

		// Disable TechnoClass_OverrideMission_SyncLog
		Patch::Apply_RAW(0x7013A0, { 0x8B, 0x54, 0x24, 0x4, 0x56 });

		Phobos::Optimizations::DisableSyncLogging = true;
	}

	Phobos::Optimizations::Applied = true;
}
