#include "CustomTheater.h"
#include <Ext/Scenario/Body.h>

std::unique_ptr<CustomTheater> CustomTheater::Instance = std::make_unique<CustomTheater>();

void CustomTheater::Init(const char* id)
{
	Debug::Log("Initialized CustomTheater: %s\n", id);
	Game::SetProgress(8);

	// Just like the vanilla game, we don't reload the theater as it's an expensive operation
	if (_stricmp(this->ID, id) != 0)
	{
		ScenarioClass::LastTheater = TheaterType::None; // Causes the game to reload tiles files

		strcpy_s(this->ID, id);
		sprintf_s(this->TheaterFileName, "%s.theater.ini", this->ID);

		auto pProto = TheaterProto::Get(this->ID);
		this->LoadFromProto(pProto);

		this->UnloadMIXes();
		MixFileClass::DestroyCache();
		Game::SetProgress(6);
		if (auto pConfig = CCINIClass::LoadINIFile(this->TheaterFileName))
		{
			this->LoadFromINIFile(pConfig);
			this->LoadMIXesFromINIFile(pConfig);
			CCINIClass::UnloadINIFile(pConfig);
		}
		else
		{
			this->LoadMIXesFromProto(pProto);
		}

		this->Patch();

		Game::SetProgress(12);

		Debug::Log("Loading SpecificMixes\n");
		for (auto pMix : this->SpecificMixes)
			Debug::Log("\t%s\n", pMix->FileName);
	}

	ScenarioClass::Instance->Theater = this->Slot;
}

void CustomTheater::Patch()
{
	// These variables are used many times throughout the game's code.
	// Therefore, it is easier to overwrite their values ​​than to change all pointers to them.
	// Also causing a problem is the fact that they are also used in Ares

	// Makes memory available for writing, if not already done
	static DWORD protectFlag = 0;
	if (protectFlag == 0)
		VirtualProtect(Theater::Array, sizeof(Theater) * 6, PAGE_EXECUTE_READWRITE, &protectFlag);

	// To avoid a name conflict, we must clear the name of the first slot
	if (this->Slot == TheaterType::Snow)
		Theater::Array[0].ID[0] = 0;

	// The new ID has a larger size 64 vs 16 in the vanilla variable
	// This is us using the memory that was occupied by values that are no longer used
	auto slot = Theater::Get(this->Slot);
	memcpy(slot->ID, this->ID, sizeof(this->ID));
	strcpy_s(slot->Extension, this->Extension);
	slot->Letter[0] = this->Letter[0];

	// This variable is only used on 0x47C318, but it's convenient to set it here
	slot->RadarTerrainBrightness = this->RadarBrightness;
}

void CustomTheater::LoadFromProto(TheaterProto* pTheaterProto)
{
	strcpy_s(this->TerrainControl, pTheaterProto->TerrainControl);
	strcpy_s(this->ExtendedRules, NONE_STR);

	strcpy_s(this->PaletteISO, pTheaterProto->PaletteISO);
	strcpy_s(this->PaletteOverlay, pTheaterProto->PaletteOverlay);
	strcpy_s(this->PaletteUnit, pTheaterProto->PaletteUnit);

	strcpy_s(this->Extension, pTheaterProto->Extension);
	strcpy_s(this->Letter, pTheaterProto->Letter);

	this->Slot = pTheaterProto->IsArctic ? TheaterType::Snow : TheaterType::Temperate;
	this->RadarBrightness = pTheaterProto->RadarBrightness;
}

void CustomTheater::LoadFromINIFile(CCINIClass* pINI)
{
	const char* pSection = "Theater";

	pINI->ReadString(pSection, "TerrainControl", this->TerrainControl, this->TerrainControl);
	if (_strcmpi(this->TerrainControl, "<self>") == 0)
		strcpy_s(this->TerrainControl, this->TheaterFileName);

	pINI->ReadString(pSection, "ExtendedRules", this->ExtendedRules, this->ExtendedRules);
	if (_strcmpi(this->ExtendedRules, "<self>") == 0)
		strcpy_s(this->ExtendedRules, this->TheaterFileName);

	this->Slot = pINI->ReadBool(pSection, "IsArctic", (bool)this->Slot) ? TheaterType::Snow : TheaterType::Temperate;

	pINI->ReadString(pSection, "PaletteISO", this->PaletteISO, this->PaletteISO);
	pINI->ReadString(pSection, "PaletteOverlay", this->PaletteOverlay, this->PaletteOverlay);
	pINI->ReadString(pSection, "PaletteUnit", this->PaletteUnit, this->PaletteUnit);

	pINI->ReadString(pSection, "Extension", this->Extension, this->Extension);
	pINI->ReadString(pSection, "Letter", this->Letter, this->Letter);

	this->RadarBrightness = (float)pINI->ReadDouble(pSection, "RadarBrightness", this->RadarBrightness);
}

void CustomTheater::LoadMIXesFromProto(TheaterProto* pTheaterProto)
{
	char* context = nullptr;
	strcpy_s(Phobos::readBuffer, pTheaterProto->SpecificMixes);
	char* fileName = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context);
	while (fileName)
	{
		auto pFile = CCFileClass(fileName);
		if (pFile.Exists())
			this->SpecificMixes.AddItem(GameCreate<MixFileClass>(fileName));

		fileName = strtok_s(nullptr, Phobos::readDelims, &context);
	}
}

void CustomTheater::LoadMIXesFromINIFile(CCINIClass* pINI)
{
	const char* pSection = "SpecificMixes";

	if (!pINI->GetSection(pSection))
	{
		this->LoadMIXesFromProto(TheaterProto::Get(this->ID));
		return;
	}

	const int itemsCount = pINI->GetKeyCount(pSection);
	for (int i = 0; i < itemsCount; ++i)
	{
		auto pKey = pINI->GetKeyName(pSection, i);

		char fileName[0x80];
		pINI->ReadString(pSection, pKey, NONE_STR, fileName);

		if (fileName[0] != 0 || _strcmpi(fileName, NONE_STR) != 0)
		{
			auto pFile = CCFileClass(fileName);
			if (pFile.Exists())
				this->SpecificMixes.AddItem(GameCreate<MixFileClass>(fileName));

		}
	}
}

void CustomTheater::UnloadMIXes()
{
	if (this->SpecificMixes.Count == 0)
		return;

	for (auto pMixFile : this->SpecificMixes)
		GameDelete(pMixFile);

	this->SpecificMixes.Clear();
}
