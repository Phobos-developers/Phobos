#pragma once

struct TheaterProto
{
	char ID[0x40];
	char UIName[0x20];

	char ControlFileName[0x80];

	char PaletteOverlay[0x80];
	char PaletteUnit[0x80];
	char PaletteISO[0x80];

	char Extension[4];
	char Letter[2];

	char SpecificMixFiles[0x80];

	bool IsArctic;
	float RadarBrightness;

	// static
	static TheaterProto Array[];
	static const int ArraySize;

	static TheaterProto* Temperate;
	static TheaterProto* Snow;
	static TheaterProto* Urban;
	static TheaterProto* Desert;
	static TheaterProto* NewUrban;
	static TheaterProto* Lunar;

	static int FindIndex(const char* id);
	static TheaterProto* Get(const char* name);
};
