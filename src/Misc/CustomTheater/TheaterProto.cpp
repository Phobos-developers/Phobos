#include <string.h>
#include "TheaterProto.h"

// This is listed default values for vanilla theaters
TheaterProto TheaterProto::Array[] = {
	TheaterProto /* TEMPERATE */ {
		"TEMPERATE",
		"Name:Temperate",
		"TEMPERATMD.INI",
		false,
		"ISOTEM.PAL",
		"TEMPERAT.PAL",
		"UNITTEM.PAL",
		"TEM",
		"T",
		1.0f,
		"TEMPERAT.MIX,TEM.MIX,ISOTEMMD.MIX,ISOTEMP.MIX"
	},
	TheaterProto /* SNOW */ {
		"SNOW",
		"Name:Snow",
		"SNOWMD.INI",
		true,
		"ISOSNO.PAL",
		"SNOW.PAL",
		"UNITSNO.PAL",
		"SNO",
		"A",
		0.8f,
		"SNOWMD.MIX,SNOW.MIX,SNO.MIX,ISOSNOMD.MIX,ISOSNOW.MIX"
	},
	TheaterProto /* URBAN */ {
		"URBAN",
		"Name:Urban",
		"URBANMD.INI",
		false,
		"ISOURB.PAL",
		"URBAN.PAL",
		"UNITURB.PAL",
		"URB",
		"U",
		1.0f,
		"URBAN.MIX,URB.MIX,ISOURBMD.MIX,ISOURB.MIX"
	},
	TheaterProto /* DESERT */ {
		"DESERT",
		"Name:Desert",
		"DESERTMD.INI",
		false,
		"ISODES.PAL",
		"DESERT.PAL",
		"UNITDES.PAL",
		"DES",
		"D",
		1.0f,
		"DESERT.MIX,DES.MIX,ISODESMD.MIX,ISODES.MIX"
	},
	TheaterProto /* NEWURBAN */ {
		"NEWURBAN",
		"Name:New Urban",
		"URBANNMD.INI",
		false,
		"ISOUBN.PAL",
		"URBANN.PAL",
		"UNITUBN.PAL",
		"UBN",
		"N",
		1.0f,
		"URBANN.MIX,UBN.MIX,ISOUBNMD.MIX,ISOUBN.MIX"
	},
	TheaterProto /* LUNAR */ {
		"LUNAR",
		"Name:Lunar",
		"LUNARMD.INI",
		false,
		"ISOLUN.PAL",
		"LUNAR.PAL",
		"UNITLUN.PAL",
		"LUN",
		"L",
		1.0f,
		"LUNAR.MIX,LUN.MIX,ISOLUNMD.MIX,ISOLUN.MIX"
	}
};
constexpr int TheaterProto::ArraySize = sizeof(TheaterProto::Array) / sizeof(*TheaterProto::Array);

TheaterProto* TheaterProto::Temperate = &TheaterProto::Array[0];
TheaterProto* TheaterProto::Snow = &TheaterProto::Array[1];
TheaterProto* TheaterProto::Urban = &TheaterProto::Array[2];
TheaterProto* TheaterProto::Desert = &TheaterProto::Array[3];
TheaterProto* TheaterProto::NewUrban = &TheaterProto::Array[4];
TheaterProto* TheaterProto::Lunar = &TheaterProto::Array[5];

int TheaterProto::FindIndex(const char* id, int iDefault)
{
	for (int i = 0; i < TheaterProto::ArraySize; i++)
	{
		if (_stricmp(TheaterProto::Array[i].ID, id) == 0)
			return i;
	}
	return iDefault;
}

TheaterProto* TheaterProto::Get(const char* id, TheaterProto* nDefault)
{
	const int index = TheaterProto::FindIndex(id);
	if (index >= 0)
		return &TheaterProto::Array[index];

	return nDefault;
}
