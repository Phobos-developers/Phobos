#pragma once
#include <Utilities/Constructs.h>

class MixFileClass;
struct TheaterProto;

class CustomTheater
{
public:

	// properties:
	char ID[0x40];

	char TheaterFileName[0x80];
	char ControlFileName[0x80];

	char PaletteOverlay[0x80];
	char PaletteUnit[0x80];
	char PaletteISO[0x80];

	char Extension[4];
	char Letter[2];

	TheaterType Slot;
	float RadarBrightness;
private:
	DynamicVectorClass<MixFileClass*> SpecificMixFiles;

public:
	// methods:
	void Init();

	void LoadFromProto(TheaterProto* pTheaterProto);
	void LoadFromINIFile(CCINIClass* pINI);
	void LoadMIXesFromProto(TheaterProto* pTheaterProto);
	void LoadMIXesFromINIFile(CCINIClass* pINI);
	void UnloadMIXes();

	static std::unique_ptr<CustomTheater> Instance;
private:
	void Patch();
};
