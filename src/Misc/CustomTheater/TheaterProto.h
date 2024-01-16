#pragma once

struct TheaterProto
{
	char ID[0x40];
	char UIName[0x20];

	char TerrainControl[0x80];
	bool IsArctic;

	char PaletteISO[0x80];
	char PaletteOverlay[0x80];
	char PaletteUnit[0x80];

	char Extension[4];
	char Letter[2];

	float RadarBrightness;
	char SpecificMixes[0x80];

	// static
	static TheaterProto Array[];
	static const int ArraySize;

	static TheaterProto* Temperate;
	static TheaterProto* Snow;
	static TheaterProto* Urban;
	static TheaterProto* Desert;
	static TheaterProto* NewUrban;
	static TheaterProto* Lunar;

	static int FindIndex(const char* id, int iDefault = -1);
	static TheaterProto* Get(const char* name, TheaterProto* nDefault = TheaterProto::Temperate);
};
