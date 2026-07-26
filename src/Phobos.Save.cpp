#include "Phobos.h"

#include <filesystem>

#include <ScenarioClass.h>
#include <SessionClass.h>
#include <MessageListClass.h>
#include <HouseClass.h>
#include <GameOptionsClass.h>

#include <Utilities/Parser.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/Macro.h>
#include <Utilities/Debug.h>

bool Phobos::ShouldSave = false;
std::wstring Phobos::CustomGameSaveDescription {};

void Phobos::ScheduleGameSave(const std::wstring& description)
{
	ScenarioClass::WasGameSaved = false;
	Phobos::ShouldSave = true;

	if (SessionClass::IsCampaign())
		Phobos::CustomGameSaveDescription = ScenarioClass::Instance->UINameLoaded;
	else
		Phobos::CustomGameSaveDescription = ScenarioClass::Instance->Name;
	Phobos::CustomGameSaveDescription += L" - ";
	Phobos::CustomGameSaveDescription += description;
}

void Phobos::PassiveSaveGame()
{
	auto PrintMessage = [](const wchar_t* pMessage)
	{
		MessageListClass::Instance.PrintMessage(
			pMessage,
			RulesClass::Instance->MessageDelay,
			HouseClass::CurrentPlayer->ColorSchemeIndex,
			/* bSilent: */ true
		);

		// Force a redraw so that our message gets printed.
		if (Game::SpecialDialog == 0)
		{
			MapClass::Instance.MarkNeedsRedraw(2);
			MapClass::Instance.Render();
		}
	};

	PrintMessage(StringTable::LoadString(GameStrings::TXT_SAVING_GAME));
	char fName[0x80];

	if (SessionClass::IsSingleplayer())
	{
		SYSTEMTIME time;
		GetLocalTime(&time);

		_snprintf_s(fName, sizeof(fName), "Map.%04u%02u%02u-%02u%02u%02u-%05u.sav",
			time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);
	}
	else if (SessionClass::IsMultiplayer())
	{
		// Support for this is in the YRpp Spawner, be sure to read the respective comments

		_snprintf_s(fName, sizeof(fName), GameStrings::SAVEGAME_NET);
	}

	if (ScenarioClass::SaveGame(fName, Phobos::CustomGameSaveDescription.c_str()))
		PrintMessage(StringTable::LoadString(GameStrings::TXT_GAME_WAS_SAVED));
	else
		PrintMessage(StringTable::LoadString(GameStrings::TXT_ERROR_SAVING_GAME));

	Phobos::ShouldSave = false;
	Phobos::CustomGameSaveDescription.clear();
}

#define SVGM_XXX_NET "SVGM_XXX.NET"
#define SVGM_FORMAT "SVGM_%03d.NET"
#define SVGM_MAX 999

namespace SaveGameTemp
{
	int NextSaveIndex = 0;
	bool IsSavingMPSave = false;
}

// If we load the same game multiple times, we have to NOT reset the save index,
// so we can continue saving to the next free index. Consistent with XNA CnCNet Client.
DEFINE_HOOK(0x67E6DA, LoadGame_AfterInit_DetermineNextMPSaveIndex, 0x6)
{
	if (SessionClass::IsSingleplayer())
		return 0;

	GET(const char* const, fileName, ESI);

	namespace fs = std::filesystem;

	try
	{
		fs::path savePath(fileName);
		fs::path saveDir = savePath.parent_path();

		// Check all possible save files starting from the highest index down to 0
		for (int i = SVGM_MAX; i >= 0; --i)
		{
			char buf[sizeof(SVGM_XXX_NET)];
			std::snprintf(buf, sizeof(buf), SVGM_FORMAT, i);
			fs::path svgmPath = saveDir / buf;

			if (fs::exists(svgmPath))
			{
				Debug::Log("Found existing MP save file: %s\n", svgmPath.string().c_str());
				SaveGameTemp::NextSaveIndex = std::min(i + 1, SVGM_MAX); // Next index to use
				break;
			}
		}

		Debug::Log("Determined latest MP save index: %d\n", SaveGameTemp::NextSaveIndex);
	}
	catch (const std::exception& e)
	{
		Debug::Log("Failed to determine next save index: %s\n", e.what());
	}

	return 0;
}

// Existing XNA CNCNet Client can't handle renaming savegames when they
// are being saved too fast, which may happen when quicksaving f.ex., hence
// we do this here. Hooked at low level saving function for better compatibility
// with other DLLs that may save MP games, like the spawner itself.
// - Kerbiter
DEFINE_HOOK(0x67CEF0, ScenarioClass_SaveGame_AdjustMPSaveFileName, 0x6)
{
	GET(const char* const, fileName, ECX);

	// SAVEGAME.NET -> SVGM_XXX.NET
	if (_strcmpi(fileName, GameStrings::SAVEGAME_NET) == 0 || _strcmpi(fileName, "SAVEGAME.NET") == 0)
	{
		static char newFileName[sizeof(SVGM_XXX_NET)];
		_snprintf_s(newFileName, sizeof(newFileName), SVGM_FORMAT, SaveGameTemp::NextSaveIndex);

		R->ECX(newFileName);

		Debug::Log("Changed multiplayer save file name from %s to %s\n", fileName, newFileName);
		SaveGameTemp::IsSavingMPSave = true; // Set this so that we cang increment the save index later
	}

	return 0;
}

// This hook is very strategically placed so that it is called
// after spawner changes the directory, so we can also copy the
// spawn.ini to [savegame dir]/spawnSG.ini and cleanup old files.
// This also replicates XNA CNCNet Client behavior.
// - Kerbiter
DEFINE_HOOK(0x67CF16, ScenarioClass_SaveGame_CopySpawnIni, 0x5)
{
	// We only want to do this when saving a multiplayer game the first time
	if (!SaveGameTemp::IsSavingMPSave || SaveGameTemp::NextSaveIndex != 0)  // Not incremented yet so check against 0
		return 0;

	GET(const char* const, fileName, EDI);

	namespace fs = std::filesystem;

	try
	{
		fs::path spawnIni = "spawn.ini";

		// Parse the save file path and replace filename with spawnSG.ini
		fs::path savePath(fileName);
		fs::path saveDir = savePath.parent_path();

		if (fs::exists(spawnIni))
			fs::copy_file(spawnIni,saveDir / "spawnSG.ini", fs::copy_options::overwrite_existing);

		// Clean up old network save files
		for (int i = 0; i <= SVGM_MAX; ++i)
		{
			char buf[sizeof(SVGM_XXX_NET)];
			std::snprintf(buf, sizeof(buf), SVGM_FORMAT, i);
			fs::path svgmPath = saveDir / buf;

			if (fs::exists(svgmPath))
				fs::remove(svgmPath);
		}
		Debug::Log("Copied spawn.ini to %s/spawnSG.ini and cleaned up previous network saves\n", saveDir.string().c_str());
	}
	catch (const std::exception& e)
	{
		Debug::Log("Failed to copy spawn.ini and cleanup previous network saves: %s\n", e.what());
	}

	return 0;
}

// Only increment after it's confirmed that the file is created to mimic
// the behavior that XNA CNCNet Client has and expects.
DEFINE_HOOK(0x67CF64, ScenarioClass_SaveGame_IncrementMPSaveIndex, 0x7)
{
	if (SaveGameTemp::IsSavingMPSave)
	{
		// consistent with XNA CNCNet Client. don't ask me - Kerbiter
		SaveGameTemp::NextSaveIndex = std::min(SaveGameTemp::NextSaveIndex + 1, SVGM_MAX);
		SaveGameTemp::IsSavingMPSave = false; // Reset this so that we don't increment again
	}

	return 0;
}

DEFINE_HOOK(0x55DBCD, MainLoop_SaveGame, 0x6)
{
	// This happens right before LogicClass::Update()
	enum { SkipSave = 0x55DC99, InitialSave = 0x55DBE6 };

	if (!ScenarioClass::WasGameSaved)
	{
		ScenarioClass::WasGameSaved = true;
		if (Phobos::ShouldSave)
			Phobos::PassiveSaveGame();
		else if (Phobos::Config::SaveGameOnScenarioStart && SessionClass::IsCampaign())
			return InitialSave;
	}

	return SkipSave;
}

#undef SVGM_MAX
#undef SVGM_FORMAT
#undef SVGM_XXX_NET
