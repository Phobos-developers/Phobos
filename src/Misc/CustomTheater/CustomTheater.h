#pragma once

#include "TheaterProto.h"

#include <CCINIClass.h>
#include <GeneralDefinitions.h>
#include <MixFileClass.h>

class CustomTheater
{
public:
	// properties:
	char ID[0x40];

	char TerrainControl[0x80];
	char ExtendedRules[0x80];
	TheaterType Slot;

	char PaletteISO[0x80];
	char PaletteOverlay[0x80];
	char PaletteUnit[0x80];

	char Extension[4];
	char Letter[2];

	float RadarBrightness;

private:
	char TheaterFileName[0x80];
	DynamicVectorClass<MixFileClass*> SpecificMixes;

public:
	// methods:
	void Init(const char* id);

	void LoadFromProto(TheaterProto* pTheaterProto);
	void LoadFromINIFile(CCINIClass* pINI);
	void LoadMIXesFromProto(TheaterProto* pTheaterProto);
	void LoadMIXesFromINIFile(CCINIClass* pINI);
	void UnloadMIXes();

	static std::unique_ptr<CustomTheater> Instance;
private:
	void Patch();
};
