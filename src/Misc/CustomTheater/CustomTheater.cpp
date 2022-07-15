#include "CustomTheater.h"

#include <Ext/Scenario/Body.h>
#include <MixFileClass.h>
#include "TheaterProto.h"

std::unique_ptr<CustomTheater> CustomTheater::Instance = std::make_unique<CustomTheater>();

void CustomTheater::Init()
{
	const char* id = ScenarioExt::Global()->CustomTheaterID.data();
	Debug::Log("Initialized CustomTheater: %s\n", id);

	// Just like the vanilla game, we don't reload the theater as it's an expensive operation
	if (_stricmp(this->ID, id) == 0)
		return;

	strcpy_s(this->ID, id);
	sprintf_s(this->TheaterFileName, "%s.Theater.ini", this->ID);

	auto pProto = TheaterProto::Get(this->ID);
	this->LoadFromProto(pProto);

	this->UnloadMIXes();
	if (auto pConfig = Phobos::OpenConfig(this->TheaterFileName))
	{
		this->LoadFromINIFile(pConfig);
		this->LoadMIXesFromINIFile(pConfig);
		Phobos::CloseConfig(pConfig);
	}
	else
	{
		this->LoadMIXesFromProto(pProto);
	}

	this->Patch();

	Debug::Log("\tLoad [SpecificMixes]\n");
	for (auto pMix : this->SpecificMixFiles)
		Debug::Log("\t\t%s\n", pMix->FileName);

	ScenarioClass::Instance->Theater = this->Slot;
}

void CustomTheater::Patch()
{
	// These variables are used many times throughout the game's code
	// Therefore, it is easier to overwrite their values ​​than to change all pointers to them
	// Also causing a problem is the fact that they are also used in Ares

	// Makes memory available for writing, if not already done
	static DWORD protectFlag = 0;
	if (protectFlag == 0)
		VirtualProtect(Theater::Array, sizeof(Theater) * 6, PAGE_EXECUTE_READWRITE, &protectFlag);

	//To avoid a name conflict, first we zero out the names in both slots
	Theater::Array[0].ID[0] = 0;
	Theater::Array[1].ID[0] = 0;

	auto slot = Theater::Get(this->Slot);

	// The new ID has a larger size 64 vs 16 in the vanilla variable
	// This is us using the memory that was occupied by values that are no longer used
	memcpy(slot->ID, this->ID, sizeof(this->ID));
	strcpy_s(slot->Extension, this->Extension);
	slot->Letter[0] = this->Letter[0];

	// This variable is only used on 0x47C318, but it's convenient to set it here
	slot->RadarTerrainBrightness = this->RadarBrightness;
}

void CustomTheater::LoadFromProto(TheaterProto* pTheaterProto)
{
	strcpy_s(this->ControlFileName, pTheaterProto->ControlFileName);

	strcpy_s(this->PaletteOverlay, pTheaterProto->PaletteOverlay);
	strcpy_s(this->PaletteUnit, pTheaterProto->PaletteUnit);
	strcpy_s(this->PaletteISO, pTheaterProto->PaletteISO);

	strcpy_s(this->Extension, pTheaterProto->Extension);
	strcpy_s(this->Letter, pTheaterProto->Letter);

	this->Slot = pTheaterProto->IsArctic ? TheaterType::Snow : TheaterType::Temperate;
	this->RadarBrightness = pTheaterProto->RadarBrightness;
}

void CustomTheater::LoadFromINIFile(CCINIClass* pINI)
{
	INI_EX exINI(pINI);

	Debug::Log("\tLoad TheaterFile = %s\n", this->TheaterFileName);
	const char* pSection = "Theater";

	pINI->ReadString(pSection, "ControlFileName", this->ControlFileName, this->ControlFileName);
	if (_strcmpi(this->ControlFileName, "<self>") == 0)
		strcpy_s(this->ControlFileName, this->TheaterFileName);

	pINI->ReadString(pSection, "PaletteOverlay", this->PaletteOverlay, this->PaletteOverlay);
	pINI->ReadString(pSection, "PaletteUnit", this->PaletteUnit, this->PaletteUnit);
	pINI->ReadString(pSection, "PaletteISO", this->PaletteISO, this->PaletteISO);

	pINI->ReadString(pSection, "Extension", this->Extension, this->Extension);
	pINI->ReadString(pSection, "Letter", this->Letter, this->Letter);

	this->Slot = pINI->ReadBool(pSection, "IsArctic", (bool)this->Slot) ? TheaterType::Snow : TheaterType::Temperate;
	this->RadarBrightness = (float) pINI->ReadDouble(pSection, "RadarBrightness", this->RadarBrightness);
}

void CustomTheater::LoadMIXesFromProto(TheaterProto* pTheaterProto)
{
	char* context = nullptr;
	strcpy_s(Phobos::readBuffer, pTheaterProto->SpecificMixFiles);
	char* fileName = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context);
	while (fileName)
	{
		auto pFile = GameCreate<CCFileClass>(fileName);
		bool isExists = pFile->Exists();
		GameDelete(pFile);

		if (isExists)
			this->SpecificMixFiles.AddItem(GameCreate<MixFileClass>(fileName));

		fileName = strtok_s(nullptr, Phobos::readDelims, &context);
	}
}

void CustomTheater::LoadMIXesFromINIFile(CCINIClass* pINI)
{
	const char* pSection = "SpecificMixes";
	const int itemsCount = pINI->GetKeyCount(pSection);
	if (itemsCount <= 0)
	{
		this->LoadMIXesFromProto(TheaterProto::Get(this->ID));
		return;
	}

	for (int i = 0; i < itemsCount; ++i)
	{
		auto pKey = pINI->GetKeyName(pSection, i);

		char fileName[0x80];
		pINI->ReadString(pSection, pKey, "<none>", fileName);

		if (fileName[0] != 0 || _strcmpi(fileName, "<none>") != 0)
		{
			auto pFile = GameCreate<CCFileClass>(fileName);
			bool isExists = pFile->Exists();
			GameDelete(pFile);

			if (isExists)
				this->SpecificMixFiles.AddItem(GameCreate<MixFileClass>(fileName));

		}
	}
}

void CustomTheater::UnloadMIXes()
{
	for (auto pMixFile : this->SpecificMixFiles)
	{
		GameDelete(pMixFile);
	}
	this->SpecificMixFiles.Clear();
	MixFileClass::DestroyCache();
}
